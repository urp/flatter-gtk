/***************************************************************************
 *            spring-force_t.h
 *
 *  Thu Aug  6 00:26:04 2009
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

# include "common.h"
# include "surface.h"

# include <cassert>
# include <fstream>
# include <sstream>

# include <boost/accumulators/accumulators.hpp>
# include <boost/accumulators/statistics/min.hpp>
# include <boost/accumulators/statistics/max.hpp>

#pragma GCC visibility push(default)

namespace spring
{
  using namespace flat;

  using flat::time_t;

  typedef double coeff_type;
  
  struct Spring : public Surface::vertex_pair
  {
    //Surface::vertex_descriptor 	a, b;
    distance_t	length;

    Spring() = default;

    Spring& operator= ( const Spring& other ) = default;
    
    
	Spring( const Surface::vertex_pair& pair 
          , const distance_t distance )
	: Surface::vertex_pair( pair ), length( distance )
	{	}
		  
  };


  inline std::vector< Spring > make_springs( const std::shared_ptr< Surface >& surface )
  {
    // TODO - optimize
    std::vector< Surface::vertex_pair > pairs( std::move( surface->neighbors() ) );
    std::vector< Spring >      springs;
    springs.reserve( pairs.size() );
    std::for_each( pairs.begin(), pairs.end()
                 , [ &springs, &surface ] ( const Surface::vertex_pair& pair )
                   { springs.push_back( Spring( pair, surface->initial_distances( pair.first, pair.second ) ) ); }
                 );
    return springs;
  }

  
  template< size_t Dim >
  struct dimension_traits
  {
	static const size_t dim = Dim ;
	
    typedef utk::veca< location_t::value_type, Dim > location_type;
    typedef utk::vecn< location_t::value_type, Dim > location_handle;

    typedef utk::veca< velocity_t::value_type, Dim > velocity_type;
    typedef utk::vecn< velocity_t::value_type, Dim > velocity_handle;

    typedef utk::veca< force_t::value_type, Dim > force_type;
    typedef utk::vecn< force_t::value_type, Dim > force_handle;
  };

  // forward
  template< size_t Dim >  struct Force;
  template< size_t Dim >  struct Acceleration;
  
  class EulerIntegrator
  {
      std::shared_ptr< Surface >  m_surface;
	  
      time_t    m_stepsize;

    public:

	  EulerIntegrator( const std::shared_ptr< Surface >& surface ) 
      : m_surface( surface ), m_stepsize( 1e-4 )	{	}
      
	  const time_t	get_stepsize()	const	{ return m_stepsize; }
    
	  void set_stepsize( const time_t stepsize )
      { 
        assert( stepsize > 0 );
	    m_stepsize = stepsize;
	  }

	  const std::shared_ptr< Surface >&	get_surface()	const	{ return m_surface; }
	  
      virtual void    set_surface( std::shared_ptr< Surface > surface )
      { m_surface = surface; }

  };
  
  template< size_t Dim >  
  class EulerForceIntegrator    : public EulerIntegrator
  {
	  typedef typename dimension_traits< Dim >::location_handle	location_handle;
	  typedef typename dimension_traits< Dim >::velocity_type	velocity_type;
	  typedef typename dimension_traits< Dim >::velocity_handle	velocity_handle;
    
      std::vector< velocity_type >	m_velocities; // the velocities associated with each sample

	  mutable std::pair< energy_type, energy_type > m_energy;

	public:

	  EulerForceIntegrator( const std::shared_ptr< Surface >& surface )
	  : EulerIntegrator( surface )
      , m_velocities( surface->num_vertices(), velocity_type(0.) )
	  , m_energy{ { std::numeric_limits< energy_type >::infinity(), - std::numeric_limits< energy_type >::infinity() } }
	  {	}

	  const energy_type&	min_energy()	const;
      const energy_type&	max_energy()	const;
	 
      void comp_energy_min_max()	const;

	  void update_force( Force< Dim >& force )
	  { force.update( m_velocities ); }
	  
      void operator() ( Force< Dim >& );

      void set_surface( std::shared_ptr< Surface > surface )
      {
        EulerIntegrator::set_surface( surface );
		m_velocities.clear();
		m_velocities.resize( surface->num_vertices(), velocity_type( 0 ) );
      }

  };

  template< size_t Dim >  
  struct EulerAccelerationIntegrator : public EulerIntegrator
  {
    typedef typename dimension_traits< Dim >::location_handle	location_handle;
	
	EulerAccelerationIntegrator( const std::shared_ptr< Surface >& surface )
	: EulerIntegrator( surface )	{	}
	
	void update_force( Acceleration< Dim >& acceleration )
	{ acceleration.update(); }

    void operator() ( Acceleration< Dim >& );
  };



  template< size_t Dim > class SpringForce
  {
      typedef typename dimension_traits< Dim >::location_type   location_type;
      typedef typename dimension_traits< Dim >::location_handle location_handle;
      typedef typename dimension_traits< Dim >::velocity_type 	velocity_type;
      typedef typename dimension_traits< Dim >::force_type    	force_type;

      coeff_type stiffness;
      coeff_type dampening;

    public:
      
      SpringForce() throw(std::bad_alloc) : stiffness( 1. ), dampening( 10 )  {   }

      std::pair< force_type, force_type > operator() ( const Surface::vertex_handle_pair& pair, const distance_t distance )
      {
	    const location_type dr  = location_handle( pair.first.location() ) - location_handle( pair.second.location() ); 
	    const distance_t    dR  = utk::length( dr );
        const distance_t    displace = distance - dR;
        const force_type    reset    = dr * ( displace * stiffness / dR );	

        return { reset, -reset };
      }
        
      const coeff_type&	get_stiffness()	const	{ return stiffness; }	

      void	set_stiffness( const coeff_type& v )	{ stiffness = v; }
  };

  
  template< size_t Dim > class NoSpringForce
  { 
	typedef typename dimension_traits< Dim >::force_type        force_type;
	std::pair< force_type, force_type > operator() ( const Surface::vertex_handle_pair& pair, const distance_t distance )	
	{ return { force_type( 0 ), force_type( 0 ) }; } 
  };

  
  template< size_t Dim > class Dampening
  {
      typedef typename dimension_traits< Dim >::location_type     location_type;
      typedef typename dimension_traits< Dim >::location_handle   location_handle;
      typedef typename dimension_traits< Dim >::velocity_type     velocity_type;
      typedef typename dimension_traits< Dim >::velocity_handle   velocity_handle;
      typedef typename dimension_traits< Dim >::force_type        force_type;

      coeff_type    dampening;

    public:

      Dampening() : dampening( .5 ) {    }
      
      std::pair< force_type, force_type > 
        operator() ( const Surface::vertex_handle_pair& pair, const distance_t distance, std::vector< velocity_type > velocities )
      {
	    const location_type dr  = location_handle( pair.first.location() ) - location_handle( pair.second.location() ); 
        const velocity_type dv  = velocity_handle( velocities[ pair.second ] ) - velocity_handle( velocities[ pair.first ] );
	    const distance_t    dR  = utk::length(dr);
        const force_type    damp    =  dr * ( dampening * dot( dr, dv ) / utk::sqr(dR) );	

        return { damp, -damp };
      }
    
      const coeff_type&	get_dampening()	const	{ return dampening; }	
      void	set_dampening( const coeff_type v )	{ dampening = v; }
  };

  
  template< size_t Dim > struct NoDampening
  {
      typedef typename dimension_traits< Dim >::velocity_type     velocity_type;
      typedef typename dimension_traits< Dim >::force_type        force_type;

      std::pair< force_type, force_type > operator() ( const Surface::vertex_handle_pair& pair, const distance_t distance, const std::vector< velocity_type >& velocities )
      { return { force_type( 0 ), force_type( 0 ) };   }
  };


  
  template< size_t Dim > class GroundAttractor
  {
	  static_assert( Dim == 3, "only available in 3d." );

      typedef typename dimension_traits< Dim >::force_type        force_type;
	
      coeff_type attraction;

    public:

	  GroundAttractor() : attraction( 1. )	{	}
      
      force_type operator() ( const Surface::vertex_handle& vertex )  
      { return force_type( 0., 0. , - attraction * vertex.location()[ Dim-1 ] ); }

      const coeff_type&	get_ground_attraction()	const	{ return attraction; }
    
      void set_ground_attraction(const coeff_type& v)	{ attraction = v; }
  };

  template< size_t Dim > struct NoGroundAttractor
  {
      typedef typename dimension_traits< Dim >::force_type        force_type;

      force_type operator() ( const Surface::vertex_handle& particle )          
      { return force_type( 0 ); }
  };

  
  template< size_t Dim >  class Friction
  {
      typedef typename dimension_traits< Dim >::location_type     location_type;
      typedef typename dimension_traits< Dim >::location_handle   location_handle;
      typedef typename dimension_traits< Dim >::velocity_type     velocity_type;
      typedef typename dimension_traits< Dim >::velocity_handle   velocity_handle;
      typedef typename dimension_traits< Dim >::force_type        force_type;
    
      coeff_type	friction;
      size_t		friction_exponent;
	
    public:
      
      Friction() : friction( .3 ), friction_exponent( 1 )   {   }
    
      const coeff_type&	get_friction()			const	{ return friction; }	
      const size_t&	    get_friction_exponent()	const	{ return friction_exponent; }	

      void	set_friction( const coeff_type v )		    	{ friction = v; }
      void	set_friction_exponent( const size_t v )		{ friction_exponent = v; }

      //TODO: check if vecn doesn't copy
      
      force_type operator() ( const Surface::vertex_handle& particle, const std::vector< velocity_type >& velocities )
      { 
	    const velocity_type velocity( velocities[ particle ] );
	    //const auto magnitude = velocity.length();
        
        return velocity * ( - friction /* magnitude * utk::ipow( magnitude, friction_exponent )*/  );
      }
  };

  template< size_t Dim > struct NoFriction
  {
      typedef typename dimension_traits< Dim >::velocity_type     velocity_type;
      typedef typename dimension_traits< Dim >::force_type        force_type;

      force_type operator() ( const Surface::vertex_handle& particle, const std::vector< velocity_type >& velocities ) 
      { return force_type( 0 );   }
  };

  template< bool NoInertia, bool InSubspace > struct solver_traits
  {  };

  template< > struct solver_traits< false, false > : public dimension_traits< 3 >
  {  
    //static const size_t dim = 3;
    typedef SpringForce< dim >  	spring_component;
    typedef Dampening< dim >    	dampening_component;
    typedef GroundAttractor< dim >  ground_component;
    typedef Friction< dim >     	friction_component;

	typedef Force< dim >				 force_type;
    typedef EulerForceIntegrator< dim >  integrator_type; 

    static std::string  name()  { return "(inertial, embedded)"; }
  };

  template< > struct solver_traits< true, false > : public dimension_traits< 3 >
  {  
    //static const size_t dim = 3;
    typedef SpringForce< dim >  	spring_component;
    typedef NoDampening< dim >  	dampening_component;
    typedef GroundAttractor< dim >	ground_component;
    typedef NoFriction< dim >   	friction_component;

	typedef Acceleration< dim >					force_type;
    typedef EulerAccelerationIntegrator< dim >  integrator_type;

    static std::string  name()  { return "(massless, embedded)"; }
  };
  
  template< > struct solver_traits< false, true > : public dimension_traits< 2 >
  {  
    //static const size_t dim = 2;
    typedef SpringForce< dim >		 spring_component;
    typedef Dampening< dim >  		 dampening_component;
    typedef NoGroundAttractor< dim > ground_component;
    typedef Friction< dim >    		 friction_component;

	typedef Force< dim >				 force_type;
    typedef EulerForceIntegrator< dim >  integrator_type; 

    static std::string  name()  { return "(inertial, subspace)"; }
  };

  template< > struct solver_traits< true, true > : public dimension_traits< 2 >
  {  
    //static const size_t dim = 2;
    typedef SpringForce< dim > 		  spring_component;
    typedef NoDampening< dim >     	  dampening_component;
    typedef NoGroundAttractor< dim >  ground_component;
    typedef NoFriction< dim >     	  friction_component;

	typedef Acceleration< dim >						force_type;
    typedef EulerAccelerationIntegrator< dim >  integrator_type; 

    static std::string  name()  { return "(massless, subspace)"; }
  };
  
  
  
  template< size_t Dim >
  class Storage : public std::vector< typename dimension_traits< Dim >::force_type >
  {  
	public:
      
      typedef typename dimension_traits< Dim >::force_type      force_type;
      typedef typename dimension_traits< Dim >::force_handle    force_handle;

	protected:

	  std::shared_ptr< Surface >  m_surface;

      std::vector< Spring >		  m_springs;	  // the springs attached to the surface
	  
      // biggest and smallest force magnitudes 
	  mutable typename force_type::value_type	force_min;
	  mutable typename force_type::value_type	force_max;

	public:
      
      Storage( std::shared_ptr< Surface > surface )
      : std::vector< force_type >( surface->num_vertices(), force_type( 0 ) )
	  , m_surface( surface )
	  , m_springs( make_springs( surface ) )
      , force_min( std::numeric_limits< typename force_type::value_type >::infinity() )
	  ,	force_max(-std::numeric_limits< typename force_type::value_type >::infinity() ) {   }


      void set_surface( std::shared_ptr< Surface > surface )
	  {
        m_surface = surface;
        m_springs = make_springs( surface );
        resize( surface->num_vertices() );
      }

      const std::vector< Spring >&	springs()	{ return m_springs; }
	  
	  const typename force_type::value_type&	min_magnitude()	const	{ return force_min;	}
	  const typename force_type::value_type&	max_magnitude()	const	{ return force_max;	}

      void resize( const size_t size )
      { 
		std::vector< force_type >::clear(); 
		std::vector< force_type >::resize( size, force_type( 0. ) ); 
	  }
  };


  template< size_t Dim >
  struct Acceleration
  : public Storage< Dim >
  , public solver_traits< true, (Dim < 3) >::spring_component
  , public solver_traits< true, (Dim < 3) >::ground_component
  {
      typedef typename dimension_traits< Dim >::force_type      force_type;
      typedef typename dimension_traits< Dim >::velocity_type   velocity_type;

	  Acceleration( const std::shared_ptr< Surface >& surface ) : Storage< Dim >( surface )	{	}
	
	  force_type update()
      {
		std::fill( Storage< Dim >::begin(), Storage< Dim >::end(), force_type( 0 ) );
        // spring reset forces
        for( auto spring = Storage< Dim >::m_springs.begin(); spring != Storage< Dim >::m_springs.end(); ++spring )
        {
          auto force = solver_traits< true, (Dim < 3) >::spring_component::operator() 
			( Surface::make_vertex_handle_pair( *spring, *Storage< Dim >::m_surface ), spring->length );
          Storage< Dim >::operator[] ( spring->first  ) += force.first; 
          Storage< Dim >::operator[] ( spring->second ) += force.second;
        }

        for( auto its = Storage< Dim >::m_surface->vertex_handles(); its.first != its.second; ++its.first )
        {
          force_type force = solver_traits< true, (Dim < 3) >::ground_component::operator() ( *its.first );
          Storage< Dim >::operator[] ( *its.first ) += force;
        }  
      }
  };
  
  template< size_t Dim >
  struct Force 
  : public Storage< Dim >
  , public solver_traits< false, (Dim < 3) >::spring_component
  , public solver_traits< false, (Dim < 3) >::ground_component
  , public solver_traits< false, (Dim < 3) >::dampening_component
  , public solver_traits< false, (Dim < 3) >::friction_component
  {
      typedef typename dimension_traits< Dim >::force_type      force_type;
      typedef typename dimension_traits< Dim >::velocity_type   velocity_type;

	  Force( const std::shared_ptr< Surface >& surface ) : Storage< Dim >( surface )	{	}


   	  force_type update( const std::vector< velocity_type >& velocities )
      {
		std::fill( Storage< Dim >::begin(), Storage< Dim >::end(), force_type( 0 ) );
        // spring resetting forces
        for( auto spring = Storage< Dim >::m_springs.begin(); spring != Storage< Dim >::m_springs.end(); ++spring )
        {
          std::pair<force_type,force_type> force = solver_traits< false, (Dim < 3) >::spring_component::operator() 
						( Surface::make_vertex_handle_pair( *spring, *Storage< Dim >::m_surface ), spring->length );
          Storage< Dim >::operator[] ( spring->first  ) += force.first; 
          Storage< Dim >::operator[] ( spring->second ) += force.second;

          force = solver_traits< false, (Dim < 3) >::dampening_component::operator() 
		  			( Surface::make_vertex_handle_pair( *spring, *Storage< Dim >::m_surface ), spring->length, velocities );
          Storage< Dim >::operator[] ( spring->first  ) += force.first; 
          Storage< Dim >::operator[] ( spring->second ) += force.second;
        }

        for( auto its = Storage< Dim >::m_surface->vertex_handles(); its.first != its.second; ++its.first )
        {
          force_type force = solver_traits< false, (Dim < 3) >::ground_component::operator() ( *its.first );
          Storage< Dim >::operator[] ( *its.first ) += force;

          force = solver_traits< false, (Dim < 3) >::friction_component::operator() ( *its.first, velocities );
          Storage< Dim >::operator[] ( *its.first ) += force;
      }  
    }
  };

  

  //////////////////////////////////////////////////////////////////////////////
  //IMPLEMENTATION==============================================================
  
  template< size_t Dim >
  void	EulerForceIntegrator< Dim >::comp_energy_min_max()	const
  { 
    using namespace boost::accumulators;
    accumulator_set< energy_type, features< tag::min, tag::max > > acc;
    std::vector< velocity_t >::const_iterator it = m_velocities.begin();
    while( it != m_velocities.end() ) acc( sqr(*it++).sum() );

    m_energy = { boost::accumulators::min(acc), boost::accumulators::max(acc) };
  }

  template< size_t Dim >
  const energy_type&	EulerForceIntegrator<Dim>::max_energy() const	
  { 
    if( m_energy.first == -std::numeric_limits< energy_type >::infinity() )
      comp_energy_min_max();
    return m_energy.first;
  }

  template< size_t Dim >
  const energy_type&	EulerForceIntegrator<Dim>::min_energy() const	
  { 
    if( m_energy.second == std::numeric_limits< energy_type >::infinity() )
      comp_energy_min_max();
    return m_energy.second;
  }

  template< size_t Dim >  
  void EulerForceIntegrator<Dim>::operator() ( Force< Dim >& force )
  {
    //std::clog <<"spring::EulerForceIntegrator::step"<<std::endl<<std::flush;

    m_energy = { std::numeric_limits< energy_type >::infinity(), - std::numeric_limits< energy_type >::infinity() };  
    
    auto vel_it    = m_velocities.begin();
    auto frc_it    = force.begin();
  
    for( auto vertex_it = get_surface()->vertex_handles(); vertex_it.first != vertex_it.second; ++vertex_it.first )
    {

      location_t location( vertex_it.first->location() );
	
      *vel_it	+= (*frc_it++) * EulerIntegrator::get_stepsize();
    
      location_handle( location ) += (*vel_it++) * EulerIntegrator::get_stepsize();
    
      vertex_it.first->set_location( location );
    }
                               
    update_force( force );
  
    //std::clog << "spring::EulerForceIntegrator::step"
	//  		  << "\t|complete" << std::endl << std::flush;
  }


  template< size_t Dim >  
  void EulerAccelerationIntegrator<Dim>::operator() ( Acceleration< Dim >& force )
  {
    //std::clog << "spring::EulerAccelerationIntegrator::step" << std::endl << std::flush;

    auto frc_it = force.begin();
  
    for( auto vertex_it = get_surface()->vertex_handles(); vertex_it.first != vertex_it.second; ++vertex_it.first )
    {
      location_t location( vertex_it.first->location() );
          
      location_handle( location ) += (*frc_it++) * EulerIntegrator::get_stepsize();
   
      vertex_it.first->set_location( location );
    }

    update_force( force );
  
    //std::clog << "spring::EulerAccelerationIntegrator::step"
	//	      << "\t|complete" << std::endl << std::flush;
  }
  
};

#pragma GCC visibility pop

