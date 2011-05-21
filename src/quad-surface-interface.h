
/***************************************************************************
 *            quad-surface-interface.h
 *
 *  Sat Nov 14 20:42:23 2009
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

#include "common.h"

#include "quad-surface.h"
#include "surface-interface.h"

namespace gl
{
  using namespace flat;
  
  class QuadSurfaceDrawable : public SurfaceDrawable
  {
      std::shared_ptr< QuadSurface > m_surface;
	  
    public:
        
      static const mode_t VORONOI_GAUSSIAN_CURVATURE_VERTEX_MODE;

    public:
        
	  QuadSurfaceDrawable( const std::shared_ptr< QuadSurface >& surface )
	  : SurfaceDrawable( surface ), m_surface( surface )	
      { append_vertex_modes( { VORONOI_GAUSSIAN_CURVATURE_VERTEX_MODE } ); }

	  virtual bool  gl_draw_vertices( const mode_t& )	const;
		
	  const std::shared_ptr< QuadSurface >& get_surface()	const	{ return m_surface; };
  };    
}
  
namespace gtk
{
    /* controller && drawable
    class QuadSurfaceDrawableUI	: public SurfaceDrawableUI
    {

      std::shared_ptr< gl::QuadSurfaceDrawable > m_drawable;
      
	  protected:
        static std::string  get_builder_filename() { return "surface.ui"; }
 	  public:		

        QuadSurfaceDrawableUI( const std::shared_ptr< gl::QuadSurfaceDrawable >& drawable )
        : SurfaceDrawableUI( drawable ), m_drawable( drawable )  {  }

    };*/
  }
