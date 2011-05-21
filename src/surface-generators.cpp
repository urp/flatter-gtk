//           surface-generators.cpp
//  Fri Jan 28 14:37:40 2011
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

# include "surface-generators.h"

# include "surface.h"

using namespace flat;

void    SimpleRectlinearTriangulator::operator() ( const std::shared_ptr< Surface >& surface )
{
  const size_t& m = std::get<0>( vertices_size );
  const size_t& n = std::get<1>( vertices_size );

  assert( m * n == surface->num_vertices() );
  
  std::clog << "SimpleRectlinearTriangulator()\t|" 
            << " creating the triangulation of size (" << m << "," << n << ") ..." << std::endl;

  for( Surface::vertex_descriptor descriptor = m + 1 ; descriptor < m*n; ++descriptor )
  {
	if( descriptor % m )
	{ 
	  //----|triangulation of the quad-face  
	  const Surface::vertex_descriptor a = descriptor;		    //  c<--b
	  const Surface::vertex_descriptor b = descriptor     - m;	//	|\  ^
	  const Surface::vertex_descriptor c = descriptor - 1 - m;	//  v \ |
	  const Surface::vertex_descriptor d = descriptor - 1;	    //  d-->a = vertex(i,j)

      # if defined DBG_FLAT_SIMPLE_RECTLINEAR_TRIAGULATOR__PER_QUAD
	  std::clog << "SimpleRectlinearTriangulator()\t|" 
    	        << " quad (" << a << "," << b << "," << c << "," << d << ")" << std::endl;
      # endif
      
	  surface->create_face( a, c, d ); //right face
	  surface->create_face( a, b, c ); //left face	
    }
  }
  
  std::clog << "SimpleRectlinearTriangulator()\t|"  << "complete"<<std::endl;	 
}
	  
void RandomHeightGenerator::add_noise( const std::shared_ptr< Surface >& surface, const coord_t amplitude )
{
  for( auto vit = surface->vertex_handles(); vit.first != vit.second; ++vit.first )
  { location_t location( vit.first->location() );
    location[2] += amplitude * ( uniform_real<coord_t>() - .5 );
    vit.first->set_location( location );
  }
}

void RandomHeightGenerator::operator() ( const std::shared_ptr< Surface >& surface )
{
  assert( num_vertices() == surface->num_vertices() );
  assert( texture_size() == surface->texture().size );

  for( auto vit = surface->vertex_handles(); vit.first != vit.second; ++vit.first )
  {
    const Surface::vertex_descriptor descriptor = vit.first->descriptor();
    const size_t  i = descriptor % std::get<0>( vertex_field_size() );
    const size_t  j = descriptor / std::get<0>( vertex_field_size() );
	
    const coord_t x = i / coord_t( std::get<0>( vertex_field_size() ) - 1 ) ;
    const coord_t y = j / coord_t( std::get<1>( vertex_field_size() ) - 1 ) ;
	const coord_t z = 0.;
	
    vit.first->set_location( location_t( x - .5, y - .5, z ) );
    vit.first->set_texture_coordinate( vertex_texture_coord_t::type( x, y ) );
	const color_channel_t	col( (i+j) % 2 );
    surface->texture().set_pixel( descriptor, rgba_color_t( col, col, col, 1. ) );
  }

  add_noise( surface, noise_amplitude );
}

void WaveGenerator::operator() ( const std::shared_ptr< Surface >& surface )
{
  assert( num_vertices() == surface->num_vertices() );
  assert( texture_size() == surface->texture().size );
  
  for( auto vit = surface->vertex_handles(); vit.first != vit.second; ++vit.first )
  {
    const Surface::vertex_descriptor descriptor = vit.first->descriptor();

	const size_t  i = descriptor % std::get<1>( vertex_field_size() );
    const size_t  j = descriptor / std::get<0>( vertex_field_size() );
	
    const coord_t x = i / coord_t( get<0>( vertex_field_size() ) - 1 ) ;
    const coord_t y = j / coord_t( get<1>( vertex_field_size() ) - 1 ) ;
	const coord_t z = .2 * ( std::sin( x * 1.5 * M_PI ) + x );
	
    vit.first->set_location( location_t( x - .5 , y - .5 , z ) );

    std::clog << "flat::WaveGenerator\t| vertex " << *vit.first << std::endl;
    
    vit.first->set_texture_coordinate( vertex_texture_coord_t::type( x, y ) );
	const color_channel_t	col( (i+j) % 2 );
    surface->texture().set_pixel( descriptor, rgba_color_t( col, col, col, 1. ) );
  }
  add_noise( surface, RandomHeightGenerator::noise_amplitude );
}

// - shifts the surface such that the pivot (position averaged over all samples)  
// fit into the cube [pivot-0.5,pivot+.5]^3) along dimensions n for which rescale_flag[n]==true
// - rescales the surface such that the bounds of the surface fit into the cube [pivot-0.5,pivot+.5]^3) 
// along dimensions n for which rescale_flag[n]==true
// returns the new bounds of the sample
void	CenterRescaleTransform::operator() ( const std::shared_ptr< PointCloud> & cloud )   const
{
  std::clog << "flat::CenterRescaleTransform"
            << "\t|center flags " << center_flags
		    << "\t|fit-box flags " << rescale_flags
		    << std::endl;
  
  location_t  old_min = cloud->min_location();
  location_t  old_max = cloud->max_location();
  
  const location_t	extent(	  old_max - old_min        );
  const location_t  mid   ( ( old_max + old_min ) / 2 );
  
  assert( extent >= 0 );

  const location_t	shift ( center_flags[0] ? -mid.x() : 0,
	                        center_flags[1] ? -mid.y() : 0,
	                        center_flags[2] ? -mid.z() : 0
	                      );
	
  const coord_t invscale = std::max( rescale_flags[0] ? extent[0] : 1
				   				   , std::max( rescale_flags[1] ? extent[1] : 1
					     					 , rescale_flags[2] ? extent[2] : 1 ) );
  
  for( auto vertex_it = cloud->vertex_handles(); vertex_it.first != vertex_it.second; vertex_it.first++ )
  { 
    location_t new_location = vertex_it.first->location();
    new_location += shift;
    new_location /= invscale;
    vertex_it.first->set_location( new_location );
  }

  old_min += shift;
  old_min /= invscale;
  old_max += shift;
  old_max /= invscale;

  cloud->set_min_max( old_min, old_max );
  
  std::clog<< "flat::CenterRescaleTransform" << "\t|complete" << std::endl;
}
