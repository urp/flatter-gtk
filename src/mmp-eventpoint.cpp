//           mmp-eventpoint.cpp
//  Sun Apr 25 13:56:30 2010
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

# include "mmp-eventpoint.h"
# include "mmp-utilities.h"

using namespace mmp;

std::ostream& mmp::operator<<( std::ostream& os, const EventPoint& ev )
{ 
  os << "event point"
     << "(" << ev.window()->id
     << ":";
  if(ev.flags()&EventPoint::LEFT_END ) os << 'l';
  if(ev.flags()&EventPoint::RIGHT_END) os << 'r';
  if(ev.flags()&EventPoint::FRONTIER ) os << 'f';

  os << ") point " << ev.point()
     << " distance " << ev.distance();
  # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
  if( ev.adjacent<LEFT>() )
  { 
    const Window&  left_window = *ev.adjacent< LEFT>()->window();
    
    os << " <" << (ev.colinear<LEFT>() ? '=' : 'X') << left_window.id
       << " (";
    
    if( left_window.edge == ev.window()->edge ) 
      os << distance_error( ev.window()->bound<LEFT>()
                          , left_window, *ev.window()
                          , left_window.pseudosource()
                          , ev.window()->pseudosource() );

    assert( left_window.predeccessor() );
    if( left_window.predeccessor()->edge == ev.window()->edge ) 
      os << " adv " << distance_error( ev.window()->bound<LEFT>()
                                     , *left_window.predeccessor(), *ev.window()
                                     , left_window.predeccessor()->pseudosource(), ev.window()->pseudosource() );
                            
    assert( ev.window()->predeccessor() );
    if( left_window.edge == ev.window()->predeccessor()->edge ) 
      os << " lag " << distance_error( ev.window()->predeccessor()->bound<LEFT>()
                                     , left_window, *ev.window()->predeccessor()
                                     , left_window.pseudosource(), ev.window()->predeccessor()->pseudosource() );
    os << ")";
  }

  
  if( ev.adjacent<RIGHT>() )
  { 
    const Window& right_window = *ev.adjacent<RIGHT>()->window();

    os << " >" << (ev.colinear<RIGHT>() ? '=' : 'X') << ev.adjacent<RIGHT>()->window()->id
       << " (";

    if( right_window.edge == ev.window()->edge ) 
      os << distance_error( ev.window()->bound<RIGHT>()
                          , *ev.window(), right_window
                          , ev.window()->pseudosource(), right_window.pseudosource() );

    assert( right_window.predeccessor() );
    if( right_window.predeccessor()->edge == ev.window()->edge ) 
      os << " adv " << distance_error( ev.window()->bound<RIGHT>()
                                     , *ev.window(), *right_window.predeccessor()
                                     , ev.window()->pseudosource(), right_window.predeccessor()->pseudosource() );
                            
    assert( ev.window()->predeccessor() );
    if( right_window.edge == ev.window()->predeccessor()->edge ) 
      os << " lag " << distance_error( ev.window()->predeccessor()->bound<RIGHT>()
                                     , *ev.window()->predeccessor(), right_window
                                     , ev.window()->predeccessor()->pseudosource(), right_window.pseudosource() );
    os << ")";
  }
  # endif
  //<< "\t"   << *ev.window();
  return os;
}

# if defined FLAT_MMP_MAINTAIN_WAVEFRONT
void	mmp::couple( EventPoint* const left, EventPoint* const right, const bool colinear )
{
  assert( left );
  assert( right );

  # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
  std::clog << "mmp::couple_adjacent\t\t| "
			<< " window(" <<  left->window()->id << ") "
            << "<- " << ( colinear ? '=' : 'X') << " ->"
			<< " window(" << right->window()->id << ") "
			<< std::endl;
  # endif

  // check valid edges
  assert(   !colinear
         || left->window()->edge                  == right->window()->edge 
	  	 || left->window()->predeccessor()->edge  == right->window()->edge
         || left->window()->edge                  == right->window()->predeccessor()->edge );
  

  // if events are on same edge, their windows have to touch
  assert(   left->window()->edge                  != right->window()->edge 
         || left->window()->bound< RIGHT >() == right->window()->bound< LEFT >() );
  
  left->decouple<RIGHT>();
  right->decouple<LEFT>();

  right->set_adjacent<  LEFT >(  left, colinear );
  left ->set_adjacent< RIGHT >( right, colinear );
}
# endif
