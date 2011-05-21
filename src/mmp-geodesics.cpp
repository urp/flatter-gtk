//           mmp-geodesics.cpp
//  Thu Feb  4 01:43:34 2010
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

# include "mmp-geodesics.h"

# include "mmp-utilities.h"
# include "mmp-trim_ac.h"

# include <boost/lexical_cast.hpp>
# include <boost/accumulators/accumulators.hpp>
# include <boost/accumulators/statistics/min.hpp>
# include <boost/accumulators/statistics/max.hpp>
# include <boost/accumulators/statistics/mean.hpp>

# if defined DBG_MMP__USE_CAIRO
# include "mmp-visualizer-cairo.h"
# endif

using namespace mmp;

//initialize with subgraph???
Geodesics::Geodesics( surface_type& surface, const vertex_descriptor source )
: surf(surface), m_source(source), windows( surface.get_property_map<boost::edge_index_t>() )
, vertex_labels( surface.num_vertices() ), event_queue(), m_max_distance( 0 )
{ 
  std::clog << "mmp::Geodesics::Geodesics\t\t|"
            << "source " << source
            << std::endl;

  assert( surface.has_triangulation() );

  // TODO: this is bad
  Window::next_id = 0;
}

Geodesics::~Geodesics()
{ 
  # if defined DBG_MMP_GEODESICS__DESTRUCTOR
  std::clog << "mmp::Geodesics::~Geodesics\t|"
            << "deleting windows"
            << std::endl;
  # endif
  std::for_each( windows.storage_begin(), windows.storage_end(), delete_windows_on_edge );
}


Geodesics::ac_t  Geodesics::access_channel(const Window& candidate, winlist_t& wlist)
{
  if(wlist.empty()) return std::make_pair( wlist.rend(), wlist.end() );

  const Window& wpre = *candidate.predeccessor();
  const coord_t fp   = wpre.frontier_point();
 
  const bool left_edge_candidate = edge_handle(candidate.edge,surf).source()
                                   == edge_handle(wpre.edge,surf).source();

  winlist_t::iterator itr = wlist.begin();

  // iterate until the ac-candidate and insertion-candidate predeccessors lay on the same edge
  if( !left_edge_candidate ) while( itr != wlist.end()  && (*itr)->predeccessor()->edge != wpre.edge ) ++itr;

  //find right bounding window on the shared edge
  while( itr != wlist.end()  && (*itr)->predeccessor()->edge == wpre.edge && (*itr)->predeccessor()->frontier_point() < fp ) 
  { 
    # if defined DBG_FLAT_MMP_ACCESS_CHANNEL_VERBOSE
    std::clog << "mmp::access_channel\t\t|"
              << " right pred candidate " << *(*itr)->predeccessor()
              << std::endl;
    # endif
    ++itr;
  }

  // QUIRK to decide in case of equal frontier points 
  if( itr != wlist.end() )
  { 
	// siblingss are inserted after another - side lobes are inserted first  
    const bool putright = left_edge_candidate;

    const bool sameparent = &wpre == (*itr)->predeccessor();
	
    if( ( sameparent && !putright ) || ( !sameparent && wpre.bound< LEFT >() == (*itr)->predeccessor()->bound< RIGHT >() ) )
    { 
      # if defined DBG_FLAT_MMP_ACCESS_CHANNEL
      std::clog << "mmp::access_channel\t\t|"
                << " using QUIRK - shifting right bound "
                << std::endl << std::flush;
      # endif
      ++itr;
    }
  }
  
  // the left ac bound
  winlist_t::reverse_iterator itl( itr ) ;

  //debugging
  # if defined DBG_FLAT_MMP_ACCESS_CHANNEL

  if(itl != wlist.rend()) 
    std::clog << "mmp::access_channel\t\t|"
              << " pred left  " << *(*itl)->predeccessor() << std::endl<< "\t\t\t\t\t|";

  else 
    std::clog << "mmp::access_channel\t\t|";
  
  std::clog << " pred cand   "<< wpre << " (on " << (left_edge_candidate ? "left" : "right") << " edge)" << std::endl;
  
  if(itr != wlist.end()) 
	std::clog << "\t\t\t\t\t|" << " pred right " << *(*itr)->predeccessor() << std::endl;

	
  if(itl != wlist.rend()) 
	std::clog << "\t\t\t\t\t| left  " << **itl << std::endl;

  std::clog << "\t\t\t\t\t| cand  " << candidate << std::endl;
	
  if(itr != wlist.end())  
	std::clog << "\t\t\t\t\t| right " << **itr << std::endl;

  # endif  
  
  return std::make_pair( itl, itr );
}

EventPoint*  Geodesics::insert_event_points( Window* win, const ps_t& ps )
{
  const coord_t fp = win->frontier_point(ps);
  const bool fpl = fp == win->bound<LEFT>();
  const bool fpr = fp == win->bound<RIGHT>();

  std::pair< EventPoint*, EventPoint* > its;
  
  its.first = event_queue.push( new EventPoint( (fpl ? EventPoint::FRONTIER : 0)|EventPoint::LEFT_END,  win, win->source_distance< LEFT>() ) );
  
  its.second= event_queue.push( new EventPoint( (fpr ? EventPoint::FRONTIER : 0)|EventPoint::RIGHT_END, win, win->source_distance<RIGHT>() ) );
  
  if( !(fpl || fpr) ) // interior frontier point
    return event_queue.push( new EventPoint( EventPoint::FRONTIER, win, win->source_distance( fp, ps ) ) );

  return fpl ? its.first : its.second;
}


