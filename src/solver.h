/***************************************************************************
 *            solver.h
 *
 *  Wed Aug  5 18:39:39 2009
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

# pragma GCC visibility push(default)

# include "common.h"

# include "surface.h"

namespace flat
{
  class Solver
  {
	  std::shared_ptr< Surface >  m_surface;


    public:

	  Solver() = delete;
		  
	  Solver( const std::shared_ptr< Surface >& surface ) : m_surface( surface ) 	{	}
      
      virtual	~Solver()	{	}


		  
      virtual void	prepare_step()
  	  { 
		get_surface()->prepare_step(); 
	  }
	  
      virtual void	step() = 0;

	  virtual void	set_surface( const std::shared_ptr< Surface >& surface) 
	  { 
		assert( surface ); 
		m_surface = surface; 
	  }
		  
	  const std::shared_ptr< Surface >&	get_surface()	const	{ return m_surface;	}

  };

}

#pragma GCC visibility pop

