/***************************************************************************
 *            mmp-trim_ac.h
 *
 *  Wed Jan  5 16:14:18 2011
 *  Copyright  2011  urp
 *  <urp@<host>>
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

//# define DBG_FLAT_MMP_TRIM_AC
//# define DBG_FLAT_MMP_WINDOW_BISECTOR
//# define DBG_FLAT_MMP_CHECK_BISECTOR
//# define DBG_FLAT_MMP_CHECK_BISECTOR__USE_CAIRO
//# define DBG_FLAT_MMP_DOMINATE_CROSSING

# include "mmp-common.h"
# include "mmp-geodesics.h"

  namespace mmp
  {

    struct bisector_result
    {
      typedef enum { CANDIDATE_DOMINATES = 1, TRIM = 2, AC_WINDOW_DOMINATES = 4 } result_type;

      coord_t       point;
      result_type   type;    
    };

    
    template< side_t Side >
    bisector_result	window_bisector( const Window&, const Window&, const ps_t&, const ps_t& );	

	// finds the candidates 'interval of optimality' in the access channel.
	// trims or cuts all windows dominated by the candidate interval.
	// returns the connected neighbor-event
	template< side_t Side >
	std::pair< EventPoint*, bool >	trim_ac( Window* candidate
                                           , Geodesics::ac_t& ac
                                           , Geodesics::winlist_t& wlist
                                           , const utk::ray<coord_t,2>& edge_ray
                                           , const ps_t& pred_ps, const ps_t& psc, ps_t& psac
                                           , Geodesics& geodesics );

    template< side_t Side >
    bool  check_bisector( const bisector_result& bisector
                        , Window& candidate, const Window& ac_window
                        , const ps_t& candidate_ps, const ps_t& ac_ps );

  }


// IMPLEMENTATION


  namespace mmp
  {
	namespace impl
	{

      coord_t linear_linear_window_bisector( const Window& a , const Window& b
		                                   , const ps_t&   a_ps, const ps_t&   b_ps );
	 
	  coord_t linear_curved_window_bisector( const Window& linear , const Window& curved
		                                   , const ps_t&   linear_ps, const ps_t&   curved_ps );
	  
	  std::pair< coord_t, coord_t > curved_curved_window_bisector( const Window& a , const Window& b
		                                                         , const ps_t&   a_ps, const ps_t&   b_ps );

	  template< side_t Side > struct trim_ac_bounds_check {};

	  template< > struct trim_ac_bounds_check<LEFT> 
	  { static std::pair< std::greater_equal<coord_t>, std::greater<coord_t> > checks()
		{ return std::make_pair( std::greater_equal<coord_t>(), std::greater<coord_t>() ); }
	  };

	  template< > struct trim_ac_bounds_check<RIGHT> 
	  { static std::pair< std::less_equal<coord_t>, std::less<coord_t> > checks()
		{ return std::make_pair( std::less_equal<coord_t>(), std::less<coord_t>() ); }
	  };  
		
      template< side_t Side >
      distance_t is_crossing_dominated( const Window& candidate, const Geodesics::ac_t& ac, const utk::ray<coord_t,2>& edge_ray
                               	      , const ps_t& pred_ps, const ps_t& candidate_ps, const ps_t& ac_ps )
      {
        const Window&  ac_window = **get< Side >(ac);
		const coord_t& ac_bound  = ac_window.bound<Side>();

		assert( !has_intersection( candidate, ac_window ) );
	   
        // check distance from ac_check to closest candidate point
        const auto ac_check = std::make_pair( ac_window.bound< Side >(), ac_window.source_distance< Side >() ); //ac_window.min_source_distance( ac_ps );
        
        distance_t  ac_distance   = ac_check.second;
        distance_t  candidate_distance;
        
        assert( !is_inner_sidelobe( candidate ) ); 
        
        if( is_outer_sidelobe( candidate) ) // use candidate coordinates
          candidate_distance = utk::distance( candidate_ps, coord2_t( ac_check.first, 0 ) ) + candidate.subpath();
        else // use candidate-source window coordinates
        {
          assert( candidate.predeccessor() );
          const Window& pred_window = *candidate.predeccessor();
          
          const coord2_t ac_point = edge_ray.at_arc_length( ac_check.first );

          const coord2_t pred_point( std::max( pred_window.bound<LEFT>(), std::min( ac_point[0], pred_window.bound<RIGHT>() ) ) ,0 );
          
          candidate_distance = utk::distance( pred_point, ac_point ) + pred_window.source_distance( pred_point[0], pred_ps );
        }
        
        const bool candidate_dominates = candidate_distance <= ac_distance;
        # if defined DBG_FLAT_MMP_DOMINATE_CROSSING
        std::clog << " mmp::impl::is_crossing_dominated\t| (" << side_traits< Side >::string() << ')' 
                  << " candidate " << ( candidate_dominates ? "dominates" : " dominated by " )
                  << " ac " << ac_window << " (diff " << ( candidate_distance - ac_distance ) << ")" << std::endl;
        # endif
        
		return candidate_dominates;
      }
	  
	} // of namespace impl
  } // of namespace mmp


template< mmp::side_t Side >
std::pair< mmp::EventPoint*, bool >
mmp::trim_ac( Window* candidate
                  , Geodesics::ac_t& ac
                  , Geodesics::winlist_t& wlist
                  , const utk::ray<coord_t,2>& edge_ray
                  , const ps_t& pred_ps
                  , const ps_t& psc, ps_t& psac
                  , Geodesics& geodesics )
{

  auto boundscheck = impl::trim_ac_bounds_check<Side>::checks();
  const side_t OppSide = side_traits<Side>::opposite;

  Geodesics::ac_t listend( wlist.rend(), wlist.end() );

  std::pair< EventPoint*, bool > ac_trim = { 0, false };

  // TODO: assert that no windows are deleted if candidate is not inserted

  
  // delete dominated ac windows at "Side"
  while( get<Side>(ac) != std::get<Side>(listend)
	     && boundscheck.first( (*get<Side>(ac))->template bound<Side>(), candidate->bound<Side>() ) )
  { 
	const Window&  ac_window = **get<Side>(ac);
	const coord_t& ac_side_bound = ac_window.bound<Side>();

    // inner sidelobes can not have the same ps
    assert( ! ( ( candidate->has_ps_vertex< Side >() && ac_window.has_ps_vertex< Side >() )
              ||( candidate->has_ps_vertex< OppSide >() && ac_window.has_ps_vertex< OppSide >() ) ) ); 

    
	if( boundscheck.first( candidate->bound<OppSide>(), ac_side_bound ) ) // is ac opp-bound contained in candidate?
	{ 
      assert( contained( ac_side_bound, *candidate ) );
	  if( candidate->source_distance( ac_side_bound, psc ) > ac_window.source_distance< Side >() )
	    break; // -> trim
	}else // disjoint crossing 
	{ 
	  assert( !contained( ac_side_bound, *candidate ) );
	  // delete either candidate or ac window		  
	  if( !impl::is_crossing_dominated<Side>( *candidate, ac, edge_ray, pred_ps, psc, ac_window.pseudosource() ) )
	  {	
		# if defined DBG_FLAT_MMP_TRIM_AC
		std::clog << "mmp::trim_ac\t\t\t|" << " (" << side_traits<Side>::string() << ')'
        	      << " candidate dominated by ac-" << **get<Side>(ac) << std::endl;
		# endif

        candidate->invalidate();
        return ac_trim;
	  }
	}
	
	# if defined DBG_FLAT_MMP_TRIM_AC
	std::clog << "mmp::trim_ac\t\t\t|" << " (" << side_traits<Side>::string() << ')'
              << " delete dominated ac-"  << **get<Side>(ac) << std::endl;
	# endif

	ac_trim.first = geodesics.delete_ac_window<Side>( ac, wlist, candidate );

	//TODO: optimize updates
	listend = std::make_pair( wlist.rend(), wlist.end() );
  }

 
  if( get<Side>(ac) == get<Side>(listend) ) { return { 0, false }; }

  // trim ac window at "Side"
  
  Window& ac_window = **get<Side>(ac);

  // inner sidelobes can not have the same ps
  assert( ! ( ( candidate->has_ps_vertex< Side >() && ac_window.has_ps_vertex< Side >() )
            ||( candidate->has_ps_vertex< OppSide >() && ac_window.has_ps_vertex< OppSide >() ) ) ); 

  //assert( boundscheck.first( candidate->bound<Side>(), ac_window.bound<Side>() ) );
  
  if( boundscheck.second( ac_window.bound< OppSide >(), candidate->bound<Side>() ) )
  { 
	assert( has_intersection( *candidate, ac_window ) );

    if( is_outer_sidelobe( *candidate ) && is_outer_sidelobe( ac_window ) )
    {
      assert( candidate->ps == ac_window.ps );

      candidate->invalidate();
      return ac_trim;
    }
    
	psac = ac_window.pseudosource();
    
	bisector_result bisector = window_bisector< Side >( *candidate, **get<Side>(ac), psc, psac );

	# if defined DBG_FLAT_MMP_TRIM_AC
	std::clog << "mmp::Geodesics::trim_ac\t\t|"
	      	  << " (" << side_traits<Side>::string() << ") "
              << ( bisector.type == bisector_result::AC_WINDOW_DOMINATES ? "ac-window dominates -> cut candidate" : "" )
              << ( bisector.type == bisector_result::CANDIDATE_DOMINATES ? "candidate dominates -> cut ac-window" : "" )
              << ( bisector.type == bisector_result::TRIM                ? "trim" : "" )
              << " at bisector " << bisector.point << std::endl;
	# endif

    assert( !std::isnan( bisector.point ) );
	assert( check_bisector< Side >( bisector, *candidate, **get<Side>(ac), psc, psac ) );

    if( bisector.type & ( bisector_result::AC_WINDOW_DOMINATES | bisector_result::TRIM ) )
    {  
      candidate->set<Side>( bisector.point, psc); 
      ac_trim.second = true;
    }

    if( bisector.type & ( bisector_result::CANDIDATE_DOMINATES | bisector_result::TRIM ) )
    {
      ac_window.set< OppSide >( bisector.point, psac);
      ac_trim.first = geodesics.update_event_points< Side==LEFT ? EventPoint::RIGHT_END : EventPoint::LEFT_END >( &ac_window, psac );
    }

    assert( ac_window.bound<OppSide>() == candidate->bound<Side>() );
    
  } // of cut/trim section
  else ac_trim.first = 0; // forget neighbor of deleted window - because it not toutches the candidate
    
  return ac_trim;
} // of function trim_ac



template< mmp::side_t Side >
mmp::bisector_result mmp::window_bisector( const Window&  candidate, const Window& ac_window
                                                     , const ps_t& candidate_ps, const ps_t&   ac_ps )
{
  using utk::sqr;

  assert( has_intersection( ac_window, candidate ) );

  auto boundscheck = impl::trim_ac_bounds_check<Side>::checks();

  const side_t OppSide = side_traits<Side>::opposite;

  std::pair< coord_t, coord_t > is = intersection_interval( ac_window, candidate );  


  if( boundscheck.second( candidate.bound< Side >(), ac_window.bound< Side >() ) )
  { // candidate side bound contained in intersection
	assert( contained( candidate.bound< Side >(), is ) );
	if( candidate.source_distance< Side >() <= ac_window.source_distance( candidate.bound< Side >(), ac_ps ) )
	{
	  # if defined DBG_FLAT_MMP_WINDOW_BISECTOR
      std::clog << "mmp::window_bisector\t\t|"
                << " candidate distance is smaller at " << side_traits< Side >::string() << " bound" 
                << std::endl;
      # endif	  
	  return { candidate.bound< Side >(), bisector_result::CANDIDATE_DOMINATES };
	}  
  }

  if( boundscheck.first( ac_window.bound< OppSide >(), candidate.bound< OppSide >() )
  &&  candidate.source_distance< OppSide >() < ac_window.source_distance( candidate.bound< OppSide >(), ac_ps ) )
  {
    assert( contained( candidate.bound< OppSide >(), is ) );
	# if defined DBG_FLAT_MMP_WINDOW_BISECTOR
    std::clog << "mmp::window_bisector\t\t|"
              << " candidate dominating at " << side_traits< OppSide >::string() << " is bound (of the candidate)" 
              << std::endl;
    # endif	  
  } 
  else if( boundscheck.second( candidate.bound< OppSide >(), ac_window.bound< OppSide >() )
       &&  candidate.source_distance( ac_window.bound< OppSide >(), candidate_ps ) < ac_window.source_distance< OppSide >() )
  {
    assert( contained( ac_window.bound< OppSide >(), is ) );
	# if defined DBG_FLAT_MMP_WINDOW_BISECTOR
    std::clog << "mmp::window_bisector\t\t|"
              << " candidate dominating at " << side_traits< OppSide >::string() << " is bound (of the ac window)" 
              << std::endl;
    # endif	  
  } 
  else
  {
	# if defined DBG_FLAT_MMP_WINDOW_BISECTOR
    std::clog << "mmp::window_bisector\t\t|"
              << " ac-window dominating at " << side_traits< OppSide >::string() << " is bound" 
              << std::endl;
    # endif	  
    return { ac_window.bound< OppSide >(), bisector_result::AC_WINDOW_DOMINATES };
  }


  // the bisection point
  coord_t p;
  
  // pseudosources at vertex -> linear distance function
  const bool linear_ac       = is_inner_sidelobe( ac_window );
  const bool linear_candidate = is_inner_sidelobe( candidate );
  
  if( linear_ac && linear_candidate ) // linear - linear
  {
    assert( ac_window.has_ps_vertex<LEFT>() != candidate.has_ps_vertex<LEFT>() );
    //  return std::numeric_limits<coord_t>::quiet_NaN();
    p = impl::linear_linear_window_bisector( ac_window, candidate, ac_ps, candidate_ps );
	utk::clamp( p, is.first, is.second );
  }
  /*else if( ( linear_old || linear_candidate ) ) // linear - curved
  { 
    assert( !( linear_old && linear_candidate ) );

    const std::pair< const Window&, const ps_t& > linear ( linear_old ? std::make_pair( old, old_ps ) : std::make_pair( candidate, candidate_ps ) ); 
    const std::pair< const Window&, const ps_t& > curved ( linear_old ? std::make_pair( candidate, candidate_ps ) : std::make_pair( old, old_ps ) ); 

    p = impl::linear_curved_window_bisector( linear.first, curved.first, linear.second, curved.second );
    
  }*/
  else
  { 
    std::pair<coord_t,coord_t> p12 = impl::curved_curved_window_bisector( ac_window, candidate, ac_ps, candidate_ps);    

    const bool  p1c = contained(p12.first ,is);
    const bool  p2c = contained(p12.second,is);
    
    if( p1c != p2c) 
      return { p1c ? p12.first : p12.second , bisector_result::TRIM };
   else if( p1c && p2c )
    {
      const distance_t p1d = distance_error( p12.first , ac_window, candidate, ac_ps, candidate_ps );
      const distance_t p2d = distance_error( p12.second, ac_window, candidate, ac_ps, candidate_ps );
      
      return { p1d <= p2d ? p12.first : p12.second , bisector_result::TRIM };
    }
    
    # if defined FLAT_MMP_WINDOW_BISECTOR_INTERVAL_BOUNDS_SNAPPING
    { // snap to intersection bounds
	  const bool np1nan = !std::isnan(p12.first);
	  const bool np2nan = !std::isnan(p12.second);
      const bool snap   =    ( np1nan && snap_check( p12.first  , get< Side >( is ) ) )
						  || ( np2nan && snap_check( p12.second , get< Side >( is ) ) );

      if( snap ) 
      {
	    # if defined DBG_FLAT_MMP_WINDOW_BISECTOR
  	  	std::clog << "mmp::window_bisector\t\t| (" << side_traits< Side >::string() << ')'
                  << " QUIRK - snap to " << side_traits< Side >::string() << " intersection bound " << get< Side >(is)
		  	      << std::endl;
      	# endif
        
		return { candidate.bound< Side >(), bisector_result::CANDIDATE_DOMINATES };
      }
    } // of is bound snapping
    # endif    
    
    return { ac_window.bound< OppSide >(), bisector_result::AC_WINDOW_DOMINATES };
	
  } // of curved-curved

  assert( contained( p, is ) );
  return { p, bisector_result::TRIM };
} 