Geodesics::insert_result Geodesics::insert_window( Window* candidate, const utk::ray<coord_t,2>& edge_ray, const ps_t& pred_ps )
{
  using boost::tie;

  winlist_t& wlist = windows[candidate->edge];

  const edge_handle ehc( candidate->edge, surf );
  
  const ps_t psc( candidate->pseudosource() );
  
  # if defined DBG_FLAT_MMP_INSERT_WINDOW 
  std::clog	<< "mmp::Geodesics::insert_window\t|^"
            << *candidate
		 	<< " -> " << ehc << " with " << wlist.size() << " windows"
			<< std::endl << std::flush;
  # endif

  Geodesics::insert_result result;
  
  if( !wlist.empty() && !wlist.front()->predeccessor() )
  { 
    # if defined DBG_FLAT_MMP_INSERT_WINDOW
    std::clog << "mmp::Geodesics::insert_window\t|" << "complete - arrived at source" << std::endl;
    # endif
    delete candidate;
    return result;
  }

  // determines the window(s) between which the candidate might be inserted
  ac_t ac = access_channel( *candidate, wlist );

  // trim access channel - find the optimal interval of the candidate
  ps_t psl, psr;
  tie( get< LEFT>(result.ac_ev), get< LEFT>(result.trimmed) ) 
        = mmp::trim_ac<LEFT> ( candidate, ac, wlist, edge_ray, pred_ps, psc, psl, *this );
  if( candidate->is_valid() )
    tie( get<RIGHT>(result.ac_ev), get<RIGHT>(result.trimmed) )  
        = mmp::trim_ac<RIGHT>( candidate, ac, wlist, edge_ray, pred_ps, psc, psr, *this ); 


  //----|insert candidate
  if( candidate->is_valid() )
  { 
    # if defined DBG_FLAT_MMP_INSERT_WINDOW    
    std::clog << "\t\t\t\t\t|" << " inserting " << *candidate << std::endl;
    # endif
      
	//----|insert the candidate and its event points
	 
	result.event = insert_event_points( candidate, psc );
      
	wlist.insert( ac.second, candidate ); 
	
	
  }else{ delete candidate; candidate = 0; } // delete dominated window

  # if defined DBG_FLAT_MMP_INSERT_WINDOW
  std::clog	<< "mmp::Geodesics::insert_window\t|" << "complete - "
  			<< wlist.size() << " edge windows"
            << " - adjacent events (" 
            << (get< LEFT>(result.ac_ev) ? get< LEFT>(result.ac_ev)->window()->id : 0 ) << "<>"
            << (get<RIGHT>(result.ac_ev) ? get<RIGHT>(result.ac_ev)->window()->id : 0 ) << ")"
            << std::endl << std::flush;
  # endif

  return result;
}

// add windows on the 1-ring edges around the source vertex
void	Geodesics::initialize()
{
  # if defined DBG_FLAT_MMP_INITIALIZE
  std::clog << "mmp::Geodesics::initialize\t|"
            << "source " << source() << std::endl;
  # endif

  // initialize vertex labels to infinity
  std::fill( vertex_labels.storage_begin(), vertex_labels.storage_end(), std::numeric_limits<distance_t>::infinity() );
  
  vertex_labels[ source() ] = 0;
  
  for(auto outits = get_surface().vertex( source() ).out_edges(); outits.first != outits.second; outits.first++ )
  {	
	const edge_handle out( *outits.first, surf );
    
    Window* wout = Window::create_initial( source(), out , coord_t(0), out.length(), distance_t(0), out.length() );
    # if defined DBG_FLAT_MMP_INITIALIZE
    std::clog << "\t\t\t\t\t|"
              << " out edge  - " << *wout
              << std::endl;
    # endif
    windows[out].push_back( wout );    
    event_queue.push( new EventPoint( EventPoint::LEFT_END , wout, wout->ps_distance< LEFT>() ) );
    event_queue.push( new EventPoint( EventPoint::RIGHT_END, wout, wout->ps_distance<RIGHT>() ) );
    
    const edge_handle  ring( out.next() );
    const edge_handle  in( ring.next() );
    
    Window* wring = Window::create_initial( source(), ring, coord_t(0), ring.length(), out.length(), in.length() );
    # if defined DBG_FLAT_MMP_INITIALIZE
    std::clog << "\t\t\t\t\t|"
              << " ring edge - " << *wring
              << std::endl;
    # endif
    windows[ring].push_back( wring );
	insert_event_points( wring, wring->pseudosource() );
	
    Window* win = Window::create_initial( source(), in, coord_t(0), in.length(), in.length(), distance_t(0) );
    # if defined DBG_FLAT_MMP_INITIALIZE    
    std::clog << "\t\t\t\t\t|"
              << " in edge   - " << *win
              << std::endl;
    # endif
    windows[in].push_back( win );    
    event_queue.push( new EventPoint( EventPoint::LEFT_END , win, win->ps_distance< LEFT>() ) );
    event_queue.push( new EventPoint( EventPoint::RIGHT_END, win, win->ps_distance<RIGHT>() ) );

  }
  # if defined DBG_FLAT_MMP_INITIALIZE
  std::clog << "mmp::Geodesics::initialize\t|complete" << std::endl;
  # endif
}

void	Geodesics::handle_event( EventPoint* ev )
{
  edge_handle eh( ev->window()->edge, surf );

  # if defined DBG_FLAT_MMP_HANDLE_EVENTS
  std::clog << "mmp::Geodesics::handle_event\t|#" << *ev << std::endl;
  # endif
  
  // permanently label vertex events

  const bool  left_bound  = ( ev->flags() & EventPoint:: LEFT_END ) && ev->point() == coord_t(0);
  const bool  right_bound = ( ev->flags() & EventPoint::RIGHT_END ) && utk::close_ulps( ev->point(), eh.length() );
	
  if( left_bound || right_bound )
  { 
    distance_t& label = vertex_labels[ left_bound ? eh.source() : eh.target() ];

    # if defined DBG_FLAT_MMP_HANDLE_EVENTS
	std::clog << "\t\t\t\t\t|"
              << " labeling " << (left_bound ? eh.source() : eh.target()) << " (" << label << ")"
              << " with " << ev->distance() 
              << " error "<< ( ev->distance() - label )
              << std::endl;
    # endif
	  
    if( !std::isfinite(label) ) label = ev->distance();
    else                        assert( ev->distance() >= label || utk::close_ulps( ev->distance(), label ) );
  }

  // propagate frontier event points
  if( ev->flags() & EventPoint::FRONTIER )
    propagate_window( *ev );
}

