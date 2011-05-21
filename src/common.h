/***************************************************************************
 *            common.h
 *
 *  Thu Aug  6 13:43:34 2009
 *  Copyright  2009  Peter Urban
 *  <s9peurba@stud.uni-saarland.de>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
# pragma once

# include "utk/basic_math.h"
# include "utk/numtypes.h"

# include <boost/shared_ptr.hpp>
# include <boost/bind.hpp>
# include <boost/numeric/ublas/triangular.hpp>
# include <boost/numeric/ublas/symmetric.hpp>
# include <boost/numeric/ublas/matrix.hpp>
# include <boost/numeric/ublas/io.hpp>

# include <cassert>
# include <array>
# include <utility>
# include <iostream>
# include <iomanip>
# include <random>
# include <functional>
 
namespace flat
{
 
  enum bound_t   { NONE = 0, LOWER = 1, RIGHT = 2, UPPER = 4, LEFT = 8 };

  enum corners_t { UPPER_LEFT = 12, UPPER_RIGHT = 6, LOWER_RIGHT = 3, LOWER_LEFT = 9 };

  enum nb8_t	 { NB8_NONE = 0,
				   NB8_UPPER_LEFT = 1, NB8_UPPER = 2, NB8_UPPER_RIGHT = 4,
				   NB8_RIGHT = 8,
				   NB8_LOWER_RIGHT = 16, NB8_LOWER = 32, NB8_LOWER_LEFT = 64,
				   NB8_LEFT = 128,
				   NB8_ALL = 255,
  				 };
  typedef int nb8_mask_t;

  //----|basic typedefs

  //using utk::size_t;

  typedef std::pair < size_t, size_t >  size_pair;

  typedef double    coord_t;
  typedef double    scale_t;
  typedef double	distance_t;
  typedef double 	time_t;
  typedef double    angle_t;
  typedef double    area_t;
  typedef double    curvature_t;
  typedef double    energy_type;
  typedef float     color_channel_t;
  
  typedef utk::veca<coord_t,3>	location_t;
  typedef utk::vecn<coord_t,3>	location_ref_t;
  
  typedef utk::veca<coord_t,2>	location2d_t;
  typedef utk::vecn<coord_t,2>	location2d_ref_t;
  
  typedef utk::veca<coord_t,3>	force_t;
  typedef utk::vecn<coord_t,3>	force_ref_t;
  
  typedef utk::veca<coord_t,3>	velocity_t;
  typedef utk::vecn<coord_t,3>	velocity_ref_t;

  typedef utk::veca< color_channel_t, 3 >	rgb_color_t;
  typedef utk::vecn< color_channel_t, 3 >	rgb_color_ref_t;
  
  typedef utk::veca< color_channel_t, 4 >	rgba_color_t; 
  typedef utk::vecn< color_channel_t, 4 >	rgba_color_ref_t; 

  //----|functions

  //----|random number generator

  template< typename T >
  inline T uniform_real()
  { 
    static_assert( ! std::numeric_limits<T>::is_integer, "Type T must represent a floating point number." );
	static std::mt19937 engine;
	static std::uniform_real_distribution<T> distribution( 0, 1 );
	static auto generator = std::bind( distribution, engine );

    return generator();
  }
  
  //----|linear algebra

  template< typename T >
  boost::numeric::ublas::triangular_matrix< T, boost::numeric::ublas::lower >	 cholesky_decomposition( const boost::numeric::ublas::symmetric_matrix< T, boost::numeric::ublas::upper >& A)
  {
    std::clog<<"flat::cholesky_decomposition"<<std::endl<<std::flush;
    assert(A.size1()==A.size2());
  
    //Cholesky Decomposition
    //Numerical Recipes in C++ (Second Edition), page 100

    std::vector< T > d( A.size1() );

    T sum;
    boost::numeric::ublas::triangular_matrix< T, boost::numeric::ublas::lower > L( A.size1(), A.size2() );

	for( size_t i = 0; i < A.size1(); i++ )
	{ 
	  for( size_t j = 0; j < i; j++ )
	  {	 sum = A(i,j);
	     for(size_t k = 0; k < j; ++k)
		   sum -= L(i,k)*L(j,k);
	     L(i,j) = sum / L(j,j);
	  }
		
	  sum = A(i,i);
	  for(size_t k = 0; k < i; ++k)
		sum -= L(i,k) * L(i,k);
	  assert( sum > 0 );
	  L(i,i) = std::sqrt( sum );	
	}	

    /*for( size_t i = 0; i < A.size1(); i++ )
	  for( size_t j = i; j < A.size1(); j++ )
	  { 
	    sum = A(i,j);
	    for( int k = int(i)-1; k >= 0; k-- ) 
	      sum -= A(i,k) * A(j,k);
	    if( i == j )
	    { 
	      std::cerr << "choleski sum " << sum << std::endl;
		  assert( sum > 0 ); // is A really positive definite?
		  d[i] = std::sqrt( sum );
	    } else L(j,i) = sum / d[i];
	  }
    for( size_t i = 0; i < A.size1(); ++i )	  L(i,i) = d[i];
    */
    std::clog<<"flat::cholesky_decomposition\t|complete"<<std::endl<<std::flush;

    return L;
  };

  template< typename T >
  boost::numeric::ublas::matrix< T >    pseudoinverse( const boost::numeric::ublas::symmetric_matrix< coord_t, boost::numeric::ublas::upper >& X )
  {
    using namespace boost::numeric::ublas;
    
    std::clog << "flat::pseudoinverse" << "\t| computing ..."//<<std::endl<<V
	  		  << std::endl << std::flush;

    // TODO regularizer. right name?
    
    triangular_matrix< T, lower > L( cholesky_decomposition< T >( X ) );

    std::clog << "flat::pseudoinverse" << "\t| L"
              << std::endl << L << std::endl << std::flush;
  
    auto Xinverse = solve( L, solve( L, identity_matrix< T >( X.size1() ), lower_tag() ), lower_tag() );

    std::clog << "flat::pseudoinverse" << "\t| complete - pseudoinverse of V " << std::endl 
              << Xinverse << std::endl;

    return Xinverse;
  }
}
