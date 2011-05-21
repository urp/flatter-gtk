/***************************************************************************
 *            mmp-geodesics.h
 *
 *  Thu Feb  4 01:43:34 2010
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

# include "utk/geometry.h"
# include "surface.h"
# include "mmp-common.h"
# include "mmp-window.h"
# include "mmp-eventpoint.h"
# include "mmp-utilities.h"

// debugging

//# define DBG_MMP__USE_CAIRO

//# define DBG_FLAT_MMP_FORCE_SANITY_CHECK

//# define DBG_FLAT_MMP_INITIALIZE
//# define DBG_MMP_GEODESICS__DESTRUCTOR

//# define DBG_FLAT_MMP_HANDLE_EVENTS
//# define DBG_FLAT_MMP_PULL_EVENT
//# define DBG_FLAT_MMP_PROJECT_BOUNDS
//# define DBG_FLAT_MMP_PROPAGATE_WINDOW
//# define DBG_FLAT_MMP_ACCESS_CHANNEL
//# define DBG_FLAT_MMP_INSERT_WINDOW
//# define DBG_FLAT_MMP_DELETE_AC_WINDOW
//# define DBG_FLAT_MMP_QUERY_DISTANCE

//----|friend forward
namespace gl  { class GeodesicsDrawable; }
namespace gtk { class GeodesicsInspector; }


  namespace mmp
  {
	// check | distance(a,b) - distance(b,a) |
    bool vertex_pair_check( const std::shared_ptr< Surface >& surface, const distance_t& tolerance );
    
	// forward declarations
	class Geodesics;
	
	class Geodesics
  	{
	  public:
        //----|typedefs
          
        typedef Surface  surface_type;
          
	    typedef surface_type::vertex_descriptor vertex_descriptor;
	    typedef surface_type::edge_descriptor	edge_descriptor;
	    typedef surface_type::edge_handle 		edge_handle;
	    typedef surface_type::face_descriptor	face_descriptor;
  	    typedef surface_type::face_handle       face_handle;

		struct edge_info 
		{ const edge_descriptor& 		descriptor; // the target edge
		  const coord_t&				length;     // the length of the target edge
		  const coord_t&				srcbound;   // the left or right interval bouindary of the source window
  		  const coord_t&				edgebound;  // the corresponding boundary ( 0 or length ) on the target edge 
		  const utk::ray<coord_t,2>& 	ray;
		};

        // contains the frontier point and the event points
        // of the last modified or deleted windows in the access channel
        struct insert_result
        {
          insert_result()
          : event( 0 ), trimmed( false, false ), ac_ev( 0, 0 )
          { }
            
          EventPoint*   event;
          std::pair< bool, bool > trimmed;
          ev_pair_t     ac_ev;
        };


	  private:

    	typedef PriorityQueue<EventPoint*, EventPoint::less> event_queue_t;
      	typedef std::list<Window*>	winlist_t;
        // the access cannel descriptor 
        typedef std::pair<winlist_t::reverse_iterator,winlist_t::iterator> ac_t;

        // stores a window list for every edge
        typedef boost::vector_property_map< winlist_t, boost::property_map< surface_type::graph_t, boost::edge_index_t >::type >
                                                        edge_winlist_pmap_t;
        // stores a distance label for every vertex
        typedef boost::vector_property_map<distance_t>  vertex_label_pmap_t;

	  public:
		  
		friend class gl::GeodesicsDrawable;
		friend class gtk::GeodesicsInspector;

	  	template< side_t >
		friend std::pair< EventPoint*, bool >	trim_ac( Window*
                                                       , Geodesics::ac_t&
                                                       , Geodesics::winlist_t&
                                                       , const utk::ray<coord_t,2>&
								                       , const ps_t&, const ps_t&, ps_t&
                                                       , Geodesics& );			                           

	  private:  
        //----|data members
          
		const surface_type&			surf;

		const vertex_descriptor 	m_source;
			  
        vertex_label_pmap_t         vertex_labels;
          
		edge_winlist_pmap_t			windows;
          
		event_queue_t				event_queue;        

        mutable distance_t          m_max_distance;

        
        //----|functions
          
        void                        initialize();

		void					  	handle_event( EventPoint* ev );

		bool						step()
									{ assert( !event_queue.empty() );
									  EventPoint* ev = event_queue.pop(); 
									  handle_event( ev );
									  delete ev;
									  return !event_queue.empty();
									}
		  

        std::pair< winlist_t::iterator, ev_pair_t >
									delete_window( winlist_t::iterator loc, winlist_t& wlist );

		ev_pair_t					delete_event( event_queue_t::iterator ev );

		ev_pair_t					delete_event( EventPoint* ev );
		  
		template< side_t Side >
        EventPoint*                 delete_ac_window( ac_t& ac, winlist_t& wlist, const Window* candidate );

		template< side_t Side >
		void 						pull_event( EventPoint& ev );
		  
		template< side_t Bound >    std::pair< coord_t, edge_descriptor > // TODO: const??
									project_bound( EventPoint& ev
		                                   	     , const edge_descriptor& e1, const edge_descriptor& e2
												 , const utk::ray<coord_t,2>& e1vec, const utk::ray<coord_t,2>& e2vec
		                                         , const coord_t e0l, const coord_t e1l, const coord_t e2l, const ps_t& ps );
		  
        // project pseudosource through window over the mesh 
        void    					propagate_window( EventPoint& ev );

        ac_t                        access_channel( const Window& candidate, winlist_t& wlist );

  
		EventPoint*					insert_event_points( Window* win, const ps_t& ps );

		// remove obsolete endpoint events
 	    // and update frontier event associated with the window.
	  	// returns the frontier point of the window  if it still available
        template< EventPoint::flags_t event = EventPoint::ALL >
        EventPoint*                 update_event_points( Window* w, const ps_t& ps );   
          
		// finds the candidates 'interval of optimality' in the access channel.
		// trims or cuts all windows dominated by the candidate interval.
		// returns the connected neighbor-event
		template< side_t Side >
		EventPoint*					trim_ac( Window* candidate, ac_t& ac, winlist_t& wlist, const utk::ray<coord_t,2>& edge_ray
                                           , const ps_t& pred_ps,  const ps_t& psc, ps_t& psac );
		  
        // if window has minimal distance on an edge segment add it to the edge and push it to queue
		// returns the frontier point if candidate was inserted
	  	insert_result               insert_window( Window* candidate, const utk::ray<coord_t,2>& edge_ray, const ps_t& pred_ps );
  
        // check if all edges are fully covered
        bool                        sanity_check()  const;

          
	  public:
        
		Geodesics( surface_type&, const vertex_descriptor );

        virtual ~Geodesics();

        void        propagate_paths();
		  
        // retrieve distance from source to destination vertex
        distance_t  query_distance(const vertex_descriptor& target) const;

    
        //----|backtrace
          
        coord_t backtrace(const Window&, const coord_t& )	const;

		std::pair< coord_t, coord_t > 	backtrace( const Window& window, const std::pair< coord_t, coord_t >& bounds )	const
		{ 
		  coord_t  left = backtrace( window, get<  LEFT >( bounds ) );
  		  coord_t right = backtrace( window, get< RIGHT >( bounds ) );
		  assert( left <= right );
  	      return std::make_pair( left, right );
		}

		std::pair< coord_t, coord_t > 	backtrace( const Window& window )	const
        { return backtrace( window, window ); }
            
        std::list< coord_t >&   backtrace( const Window &, std::list< coord_t >& ) const;

        const distance_t        max_distance() const  
        {
          if( ! event_queue.empty() )
          { const Window& bottom = *event_queue.bottom()->window();
            return bottom.subpath() + bottom.max_ps_distance( bottom.pseudosource() ).second;
          }
           
          if( m_max_distance != 0 ) 
            return m_max_distance;
            
          for( auto listit = windows.storage_begin(); listit != windows.storage_end(); ++listit )
            std::for_each( listit->begin(), listit->end()
                         , [&m_max_distance] ( const Window* win ) 
                           { m_max_distance = std::max( m_max_distance, win->subpath() + win->max_ps_distance( win->pseudosource() ).second ); }
                         );
          return m_max_distance;
        }

          
        std::pair< winlist_t::const_iterator, winlist_t::const_iterator >  edge_windows( const edge_descriptor& edge ) const
        { 
		  const winlist_t& wlist = windows[edge];
          return std::make_pair( wlist.begin(), wlist.end() ); 
        }

          
        const surface_type&         get_surface()   const   { return surf; }

          
        const vertex_descriptor&    source()   const    { return m_source; }

          
        // deletes all windows on a given edge
		static void 				delete_windows_on_edge(winlist_t& wins) 
        { std::for_each(wins.begin(), wins.end(), [](Window* w) { delete w; } );
          wins.clear();
        }
		  
    };

	template< side_t EdgeSide >
	void couple_edge_events( EventPoint* source 
             		       , const Geodesics::insert_result& projected
                       	   , const Geodesics::insert_result& sidelobe
                     	   , const bool two_projected 
                     	   , const Geodesics::edge_handle& edge 
                 		   , const distance_t& opp_edge_length
                		   , const distance_t& base_edge_length );
	
    
  } // of namespace mmp


//=IMPLEMENTATION===============================================================
//==============================================================================

template< mmp::side_t Side >
mmp::EventPoint*	mmp::Geodesics::delete_ac_window( ac_t& ac, winlist_t& wlist, const Window* candidate = 0 )
{
  assert( Side !=  LEFT || ac.first != wlist.rend() );
  assert( Side != RIGHT || ac.second != wlist.end() );
  
  winlist_t::iterator loc = Side == LEFT ? (++ac.first).base() : ac.second;   

  # if defined DBG_FLAT_MMP_DELETE_AC_WINDOW
  std::clog << "mmp::Geodesics::delete_ac_window\t|" << **loc << std::endl;
  # endif
  // assert that dominated window has no permanent labels
  assert( !candidate //|| ! has_intersection( **loc, *candidate ) 
          || (*loc)->min_source_distance().second >= candidate->predeccessor()->min_source_distance().second ); 

  ev_pair_t ac_ev( 0, 0 );

  std::tie( ac.second, ac_ev ) = delete_window( loc, wlist );
  
  ac.first  = winlist_t::reverse_iterator( ac.second );
  
  # if defined DBG_FLAT_MMP_DELETE_AC_WINDOW
  std::clog << "mmp::Geodesics::delete_ac_window\t| left ";
  if( ac.first != wlist.rend() ) std::clog << **ac.first; else std::clog << "NIL";
  std::clog << std::endl
            << "mmp::Geodesics::delete_ac_window\t| right ";
  if( ac.second != wlist.end() ) std::clog << **ac.second; else std::clog << "NIL";
  std::clog << std::endl;
  # endif
  return get< Side >(ac_ev);
}


template< mmp::EventPoint::flags_t PosFlags >
mmp::EventPoint*  mmp::Geodesics::update_event_points( Window* w, const ps_t& ps )
{ 
  assert( PosFlags & ( EventPoint::LEFT_END | EventPoint::RIGHT_END ) );
  
  EventPoint::Grabber< event_queue_t::iterator, PosFlags | EventPoint::FRONTIER > evgrab( event_queue.begin(), event_queue.end(), w );

  event_queue_t::iterator evf = evgrab.frontier;

  if( PosFlags & EventPoint::LEFT_END )
    if( evgrab.left != event_queue.end() && evgrab.left != evf ) 
      delete_event( evgrab.left );

  if( PosFlags & EventPoint::RIGHT_END )
    if( evgrab.right != event_queue.end() && evgrab.right != evf ) 
      delete_event( evgrab.right );

  if( evf != event_queue.end() ) 
  { (*evf)->update(ps); // update distance and endpoint flags
	return *evf;
  }

  return 0;
}


template< mmp::side_t Side >
void mmp::Geodesics::pull_event( EventPoint& ev )
{
  EventPoint* adjacent = ev.adjacent<Side>();
  
  if( adjacent && adjacent->window()->edge != ev.window()->edge 
      && adjacent->window()->source_distance< side_traits<Side>::opposite >() < ev.window()->source_distance< Side >() )
  {
    assert( !ev.colinear<Side>() || adjacent->window()->edge == ev.window()->predeccessor()->edge);
    assert( ev.colinear<Side>() 
           || ( edge_handle( adjacent->window()->edge, get_surface() ).target() == edge_handle( ev.window()->edge, get_surface() ).target() )
           || ( edge_handle( adjacent->window()->edge, get_surface() ).source() == edge_handle( ev.window()->edge, get_surface() ).source() ) 
          );

    # if defined DBG_FLAT_MMP_PULL_EVENT
    std::clog << "mmp::Geodesics::pull_event\t|"
              << '(' << side_traits<Side>::string() << ' ' << ( ev.colinear<Side>() ? "colinear" : "crossing" ) << " )" << *adjacent
              << std::endl;
    # endif

    //handle_event( adj );	
    propagate_window( *adjacent );

    // reinsert endpoint events if colinear frontier point is endpoint 
    if( adjacent->flags() & EventPoint::LEFT_END ) 
	  event_queue.push( new EventPoint( EventPoint::LEFT_END, adjacent->window(), adjacent->distance() ) );
    if( adjacent->flags() & EventPoint::RIGHT_END ) 
	  event_queue.push( new EventPoint( EventPoint::RIGHT_END, adjacent->window(), adjacent->distance() ) );
    delete_event( adjacent );

    assert( !ev.adjacent<Side>() || ev.adjacent<Side>()->window()->edge == ev.window()->edge );
  }else assert( !ev.colinear<Side>() || ev.colinear<Side>()->window()->edge != ev.window()->predeccessor()->edge );
}

template< mmp::side_t Side >
std::pair< mmp::coord_t, mmp::Geodesics::edge_descriptor > 
	mmp::Geodesics::project_bound( EventPoint& 				ev
                                       , const edge_descriptor&   	e1
                                       , const edge_descriptor&   	e2
									   , const utk::ray<coord_t,2>&	e1ray
                                       , const utk::ray<coord_t,2>&	e2ray
                                       , const coord_t				e0l
                                       , const coord_t				e1l
                                       , const coord_t				e2l
                                       , const ps_t&				ps
                                       )
{  
  const Window*	srcwin = ev.window();

  # if defined FLAT_MMP_MAINTAIN_WAVEFRONT

  // pull_colinear event to current edge
  pull_event<Side>(ev);

  EventPoint* adjacent = ev.adjacent<Side>();
  EventPoint* colinear = ev.colinear<Side>();

  # if defined DBG_FLAT_MMP_PROJECT_BOUNDS
  if( adjacent ) std::clog << "mmp::Geodesics::project_bound\t| "<< side_traits<Side>::string()
	                       << ' ' << ( colinear ? "colinear" : "crossing" ) << ' ' << *adjacent << std::endl;
  # endif
  
  assert( !adjacent || adjacent->adjacent< side_traits<Side>::opposite >() == &ev );
  assert( !colinear || colinear->colinear< side_traits< Side >::opposite >() );

  Window* 	  adjacent_win =  adjacent ? adjacent->window() : 0;

  coord_t 			new_bound;
  edge_descriptor 	new_edge;

  if( colinear && srcwin->edge == adjacent_win->predeccessor()->edge )
  { 
    // colinear window was already propagated away from window edge ( it is one step ahead )
	# if defined DBG_FLAT_MMP_PROJECT_BOUNDS
	std::clog << "mmp::Geodesics::project_window\t|"
			  << " taking intersection from adjacent window "
	  		  << std::endl;
	# endif
	new_bound = adjacent_win->bound< side_traits<Side>::opposite >();
	new_edge  = adjacent_win->edge;

    //TODO: colinear_ps_correction( left, right, psl, psr, pslerr, psrerr );
    
  }else
  # endif // of FLAT_MMP_MAINTAIN_WAVEFRONT
  {

    assert( ps.y() > 0 );
	const coord2_t b( srcwin->bound<Side>(), 0 );

	utk::plane<coord_t,2> psb = utk::plane_from_ray( utk::ray<coord_t,2>( b, ps ) );

	edge_info e1info = { e1, e1l,   0,   0, e1ray };
	edge_info e2info = { e2, e2l, e0l, e2l, e2ray };
	  
	std::pair< edge_info, edge_info > edges( Side == LEFT 
                                             ? std::make_pair( e2info, e1info ) 
	                                         : std::make_pair( e1info, e2info ) );

    // intersect with first edge
	new_bound = utk::intersection( edges.first.ray , psb );
	new_edge  = edges.first.descriptor;
	// check if intersection is valid - intersect second edge otherwise
	if( new_bound < coord_t(0) || new_bound >= edges.first.length )
	{ 
      new_bound = b[0] == edges.second.srcbound 
                  ? edges.second.edgebound 
				  : utk::intersection( edges.second.ray, psb );
      
	  new_edge = edges.second.descriptor;
	}

    # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
    // crossing window was already propagated away from window edge ( it is one step ahead ))
    if( !colinear && adjacent && adjacent_win->edge != srcwin->edge )
    {
      if( new_edge == adjacent_win->edge )
      {
        const coord_t& adjacent_opp_bound = adjacent_win->bound< side_traits<Side>::opposite >();
        if( side_traits< Side >::smaller_equal( adjacent_opp_bound, new_bound ) )
        {
          # if defined DBG_FLAT_MMP_PROJECT_BOUNDS
	      std::clog << "mmp::Geodesics::project_window\t|"
			        << " divergent bounds - window(" << side_traits<Side>::string() << ") = " << new_bound 
                    << " adjacent(" << side_traits< side_traits<Side>::opposite >::string() << ") = " << adjacent_opp_bound
                    << std::endl;
	      # endif
          
          // !!! TODO: use weighted/mean + make colinear ? ( + pscorrection? ) 
          new_bound = adjacent_opp_bound;
      } }
    }        
    # endif
  }
  
  # if defined DBG_FLAT_MMP_PROJECT_BOUNDS
  std::clog << "mmp::Geodesics::project_bound\t| "<< side_traits<Side>::string()
			<< " intersection " << new_bound 
			<< " edge " << new_edge
			<< ( is_vertex<Side>( srcwin->bound<Side>(), e0l ) ? " (vertex)" : "") 
			<< std::endl;
  # endif
  return { new_bound, new_edge };
}

template< mmp::side_t EdgeSide >
void mmp::couple_edge_events( EventPoint* source 
                		          , const Geodesics::insert_result& projected
                       			  , const Geodesics::insert_result& sidelobe
                     			  , const bool two_projected 
                     			  , const Geodesics::edge_handle& edge 
                 			      , const distance_t& opp_edge_length
                			      , const distance_t& base_edge_length )
{
  # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
  std::clog << "mmp::couple_edge_events\t\t|"
            << '(' << side_traits< EdgeSide >::string() << ") " << edge
            << std::endl;
  # endif

  const side_t OppEdgeSide = side_traits<EdgeSide>::opposite;
                 
  if( sidelobe.event )
  {
    assert( is_sidelobe( *sidelobe.event->window() ) );
    assert( sidelobe.event->window()->edge == edge.descriptor() );
    
    //----|couple events towards C
    if( get< OppEdgeSide >( sidelobe.ac_ev ) ) 
    { 
      EventPoint* ac_ev = get< OppEdgeSide >( sidelobe.ac_ev );

      //----|check if neighbor is colinear
      const bool outer_sidelobe = is_outer_sidelobe( *sidelobe.event->window(), edge ); 

      const bool colinear_flag = outer_sidelobe && ac_ev->window()->edge == edge.descriptor() // TODO: pull_event ???
                                 && is_vertex< EdgeSide >( ac_ev->window()->predeccessor()->bound< EdgeSide >(), opp_edge_length );

      # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| (" << ( outer_sidelobe ? "outer" : "inner" ) << " sidelobe)"
                << " couple " << side_traits< OppEdgeSide >::string() << " candidate-adjacent " << *ac_ev
                << std::endl;
      # endif

      sidelobe.event->couple< OppEdgeSide >( ac_ev, colinear_flag );
    }
    
    //----|couple events towards baseline
    if( get< EdgeSide >( sidelobe.ac_ev ) && get< EdgeSide >( sidelobe.ac_ev ) != get< EdgeSide >( projected.ac_ev ) ) 
    {
      # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| (sidelobe)"
                << " couple " << side_traits< EdgeSide >::string() << " candidate-adjacent " << *get< EdgeSide >( sidelobe.ac_ev )
                << std::endl;
      # endif
      sidelobe.event->couple< EdgeSide >( get< EdgeSide >( sidelobe.ac_ev ), false );
    }
  }

  if( projected.event )
  {
    assert( projected.event->window()->edge == edge.descriptor() );
    
    if( get< OppEdgeSide >( projected.ac_ev ) ) 
    { 
      assert( !sidelobe.event );
      
      //ate potentially colinear/sidelobe window ?
      EventPoint* ac_ev = get<OppEdgeSide>( projected.ac_ev );

      const bool colinear_flag = ac_ev->window()->edge == edge.descriptor() // TODO: pull_event ???
                                 && is_outer_sidelobe( *ac_ev->window(), edge ) 
                                 && is_vertex< OppEdgeSide >( source->window()->bound<OppEdgeSide>(), base_edge_length );

      # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| " << "(projected)"
                << " couple " << side_traits< OppEdgeSide >::string() << " candidate-adjacent " << *ac_ev
                << std::endl;
      # endif                                 
      projected.event->couple< OppEdgeSide >( ac_ev, colinear_flag );
    }

    // initialy couple projected and (left outer) side lobe window
    if( sidelobe.event )
    { 
      // TODO:
      //assert( !sidelobe.event->adjacent< EdgeSide >() );
      # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| " << "(projected)"
                << " initialy couple " << side_traits< OppEdgeSide >::string() << " sidelobe"
                << std::endl;
      # endif
      projected.event->couple< OppEdgeSide >( sidelobe.event, true ); 
    }
    else 
    // couple source-adjacent towards C - only if the edge contains the complete projection of the source window
    if( source->adjacent< OppEdgeSide >() && !two_projected && !get<OppEdgeSide>( projected.trimmed ) ) 
    { 
      # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| " << "(projected)"
                << " couple " << side_traits< OppEdgeSide >::string() << " source-adjacent " << *source->adjacent< OppEdgeSide >()
                << std::endl;
      # endif      
      projected.event->couple< OppEdgeSide >( source->adjacent< OppEdgeSide >(), source->colinear< OppEdgeSide >() != 0 ); 
    }

    // couple candidate adjacent towards baseline
    if( get< EdgeSide >( projected.ac_ev ) )
    {
      # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| " << "(projected)"
                << " couple " << side_traits< EdgeSide >::string() << " candidate-adjacent " << *get< EdgeSide >( projected.ac_ev )
                << std::endl;
      # endif
      projected.event->couple< EdgeSide >( get< EdgeSide >( projected.ac_ev ), false );
    }
    else // couple keep source-adjacent towards baseline
    if( source->adjacent< EdgeSide >() && !get<EdgeSide>( projected.trimmed ) )
    {
       # if defined DBG_FLAT_MMP_EVENTPOINT_COUPLING
      std::clog << "mmp::couple_edge_events\t\t| " << "(projected)"
                << " couple " << side_traits< EdgeSide >::string() << " source-adjacent " << *source->adjacent< EdgeSide >()
                << std::endl;
      # endif
      projected.event->couple< EdgeSide >( source->adjacent< EdgeSide >(), source->colinear< EdgeSide >() != 0 );
    }

  }
       
}