std::pair< Geodesics::winlist_t::iterator, ev_pair_t >  Geodesics::delete_window( winlist_t::iterator loc, winlist_t& wlist )    
{ 
  # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
  // TODO: optimize - make grabber capable to erase and output matches - for_each like
  EventPoint::Grabber< event_queue_t::iterator, EventPoint::FRONTIER > 
  	  evgrab( event_queue.begin(), event_queue.end(), *loc );

  ev_pair_t ac_ev( 0,0 ); 
  if( evgrab.frontier != event_queue.end() )
	ac_ev = delete_event( evgrab.frontier );
  # endif
  event_queue.remove_if( [&loc](const EventPoint* ev)  
		  				 { if( *loc == ev->window() ) {	delete ev; return true;} return false; } );
  //----|delete the window
  delete *loc; *loc = 0;
  //----|return next window and adjacent events 
  return std::make_pair( wlist.erase(loc), ac_ev );
}


ev_pair_t	Geodesics::delete_event( event_queue_t::iterator ev )
{
  //assert( (*ev)->adjacent<LEFT>() == 0 && (*ev)->adjacent<RIGHT>() == 0 );
  ev_pair_t ac_ev = (*ev)->adjacent(); 
  delete *ev; *ev = 0;
  event_queue.erase( ev );
  return ac_ev;
}


ev_pair_t	Geodesics::delete_event( EventPoint* ev )
{		  
  event_queue_t::iterator it = std::find(event_queue.begin(), event_queue.end(), ev);
  assert( it != event_queue.end() );
  return delete_event( it );
}      


// propagate windows over the mesh 
void	Geodesics::propagate_paths()
{
  using utk::sqr;

  std::clog << "mmp::Geodesics::propagate_paths"
            << "\t|source " << source()
            << std::endl;

  initialize();

  # if defined DBG_FLAT_MMP_HANDLE_EVENTS
  std::clog << "\t\t\t\t\t|"
            << "\t| processing queue..."
            << std::endl;
  # endif
  
  //iterate - propagate all windows in outward direction
  while( step() );

  # if defined DBG_FLAT_MMP_FORCE_SANITY_CHECK
  assert( sanity_check() );
  # endif
  std::clog << "mmp::Geodesics::propagate_paths\t|" 
			<< "complete - source " << source()
            << " with " << mmp::Window::next_id << " windows created in total"
            << std::endl;
}


