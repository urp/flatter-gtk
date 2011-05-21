//           mmp-visualizer-cairo.cpp
//  Tue Jan 18 12:51:25 2011
//  Copyright  2011  Peter Urban
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

# include "mmp-visualizer-cairo.h"

using namespace mmp::visualizer;

cairo::cairo_coord_t cairo::user_unit_distance( const Cairo::RefPtr<Cairo::Context>& cr )
{
  cairo_coord_t dist(1.);
  cr->device_to_user_distance( dist[0], dist[1] );
  return dist;
}

void cairo::draw_half_arrow( const Cairo::RefPtr<Cairo::Context>& cr, const cairo_coord_t& posA, const cairo_coord_t& posB )
{
  cr->save();
    
    cr->move_to( posA[0], posA[1] );
    
    cr->line_to( posB[0], posB[1] );

    cairo_coord_t arrow_dir    = posB - posA;
    double        arrow_length = length( arrow_dir );
    double        arrow_scale  = user_unit_distance(cr).length();
    cairo_coord_t arrow_base   = posB - arrow_dir * 10. * arrow_scale / arrow_length;
    cairo_coord_t arrow_offset ( - arrow_dir[1] * 3. * arrow_scale / arrow_length,
                                   arrow_dir[0] * 3. * arrow_scale / arrow_length );

    cr->line_to( arrow_base[0] + arrow_offset[0], arrow_base[1] + arrow_offset[1] );

    cr->stroke();
    
  cr->restore();
}

void cairo::draw_centered_text( const Cairo::RefPtr<Cairo::Context>& cr, const std::string& utf8, const cairo_coord_t& pos )
{
  cr->save();
    
  Cairo::TextExtents texts;
  cr->get_text_extents( utf8, texts );
    
  cr->move_to( pos[0] - texts.width/2. - texts.x_bearing , pos[1] );

  cr->show_text( utf8 );

  cr->restore();
}

void cairo::draw_triangle( const Cairo::RefPtr<Cairo::Context>& cr, const Geodesics::edge_handle& e0 )
{ 
  const rgba_color_t color( .8, .8, .8, .3);

  // get edge-free pair
  const Geodesics::edge_handle e1 ( e0.next() );
  const Geodesics::edge_handle e2 ( e1.next() );
  
  const coord_t e0l= e0.length();
  const coord_t e1l= e1.length();
  const coord_t e2l= e2.length();

  // coordinates of C - using circle-circle intersection ( intersect circle (w.b0,w.d0) with (w.b1,w.d1) )

  cairo_coord_t A(0.,0.);
  cairo_coord_t B(e0l,0.);
  cairo_coord_t C;
  boost::tie(C[0],C[1]) = utk::triangulate( e0l, e2l, e1l );
  //C[1] = - C[1];

  const coord2_t centroid = ( ( coord2_t(e0l, 0) += C ) /= coord_t(3) );
	
  cr->save();

    //cr->set_operator( Cairo::OPERATOR_DEST_OVER );
  
    cr->set_source_rgba( color[0], color[1], color[2], color[3] );
    
    cr->save();
      // shrink slightly towards centroid
      cr->translate( centroid[0], centroid[1] );
      cr->scale( .95, .95 );
      cr->translate( -centroid[0], -centroid[1] );
      //draw triangle
      cr->set_line_width( 1. * user_unit_distance( cr ).length() );

      draw_half_arrow( cr, A, B );

      draw_half_arrow( cr, B, C );

      draw_half_arrow( cr, C, A );


    cr->restore();
    
    cr->stroke();

  cr->restore();

  // draw text
  cr->save();

    cr->user_to_device( A[0], A[1] );
    cr->user_to_device( B[0], B[1] );
    cr->user_to_device( C[0], C[1] );
    
    cr->set_identity_matrix();
    
    //cr->select_font_face( "Purisa", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL );
    
    cr->set_source_rgba( 0., 0., 0.,.5 );
    
    cr->set_font_size( 8. /* user_unit_distance( cr ).length()*/ );
    
    std::ostringstream nrss;
    nrss << e0.source().descriptor();
    draw_centered_text( cr, nrss.str(), A );

    nrss.str("");
    nrss << e1.source().descriptor();
    draw_centered_text( cr, nrss.str(), B );

    nrss.str("");
    nrss << e2.source().descriptor();
    draw_centered_text( cr, nrss.str(), C );
    
  cr->restore();
  
}

void cairo::draw_window(  const Cairo::RefPtr< Cairo::Context>& cr, const Window& window )
{ 
  const rgba_color_t color( uniform_real<double>() * .8, uniform_real<double>() * .8,  uniform_real<double>() * .8, .5 );
  return draw_window( cr, window, color );
}

