/*
 * flatter
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


#include <cassert>
#include <iostream>

#include "common.h"
#include "gl-view.h"


gtk::GLView::GLView( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder )
: Gtk::VBox(cobject), m_builder( builder ), m_block_renderer( false )
{
  m_builder->get_widget_derived( "gl_canvas" , m_canvas );
  get_canvas()->connect_content_provider( std::bind( std::mem_fun( &GLView::gl_draw_content ), this ) );
  m_record_frames_connection = get_canvas()->connect_pixmap_observer ( std::bind( std::mem_fun( &GLView::on_record_frame), this ) );
  m_record_frames_connection.block();
  // get control widgets

  m_builder->get_widget( "save_frame_as_button" , m_save_frame_as_button );
  m_save_frame_as_button->signal_clicked().connect( sigc::mem_fun( *this, &GLView::on_save_frame_as_clicked ) ); 

  m_builder->get_widget( "record_frames_toggle" , m_record_frames_toggle );
  m_record_frames_toggle->signal_toggled().connect( sigc::mem_fun( *this, &GLView::on_record_frames_toggled ) ); 

  m_builder->get_widget( "render_target_toolmenu" , m_render_target_menu );
  m_render_target_menu->signal_clicked().connect( sigc::mem_fun( *this, &GLView::on_block_renderer_clicked ) ); 
  
  
  m_builder->get_widget( "show_origin_checkbutton" , m_origin_check );
  m_origin_check->set_active( m_canvas->get_origin_visibility() );
  m_origin_check->signal_toggled().connect( sigc::mem_fun( *this, &GLView::on_origin_toggled) ); 

  m_builder->get_widget( "show_pivot_checkbutton" , m_pivot_check );
  m_pivot_check->set_active( m_canvas->get_pivot_visibility() );
  m_pivot_check->signal_toggled().connect( sigc::mem_fun( *this, &GLView::on_pivot_toggled) ); 
}

void   save_pixmap( const Glib::RefPtr<Gdk::Pixmap>& pixmap, const std::string& filename = "frame", const std::string& format = "png" )	//const
{
  int width, height;
  pixmap->get_size( width, height );
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create( pixmap->get_image( 0, 0, width, height ), 0, 0, width, height );

  pixbuf->save( filename, format );
}

void gtk::GLView::on_record_frame( /*const Glib::RefPtr<Gdk::Pixmap>& pixmap*/ )
{
  std::clog << "gtk::GLView::on_record_frame\t|" << " file " << m_record_filename() << std::endl;
  save_pixmap( m_canvas->get_pixmap(), m_record_filename(), m_record_filename.extension );
}

void gtk::GLView::on_save_frame_as_clicked()
{
  Gtk::FileChooserDialog fcd( "Choose a png file to save the current frame", Gtk::FILE_CHOOSER_ACTION_SAVE );
  fcd.add_button( Gtk::Stock::SAVE, Gtk::RESPONSE_OK );
  if( fcd.run() == Gtk::RESPONSE_OK )
  {
	m_canvas->render_to_pixmap();

	save_pixmap( m_canvas->get_pixmap(), fcd.get_filename(), "png" );
  }
}

void gtk::GLView::on_record_frames_toggled()
{
  if( m_record_frames_toggle->get_active() )
  {
    Gtk::FileChooserDialog fcd( "Choose a folder were the save the frames", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );
    fcd.add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_OK );
	if( fcd.run() == Gtk::RESPONSE_OK )
	{
	  m_canvas->set_render_target( GLRenderTarget::PIXMAP );
      m_record_filename = Filename( fcd.get_filename(), "frame", 0, "png" );
	  m_record_frames_connection.unblock();
	}else m_record_frames_toggle->set_active( false );
  }else
  {
	m_record_frames_connection.block();
	m_canvas->set_render_target( GLRenderTarget::WINDOW );
  }
}

void gtk::GLView::on_block_renderer_clicked() 
{
  m_block_renderer = !m_block_renderer;
  m_render_target_menu->set_stock_id( m_block_renderer
                                      ? Gtk::Stock::DISCONNECT
                                      : Gtk::Stock::CONNECT );
}

void gtk::GLView::on_origin_toggled()
{
  m_canvas->set_origin_visibility( m_origin_check->get_active() );
}

void gtk::GLView::on_pivot_toggled()
{
  m_canvas->set_pivot_visibility( m_pivot_check->get_active() );
}