// propagates distance information of a single window across a triangle 
void    Geodesics::propagate_window( EventPoint& ev)
{
  Window* srcwin = ev.window();
  # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
  std::clog << "mmp::Geodesics::propagate_window"
           << "\t|*propagate " << *srcwin
           << std::endl;
  # endif
  //----|edge information
  
  const edge_handle e0 ( srcwin->edge, surf );

  // TODO: remove this
  if( !srcwin->is_valid() )
  {
    # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
    std::clog << "mmp::Geodesics::propagate_window"
              << "\t|complete - window is invalid"
              << std::endl;
    # endif

    
    # if defined FLAT_MMP_MAINTAIN_WAVEFRONT 
    // pull coupled event point to current edge before ev gets destroyed
    pull_event <  LEFT > ( ev );
    pull_event < RIGHT > ( ev );

    ev.decouple();
    # endif
    
    return;
  }
    
  // TODO: check at window creation
  if( ! e0.opposite().second ) 
  { 
    # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
    std::clog << "mmp::Geodesics::propagate_window"
              << "\t|complete - no opposite edge"
              << std::endl;
    # endif

    
    # if defined FLAT_MMP_MAINTAIN_WAVEFRONT 
    // pull coupled event point to current edge before ev gets destroyed
    pull_event <  LEFT > ( ev );
    pull_event < RIGHT > ( ev );

    ev.decouple();
    # endif
    
    return;
  }

  // the opposite edges
  const edge_handle e1 ( e0.opposite().first.next() );
  const edge_handle e2 ( e1.next() );

  const coord_t e0l = e0.length();
  const coord_t e1l = e1.length();
  const coord_t e2l = e2.length();

  //----|2d triangle reconstruction

  //			/ e0 \
  //   (0,0) = A---->B = (|e0|,0)
  //           <-----^
  // 		    \   / 
  // 		  e1 \ / e2
  //     		  v C = (xc,yc) - reconstructed using |e1| and |e2|
  
  const coord2_t A( coord_t(0), coord_t(0) );
  const coord2_t B( e0l, coord_t(0) );

  // coordinates of C - using circle-circle intersection ( intersect circle (w.b0,w.d0) with (w.b1,w.d1) )
  coord_t xc, yc; 
  boost::tie(xc,yc) = utk::triangulate( e0l, e1l, e2l );
  const coord2_t C( xc, -yc );

  utk::ray<coord_t,2> e1ray( A, C );
  utk::ray<coord_t,2> e2ray( C, B );
  
  assert( utk::close_ulps( e1ray.length(), e1l) );
  assert( utk::close_ulps( e2ray.length(), e2l) );

  // normalize such that parameter refelects position on edge
  e1ray.direction().normalize();
  e2ray.direction().normalize();

  //----|project window

  //  <-LEFT RIGHT->      
  
  // ps						//			   ps	
  //   \ b0					//			 b0/ \b1	
  // A--\--->B				//		   A--/---\->B
  //  \  \  /				//         _\/ e0  \/_ 
  //   \  \/_,C+e2b0*(B-C)	//A+e1b0*e1 /\     /\ `C+e2b1*e2
  //    \ /\ 				//  	C-A=e1\   /e2=B-C    
  //     C  \				//         	   \ /     
                            //	            C      

  // intersection of rays from pseudosource (through left/right window bounds) with the opposite edges
  // and with the line parallel to e0 touching C
	
  const bool  source_is_inner_sidelobe = is_inner_sidelobe( *srcwin ); //ps_close_to_edge(wps);

  ps_t source_ps;
  
  Window* w11 = 0; // projected on e1      A------B
  Window* w12 = 0; // side lobe on e1	w12 \    / w22
  Window* w21 = 0; // projected on e2	     \  /
  Window* w22 = 0; // side lobe on e2	   w11\/w21

  insert_result  e11, e12, e21, e22;
  
  if( source_is_inner_sidelobe )
  { 
	# if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
    std::clog << "\t\t\t\t\t| pseudosource at vertex - cover all edges" << std::endl;
	# endif

    if( srcwin->has_ps_vertex< LEFT >() )
    { 
      source_ps = ps_t{ ps_coord_t(0), 0 };
      
      // outside
      w22 = Window::create_side_lobe< LEFT >( srcwin, e2, coord_t(0), e2l, e1l, e0l, e0.source() );

      e22 = insert_window( w22, e2ray, source_ps );
      
      if( e22.event && !get<LEFT>( e22.ac_ev ) ) 
      { 
        // inside
        w12 = Window::create_side_lobe< LEFT >( srcwin, e1, coord_t(0), e1l, distance_t(0), e1l, e0.source() );

        e12 = insert_window( w12, e1ray, source_ps );
      }
  
    }else
    { 
      source_ps = ps_t{ e0l, 0 };
      
      assert( srcwin->has_ps_vertex< RIGHT >() );
      
      // outside
      w12 = Window::create_side_lobe< RIGHT >( srcwin, e1, coord_t(0), e1l, e0l, e2l, e0.target() );      

      e12 = insert_window( w12, e1ray, source_ps );

      if( e12.event && !get<RIGHT>( e12.ac_ev ) ) 
      { 
        // inside
        w22 = Window::create_side_lobe< RIGHT >( srcwin, e2, coord_t(0), e2l, e2l, distance_t(0), e0.target() );

        e22 = insert_window( w22, e2ray, source_ps );
      }
    }
  }
  else //----| project windows
  {

    source_ps = srcwin->pseudosource();
    
    coord_t 		bound0, bound1;
    edge_descriptor edge0,  edge1;
    
    std::tie( bound0, edge0 ) = project_bound<LEFT >( ev, e1, e2, e1ray, e2ray, e0l, e1l, e2l, source_ps );
  
    std::tie( bound1, edge1 ) = project_bound<RIGHT>( ev, e1, e2, e1ray, e2ray, e0l, e1l, e2l, source_ps );

    # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
    std::clog << "\t\t\t\t\t|" << " e1ray " << e1ray << std::endl;
    std::clog << "\t\t\t\t\t|" << " e2ray " << e2ray << std::endl;

    std::clog << "\t\t\t\t\t|"
              << " A = " << A
              << " B = " << B
              << " C = " << C	  		  
              << " | e0l" << e0.descriptor() << " = " << e0l  
              << " e1l" << e1.descriptor() << " = " << e1l << " error " << ( (C-A).length() - e1l )
              << " e2l" << e2.descriptor() << " = " << e2l << " error " << ( (B-C).length() - e2l )
              << std::endl;
  
    std::clog << "\t\t\t\t\t|"
              << " ps = " << source_ps
		  	  << " boundary e1 " << !e1.opposite().second << " e2 " << !e1.opposite().second
              << std::endl;
    # endif
  
    // TODO: reenable assert( bound0 <= bound1 || (edge0 == e1.descriptor() && edge1 == e2.descriptor()) );

    // window vanished TODO: reenable bounds equal
    if( bound0 >= bound1 && edge0 == edge1 ) 
    {
      # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
      std::clog << "\t\t\t\t\t| projected window is empty" << std::endl;
      # endif

      # if defined FLAT_MMP_MAINTAIN_WAVEFRONT 
      ev.couple_adjacent();
      # endif
      return;
    }

    //----|create projected windows

    if( edge1 == e2.descriptor() )
    {        
      const distance_t distance1 = utk::distance( source_ps, e2ray.at(bound1) );

	  if( edge0 == e1.descriptor() ) // two windows
      { 
		# if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
        std::clog << "\t\t\t\t\t| two windows" << std::endl;
		# endif
        
        const distance_t distance0 = utk::distance( source_ps, e1ray.at(bound0) );
        const distance_t distanceC = utk::distance( source_ps, C );
		
        w11 = Window::create_projected( srcwin, e1, bound0, e1l, distance0 ,distanceC );
        w21 = Window::create_projected( srcwin, e2, coord_t(0.), bound1, distanceC, distance1 );

      }else // window on e2
      { 
		# if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
        std::clog << "\t\t\t\t\t| window on e2" << std::endl;
		# endif

		assert( edge0 == e2.descriptor() );
        const distance_t distance0 = utk::distance( source_ps, e2ray.at(bound0) );
		
        w21 = Window::create_projected( srcwin, e2, bound0, bound1, distance0, distance1);
      }
    }else // window on e1
    { 
	  # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW        
      std::clog << "\t\t\t\t\t| window on e1" << std::endl;
	  # endif
  	  assert( edge0 == e1.descriptor() || edge1 == e1.descriptor() );	
      const distance_t distance0 = utk::distance( source_ps, e1ray.at(bound0) );
      const distance_t distance1 = utk::distance( source_ps, e1ray.at(bound1) );

	  w11 = Window::create_projected( srcwin, e1, bound0, bound1, distance0, distance1 );
    }
  
    //----|create side-lobes 
    //     in uncovered area if frontier point touches a vertex
    //     or the window includes a boundary vertex
    
    const coord_t fp = srcwin->frontier_point( source_ps );
    
    const bool fp_left  = fp == 0;
    const bool fp_right = utk::close_ulps( fp, e0l );
    
    const bool e1boundary = !e1.opposite().second;
    const bool e2boundary = !e2.opposite().second;

    //----|left side lobe

    if( edge0 == e2.descriptor() && ( fp_left || ( e1boundary && srcwin->bound<LEFT>() == coord_t(0) ) ) )
    {
      # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
      std::clog << "\t\t\t\t\t| left side lobe"
                << ", angle at b0 = " << (e0.source().total_angle()/(2.*M_PI)) << "pi"
	  	  	    << ", e1 is boundary " << e1boundary << std::endl;
      # endif
	
      assert( bound0 > 0 ); // TODO: do we need this 'if'?
      { // outer sidelobe
        const distance_t distance0 =  w21 ? utk::distance( A, e2ray.at(bound0) ) : e0l;
        w22 = Window::create_side_lobe<LEFT>( srcwin, e2, coord_t(0), w21 ? w21->bound<LEFT>() : e2l, e1l, distance0, e0.source() );
      }
	  // inner sidelobe
      w12 = Window::create_side_lobe<LEFT>( srcwin, e1, coord_t(0), e1l, distance_t(0), e1l, e0.source());
    }
  
    //----|right side lobe

    if( edge1 == e1.descriptor() && ( fp_right || ( e2boundary && srcwin->bound<RIGHT>() == e0l) ) )	
    { 																				    
	  # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW											
      std::clog << "\t\t\t\t\t| right side lobe"											
	            << ", angle at b1 = " << (e0.target().total_angle()/(2.*M_PI)) << "pi"	
	  	  	    << ", e2 is boundary " << e2boundary << std::endl;
	  # endif

	  assert( bound1 < e1l ); // TODO: do we need this 'if'
      { // outer sidelobe
        const float distance1 = w11 ? utk::distance( B, e1ray.at(bound1) ) : e0l;  
        w12 = Window::create_side_lobe<RIGHT>( srcwin, e1, w11 ? w11->bound<RIGHT>() : coord_t(0), e1l, distance1 ,e2l, e0.target() );
      }
	  // inner sidelobe
      w22 = Window::create_side_lobe<RIGHT>( srcwin, e2, coord_t(0) ,e2l, e2l, distance_t(0), e0.target() );	
    }

    //----|insert windows

    if( w12 ) e12 = insert_window( w12, e1ray, source_ps );
    if( w22 ) e22 = insert_window( w22, e2ray, source_ps );
  
    if( w11 ) e11 = insert_window( w11, e1ray, source_ps );
    if( w21 ) e21 = insert_window( w21, e2ray, source_ps );
  }
  
  # if defined FLAT_MMP_MAINTAIN_WAVEFRONT
  //----|couple eventpoints
    
  // left edge (e1) 
  couple_edge_events<  LEFT >( &ev, e11, e12, w21 != 0, e1, e2l, e0l );
  // right edge (e2) 
  couple_edge_events< RIGHT >( &ev, e21, e22, w11 != 0, e2, e1l, e0l );
  # endif

  # if defined DBG_FLAT_MMP_PROPAGATE_WINDOW
  std::clog << "mmp::Geodesics::propagate_window" << "\t|complete - " 
			<< ( int( w11 != 0 ) + int( w12 != 0 ) + int( w21 != 0 ) + int( w22 != 0 ) ) << " candidate(s) -> "
   			<< ( int( e11.event != 0 ) + int( e12.event != 0 ) + int( e21.event != 0 ) + int( e22.event != 0 ) ) << " new event(s)"
            << std::endl;
  #endif
}


