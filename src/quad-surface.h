/***************************************************************************
 *            quad-surface.h
 *
 *  Sun Oct 11 14:13:29 2009
 *  Copyright  2009  Peter Urban
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

# include   "surface.h"

namespace flat
{

  class QuadSurface	: public Surface
  {	
      size_pair m_vertices_size;
    
	  QuadSurface( const size_pair& vertex_size
                 , const size_pair& texture_size
                 , const std::string& name )	
	  : Surface( std::get<0>(vertex_size) * std::get<1>(vertex_size), texture_size, name ), m_vertices_size( vertex_size )
      {   }

	public:

      template< typename Generator, typename Triangulator, typename Transform >
	  static std::shared_ptr< QuadSurface > create_with_generator( Generator&                   generator
                                                                 , Triangulator&                triangulator
                                                                 , Transform&                   transform 
                                                                 , Surface::distance_function   distances = distance_function()
                                                                 )
	  {
        std::shared_ptr< QuadSurface > surface( new QuadSurface( generator.vertex_field_size(), generator.texture_size(), generator.get_name() ) );
		
        generator( surface );

        triangulator( surface );

        transform( surface );

        if( distances.neighborhood != distance_function::NONE )
          surface->initial_distances = distances ;
        else
          surface->initial_distances.compute_distances( surface, distance_function::NEIGHBORS );
        
        return surface;
	  }
	  
	  virtual   ~QuadSurface()	{ }
	  
	  const size_pair& 	vertices_size()	const	{ return m_vertices_size; }
	  
      std::vector< vertex_descriptor >      neighbors( const vertex_descriptor vertex ) const 
      { 
        return std::move( get_neighbors< NB8_LEFT | NB8_LOWER | NB8_RIGHT | NB8_UPPER >( vertex ) );
      } 

      template< nb8_mask_t Query >
      std::vector< vertex_descriptor > get_neighbors( const vertex_descriptor& v )	const	
      { 
        std::vector< vertex_descriptor > nbs;
        nbs.reserve(8);
          
        //std::clog << "flat::QuadSurface::get_neighbors" << std::endl;

        const size_t m = std::get<0>( vertices_size() );
		const size_t n = std::get<1>( vertices_size() );
        const bool upper = v < m;
        const bool lower = v >= m * ( n - 1 );
        const bool left  = v % m == 0;
        const bool right = (v+1) % m == 0;
        
        if( !upper )
        { if( Query & NB8_UPPER_LEFT  && !left  ) nbs.push_back( v - m - 1 );
	      if( Query & NB8_UPPER )                 nbs.push_back( v - m     );
	      if( Query & NB8_UPPER_RIGHT && !right ) nbs.push_back( v - m + 1 );
        }
      
        if( Query & NB8_RIGHT && !right ) nbs.push_back( v + 1 );
      
        if( !lower )
        { if( Query & NB8_LOWER_RIGHT && !right ) nbs.push_back( v + m + 1 );
	      if( Query & NB8_LOWER )                 nbs.push_back( v + m     );
	      if( Query & NB8_LOWER_LEFT  && !left  ) nbs.push_back( v + m - 1 );
        }
      
        if( Query & NB8_LEFT && !left )   nbs.push_back( v - 1 );

        return std::move( nbs );
      }

      std::vector< vertex_pair > neighbors() const;
  };
}
