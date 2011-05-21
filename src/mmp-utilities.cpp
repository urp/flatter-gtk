//           mmp-utilities.cpp
//  Fri Apr  9 12:40:54 2010
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

#include "mmp-utilities.h"
//#include "mmp-visualizer.h"

#include "utk/ray.h"
#include "utk/geometry.h"

using namespace mmp;

bool    mmp::interior(const coord_t& a,const std::pair<coord_t,coord_t>& i)
{ return a < i.second && a > i.first; }


bool    mmp::contained(const coord_t& a,const std::pair<coord_t,coord_t>& i)
{ return a <= i.second && a >= i.first; }


bool    mmp::has_intersection(const std::pair<coord_t,coord_t>& a,const std::pair<coord_t,coord_t>& b)
{ return a.first < b.second && a.second > b.first; }


std::pair< coord_t, coord_t >   mmp::intersection_interval( const Window& a, const Window& b )
{ 
  assert( has_intersection( a, b ) );
  
  const coord_t left  = std::max( a.bound< LEFT>(), b.bound< LEFT>() );
  const coord_t right = std::min( a.bound<RIGHT>(), b.bound<RIGHT>() );
  
  return std::make_pair( left, right );
}


bool  mmp::snap_check( const coord_t& a, const coord_t& b )
{
  return ( utk::close_to_zero( a-b, interval_absolute_tolerance ) || utk::close_ulps( a, b, interval_ulp_tolerance ) );
}


bool  mmp::dist_snap_check( const distance_t& a, const distance_t& b )
{
  return utk::close_ulps( a, b, distance_jump_ulp_tolerance ) || utk::close_rel( a, b, distance_jump_relative_tolerance );
}


/* reprojecting pseudosources to common line  definded by window with lower ps_error()
void    mmp::assign_stable_ps_direction( Window& left, Window& right
                                             , ps_t&   psl , ps_t&   psr ) 
{
  // we assume a snapped window 
  assert( left.b1 == right.b0 );
  
  const ps_t p( left.b1, 0. );
  
  if( left.ps_error() < right.ps_error() )
  {
    utk::ray<ps_coord_t,2u> dir( p, psl);
    const ps_coord_t dpr = utk::length( p - psr );
    
    psr      = dir.at_arc_length( dpr );
    right.d0 = dpr;
    right.d1 = std::hypot( psr.x() - p.x(), psr.y() );
  }else
  {
    utk::ray<ps_coord_t,2u> dir( p, psr);
    const ps_coord_t dpl = utk::length( p - psl );
    
    psl      = dir.at_arc_length( dpl );
    left.d0  = std::hypot( psl.x() - p.x(), psl.y() );
    left.d1  = dpl;
  }

}

// reprojecting pseudosources to common line
void    mmp::colinear_ps_correction( Window& left, Window& right
                                         , ps_t& psl, ps_t& psr
                                         , const ps_coord_t& pslerr , const ps_coord_t& psrerr ) 
{
  // TODO: choose ps baseed on error estimate
  
  // we assume a snapped window 
  assert( left.b1 == right.b0 );
  
  const ps_t p( left.b1, 0. );

  const utk::circle<ps_coord_t> circlel( ps_t( left.b0, 0.), psl );
  const utk::circle<ps_coord_t> circler( ps_t(right.b1, 0.), psr );

  std::clog << "mmp::colinear_ps_correction\t|"
            << " directions left " << (psl-p).normal() << " right " << (psr-p).normal()
			<< " <(psl,psr) " << dot( (psl-p).normal(), (psr-p).normal() )
			<< std::endl;
  
  // directions should be almost equal
  assert( fabs(dot( (psl-p).normal(), (psr-p).normal() ) ) > ps_coord_t(.99) );
  
  // ps direction estimation weighted by estimated pseudosource reconstruction errors
  const ps_coord_t psrel = pslerr / ( pslerr + psrerr );
  const ps_t psmid = psl * ( ps_coord_t(1) - psrel ) + psr * psrel;
  const utk::ray< ps_coord_t,2u > npsray( p, psmid );

  // get line parameters for ray-circle intersection points
  const std::pair< ps_coord_t, ps_coord_t > isl = utk::intersection( npsray, circlel );
  const std::pair< ps_coord_t, ps_coord_t > isr = utk::intersection( npsray, circler );

  std::clog << "\t\t\t\t\t|"
            << " nps relation " << psrel 
			<< " direction "  << npsray.direction().normal()
			<< " <(nps,psl) " << dot( npsray.direction().normal(), (psl-p).normal() ) << "/"
			<< " <(nps,psr) " << dot( npsray.direction().normal(), (psr-p).normal() ) 
            << std::endl;
  

  const ps_t npsl = npsray.at( isl.first > ps_coord_t(0.) ? isl.first : isl.second );
  const ps_t npsr = npsray.at( isr.first > ps_coord_t(0.) ? isr.first : isr.second );
  
  //left.d0  = std::hypot( npsl.x() - left.b0, npsl.y() );
  left.d1    = std::hypot( npsl.x() - p.x(), npsl.y() ); //circlel.radius();
  right.d0   = std::hypot( npsr.x() - p.x(), npsr.y() ); //circler.radius();
  //right.d1 = std::hypot( npsr.x() - right.b1, npsr.y() );

  psl = npsl;
  psr = npsr;                                            
  
  std::clog << "\t\t\t\t\t|" << " new left " << left << std::endl;
  std::clog << "\t\t\t\t\t|" << " new right " << right << std::endl;
}*/



std::pair< coord_t, distance_t >    mmp::max_distance_error( const Window& wa, const Window& wb, const ps_t& psa, const ps_t& psb )
{
  assert( has_intersection(wa,wb) );
  std::pair<coord_t,coord_t> is = intersection_interval( wa, wb );

  coord_t    maxpos = is.first; // initialize with left intersection bound
  distance_t maxerr = std::fabs( wa.source_distance(is.first, psa) - wb.source_distance(is.first, psb) ),
             terr   = std::fabs( wa.source_distance(is.second, psa) - wb.source_distance(is.second, psb) );

  if( terr < maxerr ) 
  { // check right intersection bound
    maxerr = terr;
    maxpos = is.second;
  }

  const ps_coord_t a = utk::sqr( psa.y() ) - utk::sqr( psb.y() );
  const ps_coord_t b = ps_coord_t(2.) * ( psa.x() * utk::sqr(psb.y()) - psb.x() * utk::sqr(psa.y()) );
  const ps_coord_t c = utk::sqr(psb.x()) * utk::sqr(psa.y()) - utk::sqr(psa.x()) * utk::sqr(psb.y());
    
  const std::pair<ps_coord_t,ps_coord_t> errext = utk::solve_quadratic_equation(a,b,c);

  if( contained( errext.first, is ) 
      && ( terr = std::fabs( wa.source_distance(errext.first, psa) - wb.source_distance(errext.first, psb) ) ) > maxerr )
  {
    maxerr = terr;
    maxpos = errext.first;
  }

  if( contained( errext.second, is ) 
      && ( terr = std::fabs( wa.source_distance( errext.second, psa ) - wb.source_distance( errext.second, psb ) ) ) > maxerr )
  {
    maxerr = terr;
    maxpos = errext.second;
  }

  return std::make_pair(maxpos,maxerr);
}







