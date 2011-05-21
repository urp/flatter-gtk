
/*  mmp-visualizer.h - Copyright Peter Urban 2010

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

# pragma once

# include "drawable.h"

# include "mmp-geodesics.h"

# include "surface-drawable.h"
# include "gl-view.h"

# include <utk/geometry.h>

# include <gtkmm/window.h>

# include <fstream>

/* For testing propose use the local (not installed) glade file */
/* #define GTK_MMP_OBSERVER_BUILDER_FILE PACKAGE_DATA_DIR"/gtk_flatdoc/glade/mmp-propagation-observer.ui" */
# define GTK_MMP_OBSERVER_BUILDER_FILE "src/mmp-propagation-observer.ui"


//#define DBG_FLAT_MMP_VISUALIZER_DRAW_WINDOW


namespace gl
{
  using namespace mmp;
  
  class GeodesicsDrawable : public ::gl::Drawable
  {
    public: // types
        
      typedef enum { FLAT_SHADING
				   , ABSOLUTE_DISTANCE_SHADING
				   , TEXTURE_DISTANCE_SHADING	}   shading_t;
        
	private: // data members

	  Geodesics* m_geodesics;
	  
	  EventPoint::flags_t  m_events_visible;
      bool                 m_covering_visible;

	  GLuint 		m_equidist_texture;

	  bool m_gl_initialized;

	private: // functions

	  void gl_init_textures();
		
	  void gl_draw_wavefront_indicators( EventPoint* 	 ev
                                       , rgba_color_ref_t crossing_color
                                       , rgba_color_ref_t colinear_color 
                                       );

      void gl_draw_window_edges( const Window& window, const Window::types& window_type
                               , const location_t& win_left, const location_t& win_right
                               , const location_t& pre_left, const location_t& pre_right 
                               , const rgba_color_ref_t edge_color
                               , const rgba_color_ref_t source_edge_color );

      bool gl_draw_window( const Window&          window
                         , const rgba_color_ref_t edge_color
                         , const rgba_color_ref_t fill_color
                         , const rgba_color_ref_t source_color 
                         , const shading_t        shading );
      
      bool gl_draw_window_subdivision( const Window& window
                                     , const rgba_color_ref_t edge_color
                                     , const rgba_color_ref_t fill_color
                                     , const rgba_color_ref_t source_color 
                                     , const shading_t shading
                                     );

      void gl_do_draw_subdivision( const Window&           window
                                      , const Window::types     window_type
                                      , const distance_t        recursion_threshold
                                      , const rgba_color_ref_t  edge_color
                                      , const rgba_color_ref_t  fill_color
                                      , const rgba_color_ref_t  source_color
                                      , const shading_t         shading
                                      , const location_ref_t    normal
                                      , const std::pair< const coord_t&   , const coord_t&    >& bounds
                                      , const std::pair< const distance_t&, const distance_t& >& win_distances
                                      , const std::pair< const distance_t&, const distance_t& >& pre_distances
                                      , const std::pair< const location_t&, const location_t& >& win_points
                                      , const std::pair< const location_t&, const location_t& >& pre_points
                                      , const ps_t& ps
                                      , const ps_t& pre_ps
                                      );

      void gl_draw_interval( const Window&             window
                           , const Window::types       window_type
                           , const rgba_color_ref_t    edge_color
                           , const rgba_color_ref_t    fill_color
                           , const rgba_color_ref_t    source_color
                           , const shading_t           shading
                           , const location_ref_t      normal
                           , const std::pair< const distance_t&, const distance_t& >& win_distances
                           , const std::pair< const distance_t&, const distance_t& >& pre_distances
                           , const std::pair< const location_t&, const location_t& >& win_points
                           , const std::pair< const location_t&, const location_t& >& pre_points );

      
      void gl_draw_event_point( EventPoint* ev );

      void  gl_draw_window_sequence( const Window& window
                                   , const rgba_color_ref_t edge_color
                                   , const rgba_color_ref_t fill_color
                                   , const rgba_color_ref_t source_color
                                   , const shading_t         shading );

      void gl_draw_wavefront( const shading_t );

      void gl_draw_covering( const shading_t );
        
	public: // functions
        
	  GeodesicsDrawable( Geodesics* g );

	  virtual ~GeodesicsDrawable() { std::clog << "flat::gl::GeodesicsDrawable::~GeodesicsDrawable\t" << std::endl; }
		
	  void gl_draw();
	  

      const EventPoint::flags_t&    get_events_visibility() const
      { return m_events_visible; }

      void set_events_visibility( const EventPoint::flags_t& visibility )
      { m_events_visible = visibility; }
        
      const bool& get_covering_visibility() const 
      { return m_covering_visible; }

      void set_covering_visibility( const bool& visibility ) 
      { m_covering_visible = visibility; }

