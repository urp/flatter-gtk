/***************************************************************************
 *            surface-drawable.h
 *
 *  Sun Oct 24 14:58:31 2010
 *  Copyright  2010  urp
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

# include "surface.h"
# include "drawable.h"
# include "interface.h"
# include "gl-view.h"

# include <gtkmm/comboboxtext.h>
# include <gtkmm/spinbutton.h>

namespace gl
{
  class SurfaceDrawable : public gl::Drawable
  {
	public:

	  typedef std::string 		mode_t;
	  typedef std::list<mode_t> mode_list_t;

	  // vertex modes
	  static const mode_t GAUSSIAN_CURVATURE_VERTEX_MODE;
	  static const mode_t HEIGHT_VERTEX_MODE;
	  static const mode_t INVISIBLE_VERTEX_MODE;
	  static const mode_t SOLID_VERTEX_MODE;
	  static const mode_t TEXTURE_VERTEX_MODE;

	  // edge modes
	  static const mode_t INVISIBLE_EDGE_MODE;
	  static const mode_t SOLID_EDGE_MODE;
	  // face modes
	  static const mode_t INVISIBLE_FACE_MODE;
	  static const mode_t SOLID_FACE_MODE;
	  static const mode_t TEXTURE_FACE_MODE;

	private:

	  // the active modes
	  mode_t	  m_vertex_mode, m_edge_mode, m_face_mode;
	  // available modes
	  mode_list_t m_vertex_modes, m_edge_modes, m_face_modes;

	public:

	  const mode_t&   get_vertex_mode()   const   { return m_vertex_mode; }
	  const mode_t&   get_edge_mode()     const   { return m_edge_mode; }
	  const mode_t&   get_face_mode()     const   { return m_face_mode; }

	  virtual mode_list_t get_vertex_mode_list()  const	{ return m_vertex_modes; }
	  virtual mode_list_t get_edge_mode_list()    const	{ return m_edge_modes; }
	  virtual mode_list_t get_face_mode_list()    const	{ return m_face_modes; }

	  bool	is_vertex_mode_supported( const mode_t& mode )	
	  { return std::find( m_vertex_modes.begin(), m_vertex_modes.end(), mode ) != m_vertex_modes.end(); }

	  bool	is_edge_mode_supported( const mode_t& mode )	
	  { return std::find( m_edge_modes.begin(), m_edge_modes.end(), mode ) != m_edge_modes.end(); }

	  bool	is_face_mode_supported( const mode_t& mode )	
	  { return std::find( m_face_modes.begin(), m_face_modes.end(), mode ) != m_face_modes.end(); }

	protected:

	  void append_vertex_modes( const mode_list_t& modes )
	  {
	    m_vertex_modes.insert( m_vertex_modes.end(), modes.begin(), modes.end() );
	    m_vertex_modes.sort();
	    m_vertex_modes.unique();
	  }

	  void append_edge_modes( const mode_list_t& modes )
	  {
	    m_edge_modes.insert( m_edge_modes.end(), modes.begin(), modes.end() );
	    m_edge_modes.sort();
	    m_edge_modes.unique();
	  }

	  void append_face_modes( const mode_list_t& modes )
	  {
	    m_face_modes.insert( m_face_modes.end(), modes.begin(), modes.end() );
	    m_face_modes.sort();
	    m_face_modes.unique();
	  }

	private:

	  std::shared_ptr< const flat::Surface >	m_surface;

	  mutable bool 		m_gl_texture_initialized;
	  mutable GLuint	m_gl_texture_handle;

	  flat::location_t	m_global_scale;
	  float			m_vertex_size;

	  void	gl_init_textures() const;
	  void  gl_draw_gaussian_curvature_vertices() const;
	  void  gl_draw_textured_faces()    const;

	public:

	  SurfaceDrawable( const std::shared_ptr< const flat::Surface >& surface )
	  : m_surface( surface ) 
	  , m_vertex_mode ( SOLID_VERTEX_MODE )
	  , m_vertex_modes( { GAUSSIAN_CURVATURE_VERTEX_MODE, HEIGHT_VERTEX_MODE, INVISIBLE_VERTEX_MODE, SOLID_VERTEX_MODE, TEXTURE_VERTEX_MODE } )
	  , m_edge_mode   ( SOLID_EDGE_MODE )
	  , m_edge_modes  ( { INVISIBLE_EDGE_MODE, SOLID_EDGE_MODE } )
	  , m_face_mode   ( SOLID_FACE_MODE )
	  , m_face_modes  ( { INVISIBLE_FACE_MODE, SOLID_FACE_MODE, TEXTURE_FACE_MODE } )
	  , m_gl_texture_initialized( false )
	  , m_global_scale( 1. ), m_vertex_size( 2. )	
	  { std::clog << "gl::SurfaceDrawable::SurfaceDrawable" << std::endl; }

	  virtual ~SurfaceDrawable() { std::clog << "gl::SurfaceDrawable::~SurfaceDrawable" << std::endl; }

	  virtual bool	gl_draw_vertices( const mode_t& )   const;												
	  virtual bool	gl_draw_edges   ( const mode_t& )   const;
	  virtual bool	gl_draw_faces   ( const mode_t& )   const;												

	  virtual void    gl_draw_others() const {	};		

	  void set_surface( const std::shared_ptr< flat::Surface >& surface )
	  { m_surface = surface; invalidate(); } 

	  const std::shared_ptr< const flat::Surface >& get_surface() const { return m_surface; }

	  // specifying gl::Drawable::gl_draw
	  virtual void gl_draw() 
	  { 
	    //std::clog << "flat::gl::SurfaceDrawable::gl_draw"
	    //    	    << std::flush << std::endl;

	    if( ! gl_draw_vertices( get_vertex_mode() ) ) 
		  std::cerr << "gl::SurfaceDrawable::gl_draw\t|" << " vertex mode \"" << get_vertex_mode() << "\" not rendered" << std::endl;

	    if( ! gl_draw_edges( get_edge_mode() ) ) 
		  std::cerr << "gl::SurfaceDrawable::gl_draw\t|" << " edge mode \"" << get_edge_mode() << "\" not rendered" << std::endl;

	    if( ! gl_draw_faces( get_face_mode() ) ) 
		  std::cerr << "gl::SurfaceDrawable::gl_draw\t|" << " face mode \"" << get_face_mode() << "\" not rendered" << std::endl;

	    gl_draw_others();

	  };

	  // sets the display mode of the surfaces vertices
	  // returns true if drawing in this mode is possible
	  // returns false if the mode is not supported
	  bool set_vertex_mode( const mode_t& mode )	
	  { 
	    if( mode == get_vertex_mode() ) return true;

	    if( !is_vertex_mode_supported( mode ) )
	    {	std::cerr << "gl::SurfaceDrawable::set_vertex_mode\t|"
		    		  << "WARNING: requested verex display mode \"" << mode <<"\""
		    		  << " not supported." << std::endl;
		  m_vertex_mode = INVISIBLE_VERTEX_MODE;
		  return false;
	    }
	    m_vertex_mode = mode;
	    invalidate();
	    return true;
	  }

	  bool set_edge_mode( const mode_t& mode )			
	  { 
	    if( mode == get_edge_mode() ) return true;

	    if( !is_edge_mode_supported( mode ) )
	    {	std::cerr << "gl::SurfaceDrawable::set_edge_mode\t|"
		  			  << "WARNING: requested edge display mode \"" << mode <<"\""
		  			  << " not supported." << std::endl;
		  m_edge_mode = INVISIBLE_EDGE_MODE;
		  return false;
	    }
	    m_edge_mode = mode;
	    invalidate();
	    return true;
	  }

	  bool set_face_mode( const mode_t& mode )			
	  { 
	    if( mode == get_face_mode() ) return true;

	    if( !is_face_mode_supported( mode ) )
	    {	std::cerr << "gl::SurfaceDrawable::set_face_mode\t|"
		    		  << "WARNING: requested face display mode \"" << mode <<"\""
		    		  << " not supported." << std::endl;
		  m_face_mode = INVISIBLE_FACE_MODE;
		  return false;
	    }
	    m_face_mode = mode;
	    invalidate();
	    return true;
	  }

	  const float& get_vertex_size() const	
	  { return m_vertex_size; }

	  virtual void set_vertex_size( const float& size )
	  { 
	    assert( size >= 0. );
	    if( size == m_vertex_size )	return;
	    m_vertex_size = size;
	    invalidate();
	  }

	  const flat::location_ref_t&	get_global_scale()	const	
	  { return m_global_scale; }

	  virtual void set_global_scale( const flat::location_ref_t& scale )
	  { if( ( m_global_scale == scale ) == true ) return;
	    m_global_scale = scale;
	    invalidate();
	  }

	  template< typename ColorIt >
	  void gl_draw_scaled_vertices( ColorIt col_it )	const;

  };

  class SharedSurfaceDrawable  : public SurfaceDrawable, public flat::View< SurfaceDrawable >
  {
	public:

	  SharedSurfaceDrawable( const std::shared_ptr< flat::Surface >& surface )
	  : SurfaceDrawable( surface )	{ std::clog << "gl::SharedSurfaceDrawable::SharedSurfaceDrawable" << std::endl; }

      virtual ~SharedSurfaceDrawable()   { std::clog << "gl::SharedSurfaceDrawable::~SharedSurfaceDrawable" << std::endl; }
      
	  //----| Drawable interface

	  void    gl_draw()
	  { 
		# if defined DBG_GL_SHAREDSURFACEDRAWABLE
	    std::clog << "gl::SharedSurfaceDrawable::gl_draw\t|"
	   	  		  << " vertex mode = \"" << get_vertex_mode() << "\""
		  		  << " edge mode = \"" << get_edge_mode() << "\""
		  		  << " surface mode = \"" << get_face_mode() << "\"" << std::endl;
		# endif
	    bool vdone = gl_draw_vertices( get_vertex_mode() );
	    bool edone = gl_draw_edges( get_edge_mode() );
	    bool fdone = gl_draw_faces( get_face_mode() );

	    for( auto it = flat::View< SurfaceDrawable >::begin(); it != flat::View< SurfaceDrawable >::end(); it++ )
	    { 
		  if( ! vdone ) vdone = (*it)->gl_draw_vertices( get_vertex_mode() );
		  if( ! edone ) edone = (*it)->gl_draw_edges( get_edge_mode() );
		  if( ! fdone ) fdone = (*it)->gl_draw_faces( get_face_mode() );

		  (*it)->gl_draw_others();
	    }        
	  } 

	  //----| View interface

	  void    add_drawable( SurfaceDrawable* d )
	  {
	    std::clog << "flat::SharedSurfaceDrawable::add_drawable" << std::endl << std::flush;

	    flat::View< SurfaceDrawable >::add_drawable( d );

	    append_vertex_modes( d->get_vertex_mode_list() );
	    append_edge_modes  ( d->get_edge_mode_list() );
	    append_face_modes  ( d->get_face_mode_list() );
	  }

	  void	invalidate( SurfaceDrawable* d )
	  { 
	    std::clog << "gl::SharedSurfaceDrawable::invalidate\t| invalidating " << ( d ? "drawable" : "view" ) << std::endl;
	    SurfaceDrawable::invalidate();
	  }

	  void    set_vertex_size( const float& s )
	  { 
	    for( auto it = flat::View< SurfaceDrawable >::begin(); it != flat::View< SurfaceDrawable >::end(); it++ )
		  (*it)->set_vertex_size(s);
	    SurfaceDrawable::set_vertex_size( s );
	    invalidate_view();
	  }

	  void    set_global_scale( const flat::location_ref_t& s )
  	  {
	    for( auto it = flat::View< SurfaceDrawable >::begin(); it != flat::View< SurfaceDrawable >::end(); it++ )
		  (*it)->set_global_scale( s );
	    SurfaceDrawable::set_global_scale( s );
	    invalidate_view();
	  }
  };
}

namespace gtk
{
  class SurfaceDrawableUI 	: public UI
  {
	private:

	  std::shared_ptr< gl::SurfaceDrawable > 	m_drawable;

	  Gtk::ComboBoxText*	m_vertex_mode_combo;
	  Gtk::ComboBoxText*	m_edge_mode_combo;
	  Gtk::ComboBoxText*	m_face_mode_combo;

	  // gtk connections TODO : remove?
	  sigc::connection	m_vertex_check_connection;
	  sigc::connection	m_vertex_mode_combo_connection;

	  sigc::connection	m_edge_check_connection;
	  sigc::connection	m_edge_mode_combo_connection;

	  sigc::connection	m_face_check_connection;
	  sigc::connection	m_face_mode_combo_connection;

	  sigc::connection	m_vertex_size_spin_connection;
	  sigc::connection	m_rescale_z_spin_connection;

	protected:

	  const std::shared_ptr< gl::SurfaceDrawable >& get_drawable()    const   { return m_drawable; }   

	  static std::string	get_builder_filename()	{ return "surface-drawable.ui"; }		

	  void	on_vertex_mode_changed( Gtk::ToggleButton* check, Gtk::ComboBoxText* combo ) 
	  { m_drawable->set_vertex_mode( !check->get_active() ? gl::SurfaceDrawable::INVISIBLE_VERTEX_MODE : std::string( combo->get_active_text() ) ); }

	  void	on_edge_mode_changed( Gtk::ToggleButton* check, Gtk::ComboBoxText* combo ) 
	  { m_drawable->set_edge_mode( !check->get_active() ? gl::SurfaceDrawable::INVISIBLE_EDGE_MODE : std::string( combo->get_active_text() ) ); }

	  void	on_face_mode_changed( Gtk::ToggleButton* check, Gtk::ComboBoxText* combo ) 
	  { m_drawable->set_face_mode( !check->get_active() ? gl::SurfaceDrawable::INVISIBLE_FACE_MODE : std::string( combo->get_active_text() ) ); }

	  void	on_vertex_size_spin_changed( Gtk::SpinButton* spin )
	  { m_drawable->set_vertex_size( spin->get_value() ); }

	  void	on_rescale_z_spin_changed( Gtk::SpinButton* spin )
	  { m_drawable->set_global_scale( flat::location_t( 1, 1, spin->get_value() ) ); }

	  SurfaceDrawableUI() = delete; 

	public:

	  SurfaceDrawableUI( const std::shared_ptr< gl::SurfaceDrawable >& drawable );

      virtual	~SurfaceDrawableUI();

	  void    update_vertex_combo();

	  void    update_edge_combo();

	  void    update_face_combo();

	  void    update_mode_combos()
	  {
	    update_vertex_combo();
	    update_edge_combo();
	    update_face_combo();
	  }
  }; 
}

template< typename ColorIt >
void gl::SurfaceDrawable::gl_draw_scaled_vertices( ColorIt col_it )	const	
{ 
  glPushMatrix();
  Scale( m_global_scale );
  glBegin( GL_POINTS );
	for( auto vertex_its = m_surface->vertex_handles(); vertex_its.first != vertex_its.second; ++vertex_its.first )	
	{ Color( *col_it++ );
	  Vertex( vertex_its.first->location() );
	}
  glEnd();
  glPopMatrix();
}

