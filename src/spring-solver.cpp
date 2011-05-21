//           spring-solver.cpp
//  Sun Nov 15 22:03:55 2009
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

#include	"spring-solver.h"


/*
using namespace flat;

SpringSolver::SpringSolver( const std::shared_ptr< Surface >& surface, const time_t stepsize, const SpringForce& force )
: Solver( surface )
, m_velocities( surface->num_vertices(), velocity_t( 0 ) )
, m_force( force )
, m_stepsize( stepsize )
, m_energy_min(  std::numeric_limits< energy_type >::infinity() )
, m_energy_max( -std::numeric_limits< energy_type >::infinity() )
{ 
  set_surface( surface );
}



void	flat::SpringSolver::step_massless()
{ 
  std::clog <<"flat::SpringSolver::step"<<std::endl<<std::flush;

  using namespace boost::accumulators;
  accumulator_set< coord_t, features< tag::min, tag::max > > zacc;
  auto vel_it    = velocity_begin();
  auto frc_it    = m_force.begin();
  
  for( auto vertex_it = get_surface()->vertex_handles(); vertex_it.first != vertex_it.second; ++vertex_it.first )
  {
    location_t loc = vertex_it.first->location();

    zacc( loc[2] );
	
    *vel_it	+= (*frc_it++) * m_stepsize;
    
    loc	+= (*vel_it++) * m_stepsize;
    //loc[2] = 0;
    
    vertex_it.first->set_location( loc );
  }

  // update_surface height since required for drawing in HEIGHT_GRID_MODE
  get_surface()->set_min_max_z( min(zacc) , max(zacc) );
                               
  m_force.update( *this );
  
  std::clog <<"flat::SpringSolver::step"
			<<"\t|complete"<<std::endl<<std::flush;
}*/