void cairo::draw_window( const Cairo::RefPtr<Cairo::Context>& cr, const Window& window, const rgba_color_ref_t& color )
{ 

  const ps_t ps = window.pseudosource();

  cairo_coord_t B0( window.bound<LEFT>(), 0 );
  cairo_coord_t B1( window.bound<RIGHT>(), 0 );
  cairo_coord_t PS( ps[0], ps[1] );
    
  cr->save();

    cr->set_source_rgba( color[0], color[1], color[2], color[3] );
    
    cr->move_to( B0[0], B0[1] + cr->get_line_width() );

    cr->line_to( B1[0], B1[1] + cr->get_line_width() );

    cr->stroke();

    std::vector<double> dashes(2);
    dashes[0] = 2. * user_unit_distance( cr ).length();
    dashes[1] = 10. * user_unit_distance( cr ).length();
  
    cr->set_dash( dashes, 0. );

    cr->set_source_rgba( color[0], color[1], color[2], color[3]*.5 );
  
    cr->move_to( B1[0], B1[1] );
    
    cr->line_to( PS[0], PS[1] );

    cr->line_to( B0[0], B0[1] );

    cr->stroke();

  cr->restore();

  cr->save();

    cr->set_font_size( 8. * user_unit_distance( cr ).length() );

    cr->set_source_rgba( color[0], color[1], color[2], 1. );
    
    std::ostringstream nrss;
    nrss << window.id;

    cairo_coord_t pos = (B0+B1)*.5;
	pos[1] += 5.*user_unit_distance( cr ).length();
  
    cr->translate( pos[0], pos[1] );

    cr->scale( 1., -1. );

    draw_centered_text( cr, nrss.str() );
    
  cr->restore();
    
  #if defined DBG_FLAT_MMP_VISUALIZER_DRAW_WINDOW
  std::clog << "mmp::visualizer::draw_window\t|" 
            << window 
            << std::endl 
            << "\t\t\t\t\t|"
            << " color " << color
            << " bounds " << bb
            << std::endl;
  #endif
}

void cairo::draw_face_edge_sequences( const Cairo::RefPtr<Cairo::Context>& cr, const Geodesics::surface_type::edge_descriptor& edge, const Geodesics& m_geodesics )
{	
  const auto    e0out = Geodesics::edge_handle( edge, m_geodesics.get_surface() );
  const auto 	e0w   = m_geodesics.edge_windows( e0out );
  const auto 	e0in  = e0out.opposite();
  const coord_t e0len = e0out.length();
	
  // draw base
  draw_edge_sequences( cr, e0w.first, e0w.second, m_geodesics );

  // draw lower left edge
	
  if( ! e0in.second ) return ;
  auto e1out = e0in.first.next().opposite();

  if( e1out.second ) 
  { 
	auto e1w = m_geodesics.edge_windows( e1out.first );
    const coord_t e1len = e1out.first.length();
    cr->save();

      cr->rotate( - e0in.first.next_inner_angle() );

	  cr->translate( e1len, 0. );
	
	  cr->rotate( M_PI );
	  //cr->scale( -1., -1. );
	
      draw_edge_sequences( cr, e1w.first, e1w.second, m_geodesics );

    cr->restore();
  }

  
  // draw lower right edge

  const Geodesics::edge_handle e2in = e0in.first.previous();
  const auto e2out = e2in.opposite();

  if( ! e2out.second ) return;
	
  auto e2w = m_geodesics.edge_windows( e2out.first );

  cr->save();

	cr->translate( e0len, 0. );
	
    cr->rotate( e2in.next_inner_angle() );

    cr->scale( -1., -1. );
	
    draw_edge_sequences( cr, e2w.first, e2w.second, m_geodesics );

  cr->restore();
}

void cairo::draw_edge_sequence( const Cairo::RefPtr< Cairo::Context>& cr, const Window& window, const Geodesics& m_geodesics )
{
  const rgba_color_t color( uniform_real<double>() * .8, uniform_real<double>() * .8,  uniform_real<double>() * .8, .5 );
  draw_edge_sequence( cr, window, m_geodesics, color );
}
                                          
void cairo::draw_edge_sequence( const Cairo::RefPtr<Cairo::Context>& cr, const Window& window, const Geodesics& m_geodesics, const rgba_color_ref_t& color )
{
  Geodesics::edge_handle edge( window.edge, m_geodesics.get_surface() );
  
  draw_triangle( cr, edge );
  
  draw_window( cr, window, color );
  
  if( window.predeccessor() )
  {
    const Window&          predwin = *window.predeccessor();
    Geodesics::edge_handle prededge( predwin.edge, m_geodesics.get_surface() );
    
    assert( prededge.opposite().second );
    
    cr->save();
    
    if( prededge.source() == edge.source() ) 
    { // predeccessor on previous edge
	  assert( edge.previous().opposite().first == prededge );
      cr->rotate( edge.previous().next_inner_angle() );
    }else
    { 
      assert( prededge.target() == edge.target() );
	  assert( edge.next().opposite().first == prededge );
      // predeccessor on next edge
      cr->translate( edge.length(), 0. );
      cr->rotate( - edge.next_inner_angle() );
      cr->translate( - prededge.length(), 0. );
     
    }

    draw_edge_sequence( cr, predwin, m_geodesics, color );

    cr->restore();
  }
}
   