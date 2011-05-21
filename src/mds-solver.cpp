//           mds-solver.cpp
//  Fri Jan  8 15:05:03 2010
//  Copyright  2010  Peter Urban
//  <s9peurba@stud.uni-saarland.de>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA

# include "mds-solver.h"

# include <boost/accumulators/accumulators.hpp>
# include <boost/accumulators/statistics/min.hpp>
# include <boost/accumulators/statistics/max.hpp>

using namespace mds;

void mds::Solver::step()
{ 
  std::clog << "flat::SimpleMDSSolver::step" << std::endl << std::flush;

  //TODO: allow n iterations at once - save X copies
  mds_step();
  
  // copy locations back to the mesh
  std::clog <<"flat::SimpleMDSSolver::step\t|copying locations back... " 
  //        << std::endl << X 
            << std::endl;

  for( auto its = get_surface()->vertex_handles(); its.first != its.second; its.first++ )
    its.first->set_location( location_t( X( its.first->descriptor(), 0 ), X( its.first->descriptor(), 1 ), 0. ) );

  std::clog <<"flat::MDSEqualWeightSolver::step"
			<<"\t|complete" << std::endl << std::flush;
}

void EqualWeightSolver::mds_step()
{
  std::clog << "flat::MDSEqualWeightSolver::mds_step\t| computing equal-weights solution..."
			<< std::endl;

  const std::shared_ptr< Surface >& surface = get_surface();

  // set all elements X to zero
  X = boost::numeric::ublas::scalar_matrix< coord_t >( X.size1(), X.size2(), 0. );  
  
  for( size_t i = 0; i < X.size1(); i++ ) 
    for( size_t j = i + 1; j < X.size1(); j++ )
	{ 
	  const coord_t     rd = surface->initial_distances( i, j ) / surface->distance( i, j );
	  const location_t 	dij_rd = ( surface->vertex( i ).location() - surface->vertex( j ).location() ) * rd / X.size1() ;

      X( i, 0 ) += dij_rd[ 0 ];
	  X( i, 1 ) += dij_rd[ 1 ];
      
	  X( j, 0 ) -= dij_rd[ 0 ];
      X( j, 1 ) -= dij_rd[ 1 ];
	}
  
  std::clog << "flat::MDSEqualWeightSolver::mds_step" << "\t| complete"	<< std::endl;
}

symmetric_matrix< coord_t, upper >  mds::GeneralSolver::create_V()  const
{
  std::clog << "flat::MDSGeneralSolver::create_V" << "\t| computing matrix V ..."//<<std::endl<<V
			<< std::endl << std::flush;
  symmetric_matrix< coord_t, upper > V( X.size1(), X.size1() );

  for( Surface::vertex_descriptor i = 0; i < X.size1(); i++ )
  {
    distance_t vsum = 0;
	for( Surface::vertex_descriptor j = 0; j < X.size1(); ++j )
      if( i != j ) vsum += V( i, j ) = - W( i, j );
	V( i, i ) = -vsum;
  }
  std::clog << "flat::MDSGeneralSolver::create_V" << "\t| complete - V " << std::endl 
            ;//<< V << std::endl;
  return V;
}

void mds::GeneralSolver::initialize_weights()
{
  const std::shared_ptr< Surface >& surface = get_surface();

  //W.resize( get_surface()->num_vertices(), get_surface()->num_vertices(), false );
  W = boost::numeric::ublas::scalar_matrix< coord_t >( get_surface()->num_vertices(), get_surface()->num_vertices(), 0. );  

  // comment out to enable sparse sampling
  //surface->initial_distances.compute_distances( surface, Surface::distance_function::ALL );
  
  if( surface->initial_distances.neighborhood & Surface::distance_function::ALL )
  for(size_t i=0; i < W.size1(); i++)  
  { 
      std::clog << "mds::GeneralSolver::initialize_weights\t| "
                << "using equal weights(all-to-all)"
                << std::endl;
    //W( i, i ) = 0;
    for(size_t j= i + 1; j < X.size1(); j++)
      W( i, j ) = 1.; // weight_type( X.size1() - 1 );
  }else
  if( surface->initial_distances.neighborhood & Surface::distance_function::NEIGHBORS )
  {
    std::clog << "mds::GeneralSolver::initialize_weights\t| "
              << "using direct neighborhood"
              << std::endl;

    const std::vector< Surface::vertex_pair > pairs( std::move( surface->neighbors() ) );
    const size_t n = pairs.size();
	  
    std::for_each( pairs.begin(), pairs.end()
                 , [ &W, &n ]( const Surface::vertex_pair& pair )
                   { W( pair.first, pair.second ) = weight_type(1)/n;
                     //std::cout << "pair " << pair.first << " " << pair.second << std::endl << std::flush;
                   }
                 );
    /*for(size_t i=0; i < W.size1(); i++)  
    { 
      weight_type sum = 0;
      for(size_t j= 0; j < W.size2(); j++) sum += W(i,j);
      for(size_t j= 0; j < W.size2(); j++) W(i,j) /= sum;  
    }*/
  }

  std::cout << "weights " << W << std::endl;
}

