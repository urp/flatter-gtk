/***************************************************************************
 *            he-mesh.h
 *
 *  Sat Nov  7 18:45:50 2009
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

# include <boost/shared_ptr.hpp>
# include <boost/graph/graph_traits.hpp>
# include <boost/graph/adjacency_list.hpp>

# include "utk/ray.h"

//# define DBG_HE_MESH_APPEND_HALF_EDGE_FACE

namespace he
{
  using flat::location_t;
  using flat::coord_t;
  using flat::angle_t;
  using flat::area_t;
  using flat::distance_t;
  
  // primitive iterator handles

  template< class MeshT , class IteratorT, class HandleT>
  class DefaultPrimitiveIterator 
  {

	  IteratorT	m_begin;
	  IteratorT	m_current;
	  IteratorT	m_end;
	
	  HandleT	m_handle;

	  // end position
	  DefaultPrimitiveIterator( IteratorT begin, IteratorT current, IteratorT end, const MeshT& mesh )
	  : m_begin( begin ), m_current( current ), m_end( end )
	  , m_handle( current == end ? *begin : *current, mesh )	{	}

	  void	forward()	{ if( ++m_current != m_end ) m_handle = HandleT( *m_current, m_handle.mesh() ); }
	  
	  void  backward()  { assert( m_current != m_begin ); m_handle = HandleT( *( --m_current ), handle().mesh() ); }

	  HandleT&		 handle()		{ assert( m_handle.descriptor() == *iterator() ); return m_handle; }
	  const HandleT& handle() const	{ assert( m_handle.descriptor() == *iterator() ); return m_handle; }
	
	public:
	  
	  typedef DefaultPrimitiveIterator<MeshT,IteratorT,HandleT> type;
  
	public:

      DefaultPrimitiveIterator() = delete;

	  static std::pair< type, type > create_range( const std::pair<IteratorT,IteratorT>& its, const MeshT& mesh )
	  {	
		assert( its.first != its.second );
		return { type( its.first, its.first, its.second, mesh ), type( its.first, its.second, its.second, mesh ) }; 
	  }
	  
	  type&	operator= ( const DefaultPrimitiveIterator& o )  { m_current = o; }

      // prefix increment/decrement
  	  type&	operator++()  { forward(); return *this; }
      type&	operator--()  { backward(); return *this; }

      // postfix increment/decrement
  	  type	operator++(int) 
      { 
        type current( *this );
		forward();
		return current;
	  }

      type	operator--(int) 
      { 
        type current( *this );
		backward();
		return current;
      }
      
	  HandleT&			operator* ()	        { return handle(); }
	  const HandleT&	operator* ()	const	{ return handle(); }

      HandleT*	        operator->()	      	{ return &handle(); }
	  const HandleT*	operator->()	const  	{ return &handle(); }
      
	  bool				operator!=(const type& o)	const   { return m_current != o.iterator(); }
	  
	  bool				operator==(const type& o)	const   { return m_current == o.iterator(); }
	  
	  const MeshT&		mesh()		const	{ return m_handle.mesh(); }

	  const IteratorT&	iterator()	const	{ return m_current; }
  };
  
  // a mesh implemented by using the boost m_graph library
  template< template<class> class VertexHandleT
		  ,	template<class> class EdgeHandleT
  		  ,	template<class> class FaceHandleT
  		  ,	class CustomVertexProperties = boost::no_property
  		  ,	class CustomEdgeProperties 	 = boost::no_property
          , class CustomGraphProperties  = boost::no_property
  		  >
  class Mesh
  {	  
	public: // type definitions

      typedef Mesh<VertexHandleT,EdgeHandleT,FaceHandleT,CustomVertexProperties,CustomEdgeProperties,CustomGraphProperties> type;

      //----| primitive descriptors
      
	  typedef boost::adjacency_list_traits< boost::vecS, boost::vecS, boost::directedS, boost::listS >::vertex_descriptor  	vertex_descriptor;
	  typedef boost::adjacency_list_traits< boost::vecS, boost::vecS, boost::directedS, boost::listS >::edge_descriptor 	edge_descriptor;

      struct impl_face_type
	  {   
          typedef std::size_t  face_index_t;
          edge_descriptor edge;
		  face_index_t	  index;
          impl_face_type() = default;
		  impl_face_type( const edge_descriptor& e, const face_index_t& i )	: edge( e ), index( i )	{	}
          bool    operator== ( const impl_face_type& other )    const   { return index == other.index; }
	  };
          
      typedef impl_face_type    face_descriptor;
	  
	  //----| preinstalled primitive properties
	  struct vertex_location_t  { typedef boost::vertex_property_tag kind; };
	  struct edge_hds_t 		{ typedef boost::edge_property_tag kind; };

	  struct HDS // the half edge data structure
	  {	vertex_descriptor	source_vertex;
		edge_descriptor		next_edge;
		edge_descriptor		opposite_edge;	// equal to owning edge if no opposite edge exists 
		face_descriptor  	face;
	  };
	  
	  typedef std::size_t 			index_t;
	  
	  // vertex properties ( default location property )
	  typedef boost::property< vertex_location_t, location_t, CustomVertexProperties >	vertex_properties;
	  // edge properties ( default half edge data structure property and index map )
	  typedef boost::property< boost::edge_index_t, index_t, CustomEdgeProperties > 	edge_properties;
	  // graph properties 
      typedef CustomGraphProperties	graph_properties;

	  // graph type storing vertices and edges
	  typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS, 
	  				        		 vertex_properties, 
	  						         boost::property< edge_hds_t, HDS, edge_properties >,
	  						         graph_properties
	  						       > graph_t;	
      
      // face storage type - later accessed from half-edge datastructure
      typedef std::vector<face_descriptor>  face_vector_t;

      // size types

   	  typedef typename boost::graph_traits<graph_t>::vertices_size_type vertices_size_type;
   	  typedef typename boost::graph_traits<graph_t>::edges_size_type    edges_size_type;
	  typedef size_t													faces_size_type;
		  
	  // primitive handles
	  
	  typedef VertexHandleT< type >	    vertex_handle;
	  typedef EdgeHandleT  < type >	    edge_handle;
	  typedef FaceHandleT  < type >	    face_handle;

	  friend class VertexHandleT< type >;
	  friend class EdgeHandleT  < type >;
	  friend class FaceHandleT  < type >;
	  
	  // iterators
	  
	  typedef typename boost::graph_traits<graph_t>::vertex_iterator    vertex_iterator;
	  typedef typename boost::graph_traits<graph_t>::edge_iterator 		edge_iterator;
	  typedef typename boost::graph_traits<graph_t>::out_edge_iterator 	out_edge_iterator;
	  typedef typename face_vector_t::iterator                          face_iterator;

	  // iterator handles

	  typedef DefaultPrimitiveIterator<type,vertex_iterator,vertex_handle>	vertex_handle_iterator;
	  typedef DefaultPrimitiveIterator<type,edge_iterator,edge_handle>		edge_handle_iterator;
	  typedef DefaultPrimitiveIterator<type,out_edge_iterator,edge_handle>	out_edge_handle_iterator;
	  typedef DefaultPrimitiveIterator<type,face_iterator,face_handle>		face_handle_iterator;
		
	protected:	

	  // data members
	  
	  graph_t		m_graph;
	  face_vector_t	m_face_vec;

	  size_t		m_num_half_edges;
      size_t        m_num_full_edges;

	protected:

      struct append_return_type
	  {
		face_descriptor	face;
		edge_descriptor	edge_ab, edge_bc, edge_ca;
	  };
        
      append_return_type	append_half_edge_face( const vertex_descriptor& a, const vertex_descriptor& b, const vertex_descriptor& c,
	  											   const edge_properties& ab_prop, const edge_properties& bc_prop, const edge_properties& ca_prop );

	public:

	  Mesh() = default;
	  
	  Mesh( vertices_size_type vertex_count = 0
          , const graph_properties& graph_properties = graph_properties() )
	  : m_graph( vertex_count, graph_properties )
      , m_face_vec(), m_num_half_edges( 0 ), m_num_full_edges( 0 ) {   }


      graph_t&           get_graph()       { return m_graph; }
      const graph_t&     get_graph() const { return m_graph; }
      
	  // boost property map accessors
        
	  template<typename Tag>  typename boost::property_map<graph_t,Tag>::type
      get_property_map()			
      { return boost::get( Tag(), get_graph() ); }
      
	  template<typename Tag>  typename boost::property_map<graph_t,Tag>::const_type
      get_property_map()	    const	
      { return boost::get( Tag(), get_graph() ); }
        
	  template<typename Tag>  typename boost::graph_property<graph_t,Tag>::type
      get_graph_property()    	
      { return boost::get_property( get_graph(), Tag() ); }
      
	  template<typename Tag>  typename boost::graph_property<graph_t,Tag>::const_type
      get_graph_property()    const	
      { return boost::get_property( get_graph(), Tag() ); }

	  template<typename Tag>
	  typename boost::property_traits< typename boost::property_map<graph_t,Tag>::type >::value_type
      get( const vertex_descriptor& vertex )
      { return boost::get( Tag(), get_graph(), vertex ); }
        
	  template<typename Tag>
	  typename boost::property_traits< typename boost::property_map<graph_t,Tag>::const_type >::value_type
      get( const vertex_descriptor& vertex )	const	
      { return boost::get( Tag(), get_graph(), vertex ); }
	  
	  template<typename Tag>
	  typename boost::property_traits< typename boost::property_map<graph_t,Tag>::type >::value_type
      get( const edge_descriptor& edge )
      { return boost::get( Tag(), get_graph(), edge ); }
        
	  template<typename Tag>
	  typename boost::property_traits< typename boost::property_map<graph_t,Tag>::const_type >::value_type
      get( const edge_descriptor& edge )	const	
      { return boost::get( Tag(), m_graph, edge ); }
        
	  template<typename Tag> 
      void	put( const vertex_descriptor& vertex
               , const typename boost::property_traits< typename boost::property_map<graph_t,Tag>::type >::value_type& value ) const
      { boost::put( Tag(), const_cast<graph_t&>( m_graph ), vertex, value); }
        
  	  template<typename Tag>
	  void	put( const edge_descriptor& edge
               , const typename boost::property_traits< typename boost::property_map<graph_t,Tag>::type >::value_type& value ) const
      { boost::put( Tag(), m_graph, edge, value ); }

	  // query number of primitives

	  const vertices_size_type	num_vertices()	const	{ return boost::num_vertices(m_graph); }

	  const edges_size_type		num_edges()		const	{ return boost::num_edges(m_graph); } // TODO: profile against m_num_half_edges

	  const edges_size_type&	num_full_edges()const	{ return m_num_full_edges; }
      
	  const faces_size_type		num_faces()		const	{ return m_face_vec.size(); }

      const 
	  // iterator interface

	  std::pair< vertex_handle_iterator, vertex_handle_iterator >	
		vertex_handles()    const
      { return vertex_handle_iterator::create_range( boost::vertices( m_graph ), *this ); }
      
	  std::pair< edge_handle_iterator, edge_handle_iterator >
		edge_handles()	    const	
      { return edge_handle_iterator::create_range( boost::edges( m_graph ), *this ); }

	  std::pair< out_edge_handle_iterator, out_edge_handle_iterator >
		out_edge_handles( const vertex_descriptor& vertex )	const
	  { return out_edge_handle_iterator::create_range( boost::out_edges( vertex, m_graph ), *this ); }
		
	  std::pair< face_handle_iterator, face_handle_iterator >      
		face_handles()	const	
      { auto it_pair = std::make_pair( const_cast< face_vector_t* >( &m_face_vec )->begin()
                                     , const_cast< face_vector_t* >( &m_face_vec )->end()
                                     );
        return face_handle_iterator::create_range( it_pair, *this ); 
      }

	  // mesh modification
	  
	  void	clear()										
      { m_graph.clear();
	    m_face_vec.clear();
	  }

	  void	clear_edges()										
      {
        boost::remove_edge_if( [] ( const edge_descriptor& ) { return true; }, m_graph );
	    m_face_vec.clear();
	  } 
      
	  vertex_handle	    create_vertex( const typename type::vertex_properties& p = typename type::vertex_properties() )
	  { 
        const vertex_handle nv( boost::add_vertex( p , type::m_graph ), *this );

		# if defined DBG_HE_MESH_CREATE_VERTEX
        std::clog << "he::Mesh::create_vertex\t|"
                  <<" descriptor #" << nv.descriptor()
                  <<" location " << nv.location() << std::endl;
		# endif
		
        return nv; 
      }

      struct create_face_return_type
	  { face_handle	face;	  
		edge_handle edge_ab, edge_bc, edge_ca;
	  };
      
	  create_face_return_type		create_face( const vertex_descriptor& a, 
			  							         const vertex_descriptor& b, 
			  								     const vertex_descriptor& c,
			  								     const typename type::edge_properties& ab_prop = typename type::edge_properties(),
			  								     const typename type::edge_properties& bc_prop = typename type::edge_properties(),
			  								     const typename type::edge_properties& ca_prop = typename type::edge_properties() )
	  { 
        append_return_type added = append_half_edge_face(a,b,c,ab_prop,bc_prop,ca_prop);

        return { face_handle( added.face   , *this)
		  	   , edge_handle( added.edge_ab, *this)
			   , edge_handle( added.edge_bc, *this)
			   , edge_handle( added.edge_ca, *this)
	  		   };
	  }

	  vertex_handle     vertex(const vertex_descriptor& d)	const	{ return vertex_handle(d,*this); }

	  std::pair<edge_handle,bool>	edge( const vertex_descriptor& source, const vertex_descriptor& target ) const	
      { 
        //std::clog<<"he::Mesh::edge\t|source "<<source<<"\t|target "<<target<<std::flush;

        std::pair< edge_descriptor, bool > edge = boost::edge( source, target, m_graph );

        //std::clog<<"\t|retrieved "<<e.second<<std::endl<<std::flush;	

        return { edge_handle( edge.first, *this ), edge.second }; 
	  }
  }; //Mesh


  
  // a helper class which saves:
  // - the descriptor of a geometric primitive 
  // - a reference to the associated mesh
  // A low-level interface is provided.
  template<class Descriptor, class Storage >
  class PrimitiveSlot				
  {
      friend class Storage::vertex_handle_iterator;    
      friend class Storage::edge_handle_iterator;    
      friend class Storage::face_handle_iterator;    
    
	  const Storage&		    		m_storage;

	protected:

	  Descriptor	m_descriptor;

	public:

	  PrimitiveSlot() = delete;

	  PrimitiveSlot( const PrimitiveSlot<Descriptor,Storage>& o ) = default;
	  
	  PrimitiveSlot( const Descriptor& primitive_descriptor, const Storage& storage )
	  : m_storage( storage ), m_descriptor( primitive_descriptor )	{  }

  	  virtual		~PrimitiveSlot()		{	}

	  operator 		 Descriptor& ()			{ return m_descriptor; }

	  operator const Descriptor& ()	const	{ return m_descriptor; }

	  PrimitiveSlot<Descriptor,Storage>& operator= ( const PrimitiveSlot<Descriptor,Storage>& o )
	  { m_descriptor = o;
	    assert( &m_storage == &o.mesh() );
	    return *this;
	  }

	  const Storage&	mesh()	const	{ return m_storage; }
      
	  const Descriptor&	descriptor()	const	{ return m_descriptor; }
  };


  //----| default handles

  //----|vertex handle
  
  template< class Storage >
  class DefaultVertexHandle	: public PrimitiveSlot< typename Storage::vertex_descriptor, Storage >
  {  	
	public:
	  typedef typename Storage::vertex_descriptor 		 vertex_descriptor;
  	  typedef typename Storage::out_edge_iterator 		 out_edge_iterator;
  	  typedef typename Storage::out_edge_handle_iterator out_edge_handle_iterator;
	  
	  typedef typename boost::property_map<typename Storage::graph_t, typename Storage::vertex_location_t>::const_type	
																				const_location_map_t;
	  
	  typedef typename boost::property_traits< const_location_map_t >::value_type 		location_t;
		
	  DefaultVertexHandle(const vertex_descriptor& v,const Storage& m)
	  : PrimitiveSlot< vertex_descriptor, Storage >(v,m)
	  { 	}


	  DefaultVertexHandle(const DefaultVertexHandle<Storage>& o)
	  : PrimitiveSlot< vertex_descriptor, Storage >(o)		
	  {	}

	  const location_t&		location()	const	
	  { return this->mesh().template get_property_map< typename Storage::vertex_location_t >()[ this->descriptor() ]; }
      
	  void					set_location( const location_t& location )		
	  { this->mesh().template put< typename Storage::vertex_location_t >( this->descriptor(), location ); }

      std::pair< out_edge_iterator, out_edge_iterator > 
		out_edges()   const
      { return boost::out_edges( this->descriptor(), this->mesh().get_graph() ); }

      std::pair< out_edge_handle_iterator, out_edge_handle_iterator > 
		out_edge_handles()   const
      { return this->mesh().out_edge_handles( this->descriptor() ); }
	  
	  angle_t	total_angle()	const	
      { 
		angle_t angle = 0;
	    for( auto out_its = out_edge_handles(); out_its.first != out_its.second; out_its.first++ )
          angle += out_its.first->previous().next_inner_angle();
        //std::cout << "he::DefaultEdgeHandle::total_angle\t|" << angle << std::endl; 
	    return angle;
        
	  }

	  area_t	one_ring_area()
	  { 
		area_t area = 0;
	    for( auto out_its = out_edge_handles(); out_its.first != out_its.second; out_its.first++ )
          area += out_its.first->face().area();
        //std::cout << "he::DefaultEdgeHandle::area\t|" << area << std::endl; 
		return area;
	  }	

	  bool		is_boundary_vertex()
	  {
		for( auto out_its = out_edge_handles(); out_its.first != out_its.second; out_its.first++ )
          if( !out_its.first->opposite().second ) return true;
		return false;
	  }
  };

  
  // edge handle

  template< class Storage >
  class DefaultEdgeHandle : public PrimitiveSlot< typename Storage::edge_descriptor, Storage >
  { 
	public:
	  // primitve handles & descriptors
	  typedef typename Storage::edge_descriptor	edge_descriptor;
	  typedef typename Storage::vertex_handle	vertex_handle;
	  typedef typename Storage::edge_handle		edge_handle;
   	  typedef typename Storage::face_descriptor	face_descriptor;
	  typedef typename Storage::face_handle		face_handle;

	  // type of location stored in the graph
	  typedef typename boost::property_map< typename Storage::graph_t, typename Storage::vertex_location_t >::type::value_type
												location_t;
	  // type of half edge data structure
	  typedef typename boost::property_map< typename Storage::graph_t, typename Storage::edge_hds_t >::const_type 			
	  											hds_map_t;
	  typedef typename hds_map_t::value_type	hds_t;
	  typedef typename hds_map_t::reference		hds_ref_t;
	  // type of edge index
	  typedef typename boost::property_traits< typename boost::property_map< typename Storage::graph_t, boost::edge_index_t >::type >::value_type
												index_t;

  	  friend std::ostream&	operator<<( std::ostream& os, const DefaultEdgeHandle<Storage>& eh )
	  {
		os << "edge "      << eh.descriptor()
		   << " next "     << eh.next().descriptor()
		   << " id "       << eh.index()
		   << " op "       << eh.opposite().second
		   << " vertices " << eh.source().descriptor() << " -> " << eh.target().descriptor()
		   << " length "   << eh.length();
		return os;
	  }
	  
	private:
	  
	  // half-edge data structure
	  hds_t	m_he;
	  
	public:
		  
	  DefaultEdgeHandle( const edge_descriptor& edge, const Storage& mesh )
	  : PrimitiveSlot<edge_descriptor,Storage>( edge, mesh ),
	    m_he( mesh.template get<typename Storage::edge_hds_t>( edge ) )	{ 	}
	  
	  vertex_handle	source()    const	{ return vertex_handle( m_he.source_vertex, this->mesh() ); }

	  vertex_handle	target()	const	{ return vertex_handle( next().source().descriptor() , this->mesh() ); }
	  
	  edge_handle	next()		const	{ return edge_handle( m_he.next_edge, this->mesh() ); }
      
  	  edge_handle	previous()	const   { return next().next(); }
	  
	  std::pair< edge_handle, bool >	opposite()	const   
      { 
		edge_handle opp( m_he.opposite_edge, this->mesh() );
	    return { opp, this->operator!=( opp ) };  
	  }
      
	  face_handle   face()	const	{ return face_handle( m_he.face, this->mesh()); }
      
      index_t   	index()	const	{ return this->mesh().get< boost::edge_index_t>( this->descriptor() ); }

	  hds_ref_t     hds()	const	{ return m_he; }
	  
	  location_t    vector()const	{ return target().location() - source().location(); }

	  coord_t       length()const	{ return vector().length(); }

	  
	  utk::ray< coord_t, 3 >	ray() const	{ return utk::ray< coord_t, 3 >( source().location(), target().location() ); }
	  
      coord_t next_inner_angle() const
      { 
        location_t        first  = -vector();
        location_t        last   = next().vector();

        return std::acos( dot( first, last ) / ( utk::length( first ) * utk::length( last ) ) ); 
      }

      bool	operator==(const edge_handle& o)const   
	  {
		assert( ( o.descriptor() == this->descriptor() ) == (   o.source().descriptor() == source().descriptor() 
			        										 && o.target().descriptor() == target().descriptor() ) );
  		return o.descriptor() == this->descriptor(); 
	  }
      
	  bool	operator!=(const edge_handle& o)const   
      { return !this->operator==(o); }

  };



  // face handle
  
  template< class Storage>
  class DefaultFaceHandle : public PrimitiveSlot< typename Storage::face_descriptor, Storage >
  {	  
	public:
	  
	  typedef typename Storage::vertex_descriptor	vertex_descriptor;
	  typedef typename Storage::edge_descriptor 	edge_descriptor;  
  	  typedef typename Storage::face_descriptor 	face_descriptor;  
	  typedef typename Storage::vertex_handle		vertex_handle;
	  typedef typename Storage::edge_handle 		edge_handle;  
  	  typedef typename Storage::face_handle 		face_handle;  

	public:

	  DefaultFaceHandle( const face_descriptor& face, const Storage& mesh )
	  : PrimitiveSlot< face_descriptor, Storage >( face, mesh )	{ 	}

      edge_handle	edge()	const	
      { 
  		return edge_handle( PrimitiveSlot< face_descriptor, Storage >::descriptor().edge,
		 					PrimitiveSlot< face_descriptor, Storage >::mesh() );
	  }
      
      edge_handle	edge( const utk::size_t i )	const	
      { 
		assert( i < 3 );
		return i == 0 ? edge() : ( i == 1 ? edge().next() : edge().next().next() );
	  }

	  vertex_handle	vertex( const utk::size_t i )	const	{ return edge(i).source(); }

      location_t    normal()    const   { return cross( edge().vector(), edge().next().vector() ).normalize(); }

	  // TODO use potentially faster coordinate based computation
	  area_t		area()		const	{ return length( cross( edge().vector(), edge().next().vector() ) ) / 2;	}
  };
 
}//he

//------------------------------------------------------------------------------
// IMPLEMENTATION ==============================================================

template< template<class> class V, template<class> class E, template<class> class F, class VProp, class EProp, class GProp	>
typename he::Mesh<V,E,F,VProp,EProp,GProp>::append_return_type	
  he::Mesh<V,E,F,VProp,EProp,GProp>::append_half_edge_face( const vertex_descriptor& a, const vertex_descriptor& b, const vertex_descriptor& c
	  		 											  , const edge_properties& ab_prop, const edge_properties& bc_prop, const edge_properties& ca_prop )
{ 
  // add half edges surrounding the face
  const std::pair<edge_descriptor,bool> ab = boost::add_edge( a, b, boost::property<edge_hds_t, HDS, edge_properties>( HDS(), ab_prop ), m_graph );
  const std::pair<edge_descriptor,bool> bc = boost::add_edge( b, c, boost::property<edge_hds_t, HDS, edge_properties>( HDS(), bc_prop ), m_graph );
  const std::pair<edge_descriptor,bool> ca = boost::add_edge( c, a, boost::property<edge_hds_t, HDS, edge_properties>( HDS(), ca_prop ), m_graph );
  assert( ab.second );
  assert( bc.second );
  assert( ca.second );

  // create the face object
  const face_descriptor	f( ab.first, m_face_vec.size() );

  // get opposite edges
  const std::pair<edge_descriptor,bool> ab_op = boost::edge( b, a, m_graph );
  const std::pair<edge_descriptor,bool> bc_op = boost::edge( c, b, m_graph );
  const std::pair<edge_descriptor,bool> ca_op = boost::edge( a, c, m_graph );

  // add half-edge data-structure - use the edge descriptor to mark invalid edges
  HDS	ab_hds, bc_hds,	ca_hds;
	
  if(ab_op.second)
  { 
    ab_hds = { a, bc.first, ab_op.first, f };
    boost::get( edge_hds_t(), m_graph )[ab_op.first].opposite_edge = ab.first;
  }else { ab_hds = { a, bc.first, ab.first, f }; ++m_num_full_edges; }

  if(bc_op.second)
  { 
    bc_hds = { b, ca.first, bc_op.first, f };
    boost::get( edge_hds_t(), m_graph )[bc_op.first].opposite_edge = bc.first;
  }else { bc_hds = { b, ca.first, bc.first, f }; ++m_num_full_edges; }

  if(ca_op.second)
  { 
    ca_hds = { c, ab.first, ca_op.first, f };
    boost::get( edge_hds_t(), m_graph )[ca_op.first].opposite_edge = ca.first;
  }
  else { ca_hds = { c, ab.first, ca.first, f }; ++m_num_full_edges; }

  boost::put( edge_hds_t(), m_graph, ab.first, ab_hds );
  boost::put( edge_hds_t(), m_graph, bc.first, bc_hds );
  boost::put( edge_hds_t(), m_graph, ca.first, ca_hds );

  boost::put( boost::edge_index_t(), m_graph, ab.first, m_num_half_edges++ );
  boost::put( boost::edge_index_t(), m_graph, bc.first, m_num_half_edges++ );
  boost::put( boost::edge_index_t(), m_graph, ca.first, m_num_half_edges++ );

  # if defined DBG_HE_MESH_APPEND_HALF_EDGE_FACE

  std::clog << "he::Mesh::append_half_edge_face\t| a-b " << edge_handle( ab.first, *this ) << std::endl; 
  std::clog << "he::Mesh::append_half_edge_face\t| b-c " << edge_handle( bc.first, *this ) << std::endl; 
  std::clog << "he::Mesh::append_half_edge_face\t| c-a " << edge_handle( ca.first, *this ) << std::endl; 

  # endif

  m_face_vec.push_back(f);

  return { f, ab.first, bc.first, ca.first };
}

