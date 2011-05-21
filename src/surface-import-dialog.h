/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gtk-flatdoc
 * Copyright (C) Peter Urban 2009 <s9peurba@stud.uni-saarland.de>
 * 
 * gtk-flatdoc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gtk-flatdoc is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

# pragma once

# include "gtkmm.h"

# include "utk/numtypes.h"

# include "interface.h"

# include "controller.h"

namespace gtk
{
	
  class SurfaceImportDialog		: public Gtk::FileChooserDialog
  {
	  Glib::RefPtr< Gtk::Builder >	m_builder;
  
	  Gtk::SpinButton*	inc_x_spin;
	  Gtk::SpinButton*	inc_y_spin;

	  Gtk::CheckButton*	center_xy_check;
	  Gtk::CheckButton*	center_z_check;

	  Gtk::CheckButton*	normalize_xy_check;
	  Gtk::CheckButton*	normalize_z_check;

	  Gtk::RadioButton*	quad_topology_radiobutton;
	  Gtk::RadioButton*	triangle_topology_radiobutton;
	  
	protected:

	  static std::string get_builder_filename() { return "surface-import-dialog.ui"; }
	  
    public:
		  
	  SurfaceImportDialog( BaseObjectType* cobject
                         , const Glib::RefPtr<Gtk::Builder>& builder );

	  static SurfaceImportDialog*  create()
	  {
		Glib::RefPtr< Gtk::Builder > 
		  builder = Gtk::Builder::create_from_file( UI::get_builder_directory() 
		                                            + get_builder_filename() );
		SurfaceImportDialog*	dialog;
		  
		builder->get_widget_derived( "surface_import_dialog", dialog );
		return dialog;
	  }
		  
	  void	set_increment( const flat::size_pair& );
	  void	set_center_flags( const utk::vecn3b& );
	  void	set_rescale_flags( const utk::vecn3b& );
		
	  void	on_response( int response_id );
		
	  flat::size_pair	get_increment()		const;
		  
	  utk::vec3b	get_center_flags()	const;
	  utk::vec3b	get_rescale_flags()	const;

	  void set_model( class Controller& )  const	throw( std::bad_alloc );
  };
}
