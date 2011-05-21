/***************************************************************************
 *            mmp-eventpoint.h
 *
 *  Sun Apr 25 13:56:30 2010
 *  Copyright  2010  Peter Urban
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

//----|debugging
//#define DBG_FLAT_MMP_EVENTPOINT_GRABBER
//# define DBG_FLAT_MMP_EVENTPOINT_CONSTRUCTION
//# define DBG_FLAT_MMP_EVENTPOINT_DESTRUCTION
//# define DBG_FLAT_MMP_EVENTPOINT_COUPLING
//# define DBG_FLAT_MMP_EVENTPOINT_DECOUPLING

#include "mmp-window.h"
#include "mmp-common.h"

  namespace mmp
  {

    //----|forward declarations
    class EventPoint;

    std::ostream& operator<<(std::ostream& os,const EventPoint& ev);

    typedef std::pair< EventPoint*, EventPoint* >   ev_pair_t;

    # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
	void	couple( EventPoint* const, EventPoint* const, const bool );
    # endif

    //----|Eventpoint
    
    class EventPoint
    {
      public:
        
        typedef enum { LEFT_END = 1, RIGHT_END = 2, FRONTIER = 4, ALL = 7 } pos_flags;
        typedef enum { LEFT_COLINEAR = 8, RIGHT_COLINEAR = 16 }             colinear_flags;
        
        //TODO: use std::bitset
		typedef int flags_t;
      
		friend 	void	mmp::couple( EventPoint* const, EventPoint* const, const bool);

      private:
        Window*                         m_window;
		
        flags_t		                    m_mask;
        distance_t                      m_distance;

      # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
		ev_pair_t	m_adjacent;

        // TODO: optimize flag modification
        template< side_t Side > void    set_adjacent( EventPoint* ev, bool colinear = false )
        { assert( ev || colinear == false );
          get<Side>( m_adjacent ) = ev;
          if( colinear ) set_colinear_flag< Side >(); else unset_colinear_flag< Side >();
        }
        
        template< side_t Side > void    set_colinear_flag()   { m_mask |= Side == LEFT ? LEFT_COLINEAR : RIGHT_COLINEAR; }
        template< side_t Side > void    unset_colinear_flag() { set_colinear_flag<Side>(); m_mask ^= Side == LEFT ? LEFT_COLINEAR : RIGHT_COLINEAR; }

      # endif

      public:         
        
        EventPoint( flags_t mask, Window* win, distance_t distance )
        : m_window( win ), m_mask( mask ), m_distance( distance )
        # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
    	, m_adjacent( static_cast<EventPoint*>(0), static_cast<EventPoint*>(0) )
        # endif
        { 
 	      assert( window() );
		  assert( !( flags() & RIGHT_END && flags() & LEFT_END ) );

          # if defined DBG_FLAT_MMP_EVENTPOINT_CONSTRUCTION
          std::clog << "mmp::EventPoint::EventPoint\t| " << *this << std::endl;
          # endif
        }

        ~EventPoint()
        {
          # if defined DBG_FLAT_MMP_EVENTPOINT_DESTRUCTION
          std::clog << "mmp::EventPoint::~EventPoint\t| " << *this << std::endl;
          # endif
          
          # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
          decouple<LEFT >();
          decouple<RIGHT>();
          # endif
        }

        Window*             window()	const   { return m_window; }

		const flags_t&	    flags()	const   { return m_mask;  }

        coord_t             point(const ps_t& ps)	const   
		{ return flags() & RIGHT_END 
                 ? window()->bound < RIGHT > () 
			  	 : ( flags() & LEFT_END 
                     ? window()->bound < LEFT >()
				     : window()->frontier_point( ps ) ); 
		}
		
        coord_t             point()	const	{ return point( window()->pseudosource() ); }

		// source distance to frontier point
        const distance_t&   distance()	const	{ return m_distance; }
		
        # if defined FLAT_MMP_MAINTAIN_WAVEFRONT

		const ev_pair_t& adjacent() const	{ return m_adjacent; }

		//TODO : make really const
		template< side_t Side >  
		EventPoint* const & adjacent()	const	{ return std::get < Side > ( m_adjacent ); }

        template< side_t Side >
        EventPoint*         colinear()   const   { return  m_mask & ( Side == LEFT ? LEFT_COLINEAR : RIGHT_COLINEAR ) ? adjacent<Side>() : 0; }

        template< side_t Side >
        void                couple( EventPoint* const& adjacent, const bool& colinear_flag )            
        {
          if( Side ==  LEFT ) mmp::couple( adjacent, this, colinear_flag );
          if( Side == RIGHT ) mmp::couple( this, adjacent, colinear_flag );
        }

        bool couple_adjacent()
        {
          const ev_pair_t adj = adjacent();
		  if( get<LEFT>(adj) && get<RIGHT>(adj) ) 
          { 
            # if defined DBG_FLAT_MMP_EVENTPOINT_DECOUPLING
            std::clog << "mmp::EventPoint::couple_adjacent\t| removing " 
                     << *this << " from wavefront" << std::endl; 
            mmp::couple( get<LEFT>(adj), get<RIGHT>(adj), false );
            # endif
            return true;
          }
          return false;
        }
        
   		template< side_t Side >
		void				decouple()
        {
		  if( adjacent<Side>() ) 
		  { 
            # if defined DBG_FLAT_MMP_EVENTPOINT_DECOUPLING
			/*std::clog << "mmp::EventPoint::decouple\t\t| ("
			  		  << side_traits<Side>::string()
			  		  << ") from " << this->window()->id << " adjacent " << *adjacent< Side >()
					  << std::endl;*/
            #endif
			assert( adjacent<Side>()->adjacent< side_traits<Side>::opposite >() == this );

   			adjacent<Side>()->set_adjacent< side_traits<Side>::opposite >( 0 );
            set_adjacent<Side>( 0 );            
		  }
  	    }   

  		void				decouple()
        { decouple<LEFT>();
          decouple<RIGHT>();
        }
        
        # endif

		// update distance 
		// mark as endpoint if frontier point is identical to a boundary
        void    update(const ps_t& ps)
		{ assert( flags() & FRONTIER );
          coord_t fp;
          boost::tie( fp, m_distance ) = window()->min_source_distance(ps); 
		  if( fp == window()->bound<  LEFT >() ) { m_mask |= LEFT_END; }
          if( fp == window()->bound< RIGHT >() ) { m_mask |= RIGHT_END; }
        }
        
        // comparison operator reflecting the priority of EventPoints ( from lower distance to higher distances ) 
        bool    operator<(const EventPoint& o)  const
        { return /*utk::close_ulps( distance(), o.distance() ) ? !window()->has_ps_vertex() && o.window()->has_ps_vertex() :*/ 
                 //std::max( window()->source_distance<LEFT>(), window()->source_distance<RIGHT>() ) < std::max( o.window()->source_distance<LEFT>(), o.window()->source_distance<RIGHT>() );
                 distance() < o.distance(); 
		}

		struct less
		{
	  	  bool operator() ( EventPoint* a, EventPoint* b )	const
	  	  { return *a < *b; }
		};
		
        struct WindowPredicate
        {
          const Window*                 m_window;
          
                                        WindowPredicate(const Window* win)
                                        : m_window(win) {   }
          
          bool                          operator() (const EventPoint* ev) const   
		  								{ 
										  return m_window == ev->window();
									    }
        };

        template< typename It, flags_t PosFlags = ALL >
        struct Grabber
        {
          It    left, frontier, right;

                Grabber( It begin, It end, Window* win )
                : left(end), frontier(end), right(end)                          
		  		{ 
				  WindowPredicate pred(win); 
                  for( It r = std::find_if(begin, end, pred); r != end; r = std::find_if(++r, end, pred) )
				  { if( FRONTIER  & PosFlags & (*r)->flags() ) { assert(frontier==end); frontier=r; }
                    if( LEFT_END  & PosFlags & (*r)->flags() ) { assert(left    ==end);     left=r; }
                    if( RIGHT_END & PosFlags & (*r)->flags() ) { assert(right   ==end);    right=r; }
				  }
				  # if defined DBG_FLAT_MMP_EVENTPOINT_GRABBER
				  std::clog << "mmp::EventPoint::Grabber\t|"
					  		<< " window " << win->id
					  		<< " left " << (left!=end) 
					  		<< " frontier " << (frontier!=end) 
					  		<< " right " << (right!=end)
					  		<< std::endl;
				  # endif
                }
        };
    }; // of EventPoint
    
  } // of mmp
