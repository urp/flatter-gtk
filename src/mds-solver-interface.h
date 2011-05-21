/***************************************************************************
 *            mds-solver-interface.h
 *
 *  Fri Jan  8 15:40:40 2010
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
 
#pragma once

#include "mds-solver.h"
#include "surface-interface.h"

  namespace gl
  {
    class MDSSolverDrawable : public SurfaceDrawable
    {
        std::shared_ptr< mds::Solver > m_solver;
      
      public:
        
		static const mode_t VERTEX_DISPLACEMENT_EDGE_MODE;
		
        MDSSolverDrawable( const std::shared_ptr<mds::Solver>& solver )
        : SurfaceDrawable( solver->get_surface() ), m_solver( solver )    
        { append_edge_modes( { VERTEX_DISPLACEMENT_EDGE_MODE } ); }

        virtual bool	gl_draw_edges( const mode_t& )		const;	

		const std::shared_ptr<mds::Solver>& get_solver()	const	{ return m_solver; }
    };
  }
  
  namespace gtk
  {
    class MDSSolverDrawableUI	: public UI
    {
        std::shared_ptr< gl::MDSSolverDrawable > m_drawable;

      protected:

        static std::string get_builder_filename()   { return "mds-solver.ui"; }
        
	  public:
        
	    MDSSolverDrawableUI( const std::shared_ptr< gl::MDSSolverDrawable >& drawable )
		: UI( get_builder_filename() ), m_drawable( drawable ) {	}

	    virtual	~MDSSolverDrawableUI()  { 	}

		const std::shared_ptr< gl::MDSSolverDrawable >& get_solver() const { return m_drawable; }
    };
  }