coord_t Geodesics::backtrace( const Window& window, const coord_t& window_point )	const
{
  assert( contained( window_point, window ) );
  
  Window*const pre = window.predeccessor();

  // return predeccessors bounds if window is a ps window
  if( window.has_ps_vertex<LEFT>() ) 
  {	assert( pre->bound<LEFT>() == 0 );
	return 0;
  }

  if( window.has_ps_vertex<RIGHT>() ) 
  	return pre->bound<RIGHT>();

  // base edge (contains the windows predeccessor)
  const edge_handle e0 ( pre->edge, get_surface() ); assert( e0.opposite().second );
  const coord_t e0l= e0.length();
  
  // is the window a OUTSIDE-sidelobe ?
  const edge_handle we ( window.edge, get_surface() );
  if( window.ps == we.next().target() ) 
	return we.next() == e0.opposite().first ? coord_t(0) : e0l;
    
  // the opposite edges
  const edge_handle e1 ( e0.opposite().first.next() );
  const edge_handle e2 ( e1.next() );

  const coord_t e1l= e1.length();
  const coord_t e2l= e2.length();

  //----|2d triangle reconstruction

  //			/ e0 \
  //   (0,0) = A---->B = (|e0|,0)
  //           <-----^
  // 		    \   / 
  // 		  e1 \ / e2
  //     		  v C = (xc, -yc) - reconstructed using |e1| and |e2|
  
  const coord2_t A( coord_t(0), coord_t(0) );
  const coord2_t B( e0l, coord_t(0) );

  // coordinates of C - using circle-circle intersection
  coord_t xc, yc; boost::tie(xc,yc) = utk::triangulate( e0l, e1l, e2l );
  const coord2_t C( xc, -yc );

  //----|find 2d window point 

  const utk::ray< coord_t, 2 > edge_ray = ( window.edge == e1.descriptor() ? utk::ray<coord_t, 2>(A,C) : utk::ray<coord_t, 2>(C,B) );

  const coord2_t window_point2 = edge_ray.at_arc_length( window_point );
  
  //----|intersect ray on baseline with plane (representing the ray from pseudosource to window_point2 )

  const ps_t pre_ps = pre->pseudosource();

  const utk::plane<coord_t,2> plane = utk::plane_from_ray( utk::ray<coord_t,2>( window_point2, pre_ps ) );
  utk::ray< coord_t, 2 > baseline_ray( A, B );
  coord_t pre_point = utk::intersection( baseline_ray , plane ) * baseline_ray.length();

  # if defined DBG_FlAT_MMP_GEODESICS_BACKTRACE
  std::cout << "mmp::Geodesics::backtrace\t| win point " << window_point << " -> pre point " << pre_point 
            << " with ps " << pre_ps << " and window point " << window_point2 << std::endl
            << "\t\t\t\t| pre " << *pre << std::endl
            << "\t\t\t\t| win " << window << std::endl;
  # endif

  /**/ utk::clamp( pre_point, pre->bound<LEFT>(), pre->bound<RIGHT>() );
  assert( contained( pre_point, *pre ) ); 

  return pre_point;
}

