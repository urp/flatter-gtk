/***************************************************************************
 *            surface-interface.h
 *
 *  Sat Nov 14 16:30:06 2009
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

# include "gtkmm.h"

# include "interface.h"
# include "drawable.h"
# include "surface.h"
# include "surface-drawable.h"

# include <boost/lexical_cast.hpp>


namespace gtk
{
  // controller && drawable
  class SurfaceUI			: public UI
  {
	// TODO: check static_assert( static_cast )

	private:

 	  std::shared_ptr< flat::Surface > 	m_surface;

      // weather the min and max of the gaussian curvature should be displayed
	  bool m_curvature_update;
		
      // gtk widgets

	  Gtk::Label*	m_height_label;

	  Gtk::Label*	m_curvature_label;

	  SurfaceUI();

	protected:
	  
	  static std::string	get_builder_filename()	 { return "surface-model.ui"; }

	  void toggle_curvature_update();
		
	public:
	  
	  SurfaceUI( const std::shared_ptr< flat::Surface >& );

	  virtual   ~SurfaceUI() { }

	  virtual void update();
  };
}	

