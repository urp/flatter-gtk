//           mmp-trim_ac.cpp
//  Wed Jan  5 16:14:18 2011
//  Copyright  2011  urp
//  <urp@<host>>

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

# include "mmp-trim_ac.h"

using namespace mmp;

coord_t mmp::impl::linear_linear_window_bisector( const Window& a , const Window& b
                   			                          , const ps_t&   a_ps, const ps_t& b_ps )
{ 
  assert( a.has_ps_vertex<LEFT>() != b.has_ps_vertex<LEFT>() );
  
  const distance_t beta = b.subpath() - a.subpath();
    
  const coord_t p = ( a_ps[0] + b_ps[0] + ( a.has_ps_vertex<LEFT>() ? beta : -beta ) ) / 2;
    
  # if defined DBG_FLAT_MMP_WINDOW_BISECTOR
  std::clog << "mmp::window_bisector\t"
            << "\t| linear - linear"
            << "\t| p " << p
            << " error " << distance_error( p, b, a, b_ps, a_ps)
            << std::endl;
  # endif
  
  return p;
}

coord_t mmp::impl::linear_curved_window_bisector( const Window& linear , const Window& curved
                   			                          , const ps_t&   linear_ps, const ps_t&   curved_ps )
{
  using utk::sqr;
  // on which side does the pseudosource of the linear distance function lie
  const bool ps_left = linear.has_ps_vertex<LEFT>() || curved.has_ps_vertex<LEFT>();
    
  const distance_t beta =  curved.subpath() - linear.subpath() ;
    
  const coord_t p = ( sqr( curved_ps[0] ) + sqr( curved_ps[1] ) - sqr( linear_ps[0] ) 
                      - beta * ( beta + ( ps_left ? 2 : -2 ) * linear_ps[0] ) 
                    ) * .5 / ( curved_ps[0] - linear_ps[0] + ( ps_left ? -beta : beta ) );

  # if defined DBG_FLAT_MMP_WINDOW_BISECTOR
  std::clog << "mmp::window_bisector\t"
            << "\t| " << ( ps_left ? "linear - curved" : "curved - linear")
            << "\t| p " << p
            << " error(" << ( linear.source_distance(p, linear_ps) - curved.source_distance(p, curved_ps) ) << ")"
            << std::endl;
  # endif

  return p;
}

std::pair< coord_t, coord_t > mmp::impl::curved_curved_window_bisector( const Window& a , const Window& b
                                         			                        , const ps_t&   a_ps, const ps_t&   b_ps )
{ 
  using utk::sqr;
  //curved - curved
  const ps_coord_t aps_lsqr = sqr( a_ps[0] ) + sqr( a_ps[1] );
  const ps_coord_t bps_lsqr = sqr( b_ps[0] ) + sqr( b_ps[1] );
  // CHECK 
  const auto alpha    = b_ps[0] - a_ps[0];
  const auto beta_sqr = sqr( b.subpath() - a.subpath() );
  const auto gamma    = aps_lsqr - bps_lsqr - beta_sqr;
  
  // cooefficients of the quadratic equation
  const auto A    = sqr( alpha ) - beta_sqr;
  const auto B	= alpha * gamma + 2 * b_ps[0] * beta_sqr;
  const auto C	= .25  * sqr( gamma ) - bps_lsqr * beta_sqr;

  const std::pair< coord_t, coord_t > p12 = utk::solve_quadratic_equation( A, B, C );

  # if defined DBG_FLAT_MMP_WINDOW_BISECTOR
  std::clog << "mmp::window_bisector\t"
            << "\t| curved - curved"
            << "\t|solutions(" << p12.first  << ( contained( p12.first , intersection_interval(a,b) ) ? 'c' : 'x' ) 
            << ", "            << p12.second << ( contained( p12.second, intersection_interval(a,b) ) ? 'c' : 'x' ) << ')'
            << " errors(" << distance_error( p12.first , a, b, a_ps, b_ps )
            << ", "       << distance_error( p12.second, a, b, a_ps, b_ps ) << ')'
            << std::endl;
  # endif
  
  return p12;
}
 