std::list< coord_t >&    Geodesics::backtrace( const Window& window, std::list< coord_t >& trace ) const
{
  assert( !trace.empty() );
  coord_t pre = backtrace( window, trace.front() );

  if( !window.predeccessor() ) return trace;
  
  trace.push_front( pre );
  return backtrace( *window.predeccessor(), trace );
}

// retrieve distance from source to destination vertex
distance_t	Geodesics::query_distance( const vertex_descriptor& target ) const
{
  const distance_t dist = vertex_labels[target];

  # if defined DBG_FLAT_MMP_QUERY_DISTANCE
  std::clog << "mmp::Geodesics::query_distance\t|"
            << " minimal distance " << dist
			<< " from " << source()	<< " to " << target
	        << std::endl<<std::flush;
  # endif
  
  return dist;
}


/* retrieve distance from source to point on edge
distance_t	Geodesics::query_minimal_distance(const edge_descriptor& targetedge) const
{
  distance_t dmin = std::numeric_limits<distance_t>::infinity();

  for(auto outits=boost::out_edges(target,surf.m_graph); outits.first!=outits.second; outits.first++)
  {
    const Window& wfront = *windows[*outits.first].front();
    distance_t td = wfront.b0_source_distance();    
    if(td<dmin) 
    { dmin=td;
      std::clog <<"mmp::Geodesics::query_distance"
                <<"\t| distance candidate (out) - "<<wfront
	            <<std::endl;
    }
    
    const Window& wback = *windows[edge_handle(*outits.first,surf).previous()].back();
    td = wback.b1_source_distance();    
    if(td<dmin) 
    { dmin=td;
      std::clog <<"mmp::Geodesics::query_distance"
                <<"\t| distance candidate (in)  - "<<wback
	            <<std::endl;
    }
  }
  std::clog <<"mmp::Geodesics::query_distance"
            <<"\t| to "<<target
            <<"\t| minimal distance "<<dmin
	        <<std::endl<<std::flush;
  assert(std::isfinite(dmin));
  return dmin;
}*/


