//           quad-surface.cpp
//  Sun Oct 11 14:13:29 2009
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

# include "quad-surface.h"

//# include "force.h"

# include <cairomm/context.h>
# include <cairomm/surface.h>

using namespace flat;

std::vector< Surface::vertex_pair > QuadSurface::neighbors() const
{
  const size_t& m  = std::get<0>( vertices_size() );
  const size_t& n  = std::get<1>( vertices_size() );
  assert( m > 1 && n > 1 );

  const size_t num_pairs = ( /*interior*/ 8 * (m-2)*(n-2) + /*boundaries*/ 5 * (2*m+2*n-8)  + /*corners*/ 3 * 4 ) / 2;

  std::clog<<"flat::QuadSurface::neighbors\t|creating " << num_pairs << " pairs..." << std::endl;

  // TODO optimize
  std::vector< vertex_pair > pairs;
  pairs.reserve( num_pairs );

                              
  for( vertex_descriptor upper = 0; upper < m-1; ++upper )
  { // upper 
    const vertex_pair nbs[] = { {upper, upper + 1} };
    pairs.insert( pairs.end(), nbs, nbs + sizeof(nbs)/sizeof(*nbs) ); 
  }                 

  for( vertex_descriptor j = 1 ; j < n; ++j )
  {
    { // left 
      const vertex_descriptor v = j * m;
      const vertex_pair nbs[]   = { { v, v - m }, { v, v + 1 - m }, { v, v + 1 } };
      pairs.insert( pairs.end(), nbs, nbs + sizeof(nbs)/sizeof(*nbs) ); 
    }                 
    for( vertex_descriptor i = 1; i < m-1; ++i )
    { // interior
      const vertex_descriptor v = i + j * m;
      const vertex_pair nbs[]   = { { v, v - 1 - m }, { v, v - m }, { v, v + 1 - m }, { v, v + 1 } };
      pairs.insert( pairs.end(), nbs, nbs + sizeof(nbs)/sizeof(*nbs) ); 
    }
    { // right 
      const vertex_descriptor v = j * m + m -1 ; 
      const vertex_pair nbs[]   = { { v, v - 1 - m }, { v, v - m } };
      pairs.insert( pairs.end(), nbs, nbs + sizeof(nbs)/sizeof(*nbs) ); 
    }                 

  }
                              
                              
  assert( pairs.size() == num_pairs );
                              
  std::clog << "flat::QuadSurface::neighbors\t|completed" << std::endl;

  return std::move( pairs );
}



/*void flat::QuadSurface::write_png_file(const std::string& path,
									   const bool&	  	  weightalign,
									   const bool&		  texeloutline,
									   const bool& 	  	  blockoutline,
									   const float 	  	  texelsize)	const	throw(std::bad_alloc)
{
  // determine image size

  const location_t extent = max_location()-min_location();
  const float scale = texelsize/std::min(extent.y()/color_m(),
										 extent.x()/color_n());
    
  const size_t width = size_t(scale*extent.x());
  const size_t height= size_t(scale*extent.y());
  
  std::clog<<"flat::PointCloud::write_png_file"
	   <<"\t|file "<<path
	   <<"\t|min "<<min_location()
	   <<"\t|max "<<max_location()
	   <<"\t|image size ("<<width<<","<<height<<")"
	   <<"\t|scale "<<scale
	   <<"\t|texel outline "<<(texeloutline?"yes":"no")
	   <<"\t|block outline "<<(blockoutline?"yes":"no")
	   <<std::endl<<std::flush;

  // prepare cairo drawing
  Cairo::RefPtr<Cairo::ImageSurface> surface 
  = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, width, height);

  Cairo::RefPtr<Cairo::Context> cr  = Cairo::Context::create(surface);

  cr->save(); // save the state of the context
  cr->set_antialias(Cairo::ANTIALIAS_NONE);
  cr->translate(-min_location().x()*scale,-min_location().y()*scale);
  cr->scale(scale,scale);
  double ux=1, uy=1;
  cr->device_to_user_distance(ux, uy); 
  cr->set_line_width(std::max(ux,uy));
  /*cr->save(); // save the state of the context
  cr->set_source_rgb(0.f, 0.f, 0.f);
  cr->paint();    // fill image with the color
  cr->restore();  // color is back to black now*/
/*
  for(size_t block_index=points_n()+1;block_index<num_points();++block_index)
    for(;(block_index)%points_n();++block_index)
    {
      const TexelBlock block(block_index,loc2tex,texels,points_n(),color_n());	    
      
	  block.draw_cairo_context(cr,weightalign,texeloutline,blockoutline);
	  
      const size_t progress = size_t(100.f* float(block_index-points_n()-1)
                                     	  / (num_points()-points_m()-points_n()+1));
      std::clog<<"\r"//"flat::PointCloud::write_png_file:"
	       <<"|progress "<<progress<<" %"<<std::flush;
  }
  std::clog<<std::endl;
  
  cr->restore();
  surface->flush();
  surface->write_to_png(path);
  std::clog<<"flat::PointCloud::write_png_file:"
           <<"|complete"<<std::endl<<std::flush;
}*/




