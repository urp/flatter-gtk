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

# include "surface-import-dialog.h"

# include "surface.h"
# include "quad-surface.h"
# include "surface-generators.h"

# include "spring-solver.h"
# include "mds-solver.h"



using namespace gtk;

SurfaceImportDialog::SurfaceImportDialog( BaseObjectType* cobject
                                        , const Glib::RefPtr<Gtk::Builder>& builder )
: Gtk::FileChooserDialog(cobject), m_builder( builder )
{			
  // add response buttons
  add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
  add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_OK );

  builder->get_widget( "inc_x_spinbutton", inc_x_spin );
  inc_x_spin->set_value(1); // workaround for bug in gtkbuilder
  
  builder->get_widget( "inc_y_spinbutton", inc_y_spin );
  inc_y_spin->set_value(1);
  
  builder->get_widget( "center_xy_checkbutton", center_xy_check );

  builder->get_widget( "center_z_checkbutton", center_z_check );
  
  builder->get_widget( "normalize_xy_checkbutton", normalize_xy_check );

  builder->get_widget( "normalize_z_checkbutton", normalize_z_check );

  builder->get_widget( "quad_topology_radiobutton", quad_topology_radiobutton );

  builder->get_widget( "triangle_topology_radiobutton", triangle_topology_radiobutton );
}


void	SurfaceImportDialog::set_increment( const flat::size_pair& stride )
{
  inc_x_spin->set_value( get<0>(stride) );
  inc_y_spin->set_value( get<1>(stride) );
}

void	SurfaceImportDialog::set_center_flags( const utk::vecn3b& c )
{ assert(c.x()==c.y());
  center_xy_check->set_active(c.x());
  center_z_check ->set_active(c.z());
}

void	SurfaceImportDialog::set_rescale_flags( const utk::vecn3b& n )
{ assert(n.x()==n.y());
  normalize_xy_check->set_active(n.x());
  normalize_z_check ->set_active(n.z());
}

std::pair< size_t, size_t >	SurfaceImportDialog::get_increment()	const
{ return { inc_x_spin->get_value(), inc_y_spin->get_value() }; }


utk::vec3b	SurfaceImportDialog::get_center_flags()	const
{ return utk::vec3b(center_xy_check->get_active(),
					center_xy_check->get_active(),
					center_z_check ->get_active() ); 
}

utk::vec3b	SurfaceImportDialog::get_rescale_flags()	const
{ return utk::vec3b(normalize_xy_check->get_active(),
					normalize_xy_check->get_active(),
					normalize_z_check ->get_active() ); 
}

void	SurfaceImportDialog::set_model( Controller& controller )	const	throw(std::bad_alloc)
{
  using namespace flat;

  assert( !Gtk::FileChooserDialog::get_filename().empty() );
  
  PdmFileReader< stride_predicate, accept_all_predicate > file_reader( FileChooserDialog::get_filename(), stride_predicate( get_increment() ) );

  SimpleRectlinearTriangulator      triangulator( file_reader.vertex_field_size() );

  CenterRescaleTransform    center_rescale( get_center_flags(), get_rescale_flags() );
  
  if( quad_topology_radiobutton->get_active() )
  {	
	const std::shared_ptr< QuadSurface > surface = QuadSurface::create_with_generator( file_reader, triangulator, center_rescale );

	const std::shared_ptr< spring::SpringSolver<false,false> > solver( new spring::SpringSolver<false,false>( surface ) );
    
	controller.set_model( surface, solver );
  }

  if( triangle_topology_radiobutton->get_active() )
  {	
	const std::shared_ptr< Surface > surface = Surface::create_with_generator( file_reader, triangulator, center_rescale );

	const std::shared_ptr<mds::EqualWeightSolver>  solver( new mds::EqualWeightSolver( surface ) );

	controller.set_model( surface, solver );
  }
}

void	SurfaceImportDialog::on_response( int response_id )
{ 
  hide();
}
