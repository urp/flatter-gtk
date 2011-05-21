/***************************************************************************
 *            surface-generators.h
 *
 *  Fri Jan 28 14:37:40 2011
 *  Copyright  2011  Peter Urban
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

# include "common.h"

# include "surface.h"

# include <boost/lexical_cast.hpp>

# include <fstream>

# include <sstream>

//# define DBG_FLAT_SIMPLE_RECTLINEAR_TRIAGULATOR__PER_QUAD

# define DBG_FLAT_PDM_FILE_READER
//# define DBG_FLAT_PDM_FILE_READER_VERTEX_SAMPLE
//# define DBG_FLAT_PDM_FILE_READER_TEXTURE_SAMPLE

namespace flat
{

  struct NoTransform
  {
    void operator () ( const std::shared_ptr< PointCloud> & )    const  {   };
  };


  // - shifts the surface such that the pivot (position averaged over all samples)  
  // fit into the cube [pivot-0.5,pivot+.5]^3) along dimensions n for which rescale_flag[n]==true
  // - rescales the surface such that the bounds of the surface fit into the cube [pivot-0.5,pivot+.5]^3) 
  // along dimensions n for which rescale_flag[n]==true
  // returns the new bounds of the sample
  struct CenterRescaleTransform
  {
    const utk::vecn3b   center_flags, rescale_flags;
    
    CenterRescaleTransform( const utk::vecn3b& 	center, const utk::vecn3b& 	rescale )
    : center_flags( center ), rescale_flags( rescale )  {   }

    void operator () ( const std::shared_ptr< PointCloud> & )    const;
  };
  
  struct RectlinearTriangulator
  {
    size_pair	vertices_size;
		  
    RectlinearTriangulator( const size_pair& vertices_size ) 
    : vertices_size( vertices_size ) {   }

	virtual void operator() ( const std::shared_ptr< Surface >& surface )	= 0;
  };	

  struct SimpleRectlinearTriangulator : public RectlinearTriangulator
  {
	SimpleRectlinearTriangulator( const size_pair& vertices_size ) 
    : RectlinearTriangulator( vertices_size ) {   }

	void operator() ( const std::shared_ptr< Surface >& surface );
  };

  struct TextureGenerator
  {
	protected:
  	  size_pair			m_texture_size;

	public:
	  
	  TextureGenerator( const size_pair& texture_size ) 
      : m_texture_size( texture_size ) {   };

      const size_pair&	texture_size() const { return m_texture_size; }  
  };

	
  struct RectlinearFieldGenerator
  {
	protected:
      
      size_pair	m_vertices_size;

      RectlinearFieldGenerator() : m_vertices_size{ 0, 0 } {   }

	public:
      
      RectlinearFieldGenerator( const size_pair& vertices_size ) 
      : m_vertices_size( vertices_size ) {   }

      virtual void  operator() ( const std::shared_ptr< Surface >& ) = 0;

      virtual std::string get_name() const = 0;
      
      const size_pair&	vertex_field_size() const { return m_vertices_size; }  

	  const typename Surface::vertices_size_type 	num_vertices()	const	
      { return std::get<0>( m_vertices_size ) * std::get<1>( m_vertices_size ); }	
  };

  
  //----| point cloud generators

  struct RandomHeightGenerator : public RectlinearFieldGenerator, public TextureGenerator
  {
      coord_t   noise_amplitude;

      static void add_noise( const std::shared_ptr< Surface >&, const coord_t );
    
	  RandomHeightGenerator( const size_pair& size, const coord_t& amplitude )
      : RectlinearFieldGenerator( size ), TextureGenerator( size )
	  , noise_amplitude( amplitude ) {    }

	  void operator() ( const std::shared_ptr< Surface >& surface );

      std::string   get_name() const { return "HeightField"; }
  };
  
  struct WaveGenerator : public RandomHeightGenerator
  {
	  WaveGenerator( const size_pair& size, const coord_t& noise_amplitude = 0 )
      : RandomHeightGenerator( size, noise_amplitude )
      {   }

	  void operator() ( const std::shared_ptr< Surface >& surface );

      std::string   get_name() const { return "Wave"; }
  };

  
  //----| PdmFileReader

  struct accept_all_predicate
  { 
    size_pair result_field_size( const size_pair& field_size )     { return field_size; }

    size_t result_num_vertices( const size_pair& field_size )  const { return std::get<0>( field_size ) * std::get<1>( field_size ); }
    
    bool operator() ( const size_t ) const { return true; }
  };

  struct accept_none_predicate
  { 
    size_pair result_field_size( const size_pair& field_size )     { return {0, 0}; }

    size_t result_num_vertices( const size_pair& field_size )  const { return 0; }
    
    bool operator() ( const size_t ) const { return false; }
  };
	
  struct stride_predicate
  {

    size_pair m_stride, m_field_size;

    stride_predicate( const size_pair& stride, const size_pair& field_size = size_pair( 0, 0 ) )
    : m_stride( stride ), m_field_size( field_size )    {   }

    size_pair result_field_size( const size_pair& field_size )
    { 	  
      m_field_size = field_size;
	  // two boundary samples + equidistantly sampled interior
      return { ( get<0>(m_field_size) - 2 ) / get<0>(m_stride) + 2
             , ( get<1>(m_field_size) - 2 ) / get<1>(m_stride) + 2 };
    }

    size_t  result_num_vertices( const size_pair& field_size )  
    { 
      const size_pair field = result_field_size( field_size );
      return get<0>(field) * get<1>(field);
    }
    
    bool operator() ( const size_t sample_id ) const 
    { 
      assert( get<0>(m_field_size) && get<1>(m_field_size) );

      const size_t i = sample_id % get<0>(m_field_size) , j = sample_id / get<0>(m_field_size);

      return    (  i %  get<0>(m_stride) == 0 
                || i == ( get<0>(m_field_size) - 1 ) ) 
             && (  j %  get<1>(m_stride) == 0 
                || j == ( get<1>(m_field_size) - 1 ) ); 
    }
  };
  
  template< typename VertexPredicate = accept_all_predicate, typename TexturePredicate = accept_all_predicate >
  class PdmFileReader : public RectlinearFieldGenerator, public TextureGenerator
  {
    public:
      
      struct unknown_magic_number_exception : public std::exception
      { 
        std::string m_magic_number, m_file;

		unknown_magic_number_exception( const std::string& magic_number, const std::string& file )
        : m_magic_number( magic_number ), m_file( file ) {  }

        virtual ~unknown_magic_number_exception() throw() { };
        
        const char* what() const throw()
        { return std::string( "unknown magic number \"" + m_magic_number + "\" in file \"" + m_file + "\"" ).c_str(); } 
      };
      
    private:
	  
      typedef enum { VERTEX = 1, TEXTURE = 2, END = 4 }  sample_type_t;
      typedef int   sample_mask_t;

      std::string   m_filename;
      std::ifstream m_stream;

      mutable VertexPredicate  m_vertex_predicate;
      mutable TexturePredicate m_texture_predicate;

      size_t m_index;

      size_pair m_field_size;

	  rgba_color_t                 	texture_color;
      location_t                    vertex_location;
      vertex_texture_coord_t::type  vertex_texture_coordinate;

      sample_mask_t  next_sample();

      bool advance_index()  { return ++m_index < ( get<0>(m_field_size) * get<1>(m_field_size) ); }

      bool skip_sample()    
      { 
        # if defined DBG_FLAT_PDM_FILE_READER
        //std::clog << "flat::PdmFileReader()\t| " << " skip sample " << m_index << std::endl;
        # endif
          
        m_stream.ignore( 256, '\n' ); 
        return advance_index();
      }

      bool read_sample();

    public:
      
      PdmFileReader() = delete;

      PdmFileReader( const PdmFileReader& ) = delete;
      
      PdmFileReader( const std::string& path
                   , VertexPredicate
                   , TexturePredicate texture_predicate = TexturePredicate() ) throw( unknown_magic_number_exception );

      ~PdmFileReader() { m_stream.close(); }

	  void operator() ( const std::shared_ptr< Surface >& surface );

      std::string   get_name() const { return m_filename; }

  };

}

//==============================================================================
// IMPLEMENTATION

template< typename VertexPredicate, typename TexturePredicate >
flat::PdmFileReader< VertexPredicate, TexturePredicate >
                   ::PdmFileReader( const std::string& path
                                  , VertexPredicate    vertex_predicate
                                  , TexturePredicate   texture_predicate ) throw( unknown_magic_number_exception )
: RectlinearFieldGenerator( { 0, 0 } ), TextureGenerator( { 0, 0 } )
, m_filename( path ), m_stream(), m_vertex_predicate( vertex_predicate ), m_texture_predicate( texture_predicate )
, texture_color( 1. ), m_index( 0 ), m_field_size( 0, 0 )

{
  m_stream.exceptions( std::ifstream::failbit | std::ifstream::badbit );

  m_stream.open( path.c_str() );
  
  std::string   token;

  # if defined DBG_FLAT_PDM_FILE_READER
  std::clog << "flat::PdmFileReader::PdmFileReader\t|" << " path \"" << path << '\"' << std::endl;
  # endif
    
  // check magic number
  m_stream >> token;
  if( token != "P9" )
  {
    m_stream.close();
    throw unknown_magic_number_exception( token, path );
  }

  size_pair field_size{ 0, 0 };
  // read grid dimensions
  m_stream >> token >> token >> get<1>( m_field_size ) >> get<0>( m_field_size ) >> token;

  # if defined DBG_FLAT_PDM_FILE_READER
  std::clog << "flat::PdmFileReader::PdmFileReader\t|" << " size (" << get<0>( m_field_size ) << ',' << get<1>( m_field_size ) << ')' << std::endl;
  # endif
    
  assert( get<0>( m_field_size ) > 1 && get<1>( m_field_size ) > 1 );
  
  // resize storage
  m_vertices_size = m_vertex_predicate.result_field_size( m_field_size );
  m_texture_size  = m_texture_predicate.result_field_size( m_field_size );

  m_stream.ignore( 256, '\n' );

}

template< typename VertexPredicate, typename TexturePredicate >
void flat::PdmFileReader< VertexPredicate, TexturePredicate >::operator() ( const std::shared_ptr< Surface >& surface )
{

  assert( num_vertices() == surface->num_vertices() );

  const size_t field_size = get<0>( m_field_size ) * get<1>( m_field_size );

  size_t vertex_index = 0, texture_index = 0;

  sample_mask_t sample_type;

  do // iterate over all samples in file
  {
    sample_type = next_sample();
	
	if( sample_type & VERTEX )
    {
      Surface::vertex_handle current_vertex = surface->vertex( vertex_index++ );
      current_vertex.set_location( vertex_location );
      current_vertex.set_texture_coordinate( vertex_texture_coordinate );

      # if defined DBG_FLAT_PDM_FILE_READER_VERTEX_SAMPLE
      std::clog << "flat::PdmFileReader()\t| " << " sample " << m_index
	    		<< current_vertex  << std::endl;
      # endif
    }

    if( sample_type & TEXTURE )
    {
      surface->texture().set_pixel( texture_index++, texture_color );
      
      # if defined DBG_FLAT_PDM_FILE_READER_TEXTURE_SAMPLE
      std::clog << "flat::PdmFileReader()\t| " << " sample " << m_index
                << " pixel " + boost::lexical_cast< std::string >( texture_index )
                << " " << texture_color << std::endl;
      # endif
    }
    
  } while( sample_type != END && advance_index() );// of while next sample

}


template< typename VertexPredicate, typename TexturePredicate >
typename flat::PdmFileReader< VertexPredicate, TexturePredicate >::sample_mask_t  
  flat::PdmFileReader< VertexPredicate, TexturePredicate >::next_sample()
{
  bool vertex_ready  = m_vertex_predicate ( m_index );
  bool texture_ready = m_texture_predicate( m_index );

  while( !vertex_ready && !texture_ready )
  { 
    if( !skip_sample() ) return END;
    
    vertex_ready  = m_vertex_predicate ( m_index );
    texture_ready = m_texture_predicate( m_index );
  } 

  read_sample();
    
  return vertex_ready ? ( texture_ready ? VERTEX|TEXTURE : VERTEX ) : TEXTURE;
}


template< typename VertexPredicate, typename TexturePredicate >
bool flat::PdmFileReader< VertexPredicate, TexturePredicate >::read_sample()
{
  std::string token;
  
  // reading line from file
  std::getline( m_stream, token, '\n' );

  // extract position and color
  std::istringstream ss( token );

  ss >> vertex_location[0] >> vertex_location[1] >> vertex_location[2];
  ss >> texture_color[0] >> texture_color[1] >> texture_color[2];

  vertex_texture_coordinate[0] = ( m_index % get<0>( m_field_size ) ) / ( get<0>(m_field_size) - 1. );
  vertex_texture_coordinate[1] = ( m_index / get<0>( m_field_size ) ) / ( get<1>(m_field_size) - 1. );

  return true;
}
