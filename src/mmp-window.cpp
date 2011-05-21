//           mmp-window.cpp
//  Fri Apr  9 13:04:36 2010
//  Copyright  2010  Peter Urban
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

#include "mmp-window.h"


size_t mmp::Window::next_id=0;

mmp::Window::Window(   Window*                            p
                         , const Surface::edge_descriptor& e
                         , const coord_t&    b0, const coord_t&    b1
                         , const distance_t& d0, const distance_t& d1 
                         , const vertex_descriptor& psv, const distance_t& psdist )
: id( next_id++ ), ps( psv ), parent( p ), edge( e ), m_bounds( b0, b1 ), m_distances( d0, d1 ), d( psdist )
{ 	}


bool	mmp::Window::sanity_check()   const	
{ 
  assert( std::isfinite( d ) && ! std::isnan( d ) && d >= 0 );

  if( bound<LEFT>() < 0 ) 
  { std::cerr << "mmp::Window::sanity_check"
              << "\t|FAILED - " << *this
              << "\t| lower bound(" << bound<LEFT>() << ") < 0"
              << std::endl;
    return false;
  }
  
  if( bound<LEFT>() >= bound<RIGHT>() )
  { std::cerr << "mmp::Window::sanity_check"
              << "\t|FAILED - "<< *this
              << "\t| lower bound(" << bound<LEFT>() << ") >= upper bound(" << bound<RIGHT>() << ")"
              << std::endl;
    return false;
  }

  if( has_ps_vertex<LEFT>() && bound<LEFT>() != 0 )
  { std::cerr << "mmp::Window::sanity_check"
              << "\t|FAILED - "<< *this
              << "\t| bound not at ps-vertex" << length() << ")"
              << std::endl;
    return false;
  }

  if( utk::close< coord_t >()( bound<LEFT>() ,bound<RIGHT>() ) )
  { std::cerr << "mmp::Window::sanity_check"
              << "\t|FAILED - "<< *this
              << "\t| window is too tight - length " << length() << ")"
              << std::endl;
    return false;
  }
  
  if( ( ps_distance<LEFT>() + ps_distance<RIGHT>() ) < length() && !has_ps_vertex() )
  { std::cerr << "mmp::Window::sanity_check"
              << "\t|FAILED - " << *this
              << "\t| distance sum("<< ( ps_distance<LEFT>() + ps_distance<RIGHT>() ) << ") < length(" << length() << ")"
              << "\t|pseudosource(" << pseudosource() << ") reconstruction impossible"
              << std::endl;
    return false;
  }

  const coord_t ps_dist_diff = fabs( ps_distance<LEFT>() - ps_distance<RIGHT>() ); 
  
  if( length() < ps_dist_diff 
      && !utk::close_to_zero( length() - ps_dist_diff ) 
    )
  { std::cerr << "mmp::Window::sanity_check"
              << "\t|FAILED - "<<(*this)
              << "\t| distance diff(" << ps_dist_diff << ") > interval length(" << length() << ')'
              << "\t| pseudosource(" << pseudosource() << ") reconstruction undefined"
              << std::endl;
    return false;
  }
  return true; 
}

std::ostream& mmp::operator<<(std::ostream& os,const Window& w)
{ Window* pre = w.predeccessor();
  os << "window(" << w.id << ">";
  if(pre) 
    os << pre->id;
  else
    os << "S";
  os << ( !pre ? "" : ( is_inner_sidelobe( w ) ? "i" : ( is_outer_sidelobe( w ) ? "o" : "p" ) ) )
     << ") " << '>' << w.ps 
     << " e " << w.edge;
  if(pre) 
    os << ">"  << (pre->edge) ;
  os << "\t [" << w.bound<LEFT>() << "," << w.bound<RIGHT>() << "]"
     << " (" << w.ps_distance<LEFT>() << "," << w.ps_distance<RIGHT>() << ")"
     << " sp " << w.subpath()
     << " fp " << w.frontier_point()
     << " err " << w.ps_error();
     //<< std::endl;
  return os;
}