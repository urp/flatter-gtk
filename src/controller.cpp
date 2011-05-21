//           controller.cpp
//  Sat Aug  8 11:54:37 2009
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

# include "controller.h"

# include "surface-generators.h"
# include "drawable.h"

using namespace gtk;

const UI::widget_name_t Controller::SURFACE_MODEL_SLOT = "surface_model_container";
const UI::widget_name_t Controller::SURFACE_VIEW_SLOT 	= "surface_view_container";
const UI::widget_name_t Controller::SOLVER_MODEL_SLOT 	= "solver_model_container";
const UI::widget_name_t Controller::SOLVER_VIEW_SLOT 	= "solver_view_container";

Controller::Controller( const Glib::RefPtr<Gtk::Builder>& builder )
: m_builder( builder ), model(), m_gl_view( 0 ), m_play_connection()
{
  // derived widgets
  get_builder()->get_widget_derived( "image_export_dialog", image_export_dialog );
  image_export_dialog->set_texel_size( 3 );

  // main window
  get_builder()->get_widget( "main_window", main_window );
  
  // viewport container
  get_builder()->get_widget( "main_paned", main_paned );
  
  // toolbar buttons
  get_builder()->get_widget( "play_toolbar_button", play_button );
  play_button->signal_clicked().connect( sigc::mem_fun( *this, &Controller::on_play_button_clicked ) ); 

  get_builder()->get_widget( "new_toolbar_button", new_button );
  new_button->signal_activate().connect( sigc::mem_fun( *this, &Controller::on_new_menu_item_activate ) );

  get_builder()->get_widget( "export_toolbar_button", export_button );
  export_button->signal_clicked().connect( sigc::mem_fun( *this, &Controller::on_export_button_clicked ) ); 

  get_builder()->get_widget( "open_toolbar_button", open_button );
  open_button->signal_activate().connect( sigc::mem_fun( *this, &Controller::on_open_menu_item_activate ) ); 

  // default OpenGL view
  set_view( gtk::GLView::create() ); 
  
  // construct the default model

  //default model core components
  typedef flat::QuadSurface	default_surface_t;
  typedef spring::SpringSolver< false, false >	default_solver_t;
  
  flat::WaveGenerator         		  generator( { 10, 10 }, .01 );
  flat::SimpleRectlinearTriangulator  triangulator( generator.vertex_field_size() );
  flat::NoTransform                   transform;
  
  std::shared_ptr<default_surface_t>  	surface = default_surface_t::create_with_generator( generator, triangulator, transform );
  
  std::shared_ptr<default_solver_t>		solver( new default_solver_t( surface ) );

  // solver combo box 
  m_solver_combo = Gtk::manage( new Gtk::ComboBoxText() );

  m_solver_combo->append_text( spring::SpringSolver<false,false>::class_name() );
  m_solver_combo->append_text( spring::SpringSolver<true,false>::class_name() );
  m_solver_combo->append_text( spring::SpringSolver<false,true>::class_name() );
  m_solver_combo->append_text( spring::SpringSolver<true,true>::class_name() );
  m_solver_combo->append_text( mds::EqualWeightSolver::class_name() );
  m_solver_combo->append_text( mds::GeneralSolver::class_name() );

  //m_solver_combo->set_active_text( default_solver_t::class_name() );
  
  Gtk::Container* solver_combo_bin;
  get_builder()->get_widget( "solver_combo_bin", solver_combo_bin );
  solver_combo_bin->add( *m_solver_combo );

  m_solver_combo->signal_changed().connect( sigc::bind( sigc::mem_fun( *this, &Controller::on_solver_combo_changed ), m_solver_combo ) );
  
  m_solver_combo->show();

  set_model( surface, solver );

}

