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

# include <cassert>

# include "gtkmm.h"
# include "gtkglmm.h"

# include "view.h"
# include "gl-canvas.h"
# include "drawable.h"

# include <boost/lexical_cast.hpp>

/* For testing propose use the local (not installed) glade file */
//# define GTK_GLVIEW_BUILDER_FILE PACKAGE_DATA_DIR"/gtk_flatdoc/glade/gl-view.ui" */
# define   GTK_GLVIEW_BUILDER_FILE "src/gl-view.ui"

// debugging
# define DBG_FLAT_GLCANVAS_INVALIDATE
# define DBG_FLAT_GLCANVAS_ADD_DRAWABLE
# define DBG_FLAT_GLCANVAS_REMOVE_DRAWABLE

namespace gtk
{

  class GLView : public Gtk::VBox, public flat::View< gl::Drawable >
  {
	public:

	  struct Filename
	  {
		std::string	directory;
		std::string	filename_body;
		size_t		counter;
		std::string	extension;

		Filename( const std::string& dir, const std::string& body, const size_t& count, const std::string&	ext )
		: directory( dir ), filename_body( body ), counter( count ), extension( ext ) 
		{	}
		         
		Filename()
		: directory(), filename_body("untitled"), counter(0), extension() 
		{	}
		
		std::string operator()	()
		{ return  directory + "/"
              	+ filename_body 
              	+ boost::lexical_cast< std::string > ( counter++ ) 
              	+ ( extension.length() ? "." : "" ) 
			 	+ extension;
		}
	  };
	
	private:
	  
	  Glib::RefPtr<Gtk::Builder> m_builder;
	  
  	  // widgets
	  GLCanvas*				m_canvas;

	  Gtk::CheckButton*		m_origin_check;
  	  Gtk::CheckButton*		m_pivot_check;

      bool                  m_block_renderer;
      
	  //TODO: replace with connection
	  Gtk::ToolButton*		 m_save_frame_as_button;
	  Gtk::ToggleToolButton* m_record_frames_toggle;		
	  Gtk::MenuToolButton*	 m_render_target_menu;
      
	  boost::signals::connection m_record_frames_connection;
	  Filename					 m_record_filename;
	  
	  void	on_origin_toggled();
	  void	on_pivot_toggled();    

	  void  on_record_frames_toggled();
	  void  on_save_frame_as_clicked();
      void  on_block_renderer_clicked();
      
	  void  on_record_frame( /*const Glib::RefPtr<Gdk::Pixmap>& pixmap*/ );
	  
	  void  gl_draw_content()
	  { std::for_each( flat::View< gl::Drawable >::begin(), flat::View< gl::Drawable >::end(), std::mem_fun( &gl::Drawable::gl_draw ) ); }

	public:
	  
	  GLView( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder );
		
	  ~GLView()	{	}

	  //void 			clear()	{ m_canvas->clear(); }

      bool  is_renderer_blocked()   const 	{ return m_block_renderer; }
      
	  virtual void	invalidate( gl::Drawable* drawable = 0 )
	  { 
        # if defined DBG_FLAT_GLCANVAS_INVALIDATE
        std::clog << "flat::GLCanvas::invalidate\t|"
		  	      << ( drawable ? " drawable" : " view" )
                  << " render " << ( m_block_renderer ? "disabled" : "active" ) 
				  << std::endl;
        # endif
        if( !m_block_renderer ) m_canvas->request_redraw();	
	  }

      void  force_redraw()  { m_canvas->request_redraw(); }
	  
	  GLCanvas*		get_canvas() const	{ return m_canvas; }

	  static GLView*	create() 
	  { 
		// load the Glade file and instiate its widgets:
		Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( GTK_GLVIEW_BUILDER_FILE );
		GLView* vp = 0; 
		builder->get_widget_derived("gl_view_main_vbox",vp);
		return vp;
	  }
  };
}
