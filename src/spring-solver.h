/***************************************************************************
 *            spring-solver.h
 *
 *  Sun Nov 15 22:03:55 2009
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

# include <cassert>
# include <fstream>
# include <sstream>

# include "common.h"

# include "surface.h"
# include "solver.h"
# include "spring-force.h"
# include "spring-integrator.h"

# include <boost/accumulators/accumulators.hpp>
# include <boost/accumulators/statistics/max.hpp>

# pragma GCC visibility push(default)

namespace spring
{
  
  template< bool NoInertia, bool InSubspace >
  class SpringSolver : public Solver
  {
	public:

      typedef typename solver_traits< NoInertia, InSubspace >::force_type		force_type;
      typedef typename solver_traits< NoInertia, InSubspace >::integrator_type	integrator_type;

	private:
	  
	  integrator_type	m_integrator;
	  force_type        m_force;

	  mutable distance_t m_max_displacement;
		  
	  static void	initial_transform( const std::shared_ptr< Surface >& surface )
	  {
		if( InSubspace )
		{ auto vertices = surface->vertex_handles();
		  std::for_each( vertices.first, vertices.second
		               , []( Surface::vertex_handle& v ) { auto loc = v.location(); loc[2] = 0; v.set_location( loc ); }
		               );
		}
	  }
		  
	  void	comp_max_displacement()	const
	  { 
  	    using namespace boost::accumulators;
	    accumulator_set< coord_t, features< tag::max > > acc;
    	const std::vector< Surface::vertex_pair > pairs( std::move( get_surface()->neighbors() ) );
      
        std::for_each( pairs.begin(), pairs.end()
                 , [ this, &acc ]( const Surface::vertex_pair& pair )
                   { acc( std::fabs( this->get_surface()->initial_distances( pair.first, pair.second ) 
                                   - this->get_surface()->distance( pair.first, pair.second ) ) );
                   }
                 );
		    
	    m_max_displacement = max( acc );

	    std::clog << "flat::Solver::comp_max_displacement\t|" << m_max_displacement << std::endl;
	  }
		  
    public:  

      static std::string    class_name()    { return "SpringSolver" + solver_traits< NoInertia, InSubspace >::name(); }
      
	  SpringSolver( const std::shared_ptr< Surface >& surface )
      : Solver( surface )
      , m_integrator( surface )
	  , m_force( surface )
	  {	
		surface->initial_distances.compute_distances( surface, Surface::distance_function::NEIGHBORS ); 
		initial_transform( surface );
		update_force();
        std::clog << "spring::SpringSolver::" << class_name() << std::endl;
	  }

		
	  virtual void	prepare_step()
      { 
		set_max_displacement();
		Solver::prepare_step();
	  }

	  void  update_force() { m_integrator.update_force( m_force ); }
	  
	  void	step()	{ m_integrator( m_force ); }

	  void set_surface( const std::shared_ptr< Surface >& surface )
	  { 
 	    Solver::set_surface( surface );
        surface->initial_distances.compute_distances( surface, Surface::distance_function::NEIGHBORS );
		initial_transform( surface );
	    m_integrator.set_surface( Solver::get_surface() );
		m_force.set_surface( Solver::get_surface() );
		update_force();
	  }


      const force_type& force() const	{ return m_force; }

      force_type&		force()       	{ return m_force; }

	  const integrator_type& integrator() const	{ return m_integrator; }

      integrator_type&		 integrator()       { return m_integrator; }

	  void  set_max_displacement( const distance_t displacement = -std::numeric_limits< distance_t >::infinity() ) const
	  { m_max_displacement = displacement; }
		  
	  const coord_t&	max_displacement() const							
      { if( m_max_displacement == - std::numeric_limits< distance_t >::infinity() )
      	  comp_max_displacement();
        return m_max_displacement;
	  }
  };

}

# pragma GCC visibility pop
