/***************************************************************************
 *            Controller.h
 *
 *  Mon Jun 22 18:15:15 2009
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

#include <cassert>

#include "surface.h"
#include "solver.h"

namespace flat
{

  
  class Model
  {

	private:
      
	  std::shared_ptr<Surface>	m_surface;
	  std::shared_ptr<Solver>	m_solver;

    public:

	  Model() = default;
      
      const std::shared_ptr<Surface>&	get_surface()	const	{ return m_surface; }
    
      const std::shared_ptr<Solver>&	get_solver()	const	{ return m_solver; }

      void	step()
      { m_solver->prepare_step();
		m_solver->step(); 
	  }

	  void	set_surface( const std::shared_ptr< Surface >& surface )
	  { assert( surface );
		m_surface = surface;
	  }

	  void	set_solver( const std::shared_ptr< Solver >& solver )
	  { assert( solver );
        m_solver = solver;
	  }
  };
}
