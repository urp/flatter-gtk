
/***************************************************************************
 *            controller.h
 *
 *  Sat Aug  8 11:54:37 2009
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

# include "model.h"

# include "gl-view.h"

# include "surface-import-dialog.h"
# include "image-export-dialog.h"

# include "interface.h"
# include "surface-interface.h"

# include "quad-surface.h"
# include "quad-surface-interface.h"

# include "spring-solver.h"
# include "spring-solver-interface.h"

# include "mds-solver.h"
# include "mds-solver-interface.h"

namespace gtk
{
	
  class Controller : public boost::signals::trackable		
  {
	public:

	  typedef std::pair< std::shared_ptr< gtk::UI >, gtk::UI::widget_name_t > ui_slot_pair;
		  
	  typedef std::pair< std::list< ui_slot_pair >, std::list< std::shared_ptr< gl::Drawable > > >   ui_modules_t;

	  static const gtk::UI::widget_name_t SURFACE_MODEL_SLOT;
  	  static const gtk::UI::widget_name_t SURFACE_VIEW_SLOT;
  	  static const gtk::UI::widget_name_t SOLVER_MODEL_SLOT;
  	  static const gtk::UI::widget_name_t SOLVER_VIEW_SLOT;    
		  
	private:

      const Glib::RefPtr< Gtk::Builder >& m_builder;
      
  	  flat::Model	model;

      std::shared_ptr< gl::SharedSurfaceDrawable > shared_surface_drawable;
      std::shared_ptr< gtk::SurfaceDrawableUI > shared_surface_drawable_ui;
      
	  ui_modules_t	m_surface_ui;
	  ui_modules_t	m_solver_ui;
      
  	  // derived widgets
  	  gtk::GLView*			m_gl_view;
  	  ImageExportDialog*	image_export_dialog;
  	  // main_window
  	  Gtk::Window*			main_window;
	  // viewport container
	  Gtk::Paned*			main_paned;

  	  // toolbar widgets
  	  Gtk::ImageMenuItem*	new_button;
  	  Gtk::ImageMenuItem*	open_button;
   	  Gtk::ToolButton*		play_button; 
  	  Gtk::ToolButton* 		export_button;

      Gtk::ComboBoxText*    m_solver_combo;
      
      sigc::connection      m_play_connection;
      
  	protected:
			// connects/disconnects the gtk idle signal to Contoller::step()
			// returns true if the connection was blocked
			// returns false if connection was established
  	  void  toggle_iteration();

  	  void	on_new_menu_item_activate();
      void	on_open_menu_item_activate();
  	  void	on_play_button_clicked();
  	  void	on_export_button_clicked();        

      void  on_solver_combo_changed( Gtk::ComboBoxText* );
      
	  void	attach_ui( const std::pair< std::shared_ptr< gtk::UI >, gtk::UI::widget_name_t >& ui )
	  {
        std::clog << "flat::Controller::attach_ui\t|" << "slot " << ui.second << std::endl;
        Gtk::Container* slot = 0;
        get_builder()->get_widget( ui.second, slot );
        
        Gtk::Widget* widget = ui.first->get_widget();
        
        ui.first->connect_remover( boost::bind( &Controller::remove_ui, this, slot, widget ) );
        
		slot->add( *widget );
	  }

      void remove_ui( Gtk::Container* slot, Gtk::Widget* widget )    
      { 
        slot->remove( *widget ); 
        shared_surface_drawable_ui->update_mode_combos();
      } 
      
	  void	set_view( gtk::GLView* view )							
	  { 
		assert( !m_gl_view || m_gl_view == view ); // TODO: remove old view
		assert( view != 0 );
		m_gl_view = view;
		main_paned->add2( *m_gl_view );
		m_gl_view->show();
	  }

      template< class T >
      ui_modules_t add_ui( const std::shared_ptr< T >& obj )
	  {
        ui_modules_t ui( get_ui_modules( obj ) );

        for( auto it = ui.second.begin(); it != ui.second.end(); it++ )
        {
		  // add surface drawables to shared drawable
          if( auto surf_drw = std::dynamic_pointer_cast< gl::SurfaceDrawable >( *it ) )
            shared_surface_drawable->add_drawable( surf_drw.get() );
          else 
            m_gl_view->add_drawable( it->get() ); // add other drawables to gl view
        }

	  	shared_surface_drawable_ui->update_mode_combos();
        
        std::for_each( ui.first.begin(), ui.first.end(), boost::bind( &Controller::attach_ui, this, _1 ) );

		m_gl_view->invalidate();
		
		return ui;
	  }

      bool	step()
	  { 
		model.step();
		
		for_each( m_surface_ui.first.begin(), m_surface_ui.first.end() 
		        , [] ( ui_modules_t::first_type::const_reference ui ) { ui.first->update(); } );
		
		m_gl_view->invalidate();
		
        return true; // don't block idle connection
	  }

	  
  	public:
	  
	  Controller(const Glib::RefPtr<Gtk::Builder>& builder );

	  ~Controller() { delete m_gl_view;	
                      m_surface_ui.first.clear();
                      m_surface_ui.second.clear();
                      m_solver_ui.first.clear();
                      m_solver_ui.second.clear();
                    }

      const Glib::RefPtr<Gtk::Builder>& get_builder()   const { return m_builder; }
      
	  template<class SurfaceT,class SolverT>
	  void	set_model( const std::shared_ptr< SurfaceT >& surface
			         , const std::shared_ptr< SolverT >&  solver )
	  { 
		// set up container for surface drawables
		shared_surface_drawable.reset( new gl::SharedSurfaceDrawable( surface ) );
		// set up surface_ui
		shared_surface_drawable_ui.reset( new gtk::SurfaceDrawableUI( shared_surface_drawable ) );

		m_gl_view->add_drawable( shared_surface_drawable.get() );

        Gtk::Container* slot = 0;
        get_builder()->get_widget( SURFACE_MODEL_SLOT, slot );

        // add new widgets
        attach_ui( std::make_pair( shared_surface_drawable_ui, SURFACE_VIEW_SLOT ) );

        m_surface_ui = add_ui( surface );

        model.set_surface( surface );
        
        set_solver( solver );
	  }
      
	  template< class SolverT >
	  void	set_solver( const std::shared_ptr< SolverT >&  solver )
      { 
		model.set_solver( solver );
        m_solver_ui  = add_ui( solver );
        m_solver_combo->set_active_text( SolverT::class_name() );
	  }  
  };

  inline Controller::ui_modules_t  get_ui_modules( const std::shared_ptr< flat::Surface >& surface )
  {
    std::shared_ptr< gl::SurfaceDrawable > drawable( new gl::SurfaceDrawable( surface ) );
    return { { { std::shared_ptr< gtk::UI >( new gtk::SurfaceUI( surface ) ), Controller::SURFACE_MODEL_SLOT } }
		   , { drawable } 
		   };
  } 

  inline Controller::ui_modules_t  get_ui_modules( const std::shared_ptr< flat::QuadSurface >& surface )
  {
    std::shared_ptr< gl::QuadSurfaceDrawable > drawable( new gl::QuadSurfaceDrawable( surface ) );
    return { { { std::shared_ptr< gtk::UI >( new gtk::SurfaceUI( surface ) ), Controller::SURFACE_MODEL_SLOT } }
		   , { drawable } 
           };    
  } 

  template< bool NoInertia, bool InSubspace >
  inline Controller::ui_modules_t  get_ui_modules( const std::shared_ptr< spring::SpringSolver< NoInertia, InSubspace > >& solver )
  {
    std::shared_ptr< gl::SpringSolverDrawable< NoInertia, InSubspace > > drawable( new gl::SpringSolverDrawable< NoInertia, InSubspace >( solver ) );
    return { { { std::shared_ptr< gtk::UI >( new gtk::SpringSolverDrawableUI< NoInertia, InSubspace >( drawable ) ), Controller::SOLVER_VIEW_SLOT  }
             , { std::shared_ptr< gtk::UI >( new gtk::SpringSolverUI< NoInertia, InSubspace >( solver ) )          , Controller::SOLVER_MODEL_SLOT } }
           , { drawable } 
		   };    
  } 

  inline Controller::ui_modules_t  get_ui_modules( const std::shared_ptr< mds::Solver >& solver )
  {
    std::shared_ptr< gl::MDSSolverDrawable > drawable( new gl::MDSSolverDrawable( solver ) );
    return { { /*std::shared_ptr< gtk::UI >( new gtk::MDSSolverDrawableUI( drawable ) )
             , std::shared_ptr< gtk::UI >( new gtk::SpringSolverUI( solver ) )*/
             }
           , { drawable } 
		   };    
  }  
  
  inline Controller::ui_modules_t  get_ui_modules( const std::shared_ptr< mds::EqualWeightSolver >& solver )
  { return get_ui_modules( std::shared_ptr< mds::Solver >( solver ) ); }
  
  inline Controller::ui_modules_t  get_ui_modules( const std::shared_ptr< mds::GeneralSolver >& solver )
  { return get_ui_modules( std::shared_ptr< mds::Solver >( solver ) ); }

}