void mds::GeneralSolver::initialize_static_matrices()
{
  initialize_weights();

  const boost::numeric::ublas::scalar_matrix< coord_t > regularizer( X.size1(), X.size1(), 1. );

  std::time_t start_time = std::clock();
  # if defined FLAT_MDS_GENERAL_SOLVER_USE_INVERSE
  //----|initialize moore penrose inverse of matrix V 
    
  Vinv = pseudoinverse< coord_t >( create_V() + regularizer );
  Vinv -= regularizer;

  # else  //----|initialize V matrix 

  symmetric_matrix< coord_t, upper > V( create_V() );
  V += regularizer;
  //std::cout<< "V  " << V << std::endl;
  L = cholesky_decomposition( V );
  //std::cout<< "L  " << L << std::endl;
  //std::cout<< "LL " << prod( trans(L), L ) << std::endl;
  # endif

  std::time_t end_time = std::clock();
  std::clog << "mds::GeneralSolver::initialize_static_matrices\t"
            << " completed in" << ( end_time - start_time )/double( CLOCKS_PER_SEC )
            << std::endl;
}

void mds::GeneralSolver::mds_step()
{
  std::clog << "flat::MDSSolver::mds_step"
			<< "\t|computing (general weight) solution..."
			<< std::endl;
  
  using namespace boost::numeric::ublas;

  using namespace boost::accumulators;
  accumulator_set< distance_t, features< tag::min, tag::max > > disp_acc;
	
  // X - vertex locations
  std::clog << "flat::MDSSolver::mds_step" << "\t| copying surface locations ..." << std::endl;

  std::pair< Surface::vertex_handle_iterator
           , Surface::vertex_handle_iterator > its = get_surface()->vertex_handles();
  
  for( size_t i = 0; its.first != its.second; its.first++, i++ )
  { 
    X( i, 0 ) = its.first->location()[0];
	X( i, 1 ) = its.first->location()[1];
  }
  //std::clog <<"flat::MDSSolver::step\t|X "
  //		  <<std::endl<<X<<std::endl<<std::flush;
  
  // D - distance matrix
  std::clog << "flat::MDSSolver::mds_step" << "\t| computing distance matrix D ..."	<< std::endl;

symmetric_matrix< distance_t, upper > D( X.size1(), X.size1() );
  for(size_t i = 0; i < X.size1(); i++)
  {	D(i,i) = 0.;
	for(size_t j = i + 1; j < X.size1(); j++)
	  disp_acc( std::fabs( D(i,j) = get_surface()->distance(i,j) ) );
  }

  //set_max_displacement( max( disp_acc) );
  
  std::clog << "flat::MDSSolver::mds_step"
			<< "\t| distance matrix complete "//- max displacement " << max_displacement() //<< std::endl<<D
            << std::endl << std::flush;

  // B - Matrix 
  symmetric_matrix<coord_t,upper> B( X.size1(), X.size1() ); 
  for(size_t i=0; i<X.size1();i++)
  {
    coord_t bsum = 0.f;// vsum = 0.f;
	for(size_t j=0; j<X.size1();j++)
      if(i!=j)
	  { 
        B(i,j) = ( D(i,j)>0.f ? - W(i,j) * get_surface()->initial_distances(i,j) / D(i,j) : 0.f );
		bsum += B(i,j);
	  }
	B(i,i) = -bsum;
  }
  
  # if defined FLAT_MDS_GENERAL_SOLVER_USE_INVERSE
  // solve with inverse matrix

  matrix<coord_t> T ( prod( B, X ) );
  X = prod( Vinv, T );

  #else // solve directly

  //matrix<coord_t> T(prod( B, X ));
  //matrix<coord_t> Y( solve( L, T , lower_tag() ) );
  X = solve( L, /*Y=*/solve( L, prod( B, X ) , lower_tag() ), lower_tag() );
  
  # endif
}
