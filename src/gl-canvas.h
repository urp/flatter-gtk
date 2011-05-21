/***************************************************************************
 *            gl-canvas.h
 *
 *  Sat Jan 15 14:41:56 2011
 *  Copyright  2011  urp
 *  <urp@<host>>
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

//# define DBG_FLAT_GL_RENDER_TARGETS
//# define DBG_FLAT_GL_GLCANVAS


# include "gtkmm.h"
# include "gtkglmm.h"

# include "common.h"
# include "utk/inertial.h"

# include <array>
# include <boost/signals.hpp>

namespace gtk
{

  class View;
  
  class GLCanvas;

  using namespace flat;
  
  class GLRenderTarget
  {
	public:
	  typedef enum { WINDOW = 0, PIXMAP = 1 } types_t;
	
	protected:
	  types_t	m_type;

	  GLCanvas&	m_canvas;						
	  
      Glib::RefPtr<Gdk::GL::Drawable> m_drawable;

  	  Glib::RefPtr<Gdk::GL::Context>  m_context;


	  GLRenderTarget( const types_t& type, GLCanvas& canvas )
	  : m_type( type ), m_canvas( canvas ), m_drawable(), m_context()	{	}
	
	public:
		
	  void gl_begin_context()
	  {  
		assert( m_drawable );	assert( m_context );

		if( ! m_drawable->gl_begin( m_context ) ) 
	      std::cerr << "flat::GLCanvas::active_target::gl_begin_context\t|" << "FAILED" << std::endl; 
	  }

	  void gl_end_context() { m_drawable->gl_end(); }

	  void gl_finish() 
	  { 
		assert( m_drawable );
		glFinish();
		if( m_drawable->is_double_buffered() ) m_drawable->swap_buffers();
  		else glFlush();
	  }
	  
	  virtual bool configure( const size_t width, const size_t height) = 0;

	  const types_t& get_type() const { return m_type; }
  };

  
  class GLWindowRenderTarget : public GLRenderTarget
  {
	public:

	  GLWindowRenderTarget( GLCanvas& canvas );

	  bool configure( const size_t width, const size_t height );
  };

  class GLPixmapRenderTarget : public GLRenderTarget
  {
	private:
	
	  Glib::RefPtr< Gdk::GL::Config > m_config;

	public:
	  
  	  GLPixmapRenderTarget( GLCanvas& canvas );

	  bool configure( const size_t width, const size_t height );
  };

  
  class GLCanvas : public Gtk::DrawingArea
  { 
	public: //types

	  friend class View;
	  friend class GLWindowRenderTarget;
	  friend class GLPixmapRenderTarget;
	  
	  static flat::distance_t    cam_dist_step;
	  
	  typedef boost::signal<void()>	content_request_signal;
	  typedef boost::signal< void( const Glib::RefPtr<Gdk::Pixmap>& ) >	pixmap_update_signal;
	  
	private:

	  // drawing options
	  bool	m_show_origin;
	  bool	m_show_pivot;
	  
	  content_request_signal m_gl_content_request_signal;

	  pixmap_update_signal 	 m_pixmap_update_signal;

	  std::array< std::shared_ptr< GLRenderTarget >, 2 > m_targets;
      
	  std::shared_ptr< GLRenderTarget >	m_active_target;

	  Glib::RefPtr< Gdk::Pixmap >	  m_pixmap;

	  //used by render target
  	  void gl_initialize_context();
	  void gl_setup_view( const float width, const float height );
	  void set_pixmap( const Glib::RefPtr< Gdk::Pixmap >& pixmap )
	  { m_pixmap = pixmap; }

	  void	gl_render_scene();
	  
	  // camera 
	  coord_t					cam_dist;
	  location_t				cam_center;
	  utk::inertial<coord_t> 	cam_inertial;
	  location_t				old_cam_center;
	  utk::inertial<coord_t> 	old_cam_inertial;
	  location2d_t				old_mouse_pos;

	protected:

	  // callback functions
	  virtual bool	on_configure_event(GdkEventConfigure* event);
	  virtual bool	on_expose_event(GdkEventExpose* event);

      virtual bool 	on_button_press_event(GdkEventButton* event);
      virtual bool 	on_motion_notify_event(GdkEventMotion* event);
      virtual bool 	on_scroll_event(GdkEventScroll* event);

	public:

	  GLCanvas( BaseObjectType* cobject
		      , const Glib::RefPtr<Gtk::Builder>& builder );

	  ~GLCanvas()	{		}

	  void 	set_render_target( const GLRenderTarget::types_t target_type )
	  {
        # if defined DBG_FLAT_GL_GLCANVAS
		std::clog << "flat::GLCanvas::set_render_targets\t|"
				  << ( target_type == GLRenderTarget::WINDOW ? " window" : "" )
	  	   		  << ( target_type == GLRenderTarget::PIXMAP ? " pixmap" : "" )
		  		  << std::endl;
        # endif
		m_active_target = ( target_type == GLRenderTarget::WINDOW 
		  				    ? std::get<GLRenderTarget::WINDOW>( m_targets )
		  				    : std::get<GLRenderTarget::PIXMAP>( m_targets ) );

		m_active_target->configure( get_width(), get_height() );

		request_redraw();
	  }

	  bool render_to_window();
	  bool render_to_pixmap();
	  
	  void	request_redraw()	{ Gtk::DrawingArea::queue_draw(); }
	  
	  void	set_origin_visibility( const bool show )
	  { 
        if( show == get_origin_visibility() ) return;
		m_show_origin = show;
		request_redraw();
	  }

	  void	set_pivot_visibility( const bool show )
	  { 
        if( show == get_pivot_visibility() ) return;
		m_show_pivot = show;
		request_redraw();
	  }

  	  Glib::RefPtr< Gdk::Pixmap >& 	     get_pixmap() { return m_pixmap; };
	  const Glib::RefPtr< Gdk::Pixmap >& get_pixmap() const { return m_pixmap; };
	  
	  const bool&	get_origin_visibility()	const	{ return m_show_origin; }
	  
	  const bool&	get_pivot_visibility()	const	{ return m_show_pivot; }
	  
	  void			gl_draw_coords()	const;

	  boost::signals::connection connect_content_provider( content_request_signal::slot_type provider )
	  { return m_gl_content_request_signal.connect( provider ); }

	  boost::signals::connection connect_pixmap_observer( pixmap_update_signal::slot_type observer )
	  { return m_pixmap_update_signal.connect( observer ); }

  };
  
} // of namespace flat