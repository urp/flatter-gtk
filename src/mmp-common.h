/***************************************************************************
 *            mmp-common.h
 *
 *  Sat Apr 24 21:20:14 2010
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

// options
# define FLAT_MMP_COUPLE_COLINEAR
# define FLAT_MMP_COUPLE_CROSSING
# define FLAT_MMP_MAINTAIN_WAVEFRONT
# define FLAT_MMP_INSERT_SIDE_LOBES_FIRST
# define FLAT_MMP_WINDOW_BISECTOR_INTERVAL_BOUNDS_SNAPPING

#include "common.h"
#include "mmp-queue.h"


namespace mmp
{

  using namespace flat;
  
  //----|free parameters :-(
  static const distance_t  distance_jump_relative_tolerance   = 0.0001;
  static const size_t      distance_jump_ulp_tolerance        = 100;
  static const coord_t     interval_absolute_tolerance        = 0.0001;
  static const size_t      interval_ulp_tolerance             = 100;


  typedef coord_t                 ps_coord_t;
  typedef utk::veca<ps_coord_t,2> ps_t;

  typedef utk::veca<coord_t,2>	coord2_t;	

    
  typedef enum { LEFT_WINDOW = -1, CANDIDATE_WINDOW = 0, RIGHT_WINDOW = 1 }   ac_position_t; 
	
  typedef enum { LEFT = 0, RIGHT = 1 } side_t;

  template< side_t Side >	struct side_traits	
  { // static const side_t	opposite;
	// static std::string  string();
  };

  template<> struct side_traits<LEFT>	
  { static const side_t	opposite = RIGHT; 
	static const std::string&  	string()	{ static std::string s("left"); return s;  }
      
    template< typename T >
    static bool smaller( T smaller, T greater )   { return smaller < greater; }
    template< typename T >
    static bool smaller_equal( T smaller, T greater )   { return smaller <= greater; }

  };

  template<> struct side_traits<RIGHT>
  { static const side_t	opposite = LEFT; 
	static const std::string&  	string()	{ static std::string s("right"); return s; }

    template< typename T >
    static bool smaller( T greater, T smaller )   { return greater > smaller; }
    template< typename T >
    static bool smaller_equal( T greater, T smaller )   { return greater > smaller; }
  };
}
