/***************************************************************************
 *            mmp-utilities.h
 *
 *  Fri Apr  9 12:40:54 2010
 *  Copyright  2010  Peter Urban
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

#include "mmp-common.h"
#include "mmp-window.h"

  namespace mmp
  {
    bool                            interior ( const coord_t& a, const std::pair<coord_t,coord_t>& i );

    bool                            contained( const coord_t& a, const std::pair<coord_t,coord_t>& i );
      
    bool                            snap_check( const coord_t& a, const coord_t& b );
    
    bool                            dist_snap_check( const distance_t& a, const distance_t& b );

	inline distance_t				distance_error( coord_t p, const Window& new_window, const Window& old_window, const ps_t& new_ps, const ps_t& old_ps )
									{ return std::fabs( new_window.source_distance( p, new_ps ) - old_window.source_distance( p, old_ps ) ); }
	
    std::pair<coord_t,distance_t>   max_distance_error( const Window& wa, const Window& wb, const ps_t& psa, const ps_t& psb );
    
    bool                            has_intersection( const std::pair<coord_t,coord_t>& wa, const std::pair<coord_t,coord_t>& wb );

                                    // returns the intersection interval of two windows
                                    // (0,0) is returned if intersection is empty
    std::pair<coord_t,coord_t>      intersection_interval( const Window& wa, const Window& wb );

                                    // reprojecting pseudosources to common line  definded by window with lower ps_error()
    void                            assign_stable_ps_direction( Window& left, Window& right, ps_t& psl, ps_t& psr);
    
                                    // reprojecting pseudosources to common line definded by window with lower ps_error()
    void                            colinear_ps_correction( Window& left, Window& right
                                                          , ps_t& psl, ps_t& psr
                                                          , const ps_coord_t& pslerr = 1. , const ps_coord_t& psrerr = 1. );

  }