// check if all edges are fully and consistently covered
bool  Geodesics::sanity_check()  const
{
  std::clog << "mmp::Geodesics::sanity_check\t| "
			<< get_surface().num_edges() << " edges"  
	        << std::endl;

  bool ok = true;

  const std::string dbg_prefix = boost::lexical_cast< std::string >( source() ) + "_insane";
  
  if( !event_queue.empty() )
  { 
	std::clog << "mmp::Geodesics::sanity_check\t|"
              << "FAILED - there are unpropagated windows in the queue"
              << std::endl;
    ok = false;
  }
  
  for( auto eits = surf.edge_handles(); eits.first != eits.second; eits.first++ ) 
  { 
	const winlist_t& wlist = windows[ *eits.first ];

    const std::pair< edge_handle, bool > eop = eits.first->opposite();

    const edge_handle& eh( *eits.first );
    std::clog << "mmp::Geodesics::sanity_check\t|"
              << "?checking " << *eits.first << " with " << wlist.size() << " windows"
              << std::endl;
    
    if( wlist.empty() )
    { 
      if( eh.next().opposite().second || eh.previous().opposite().second )
      { 
        std::clog << "mmp::Geodesics::sanity_check\t|";
        if( eop.second && !windows[eop.first].empty() )
		{ std::clog << "WARNING - uncovered ( with opposite )" << std::endl; }
        else
        { std::clog << "FAILED - uncovered" << std::endl;
          ok = false;
        }
      }else
      {
        std::clog << "mmp::Geodesics::sanity_check\t|"
                  << " isolated "
                  << std::endl;
        continue;
      }  
    }else
	{
      for( winlist_t::const_iterator it = wlist.begin(); it != wlist.end(); ++it )
      { 
		assert( *it );
        std::clog << "\t\t\t\t\t| " << **it
                  << std::endl << std::flush;
	   
        if( !(*it)->sanity_check() )
        { std::cerr << "mmp::Geodesics::sanity_check\t|"
                    << "FAILED - sanity_check failed for " << *eits.first
                    << std::endl;
		  std::clog << "mmp::Geodesics::sanity_check\t|"
                    << "FAILED - sanity_check failed for " << *eits.first
                    << std::endl;
          ok = false;
        }
        
        winlist_t::const_iterator nextit = it;
        advance(nextit,1);

		if( nextit!=wlist.end() )
        { // check for overlaps and gaps
          if( (*it)->bound<RIGHT>() != (*nextit)->bound<LEFT>() )
          { const coord_t bdiff = (*it)->bound<RIGHT>() - (*nextit)->bound<LEFT>();
				
            std::cerr << "mmp::Geodesics::sanity_check\t|";
            std::cerr << "FAILED - ";
			std::clog << "FAILED - ";
            ok = false;
				
            std::cerr << (bdiff < coord_t(0.) ? "gap":"overlap") << " of " << fabs(bdiff) << " between:"
                      << std::endl << "\t\t\t\t\t| " << **it
                      << std::endl << "\t\t\t\t\t| " << **nextit
                      << std::endl;
			  
            std::clog << (bdiff < coord_t(0.) ? "gap":"overlap") << " of " << fabs(bdiff) << " between:"
                      << std::endl << "\t\t\t\t\t| " << **it
                      << std::endl << "\t\t\t\t\t| " << **nextit
                      << std::endl;

            # if defined DBG_MMP__USE_CAIRO
            visualizer::cairo::draw_edge_sequences_to_file( it, ++decltype(nextit)(nextit), *this, dbg_prefix+"_gap_overlap" );
            # endif
          }

          // check for nonintersecting access channels
          if( (*it)->predeccessor() && (*nextit)->predeccessor()   
              && !( // channel ok?
                    (*it)->frontier_point() <= (*nextit)->frontier_point() // are the frontier points in order
                     && (    (    (*it)->predeccessor()->edge != (*nextit)->predeccessor()->edge  
                               && eh.source() == edge_handle( (*it)->predeccessor()->edge, surf ).source() 
                             ) 
                          || (*it)->predeccessor() == (*nextit)->predeccessor()
                          || (*it)->predeccessor()->frontier_point() <= (*nextit)->predeccessor()->frontier_point() )         
                  ) 
            )
          {
            std::cerr << "mmp::Geodesics::sanity_check\t|"
                      << "FAILED"  << " - crossing of optimal paths"
                      << std::endl << "\t\t\t\t\t| " << **it
                      << std::endl << "\t\t\t\t\t| " << **nextit << std::endl;

            std::clog << "mmp::Geodesics::sanity_check\t|"
                      << "FAILED" << " - crossing of optimal paths" << std::endl;
			
			if( (*it)->predeccessor() )  
			  std::clog << "\t\t\t\t\t| pred " <<     *(*it)->predeccessor()<< std::endl;
			if( (*nextit)->predeccessor() )  
              std::clog << "\t\t\t\t\t| pred " << *(*nextit)->predeccessor()<< std::endl;
			
            std::clog << "\t\t\t\t\t| " << **it     << std::endl
                      << "\t\t\t\t\t| " << **nextit << std::endl;

            # if defined DBG_MMP__USE_CAIRO
        	visualizer::cairo::draw_windows_to_file( it, ++decltype(nextit)(nextit), dbg_prefix+"_ac" );
            # endif	
            ok = false;
          }
            
          // check continuity of distance function
          const distance_t wleftd0 = (*it)->source_distance<RIGHT>();
          if( !dist_snap_check( wleftd0, (*nextit)->source_distance<LEFT>()) )
          { const distance_t ddiff = (*nextit)->source_distance<LEFT>() - wleftd0;
            std::cerr << "mmp::Geodesics::sanity_check"
                      << "\t|FAILED"
                      << " - jump of " << ddiff << " in distance function between:"
                      << std::endl << "\t\t\t\t\t| " << **it
                      << std::endl << "\t\t\t\t\t| " << **nextit
                      << std::endl;

            std::clog << "mmp::Geodesics::sanity_check"
                      << "\t|FAILED"
                      << " - jump of " << ddiff << " in distance function between:"
                      << std::endl << "\t\t\t\t\t| " << **it
                      << std::endl << "\t\t\t\t\t| " << **nextit
                      << std::endl;

            # if defined DBG_MMP__USE_CAIRO
        	visualizer::cairo::draw_edge_sequences_to_file( it, ++decltype(nextit)(nextit), *this, dbg_prefix+"_jump" );
            # endif
            
            ok = false;
          }
            
        } // window has successor
      }
      
      if( wlist.front()->bound<LEFT>() != 0 )
      { 
		std::cerr << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - gap at beginning (b0=" << wlist.front()->bound<LEFT>() << ") of " << (*eits.first)
                  << std::endl;

		std::clog << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - gap at beginning (b0=" << wlist.front()->bound<LEFT>() << ") of " << (*eits.first)
                  << std::endl;

        # if defined DBG_MMP__USE_CAIRO
        visualizer::cairo::draw_edge_sequence_to_file( *wlist.front(), *this, dbg_prefix+"_gap_begin" );
        # endif
        
        ok = false;
      }

      if( !std::isfinite( vertex_labels[(*eits.first).source()] ) )
      {
        std::cerr << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - infinite vertex label (left)"
                  << std::endl;

        std::clog << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - infinite vertex label (left)"
                  << std::endl;
		
        ok = false;
      }
      
      if( !dist_snap_check( wlist.front()->source_distance<LEFT>(), vertex_labels[ eits.first->source() ] )
          && (   !eop.second  
              || windows[ eop.first ].empty() 
              || !dist_snap_check( windows[ eop.first ].back()->source_distance<RIGHT>()
                                 , vertex_labels[ eop.first.target() ] )
             )
        )
      { 
		std::cerr << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - distance mismatch - left endpoint"
                  << " (" << wlist.front()->source_distance<LEFT>() << ")"
                  << " vertex (" << vertex_labels[ eits.first->source() ] << ")"
                  << " error " << ( vertex_labels[ eits.first->source() ] - wlist.front()->source_distance<LEFT>() )
                  << std::endl;

		std::clog << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - distance mismatch - left endpoint"
                  << " (" << wlist.front()->source_distance<LEFT>() << ")"
                  << " vertex (" << vertex_labels[ eits.first->source() ] << ")"
                  << " error " << ( vertex_labels[ eits.first->source() ] - wlist.front()->source_distance<LEFT>() )
                  << std::endl;
		
        ok = false;
      }
      
      if( !utk::close_ulps( wlist.back()->bound<RIGHT>(), eh.length() ) )
      { 
		std::cerr << "mmp::Geodesics::sanity_check\t|"
                  << "FAILED - gap at end (b1=" << wlist.back()->bound<RIGHT>() << ")"
				  << " of " << *eits.first
                  << std::endl;

		std::clog << "mmp::Geodesics::sanity_check\t|"
                  << "FAILED - gap at end (b1=" << wlist.back()->bound<RIGHT>() << ")"
				  << " of " << *eits.first
                  << std::endl;
		# if defined DBG_MMP__USE_CAIRO
        visualizer::cairo::draw_edge_sequence_to_file( *wlist.back(), *this, dbg_prefix+"_gap_end" );
        # endif
		  
        ok = false;
      }

      if( !std::isfinite( vertex_labels[ eits.first->target() ] ) )
      {
        std::cerr << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - infinite vertex label (right)"
                  << std::endl;

        std::clog << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - infinite vertex label (right)"
                  << std::endl;
		
        ok = false;
      }

      
      if( !dist_snap_check( wlist.back()->source_distance<RIGHT>(), vertex_labels[ eits.first->target().descriptor() ] ) 
          && (  !eop.second  
              || windows[eop.first].empty() 
              || !dist_snap_check( windows[eop.first].front()->source_distance<LEFT>(), vertex_labels[eop.first.source()] )
             )
        )
      { 
		std::cerr << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - distance mismatch - right endpoint "
                  << " (" << wlist.back()->source_distance<RIGHT>() << ")"
                  << " vertex (" << vertex_labels[ eits.first->target().descriptor() ] << ")"
                  << " error " << ( vertex_labels[ eits.first->target().descriptor() ] - wlist.back()->source_distance<RIGHT>() )
                  << std::endl;

		std::clog << "mmp::Geodesics::sanity_check"
                  << "\t|FAILED - distance mismatch - right endpoint "
                  << " (" << wlist.back()->source_distance<RIGHT>() << ")"
                  << " vertex (" << vertex_labels[ eits.first->target().descriptor() ] << ")"
                  << " error " << ( vertex_labels[ eits.first->target().descriptor() ] - wlist.back()->source_distance<RIGHT>() )
                  << std::endl;
		
        ok = false;
      }
      
	} // end of covered
  }

  if( ok )
  {	std::cout << "mmp::Geodesics::sanity_check"
		      << "\t| completed successfully"
		      << std::endl;
	std::clog << "mmp::Geodesics::sanity_check"
		      << "\t| completed successfully"
		      << std::endl;
  }else
  {
    std::cerr << "mmp::Geodesics::sanity_check"
	  	      << "\t| FAILED"
		      << std::endl;
    std::clog << "mmp::Geodesics::sanity_check"
		      << "\t| FAILED"
		      << std::endl;
  }
  return ok;
}

