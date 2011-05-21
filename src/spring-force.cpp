//           force_t.cpp
//  Thu Aug  6 00:26:04 2009
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

#include	"spring-force.h"

/*
#include 	"spring-solver.h"

# include <boost/accumulators/accumulators.hpp>
# include <boost/accumulators/statistics/min.hpp>
# include <boost/accumulators/statistics/max.hpp>

void flat::SpringForce::update(const SpringSolver& solver)
{ 
  assert( solver.get_surface() );
  const Surface& surface = *solver.get_surface();
  const std::vector< Spring >& springs = solver.get_springs();
	
  using namespace boost::accumulators;
  accumulator_set< force_t::value_type, features< tag::min, tag::max > >  force_acc;
  accumulator_set< coord_t, features< tag::max > >  displacement_acc;

  std::fill( forces.begin(), forces.end(), force_t( 0 ) );
  forces.resize( surface.num_vertices(), force_t( 0 ) );	

  // spring resetting forces
  for( auto spring = springs.begin(); spring != springs.end(); ++spring )
  { 
	const location_t  dr  = surface.vertex( spring->second ).location() - surface.vertex( spring->first ).location(); 
    const velocity_t  dv  = solver.velocity( spring->second ) - solver.velocity( spring->first );
	const distance_t  dR  = utk::l2norm(dr);
	//assert( (dist==0.f)==false );
    const distance_t  displace = dR - spring->length;
    const force_t     reset    = dr / dR * ( stiffness * displace + dampening * dot( dr, dv ) / dR);	

    forces[ spring->first ] += reset;
	forces[ spring->second ] -= reset;
	
  	displacement_acc( fabs(displace) );

  } // for springs

  for( size_t i = 0; i < forces.size() ; i++ )
  { 
    

    // friction
    forces[i] -= ;
	force_acc( utk::l2norm(forces[i]) );
  }

  solver.set_max_displacement( max( displacement_acc ) );  

  force_min = min( force_acc );
  force_max = max( force_acc );

}
*/

