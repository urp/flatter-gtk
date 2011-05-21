/***************************************************************************
 *            surface.h
 *
 *  Wed Aug  5 23:46:44 2009
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

# include "common.h"
# include "he-mesh.h"

namespace flat
{

  template< class MeshT > class VertexHandle;
  
  // additional vertex properties
  struct vertex_texture_coord_t { typedef boost::vertex_property_tag kind; 
								  typedef utk::veca< float, 2 > type;
  								};
  template< typename MeshT >
  std::ostream&   operator<<( std::ostream&, flat::VertexHandle<MeshT> const& );
  
  // vertex handle
  template< class MeshT >
  class VertexHandle	: public he::DefaultVertexHandle< MeshT >
  {
    
	public:

      typedef typename MeshT::vertex_descriptor vertex_descriptor;
	  typedef typename MeshT::vertex_handle     vertex_handle;
      typedef typename MeshT::edge_handle       edge_handle;

	  typedef typename boost::property_map<typename MeshT::graph_t, vertex_texture_coord_t >::const_type	
																				const_texture_coord_map_t;
	  
	  typedef typename boost::property_traits<const_texture_coord_map_t>::value_type 	color_t;

      friend std::ostream&	 operator<< <MeshT>( std::ostream&, VertexHandle<MeshT> const& );
      
    public:

      
	  VertexHandle( const vertex_descriptor& v, const MeshT& m )
	  : he::DefaultVertexHandle<MeshT>(v,m)		{ 	}

	  VertexHandle( const he::DefaultVertexHandle< MeshT >& o )
	  : he::DefaultVertexHandle<MeshT>(o)		{	}

	  const vertex_texture_coord_t::type&	texture_coordinate()	const	
      { 
        return this->mesh().template get_property_map< vertex_texture_coord_t >() [ *this ]; 
      }
      
	  void	set_texture_coordinate( const vertex_texture_coord_t::type& value )	
      { 
        return this->mesh().put< vertex_texture_coord_t >( this->descriptor(), value ); 
      }
  };

  // the mesh type 
  typedef he::Mesh< VertexHandle, he::DefaultEdgeHandle, he::DefaultFaceHandle
			      , boost::property< vertex_texture_coord_t, vertex_texture_coord_t::type >
                  , boost::no_property, boost::no_property >	surface_mesh_t;

  
  class PointCloud : public surface_mesh_t
  {  

    private:
	  
	  mutable location_t	m_min_location;
	  mutable location_t	m_max_location;

	  void	comp_min_max_xy()	const;
	  void	comp_min_max_z()	const;
	  
    protected:
	  
	  PointCloud( vertices_size_type vertex_count )
	  :	surface_mesh_t( vertex_count ) 
	  , m_min_location(   std::numeric_limits<coord_t>::infinity() )
	  ,	m_max_location( - std::numeric_limits<coord_t>::infinity() )			
	  {	}

    public:
      
	  virtual   ~PointCloud()	{	}

	  const distance_t	distance( vertex_descriptor a, vertex_descriptor b)	const   
      { return utk::distance( vertex( a ).location(), vertex( b ).location() ); }		  

	  virtual void	prepare_step()  { set_min_max(); }

      
	  void	set_min_max( const location_ref_t& min = location_t( std::numeric_limits<coord_t>::infinity()), 
			           	 const location_ref_t& max = location_t(-std::numeric_limits<coord_t>::infinity()) )										
      { m_min_location = min;
	    m_max_location = max;
	  }

	  void	set_min_max_z( const coord_t min =  std::numeric_limits<coord_t>::infinity(), 
			           	   const coord_t max = -std::numeric_limits<coord_t>::infinity() )
	  { m_min_location.z() = min;
	    m_max_location.z() = max;
	  }

	  const coord_t&	min_height()    const
      { if( m_min_location.z() == std::numeric_limits<coord_t>::infinity() )
          comp_min_max_z();
        return m_min_location.z();
	  }
      
	  const coord_t&  	max_height()	const	
      { if( m_max_location.z() == - std::numeric_limits<coord_t>::infinity() )
          comp_min_max_z();
        return m_max_location.z();
	  }							

	  // TODO use bounding box interface
	  location_t    min_location()	const
      { if( ( m_min_location.xy() == std::numeric_limits<coord_t>::infinity() ) == true )
		  comp_min_max_xy();
		if( m_min_location.z() == std::numeric_limits<coord_t>::infinity() )
          comp_min_max_z();
        return m_min_location;
	  }

	  location_t    max_location()  const
      { if( ( m_max_location.xy() == - std::numeric_limits<coord_t>::infinity() ) == true )
          comp_min_max_xy();
        if( m_max_location.z() == - std::numeric_limits<coord_t>::infinity() )
          comp_min_max_z();
        return m_max_location;
      }
	  
  };

  
  class Surface : public PointCloud
  {
    public:

      typedef std::pair< vertex_descriptor, vertex_descriptor > vertex_pair;
      typedef std::pair< vertex_handle, vertex_handle > vertex_handle_pair;

      static vertex_handle_pair make_vertex_handle_pair( const vertex_pair& pair, const Surface& surface )
      { return { vertex_handle( pair.first, surface ), vertex_handle( pair.second, surface ) }; }
      
      struct texture_type
      {
		typedef	color_channel_t	channel_type;

        static const size_t num_components = 4;
		
        size_pair size;
        std::vector< channel_type > 	pixmap;

        texture_type( texture_type&& other )
        : size( std::move( other.size ) ), pixmap( std::move( other.pixmap ) )
        {   }

        texture_type( const size_pair& o_size )
        : size( o_size ), pixmap( std::get<0>(o_size) * std::get<1>(o_size) * num_components ) 
        {   }  

		void	set_pixel( size_t pixel_index
			             , const rgba_color_ref_t& color ) 
		{ 
		  for( size_t i = 0; i < num_components; ++i ) 
		   pixmap[ pixel_index * num_components + i ] = color[i];
		}
        
        const rgba_color_t get_pixel( const size_t index ) const
        { return rgba_color_t( pixmap[ index + 0 ], pixmap[ index + 1 ], pixmap[ index + 2 ], pixmap[ index + 3 ] ); }
        
        const rgba_color_t get_pixel( const size_t s, const size_t t ) const
        { return get_pixel( ( s + t * size.first ) * num_components ); }
      };

      struct distance_function
      {
        typedef boost::numeric::ublas::symmetric_matrix< distance_t, boost::numeric::ublas::upper > distance_matrix_type;

        typedef enum { NONE = 0, NEIGHBORS = 1, ALL = 2 } neighborhood_type;
        typedef int neighborhood_mask_type;

        neighborhood_mask_type  neighborhood;
          
        distance_matrix_type distance_matrix;

        //TODO move construction
        distance_function( const distance_function& d ) : neighborhood( d.neighborhood ), distance_matrix( d.distance_matrix ) { }
        distance_function( distance_function&& d ) : neighborhood( d.neighborhood ), distance_matrix( d.distance_matrix ) { }
        distance_function() : neighborhood( NONE )  {   }
        
        void compute_distances( const std::shared_ptr<Surface>&, neighborhood_mask_type );

        distance_matrix_type::const_reference operator() ( vertex_descriptor a, vertex_descriptor b )   const
        { 
          return distance_matrix(a,b); 
        }

        void set_distances( const distance_matrix_type& matrix, neighborhood_mask_type neighbors )
        { distance_matrix = matrix;
          neighborhood = neighbors;
        }
        
        private:
          void compute_all_to_all( const std::shared_ptr< Surface >& );
          void compute_all_to_neighbors( const std::shared_ptr< Surface >& );
      };
      
    public: // initial (geodesic) distances
       
      distance_function initial_distances;

      friend std::istream&   operator>>( std::istream&, Surface::distance_function& );
      friend std::ostream&   operator<<( std::ostream&, Surface::distance_function const& );
    private:

	  std::string	m_name;
	  
	  // vector containing three (rgb-)color components per texel
	  texture_type  m_texture;

	  // the minimum and maximum value of the gaussian curvature on ther surface
	  mutable std::pair<coord_t,coord_t>	curvature_extrema;
      
    protected:
     
      Surface( const vertices_size_type vertex_count = 0
             , const size_pair& texture_size = size_pair{ 0, 0 }
             , const std::string& name = "untitled" );
      
    public:

      template< typename Generator, typename Triangulator, typename Transform >
      static std::shared_ptr< Surface > create_with_generator( Generator&    generator
                                                             , Triangulator& triangulator
                                                             , Transform&    transform )
      {
        std::shared_ptr< Surface > surface( new Surface( generator.num_vertices(), generator.texture_size(), generator.get_name() ) );

        generator( surface );

        triangulator( surface );

        transform( surface );
        
        surface->initial_distances.compute_distances( surface, distance_function::NEIGHBORS );  

        return surface;
	  }

      
      virtual std::vector< vertex_descriptor > neighbors( const vertex_descriptor descriptor ) const
      { 
        auto adj = adjacent_vertices( descriptor, get_graph() ); 
       
        return std::vector< vertex_descriptor >( adj.first, adj.second ); 
      }
      
      virtual std::vector< vertex_pair > neighbors() const;

	  const	std::string&	get_name()	const	{ return m_name; }
	  
      texture_type&         texture()           { return m_texture; }
      const texture_type&   texture()   const   { return m_texture; }

      const std::pair<coord_t,coord_t>&	get_curvature_extrema()	const	{ return curvature_extrema; }
		  
	  void		set_curvature_extrema( const float min =  std::numeric_limits<float>::infinity() 
									 , const float max = -std::numeric_limits<float>::infinity() ) const
	  { curvature_extrema = { min, max }; }

	  virtual void	prepare_step()	
      { set_curvature_extrema();
	    PointCloud::prepare_step();
	  }

      // triangulation
      
      bool has_triangulation() const   { return num_edges(); }

      void remove_triangulation()   { clear_edges(); }

      std::pair< coord_t, distance_function::neighborhood_mask_type > get_squared_distance_error() const;
      
  };
  

  inline std::ostream&   operator<<( std::ostream& os, Surface::distance_function const& distances  )
  {
    os << "nb " << distances.neighborhood << ' ' << distances.distance_matrix;
    return os;
  }

  inline std::istream&   operator>>( std::istream& is, Surface::distance_function& distances  )
  {
    std::string token;
    is >> token;
    if( token != "nb" ) 
      std::cerr << "ERROR in distance function input operator - "
                << "expected token \"nb\" at the beginning of the stream." << std::endl; 
    is >> distances.neighborhood >> distances.distance_matrix;
    return is;
  }
    
  template< typename MeshT >
  std::ostream&   operator<<( std::ostream& os, flat::VertexHandle<MeshT> const& vertex )
  {
    os << "vertex "    << vertex.descriptor()
       << " location " << vertex.location()
       << " texcoord " << vertex.texture_coordinate();
    return os;
  }

}


