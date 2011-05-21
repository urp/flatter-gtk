/***************************************************************************
 *            interface.h
 *
 *  Sun Nov 15 21:25:23 2009
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

# define GTK_BUILDER_DIRECTORY PACKAGE_DATA_DIR"/gtk_flatter/ui/"
// For testing propose use the local (not installed) glade file
//# define GTK_BUILDER_DIRECTORY "src/"

# include "drawable.h"

# include <gtkmm/builder.h>

namespace gtk
{
  class UI
  {
	public:

	  typedef std::string	widget_name_t;
	  typedef boost::signal<void (gtk::UI*)>	remove_signal;

	private:

	  remove_signal	m_remove_ui;

	  Glib::RefPtr< Gtk::Builder >	m_builder;

	  const std::string	m_top_widget_name;

	public:

	  static std::string get_builder_directory() { return GTK_BUILDER_DIRECTORY; }

	  UI() = delete;

	  UI( const Glib::RefPtr< Gtk::Builder >& builder, const widget_name_t& top_widget_name = "top_widget" )
	  : m_builder( builder ), m_top_widget_name( top_widget_name )	{	}

	  UI( const std::string& builder_filename, const widget_name_t& top_widget_name = "top_widget" ) 
	  : m_builder( Gtk::Builder::create_from_file( get_builder_directory() + builder_filename ) )
	  , m_top_widget_name( top_widget_name )	{	}

	  virtual	~UI()   { m_remove_ui( this ); }

	  virtual void update()	{ std::clog << "flat::gtk::UI::update" << std::endl; }

	  boost::signals::connection    connect_remover( remove_signal::slot_type remover )
	  { return m_remove_ui.connect( remover ); }

	  virtual Gtk::Widget* get_widget()
	  { 
	    Gtk::Widget* top_widget;
	    get_builder()->get_widget( m_top_widget_name, top_widget );
	    return top_widget;
	  }
      /*
	  template< class WidgetT >	WidgetT*    get_widget_derived()    
	  { 
	    WidgetT* top_widget;
	    get_builder()->get_widget_derived( m_top_widget_name, top_widget );
	    return top_widget;
	  }*/
      
      const Glib::RefPtr< Gtk::Builder >& get_builder() const { return m_builder; }
  };
}
 