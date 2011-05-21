//           quad-surface-interface.cpp
//  Sat Nov 14 20:42:23 2009
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

#include "quad-surface-interface.h"

using namespace gl;

const QuadSurfaceDrawable::mode_t gl::QuadSurfaceDrawable::VORONOI_GAUSSIAN_CURVATURE_VERTEX_MODE = "gaussian curvature (voronoi)";

bool  QuadSurfaceDrawable::gl_draw_vertices( const mode_t& mode)   const		
{ using namespace flat;

  if( mode != VORONOI_GAUSSIAN_CURVATURE_VERTEX_MODE ) return SurfaceDrawable::gl_draw_vertices( mode );
  
  // draw position samples with color dependent on the local gaussian curvature
  float minc =   std::numeric_limits<float>::infinity();
  float maxc = - std::numeric_limits<float>::infinity();

  std::vector< rgb_color_t >	colors( get_surface()->num_vertices() );
  std::vector< rgb_color_t >::iterator 	col_it = colors.begin();
	
  for( auto vertex_it = get_surface()->vertex_handles(); vertex_it.first != vertex_it.second; col_it++, vertex_it.first++ )
  { 

	area_t  area = 0.f;
	angle_t anglesum = 0.f;
	const location_t& apos = vertex_it.first->location();

	std::vector< QuadSurface::vertex_descriptor > nbs ( std::move( get_surface()->get_neighbors< NB8_ALL >( vertex_it.first->descriptor() ) ) );

	if( nbs.size() < 8 )
	{ *col_it = rgb_color_t(.1f,.4f,.1f);
	  continue;
	}

	for( size_t tri = 0; tri < nbs.size(); ++tri)
	{
	  const location_ref_t bpos = get_surface()->vertex( nbs[ tri % nbs.size() ] ).location();	
	  const location_ref_t cpos = get_surface()->vertex( nbs[ ( tri + 1 ) % nbs.size() ] ).location();
	  const auto	ab   = bpos - apos;
	  const auto	bc   = cpos - bpos;
	  const auto	ca   = apos - cpos;
	  const auto 	absq 	= utk::sqr( ab ).sum();
	  const auto 	bcsq 	= utk::sqr( bc ).sum();
	  const auto 	casq 	= utk::sqr( ca ).sum();
	  const auto 	ablen	= std::sqrt( absq );
	  const auto 	calen	= std::sqrt( casq );
	  const bool  	alpha_obtuse = bcsq > casq + absq;
	  const bool  	obtuse = absq > bcsq + casq || alpha_obtuse || casq > absq + bcsq;
		
	  if(!obtuse)
	    area += .125 * (  std::atan( - dot( bc, ca ) / ( std::sqrt( bcsq ) * calen ) ) * absq
		                + std::atan( - dot( ab, bc ) / ( ablen * std::sqrt( bcsq ) ) ) * casq );
	  else
	    area += cross( ab, bc ).length() / ( alpha_obtuse ? 4. :8. );
		anglesum += std::acos( utk::dot( ca, ab ) / ( calen * ablen ) );
	}
	  
	const curvature_t curvature = ( 2 * M_PI - anglesum ) / area;
	  
	utk::clamp_min( minc, curvature );
	utk::clamp_max( maxc, curvature );

    *col_it = rgb_color_t( curvature<0. ? - curvature : 0.,
	                       curvature>0. ?   curvature : 0., 
	                       std::fabs( curvature ) 
	                     );
	    	//rgb_color_t(1.f - exp(-fabs(curvature)));
  }
  std::clog << "flat::gl::QuadSurfaceDrawable::gl_draw_vertices"
	        << "\t|GAUSSIAN_CURVATURE_GRID_MODE"
			<< "\t|min " << minc
			<< "\t|max " << maxc
			<< std::endl << std::flush;

  //do the drawing 
  glPointSize( get_vertex_size() );
	
  get_surface()->set_curvature_extrema( minc, maxc );
	
  for( auto it = colors.begin(); it != colors.end(); it++ )
    *it /= std::max(fabs(minc),fabs(maxc));

  gl_draw_scaled_vertices( colors.begin() );
  return true;
}
