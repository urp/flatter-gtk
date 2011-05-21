/*
 * main.cpp
 * Copyright (C) Peter Urban 2009 <s9peurba@stud.uni-saarland.de>
 * 
 * main.cc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.cc is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <gtkmm.h>
#include <iostream>

#include "controller.h"

int main (int argc, char *argv[])
{
  std::cout << std::boolalpha;
  std::clog << std::boolalpha;
  std::cerr << std::boolalpha;

  std::cout << "gtk-flatter - Copyright (C) Peter Urban 2009-2011" << std::endl;

  // init gtkmm
  Gtk::Main kit(argc, argv);

  // init gtkglextmm.
  Gtk::GL::init(argc, argv);

  // query OpenGL extension version.
  int major, minor;
  Gdk::GL::query_version(major, minor);
  std::clog << "OpenGL extension version - "
			<< major << '.' << minor
			<< std::endl;

  # if defined NDEBUG
  std::clog << "main\tdebugging has been disabled" << std::endl;
  # endif
  
  // load the Glade file and instiate its widgets:
  Glib::RefPtr<Gtk::Builder> builder;
  try
  {
    builder = Gtk::Builder::create_from_file( gtk::UI::get_builder_directory() + "gtk_flatdoc.ui" );
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << ex.what() << std::endl;
    return 1;
  }
  
  gtk::Controller controller( builder );

  Gtk::Window* main_win = 0;
  builder->get_widget( "main_window", main_win );
  
  //grid_view->invalidate();
  
  if( main_win )
  {
    std::clog << "main\t|entering main loop..." << std::endl << std::flush;
    kit.run( *main_win );
  }
  return 0;
}
