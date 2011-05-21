//           surface-interface.cpp
//  Sat Nov 14 16:30:06 2009
//  Copyright  2009  Peter Urban
//  <s9peurba@stud.uni-saarland.de>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA

#include "surface-interface.h" 

using namespace gtk;

SurfaceUI::SurfaceUI( const std::shared_ptr< flat::Surface >& surface )
: UI( get_builder_filename(), "surface_model_widget" ), m_surface( surface )
{ 
  std::clog << "flat::SurfaceUI::SurfaceUI" << std::endl; 

  Gtk::Label*	surface_name_label;
  get_builder()->get_widget( "surface_name_label", surface_name_label ); 
  surface_name_label->set_text( surface->get_name() );

  Gtk::CheckButton* curvature_update_check;
  get_builder()->get_widget( "curvature_update_check", curvature_update_check );
  curvature_update_check->signal_clicked().connect( sigc::mem_fun( *this, &SurfaceUI::toggle_curvature_update ) );
  m_curvature_update = curvature_update_check->get_active();

  Gtk::Label*	num_vertices_label;
  get_builder()->get_widget( "num_vertices_value_label", num_vertices_label );
  num_vertices_label->set_text( boost::lexical_cast<std::string>( surface->num_vertices() ) );
  
  get_builder()->get_widget( "height_value_label", m_height_label );

  get_builder()->get_widget( "curvature_value_label", m_curvature_label );
  
  update();
}

void SurfaceUI::toggle_curvature_update() 
{ 
  m_curvature_update = !m_curvature_update;
  
  update();
}

void SurfaceUI::update()
{ 
  std::clog << "flat::gtk::SurfaceUI::update\t| update curvature " << m_curvature_update << std::endl;
  flat::coord_t max_height_mag = std::max( std::fabs( m_surface->max_height() ), std::fabs( m_surface->min_height() ) );
  m_height_label->set_text( boost::lexical_cast< std::string >( max_height_mag ) );


  if( m_curvature_update )
  { 
    auto curvature_extrema = m_surface->get_curvature_extrema();
    auto max_curvature_mag = std::max( std::fabs( curvature_extrema.first ), std::fabs( curvature_extrema.first ) );
    m_curvature_label->set_text( boost::lexical_cast< std::string >( max_curvature_mag ) );
  }

  UI::update();
}
