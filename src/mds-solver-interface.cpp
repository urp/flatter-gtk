//           mds-solver-interface.cpp
//  Fri Jan  8 15:40:40 2010
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

#include "mds-solver-interface.h"

using namespace flat;


const gl::SurfaceDrawable::mode_t gl::MDSSolverDrawable::VERTEX_DISPLACEMENT_EDGE_MODE = "vertex displacement";


bool gl::MDSSolverDrawable::gl_draw_edges( const mode_t& mode )	const
{
  if( mode != VERTEX_DISPLACEMENT_EDGE_MODE ) return SurfaceDrawable::gl_draw_edges( mode );

  const std::shared_ptr<flat::Surface>& 	surface = get_solver()->get_surface();

  rgb_color_t col(0.f);

  glPushMatrix();

  gl::Scale( get_global_scale() );

  glBegin(GL_LINES);

  std::vector< Surface::vertex_pair > pairs = surface->neighbors();
  
  for( auto pair = pairs.begin(); pair != pairs.end(); ++pair )
  {
	distance_t disp = surface->distance( pair->first , pair->second ) - surface->initial_distances( pair->first, pair->second );

	col.fill( 0. );
	  
	if(disp < -std::numeric_limits< distance_t >::epsilon() )
	{ //col.r() = col.b() = -disp / get_solver()->max_displacement();
	  col.g() = 0.;
	}
	if( disp > std::numeric_limits< distance_t >::epsilon() )
	{ //col.g() = col.b() = disp / get_solver()->max_displacement();
	  col.r() = 0.;
	}

	/*std::cerr << "disp edge i "<<i<<" j "<<j
				<< " d "<<(disp/solver->max_displacement())
				<< " id "<<surface->initial_distance(i,j)
				<< " cd "<<surface->distance(i,j)
				<< " c " <<col
				<< std::endl;*/
	  
	gl::Color(col);

	const location_t loc_a = surface->vertex( pair->first  ).location() * get_global_scale();
	const location_t loc_b = surface->vertex( pair->second ).location() * get_global_scale();

	gl::Vertex( loc_a );
	gl::Vertex( loc_b );	
  }  
  glEnd();
  glPopMatrix();
  return true;
}