template< mmp::side_t Side >
bool  mmp::check_bisector( const bisector_result& bisector
                               , Window& candidate
                               , const Window& ac_window
                               , const ps_t& candidate_ps, const ps_t& ac_ps )
{
  const distance_t d_p_ac   = ac_window.source_distance( bisector.point, ac_ps );
  const distance_t d_p_cand = candidate.source_distance( bisector.point, candidate_ps );

  const bool diff_p_ok = dist_snap_check( d_p_cand, d_p_ac );

  # if defined DBG_FLAT_MMP_CHECK_BISECTOR
  std::clog << "mmp::check_bisector\t\t| "
            << ( bisector.type == bisector_result::AC_WINDOW_DOMINATES ? "AC_WINDOW_DOMINATES" : "" )
            << ( bisector.type == bisector_result::CANDIDATE_DOMINATES ? "CANDIDATE_DOMINATES" : "" )
            << ( bisector.type == bisector_result::TRIM                ? "TRIM" : "" )
            << std::endl << "\t\t\t\t\t|" 
            << " p        " << bisector.point << " " << diff_p_ok << " " << (d_p_cand - d_p_ac) 
            << " ( old " << d_p_ac << ", new " << d_p_cand << " )" << std::endl;
  # endif

  if( !has_intersection( ac_window, candidate ) ) 
  { 
    # if defined DBG_FLAT_MMP_CHECK_BISECTOR
    std::clog << "mmp::check_bisector\t| "
              << "no intersection - crossing paths?" 
	  		  << std::endl;
    # endif
	return true;
  }
  const std::pair<coord_t, coord_t> is = intersection_interval( ac_window, candidate );

  const distance_t d_l_ac   = get<LEFT>(is) == ac_window.bound<LEFT>() 
                              ? ac_window.source_distance<LEFT>() 
                              : ac_window.source_distance( get<LEFT>(is), ac_ps ); 
  const distance_t d_l_cand = get<LEFT>(is) == candidate.bound<LEFT>() 
                              ? candidate.source_distance<LEFT>() 
                              : candidate.source_distance( get<LEFT>(is), candidate_ps ); 
    
  const distance_t d_r_ac   = get<RIGHT>(is) == ac_window.bound<RIGHT>() 
                              ? ac_window.source_distance<RIGHT>() 
                              : ac_window.source_distance( get<RIGHT>(is), ac_ps ); 
  const distance_t d_r_cand = get<RIGHT>(is) == candidate.bound<RIGHT>() 
                              ? candidate.source_distance<RIGHT>() 
                              : candidate.source_distance( get<RIGHT>(is), candidate_ps ); 
  
  const std::pair<coord_t, distance_t> fp_ac   = ac_window.min_source_distance( ac_ps );
  const std::pair<coord_t, distance_t> fp_cand = candidate.min_source_distance( candidate_ps );

  const distance_t d_ac_fp_cand = candidate.source_distance( fp_ac.first  , candidate_ps );
  const distance_t d_cand_fp_ac = ac_window.source_distance( fp_cand.first, ac_ps );

  const bool l_ok = dist_snap_check( d_l_ac, d_l_cand );
  const bool r_ok = dist_snap_check( d_r_ac, d_r_cand );

  const distance_t diff_l = d_l_cand - d_l_ac; 
  const distance_t diff_r = d_r_cand - d_r_ac;
  
  # if defined DBG_FLAT_MMP_CHECK_BISECTOR
  std::clog << "\t\t\t\t"
            << "\t| is left  " << is.first  << " " << l_ok << " " << diff_l << " ( ac " << d_l_ac << ", cand " << d_l_cand << " )"
            << std::endl <<  "\t\t\t\t"
            << "\t| is right " << is.second << " " << r_ok << " " << diff_r << " ( ac " << d_r_ac << ", cand " << d_r_cand << " )"
            << std::endl << "\t\t\t\t"
            << "\t| fp ac    " << fp_ac.first   << " " << (d_ac_fp_cand - fp_ac.second)   << " ( ac " << fp_ac.second << ", cand " << d_ac_fp_cand << " )"
            << std::endl << "\t\t\t\t"
            << "\t| fp cand  " << fp_cand.first << " " << (fp_cand.second - d_cand_fp_ac) << " ( ac " << d_cand_fp_ac << ", cand " << fp_cand.second << " )"
            << std::endl;
  # endif
  
  const bool c_p_ac   = contained( bisector.point, ac_window );
  const bool c_p_cand = contained( bisector.point, candidate );

  const side_t OppSide = side_traits< Side >::opposite;
  bool all_ok = true;
  
  if( bisector.type == bisector_result::TRIM )
  { 
    //----| containment check
    if( !c_p_ac || !c_p_cand )
    {
      std::clog << "mmp::check_bisector\t\t|"
                << "FAILED - trim bisector not contained in intersection"
                << std::endl;
      all_ok = false;
    }
    //----| distance check
    if( !diff_p_ok )
    {
      std::clog << "mmp::check_bisector\t\t|"
                << "FAILED - distance check failed"
                << std::endl;
      all_ok = false;
    }
  }else
  {
    //----| dominance checks

    if( bisector.type == bisector_result::CANDIDATE_DOMINATES )
    {
      //----|distance_check
      if( ( diff_r > 0 && !r_ok ) || ( diff_l > 0 && !l_ok ) )
      {
        std::clog << "mmp::check_bisector\t\t|"
                  << "FAILED - candidate not dominant on both sides"
                  << std::endl;
        all_ok = false;
      }
      //----|coord check 
      if( bisector.point != candidate.bound<Side>() )
      {
        std::clog << "mmp::check_bisector\t\t|"
                  << "FAILED - bisector should be at " 
                  << side_traits<Side>::string() << " bound " << candidate.bound<Side>() << " of the candidate"
                  << std::endl;
        all_ok = false;
      }
    }

    if( bisector.type == bisector_result::AC_WINDOW_DOMINATES )
    {
      //----|distance_check
      if( ( diff_r < 0 && !r_ok ) || ( diff_l < 0 && !l_ok ) )
      {
        std::clog << "mmp::check_bisector\t\t|"
                  << "FAILED - ac-window not dominant on both sides"
                  << std::endl;
        all_ok = false;
      }
      //----|coord check    
      if( bisector.point != ac_window.bound<OppSide>() )
      {
        std::clog << "mmp::check_bisector\t\t|"
                  << "FAILED - bisector should be at " 
                  << side_traits<OppSide>::string() << " bound " << ac_window.bound<OppSide>() << " of the ac-window"
                  << std::endl;
        all_ok = false;
      }
    }
  }

  
  # if defined DBG_FLAT_MMP_CHECK_BISECTOR__USE_CAIRO
  std::vector<const Window*> winvec(2);
  winvec[0] = &wOld;
  winvec[1] = &wNew;

  visualizer::cairo::draw_windows_to_file( winvec.begin(), winvec.end(), "trimming_failed" );
  # endif

  if( !all_ok )
    std::clog << "mmp::check_bisector\t\t| bisector " << bisector.point << " side " << Side << std::endl
              << "\t\t\t\t\t| candidate " << candidate << std::endl
              << "\t\t\t\t\t| ac window " << ac_window << std::endl;
  
  return all_ok;
}