      Geodesics*  get_geodesics() const   { return m_geodesics; }
  };
}

  
namespace gtk
{

  using namespace flat;
  using namespace mmp;

  class GeodesicsInspector : public Gtk::Window
  {

    private: // data members
	
      std::shared_ptr< gl::GeodesicsDrawable >	m_geodesics_drawable;
	  std::shared_ptr< ::gl::SurfaceDrawable >	m_surface_drawable;

  	  mmp::Geodesics* m_geodesics;
  
      gtk::GLView*	m_view;


	  sigc::connection m_iterate_connection;

    
	  Gtk::VBox* m_vbox;

      Gtk::Entry* m_top_event_entry;
      Gtk::Entry* m_top_window_entry;

	  Gtk::Label*  m_top_distance_label;
  	  Gtk::Label*  m_queue_size_label;
    
      Gtk::ToolButton* 		m_step_button;
      Gtk::ToggleToolButton* 	m_iterate_button;

      Gtk::CheckButton*       m_check_result_check;
      Gtk::CheckButton*       m_stop_on_errors_check;
    
      bool m_check_result;
      bool m_stop_on_errors;

	  std::ofstream	m_debug_log_file;
      std::streambuf* m_clog_buffer;

      void    redirect_clog_to_file( const std::string& path )
      {
        if( m_clog_buffer ) restore_clog();
        std::clog << "mmp::Geodesics::redirect_clog_to_file\t|" << " file \"" << path << '\"' << std::endl << std::flush;
        m_debug_log_file.open( path );
        assert( m_debug_log_file.is_open() );
        m_clog_buffer = std::clog.rdbuf();
        std::clog.rdbuf( m_debug_log_file.rdbuf() );
        std::clog << "mmp::Geodesics::redirect_clog_to_file\t|" << " redirected from clog" << std::endl << std::flush;
          
      }
   
      // restore clog buffer and close log file
      void    restore_clog()  
      { 
        std::clog << "mmp::Geodesics::restore_clog\t|" << std::endl << std::flush;
        assert( m_clog_buffer );	  
        std::clog.rdbuf( m_clog_buffer );
        m_clog_buffer = 0;
   
        if( m_debug_log_file.is_open() ) m_debug_log_file.close();
        std::clog << "mmp::Geodesics::restore_clog\t| clog is back online" << std::endl << std::flush;
      }
    
    protected:
	
      Glib::RefPtr<Gtk::Builder> m_builder;

	  Gtk::VBox* 	set_view( ::gtk::GLView* view )
	  { assert(view);
		assert(!m_view || m_view == view );
		m_view = view;
		m_vbox->pack_end( *m_view );
	  }
	
	  bool step();

      void single_step()	{ step(); };
	
	  void toggle_iteration();

      void on_stop_on_errors_check_clicked()   
	  { m_stop_on_errors = m_stop_on_errors_check->get_active(); }

      void on_check_result_check_clicked()     
	  { m_check_result = m_check_result_check->get_active(); }
      
      virtual void on_hide()  
      { restore_clog(); 
        Gtk::Window::on_hide();
      }
      
    public:

      GeodesicsInspector() = delete;

      GeodesicsInspector( const GeodesicsInspector& ) = delete;
      
	  GeodesicsInspector( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder );
	
	  ~GeodesicsInspector() 
      { delete m_view; }
	
	  void run_propagation();

	  void initialize( mmp::Geodesics* geodesics, const std::shared_ptr< const Surface > surface );

	  static GeodesicsInspector* create_propagation_observer( mmp::Geodesics* geodesics, const std::shared_ptr< const Surface > surface )
  	  {
        std::cout << "mmp::visualizer::gtk::GeodesicsInspector::create_propagation_observer" << std::endl;  
	    Glib::RefPtr< Gtk::Builder > builder = Gtk::Builder::create_from_file( GTK_MMP_OBSERVER_BUILDER_FILE );
	    GeodesicsInspector* obs = 0;
	    builder->get_widget_derived( "observer_window", obs );
	    assert( obs );
	    obs->initialize( geodesics, surface );
	    return obs;
	  }
      
	  static GeodesicsInspector* create_inspector( mmp::Geodesics* geodesics, const std::shared_ptr< const Surface > surface )
      {
	    Glib::RefPtr< Gtk::Builder > builder = Gtk::Builder::create_from_file( GTK_MMP_OBSERVER_BUILDER_FILE );
	    gtk::GeodesicsInspector* obs = 0;
	    builder->get_widget_derived( "observer_window", obs );

        assert( obs );
	    obs->initialize( geodesics, surface );

        // hide propagation controls
        obs->m_step_button->hide();
        obs->m_iterate_button->hide();

        obs->m_geodesics_drawable->set_events_visibility( false );
        obs->m_geodesics_drawable->set_covering_visibility( true );
        
	    return obs;
	  }
  };
}

