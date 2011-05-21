/***************************************************************************
 *            spring-solver-interface.h
 *
 *  Sat Nov 14 23:05:47 2009
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

# include "spring-solver.h"
# include "surface-interface.h"

namespace gl
{  
  using namespace flat;

  template< bool NoInertia, bool InSubspace >
  class SpringSolverDrawable	: public SurfaceDrawable
  {
	  typedef spring::SpringSolver< NoInertia, InSubspace > solver_type;
	  
      std::shared_ptr< solver_type > m_solver;

      bool    m_spring_visibility;
	  bool	m_force_visibility;
	  scale_t	m_force_vector_scaling;

      void	gl_draw_springs()	const;
      
	public:
		
	  static const std::string FORCE_VERTEX_MODE;

      SpringSolverDrawable( const std::shared_ptr< solver_type >& solver )	
      : gl::SurfaceDrawable( solver->get_surface() ), m_solver( solver ) 
      , m_spring_visibility( true ), m_force_visibility( true ), m_force_vector_scaling( 1. )
      { 
        append_vertex_modes( { FORCE_VERTEX_MODE } );
      }

      
	  virtual bool    gl_draw_vertices( const mode_t& )   const;
      virtual void	gl_draw_others()                    const;
      
	  const bool&		get_force_visibility()   const
      { return m_force_visibility; }

	  void	set_force_visibility( const bool visibility )
      { 
        if( visibility == get_force_visibility() ) return;
	    m_force_visibility = visibility;
		invalidate();
  	  }
		
	  const scale_t& get_force_vector_scale()	const
      { return m_force_vector_scaling; }
		
	  void    set_force_vector_scale( const scale_t scale )
      { 
        if( get_force_vector_scale() == scale ) return;
		m_force_vector_scaling = scale;
		invalidate();
	  }

      const bool& get_spring_visibility() const
      { return m_spring_visibility; }

      void set_spring_visibility( const bool visibility )
      {
        if( get_spring_visibility() == visibility ) return;
        m_spring_visibility = visibility;
        invalidate();
      }
  };
}  

namespace gtk
{
    template< bool NoInertia, bool InSubspace >
    class SpringSolverDrawableUI : public UI
    {
		typedef gl::SpringSolverDrawable< NoInertia, InSubspace >	drawable_type;
	  
        std::shared_ptr< drawable_type > m_drawable;
      
	    sigc::connection	forces_check_connection;
        sigc::connection	force_scale_spin_connection;
	    sigc::connection	spring_check_connection;
	  protected:
        void				on_force_vector_scale_spin_changed( Gtk::SpinButton* );

		static std::string get_builder_filename()   { return "spring-solver.ui"; }
		
	  public:
        
	    SpringSolverDrawableUI( const std::shared_ptr< drawable_type >& drawable )
		: UI( get_builder_filename(), "spring_solver_drawable_ui" ), m_drawable( drawable )
	  	{
		  // view option widgets  

		  Gtk::CheckButton* forces_check = 0;
		  get_builder()->get_widget( "forces_checkbutton", forces_check );
		  forces_check->set_active( drawable->get_force_visibility() );
		  forces_check_connection 
			  = forces_check->signal_toggled().connect( std::bind( [] ( Gtk::ToggleButton* toggle, drawable_type* drawable ) 
	                                                               { drawable->set_force_visibility( toggle->get_active() ); }
                                                                 , forces_check, drawable.get() ) );
		  
		  Gtk::SpinButton* force_scale_spin = 0;
		  get_builder()->get_widget( "force_vector_scale_spinbutton", force_scale_spin );
		  force_scale_spin->set_value( drawable->get_force_vector_scale() );
		  force_scale_spin_connection 
			  = force_scale_spin->signal_value_changed().connect( std::bind( [] ( Gtk::SpinButton* spin, drawable_type* drawable ) 
			                                                                 { drawable->set_force_vector_scale( spin->get_value() ); }
                                                                           , force_scale_spin, drawable.get() ) );
          Gtk::CheckButton* spring_check = 0;
		  get_builder()->get_widget( "spring_check", spring_check );
		  spring_check->set_active( drawable->get_spring_visibility() );
		  spring_check_connection 
			  = spring_check->signal_toggled().connect( std::bind( [] ( Gtk::ToggleButton* toggle, drawable_type* drawable ) 
	                                                               { drawable->set_spring_visibility( toggle->get_active() ); }
                                                                 , spring_check, drawable.get() ) );
		}
       
    };

	class StiffnessUIComponent
    {
        sigc::connection	coeff_spin_connection;
	  
	  public:

        virtual ~StiffnessUIComponent() { coeff_spin_connection.disconnect(); }
        
        template< typename ForceT >
        Gtk::Widget*    aquire_widget( gtk::UI& ui, ForceT& force )    
	  	{
  		  Gtk::SpinButton* coeff_spin = 0;

  		  ui.get_builder()->get_widget( "stiffness_spinbutton", coeff_spin );
    
  		  coeff_spin->set_value( force.get_stiffness() );
  
  		  coeff_spin_connection = coeff_spin->signal_value_changed().connect( 
		    [ coeff_spin, &force ] () { force.set_stiffness( coeff_spin->get_value() ); } );
            
          Gtk::Widget* widget;
          ui.get_builder()->get_widget( "stiffness_box", widget );
          return widget;
		}

        //virtual void on_stiffness_spin_changed( Gtk::SpinButton* ) = 0;
	};
  
    class DampeningUIComponent
    {
        sigc::connection	coeff_spin_connection;
      
      public:

        virtual ~DampeningUIComponent() { coeff_spin_connection.disconnect(); }
        
        template< typename ForceT >
        Gtk::Widget*    aquire_widget( gtk::UI& ui, ForceT& force )    
	  	{
  		  Gtk::SpinButton* coeff_spin = 0;

  		  ui.get_builder()->get_widget( "dampening_spinbutton", coeff_spin );
    
  		  coeff_spin->set_value( force.get_dampening() );
  
  		  coeff_spin_connection = coeff_spin->signal_value_changed().connect( 
            [ coeff_spin, &force ] () { force.set_dampening( coeff_spin->get_value() ); } );
            
          Gtk::Widget* widget;
          ui.get_builder()->get_widget( "dampening_box", widget );
          return widget;
		}

        //void on_dampening_spin_changed(Gtk::SpinButton*);
    };

    class GroundAttractorUIComponent
    {
        sigc::connection	coeff_spin_connection;
	  
      public:

        virtual ~GroundAttractorUIComponent() { coeff_spin_connection.disconnect(); }
        
        template< typename ForceT >
        Gtk::Widget*    aquire_widget( gtk::UI& ui, ForceT& force )    
	  	{
  		  Gtk::SpinButton* coeff_spin = 0;

  		  ui.get_builder()->get_widget( "ground_attractor_spinbutton", coeff_spin );
    
  		  coeff_spin->set_value( force.get_stiffness() );
  
  		  coeff_spin_connection = coeff_spin->signal_value_changed().connect( 
		    [coeff_spin, &force] () { force.set_ground_attraction( coeff_spin->get_value() ); } ); 

          Gtk::Widget* widget;
          ui.get_builder()->get_widget( "ground_attractor_box", widget );
          return widget;
		}

        //void	on_ground_attraction_coeff_changed(Gtk::SpinButton*);
    };
  
    class FrictionUIComponent
    {
        sigc::connection	coeff_spin_connection;
        sigc::connection	exponent_spin_connection;
	  
      public:

        ~FrictionUIComponent() 
        { coeff_spin_connection.disconnect();
          exponent_spin_connection.disconnect(); 
        }

        template< typename ForceT >
        Gtk::Widget*    aquire_widget( gtk::UI& ui, ForceT& force )    
        { 
  		  Gtk::SpinButton *coeff_spin = 0, *exponent_spin = 0;

          // connect exponent spin button
  		  ui.get_builder()->get_widget( "friction_spinbutton", coeff_spin );
			
  		  coeff_spin->set_value( force.get_friction() );

          coeff_spin_connection = coeff_spin->signal_value_changed().connect( 
			[ coeff_spin, &force ] () { force.set_friction( coeff_spin->get_value() ); } );

          // connect exponent spin button
  		  ui.get_builder()->get_widget( "friction_exponent_spinbutton", exponent_spin );

  		  exponent_spin->set_value( force.get_friction_exponent() );
			
		  exponent_spin_connection = exponent_spin->signal_value_changed().connect( 
			[ exponent_spin, &force ] () { force.set_friction_exponent( exponent_spin->get_value() ); } );
          
          Gtk::Widget* widget;
          ui.get_builder()->get_widget( "friction_box", widget );
          return widget;
        }


		//void	on_coeff_spin_changed(Gtk::SpinButton*);
        //void	on_exponent_spin_changed(Gtk::SpinButton*);
    };

    template<bool NoInteria, bool InSubspace>  
    struct SpringSolverUIComponents
    {
      //Widget* get_widget( gtk::UI& )    const;
    };

    template<> struct SpringSolverUIComponents< false, false >
    : public GroundAttractorUIComponent
    , public DampeningUIComponent
    , public FrictionUIComponent
    {    
      void  fill_box( Gtk::Box* box, gtk::UI& ui, spring::Force< 3 >& force )
      {
        box->add( *GroundAttractorUIComponent::aquire_widget( ui, force ) );
        box->add( *DampeningUIComponent::aquire_widget( ui, force ) );
        box->add( *FrictionUIComponent::aquire_widget( ui, force ) );
      }
    };

    template<> struct SpringSolverUIComponents< true, false >
    : public GroundAttractorUIComponent
    {    
      void  fill_box( Gtk::Box* box, gtk::UI& ui, spring::Acceleration< 3 >& force )
      {
        box->add( *GroundAttractorUIComponent::aquire_widget( ui, force ) );
      }
    };

    template<> struct SpringSolverUIComponents< false, true >
    : public DampeningUIComponent
    , public FrictionUIComponent
    {    
      void  fill_box( Gtk::Box* box, gtk::UI& ui, spring::Force< 2 >& force )
      {
        box->add( *DampeningUIComponent::aquire_widget( ui, force ) );
        box->add( *FrictionUIComponent::aquire_widget( ui, force ) );
      }
    };
  
    template<> struct SpringSolverUIComponents< true, true >
    {    
      void  fill_box( Gtk::Box*, gtk::UI&, spring::Acceleration< 2 >& )
      { }
    };

  
    // controller
    template< bool NoInertia, bool InSubspace >
	class SpringSolverUI 
    : public gtk::UI
    , public StiffnessUIComponent
    , public SpringSolverUIComponents< NoInertia, InSubspace >
    {
	  typedef spring::SpringSolver< NoInertia, InSubspace > solver_type;
	  
        std::shared_ptr<solver_type>	m_solver;

        // solver widgets
	    sigc::connection	forces_check_connection;
        sigc::connection	force_vector_scale_spin_connection;
        sigc::connection	stepsize_spin_connection;

      protected:
        
		static std::string get_builder_filename() { return "spring-solver.ui"; }

        // solver callbacks
        void	on_stepsize_spin_changed( Gtk::SpinButton* );

      public:
      
	    SpringSolverUI( const std::shared_ptr<solver_type>& );

        virtual	~SpringSolverUI();

        Gtk::Widget* get_widget()
        {
          Gtk::Box*    force_component_box;
          get_builder()->get_widget( "force_components_box", force_component_box );
          SpringSolverUIComponents< NoInertia, InSubspace >::fill_box( force_component_box, *this, m_solver->force() );
          return UI::get_widget();
        }

    }; // of SpringSolverUI
    
} // of gtk



////////////////////////////////////////////////////////////////////////////////
//IMPLEMENTATION================================================================

template< bool I, bool S > const std::string gl::SpringSolverDrawable<I,S>::FORCE_VERTEX_MODE = "force_t magnitude";

template< bool I, bool S > bool gl::SpringSolverDrawable<I,S>::gl_draw_vertices( const mode_t& mode )	const
{ 
  if( mode != FORCE_VERTEX_MODE ) return SurfaceDrawable::gl_draw_vertices( mode );
  
  const std::shared_ptr< Surface >& surface = m_solver->get_surface();
  
  std::vector<rgb_color_t>	colors( surface->num_vertices() );

  auto pmap = surface->get_property_map< Surface::vertex_location_t >();
  
  glPointSize( get_vertex_size() );

  if( mode == FORCE_VERTEX_MODE )
  {
    const force_t::value_type maxmag = std::max( m_solver->force().max_magnitude(), - m_solver->force().min_magnitude() );

	// draw position samples with color dependent on the forces
    typename spring::solver_traits<I,S>::force_type::const_iterator frc_it = m_solver->force().begin();
	  
    for( std::vector<rgb_color_t>::iterator col_it = colors.begin(); col_it != colors.end(); col_it++ )
      *col_it = rgb_color_t( length( *frc_it++ ) / maxmag );
  }
  
  gl_draw_scaled_vertices( colors.begin() );
}

template< bool I, bool S > void gl::SpringSolverDrawable<I,S>::gl_draw_springs()	const
{
  
  const std::shared_ptr< PointCloud > 	surface = m_solver->get_surface();
  const std::vector< spring::Spring >&	springs = m_solver->force().springs();
	
  rgb_color_t col( 1. );

  glPushMatrix();
  
  gl:Scale( get_global_scale() );
  
  glBegin( GL_LINES );

  for( auto spring = springs.begin(); spring != springs.end(); ++spring )
  { 
	const location_t& loc_a = surface->vertex( spring->first ).location(); 
	const location_t& loc_b = surface->vertex( spring->second ).location(); 
      
	const distance_t displacement = utk::length( loc_b - loc_a ) - spring->length;

    //if( !utk::close_to_zero( displacement ) )
    {
	  //std::cout << "d " << displacement << " md " << m_solver->max_displacement() << std::endl;
	  if( displacement < 0 )
	  { col.r() = col.b() = - displacement / m_solver->max_displacement();
	    col.g() = 0.f;
	  }
	  if( displacement > 0 )
	  { col.g() = col.b() =   displacement / m_solver->max_displacement();
	    col.r() = 0.f;
	  }
    }
        
	gl::Color( col );

	gl::Vertex( loc_a );
	gl::Vertex( loc_b );	
  }

  glEnd();
  glPopMatrix();
}
      
template< bool I, bool S >
void gl::SpringSolverDrawable<I,S>::gl_draw_others()	const
{	
  typedef typename spring::solver_traits<I,S>::location_handle solver_location_handle;

  if( m_spring_visibility ) gl_draw_springs();

  if( m_force_visibility )
  {
    const std::shared_ptr< Surface >& 	surface = m_solver->get_surface();

    glBegin(GL_LINES);
    gl::Color(1.f, 1.f, 1.f);
    for( PointCloud::vertex_descriptor i = 0; i < surface->num_vertices(); i++ )
    { 
	  const location_t source = surface->vertex(i).location() * get_global_scale();
      gl::Vertex( source );

	  flat::location_t target = source;
	  solver_location_handle( target ) += ( m_solver->force()[i] * solver_location_handle( get_global_scale() ) ) * get_force_vector_scale();
	  gl::Vertex( target );	
    }
    glEnd();
  }
}    




template< bool I, bool S > gtk::SpringSolverUI<I,S>::SpringSolverUI( const std::shared_ptr< spring::SpringSolver<I,S> >& solver )
: UI( get_builder_filename(), "spring_solver_model_ui" ), m_solver( solver )
{
  // solver property widgets
  Gtk::SpinButton* stepsize_spin = 0;
  get_builder()->get_widget( "stepsize_spinbutton", stepsize_spin );
  
  stepsize_spin->set_value( solver->integrator().get_stepsize() );
  
  stepsize_spin->signal_value_changed().connect( sigc::bind( sigc::mem_fun( *this, &SpringSolverUI::on_stepsize_spin_changed )
                                                           , stepsize_spin ) ); 
  
  // force_t generator widgets
  const typename spring::solver_traits<I,S>::force_type& force = m_solver->force();



  Gtk::SpinButton* kd_spin = 0;

}

template< bool I, bool S > gtk::SpringSolverUI<I,S>::~SpringSolverUI()
{
  // disconnect signals
  stepsize_spin_connection.disconnect();
  forces_check_connection.disconnect();
  force_vector_scale_spin_connection.disconnect();
}


template< bool I, bool S > void gtk::SpringSolverUI<I,S>::on_stepsize_spin_changed(Gtk::SpinButton* stepsize_spin)
{ 
  m_solver->integrator().set_stepsize( stepsize_spin->get_value() );
  m_solver->update_force();
  std::clog << "gtk::Controller::on_stepsize_spin_changed"
		    << "\t|step size " << m_solver->integrator().get_stepsize()
            << std::endl << std::flush;
}
