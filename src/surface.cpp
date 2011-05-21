//  Wed Aug  5 23:46:44 2009
//  Copyright  2009  Peter Urban
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

# include "surface.h"

# include "mmp-visualizer.h"
//# include "spring-force.h"

# include <boost/accumulators/accumulators.hpp>
# include <boost/accumulators/statistics/min.hpp>
# include <boost/accumulators/statistics/max.hpp>
# include <boost/accumulators/statistics/sum.hpp>

using namespace flat;

void	flat::PointCloud::comp_min_max_xy()	const	
{ 
  using namespace boost::accumulators;

  accumulator_set< coord_t, features< tag::min, tag::max > > xacc,yacc;

  for( auto vertex_it = vertex_handles(); vertex_it.first != vertex_it.second; vertex_it.first++ )
  { 
    xacc( vertex_it.first->location()[0] );
	yacc( vertex_it.first->location()[1] );
  }
	
  m_min_location[0] = min(xacc);
  m_min_location[1] = min(yacc);
  m_max_location[0] = max(xacc);
  m_max_location[1] = max(yacc);
}

void	flat::PointCloud::comp_min_max_z()	const
{ 
  using namespace boost::accumulators;

  accumulator_set< coord_t, features< tag::min, tag::max > > zacc;

  for( auto vertex_it = vertex_handles(); vertex_it.first != vertex_it.second; vertex_it.first++ )
    zacc( vertex_it.first->location()[2] );
		
  m_min_location[2] = min( zacc );
  m_max_location[2] = max( zacc );
}

Surface::Surface( const vertices_size_type  num_vertices
                , const size_pair& 			texture_size
                , const std::string&        name )
: PointCloud( num_vertices ), m_name( name ), m_texture( texture_size )
{ 
  std::clog << "flat::Surface::Surface\t|"
            << " vertex size (" << num_vertices << ')' 
            << " texture size (" << std::get<0>(texture_size) << ", " << std::get<1>(texture_size) << ')' 
            << std::endl; 
}

void Surface::distance_function::compute_all_to_all( const std::shared_ptr< Surface >& surface )
{
  if( neighborhood & ALL ) return;
  
  //# define USE_FLAT_MMP_VISUALIZE_GTK_OBSERVER 

  # if defined USE_FLAT_MMP_VISUALIZE_GTK_OBSERVER 
  std::auto_ptr< gtk::GeodesicsInspector > obs;
  # endif
 
  //mmp::vertex_pair_check( surface, 1e-4 );

  for( vertex_descriptor source = 0; source < surface->num_vertices(); source++ )
  { 
    mmp::Geodesics gi( *surface, source );
	// TODO: prevent propagation to vertices which allready have distance information (j <= i )
	  
    # if defined USE_FLAT_MMP_VISUALIZE_GTK_OBSERVER 
	if( !obs.get() ) obs.reset( gtk::GeodesicsInspector::create_propagation_observer( &gi, surface ) );
	else obs->initialize( &gi, surface );
	obs->run_propagation();
	# else
	gi.propagate_paths();
	# endif
    
    for( vertex_descriptor query = source + 1; query < surface->num_vertices(); query++)
      distance_matrix( source, query ) = gi.query_distance( query );
  }

  neighborhood |= ALL;
}

void Surface::distance_function::compute_all_to_neighbors( const std::shared_ptr< Surface >& surface )
{
  if( neighborhood & NEIGHBORS ) return;

  const std::vector< vertex_pair >   pairs( std::move( surface->neighbors() ) );
  
  std::for_each( pairs.begin(), pairs.end()
                 , [ &surface, &distance_matrix ]( const Surface::vertex_pair& pair )
                   { distance_matrix( pair.first, pair.second ) = surface->distance( pair.first, pair.second ); 
				   }
                 );
  neighborhood |= NEIGHBORS;
}


void Surface::distance_function::compute_distances( const std::shared_ptr< Surface >& surface, neighborhood_mask_type nb )
{
  std::clog << "flat::Surface::compute_distances"
            << std::endl;

  std::time_t start_time = std::clock();
  
  if( distance_matrix.size1() != surface->num_vertices() ) 
    distance_matrix.resize( surface->num_vertices() );

  if( nb & NEIGHBORS ) compute_all_to_neighbors( surface );
  else if( nb & ALL )  compute_all_to_all( surface );
  else assert( false );    

  std::time_t end_time = std::clock();
  
  std::clog << "flat::Surface::compute_distances\t|" 
            << "complete - after " << ( (end_time - start_time)/double(CLOCKS_PER_SEC) ) 
            << std::endl;
}

std::vector< Surface::vertex_pair > Surface::neighbors() const 
{
  std::vector< vertex_pair > neighbor_pairs;
  neighbor_pairs.reserve( 3 * num_vertices() );
  for( vertex_descriptor a = 0; a < num_vertices(); ++a )
  {
    const std::vector< vertex_descriptor > local_neighbors( std::move( neighbors( a ) ) );
    std::for_each( local_neighbors.begin(), local_neighbors.end()
                 , [&] ( const vertex_descriptor b ) { assert( a < this->num_vertices() );
                                                       assert( b < this->num_vertices() );
                                                       if( a < b ) neighbor_pairs.push_back( { a, b } ); 
                                                     } 
                 );
  }
                              
  std::clog << "flat::Surface::neighbors\t| complete - " <<  neighbor_pairs.size() << " springs created" << std::endl;

  return neighbor_pairs;
}

std::pair< flat::coord_t, Surface::distance_function::neighborhood_mask_type > 
    flat::Surface::get_squared_distance_error()    const
{
  namespace acc = boost::accumulators;

  typedef location2d_ref_t handle;
  
  acc::accumulator_set< coord_t, acc::features< acc::tag::sum > >  total_sq_error;
  if( initial_distances.neighborhood & distance_function::ALL )
  { for( auto it1 = vertex_handles(); it1.first != it1.second; it1.first++ )
      for( vertex_descriptor v2 = *it1.first + 1; v2 < num_vertices(); ++v2 )
        total_sq_error( utk::sqr( utk::distance( handle( it1.first->location() ), handle( vertex( v2 ).location() ) ) - initial_distances( *it1.first, v2 ) ) );
  }else 
  if( initial_distances.neighborhood & distance_function::NEIGHBORS )
  {
    std::vector< vertex_pair > neighbors( this->neighbors() );
    for( auto pair = neighbors.begin(); pair != neighbors.end(); ++pair )
      total_sq_error( utk::sqr( utk::distance( handle( vertex( pair->first ).location() ), handle( vertex( pair->second ).location() ) ) - initial_distances( pair->first, pair->second ) ) );
  }

  return { acc::sum( total_sq_error ), initial_distances.neighborhood };
}