bool mmp::vertex_pair_check( const std::shared_ptr< Surface >& surface, const distance_t& tolerance )
{
  const size_t vertex_count = surface->num_vertices();
  
  boost::numeric::ublas::symmetric_matrix< distance_t > dists_full( vertex_count, vertex_count );

  for( PointCloud::vertex_descriptor i = 0; i < vertex_count; i++ )
  { 
    mmp::Geodesics gi( *surface, i );

	gi.propagate_paths();
    
    for( PointCloud::vertex_descriptor j = 0; j < vertex_count; j++)  dists_full( i, j ) = gi.query_distance( j );
  }

  bool distance_failed = false;
  
  using namespace boost::accumulators;
  accumulator_set< distance_t, features< tag::min, tag::max, tag::mean > > abs_acc;
  accumulator_set< distance_t, features< tag::min, tag::max, tag::mean > > rel_acc;
  
  for( PointCloud::vertex_descriptor i = 0; i < vertex_count; i++ )
    for( PointCloud::vertex_descriptor j = i+1; j < vertex_count; j++)
    {
      const bool equal_distance = dists_full( i, j ) == dists_full( j, i );
      const distance_t  diff    = dists_full( i, j ) - dists_full( j, i );

      abs_acc( diff );
      rel_acc( diff / std::min( dists_full( i, j ), dists_full( j, i ) ) );
      
      std::cout << "mmp::vertex_pair_check\t| "
                << "(" << i << ',' << j << ") ";
      if( equal_distance ) 
        std::cout << "equal " << dists_full( i, j ) << std::endl;
      else
      {
        if( std::fabs( diff ) > tolerance )
        {
          std::cout << "FAILED " << std::endl;
          distance_failed = true;
        }

        std::cout << "diff " <<  diff
                  << " (ij) " << dists_full( i, j )    
                  << " (ji) " << dists_full( j, i )
                  << std::endl;
      } 
    }

  std::cout << "mmp::vertex_pair_check\t| " 
            << ( distance_failed ? "FAILED" : "" ) 
            << "abs error: min " << min( abs_acc ) << " mean " << mean( abs_acc ) << " max " << max( abs_acc )
            << std::endl << "\t\t\t\t"
            << "rel error: min " << min( rel_acc ) << " mean " << mean( rel_acc ) << " max " << max( rel_acc )
            << std::endl;
  
  return distance_failed;
}