// connects/disconnects the gtk idle signal to Contoller::step()
// returns true if the connection is now unblocked (simulation running)
// returns false if connection is blocked (simulation stopped)
void    Controller::toggle_iteration()
{

  if( !m_play_connection.connected() ) 
  { m_play_connection = Glib::signal_idle().connect( sigc::mem_fun( *this, &Controller::step ), Glib::PRIORITY_DEFAULT_IDLE);
    m_play_connection.block();
  }
  
  if( m_play_connection.blocked() )
  { m_play_connection.unblock(); play_button->set_stock_id( Gtk::Stock::MEDIA_PAUSE ); }
  else { m_play_connection.block(); play_button->set_stock_id( Gtk::Stock::MEDIA_PLAY  ); }
}


void	Controller::on_new_menu_item_activate()
{
  //on_apply_surface_button_clicked();
}

void	Controller::on_open_menu_item_activate()
{
  std::auto_ptr< SurfaceImportDialog > surface_import_dialog( SurfaceImportDialog::create() );
  
  // ask the user for a file to open
  do{ if( surface_import_dialog->run() == Gtk::RESPONSE_CANCEL ) return; }
  while( surface_import_dialog->get_filename().empty() );

  std::clog << "Controller::on_open_surface_button_clicked" << "\t|loading surface from file "
		    << '\"' << surface_import_dialog->get_filename() << '\"' << std::endl;
  try { surface_import_dialog->set_model( *this ); }
  catch( std::bad_alloc )
  {
    Gtk::MessageDialog d( *main_window,
                          "<b>Not enough storage available for the surface</b>"
						  "\nReduce number of samples and try again",
						  true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true );
    d.run();
    return;
  }
}

void	Controller::on_play_button_clicked() 
{  
  //step();
  toggle_iteration();
}

void	Controller::on_export_button_clicked()
{ 
  // ask the user for a file to open
  do{ if( image_export_dialog->run()==Gtk::RESPONSE_CANCEL ) return; }
  while( image_export_dialog->get_filename().empty() );

  std::clog<< "Controller::on_open_surface_button_clicked\t|"
		   << "writing image to file "
		   << '\"' << image_export_dialog->get_filename() << '\"'
		   << std::endl;

  try{ image_export_dialog->write( model.get_surface() ); }
  catch(std::bad_alloc)
  {
    Gtk::MessageDialog d( *main_window,
                          "<b>Not enough memory to draw the output image</b>"
						  "\nPlease reduce image size and try again",
						  true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true 
                        );
    d.run();
  }
}


void  Controller::on_solver_combo_changed( Gtk::ComboBoxText* combo )
{
  std::string active = combo->get_active_text();

  if( active == spring::SpringSolver<false,false>::class_name() )
  {
    std::shared_ptr< spring::SpringSolver<false,false> > solver( new spring::SpringSolver<false,false>( model.get_surface() ) );
    set_solver( solver );
  }

  if( active == spring::SpringSolver<true,false>::class_name() )
  {
    std::shared_ptr< spring::SpringSolver<true,false> > solver( new spring::SpringSolver<true,false>( model.get_surface() ) );
    set_solver( solver );
  }

  if( active == spring::SpringSolver<false,true>::class_name() )
  {
    std::shared_ptr< spring::SpringSolver<false,true> > solver( new spring::SpringSolver<false,true>( model.get_surface() ) );
    set_solver( solver );
  }

  if( active == spring::SpringSolver<true,true>::class_name() )
  {
    std::shared_ptr< spring::SpringSolver<true,true> > solver( new spring::SpringSolver<true,true>( model.get_surface() ) );
    set_solver( solver );
  }
	

  
  if( active == mds::EqualWeightSolver::class_name() )
  {
    std::shared_ptr< mds::EqualWeightSolver > solver( new mds::EqualWeightSolver( model.get_surface() ) );
    set_solver( solver );
  }
  
  if( active == mds::GeneralSolver::class_name() )
  {
    std::shared_ptr< mds::GeneralSolver > solver( new mds::GeneralSolver( model.get_surface() ) );
    set_solver( solver );
  }
    
}
