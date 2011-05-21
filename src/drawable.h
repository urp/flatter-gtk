
/***************************************************************************
 *            gl-drawable.h
 *
 *  Fri Nov 13 20:00:40 2009
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

# include "gl-tools.h"

# include <boost/signals.hpp>

# include <gtkmm/widget.h>

# include "common.h"

namespace gl
{	
  class Drawable
  {
	public:

      typedef boost::signal<void()>	invalidate_signal;
	  typedef boost::signal<void()>	remove_signal;

	private:

      remove_signal					remove_drawable;
	  invalidate_signal				invalidate_drawable;

	public:
        
	  Drawable()  {	}
									
	  virtual	~Drawable()
      { std::clog << "gl::Drawable::~Drawable\t|disconnecting drawable"
		 		  << std::endl << std::flush;
		remove_drawable();
	  }

      boost::signals::connection		connect_invalidator( invalidate_signal::slot_type invalidator )
      { return invalidate_drawable.connect( invalidator ); }

	  boost::signals::connection		connect_remover( remove_signal::slot_type remover )   
      { return remove_drawable.connect( remover ); }
        
      void	invalidate()
      { 
        std::clog << "gl::Drawable::invalidate\t|invalidating drawable" << std::endl;
		invalidate_drawable();
	  }

	  virtual	void	gl_draw() = 0;
  };
}

