/***************************************************************************
 *            mmp-window.h
 *
 *  Fri Apr  9 13:04:36 2010
 *  Copyright  2010  Peter Urban
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

# include "surface.h"

# include "mmp-common.h"

# include "utk/ray.h"
//# define DBG_FLAT_MMP_WINDOW_CONSTRUCTION
//# define DBG_FLAT_MMP_WINDOW_DESTRUCTION

  namespace mmp
  {

    // forward
    class Window;

    std::ostream& operator<<(std::ostream& os,const Window& w);

    bool is_sidelobe( const Window& window );
    bool is_inner_sidelobe( const Window& window );
    bool is_outer_sidelobe( const Window& window );

    
	class Window				
	{
	  public:
        
	    typedef Surface::edge_descriptor	edge_descriptor;
        typedef Surface::vertex_descriptor	vertex_descriptor;
        
	    typedef enum { INNER_SIDELOBE = 3, OUTER_SIDELOBE = 5, PROJECTED = 8, SIDELOBE = 1 } types;

        typedef enum { NONE=0, LEFT_BOUNDARY = LEFT, RIGHT_BOUNDARY = RIGHT, ALL=3 } boundary_t;
        
	  public:
		// debugging
		static size_t		next_id;
		size_t				id;
        vertex_descriptor   ps; 

		edge_descriptor		edge;

	  private:

		// window boundaries relative to the edge
		std::pair< coord_t, coord_t	> m_bounds;

		// distances from pseudosource at the window boundaries
		std::pair< distance_t, distance_t > m_distances;

		// the parent window 
        Window*                     parent;
		
        // geodesic distance from the pseudosource to the source vertex
		distance_t					d;

        
		ps_coord_t 	b0_xps()	const	
        { 
		  const ps_coord_t l = length();
          return .5 * ( ( utk::sqr<ps_coord_t>( ps_distance<LEFT>() ) - utk::sqr<ps_coord_t>( ps_distance<RIGHT>() ) ) / l + l ); 
        }

		Window( Window* p
              , const Surface::edge_descriptor& e
		      , const coord_t&    b0, const coord_t&    b1
		      , const distance_t& d0, const distance_t& d1 
              , const vertex_descriptor& psv
              , const distance_t& psdist
              );

	  public:

        Window() = delete;
        
        static Window*	create_initial( const vertex_descriptor&           		source 
                                      , const Surface::edge_descriptor& 		edge
							          , const coord_t&    b0, const coord_t&    b1
					                  , const distance_t& d0, const distance_t& d1 )
		{ 
		  Window* new_win = new Window( static_cast<Window*>(0), edge, b0, b1, d0, d1, source, distance_t(0.) );
		  
		  # ifdef DBG_FLAT_MMP_WINDOW_CONSTRUCTION
		  std::clog << "mmp::Window::create_initial"
					<< "\t| " << *new_win
					<< std::endl << std::flush;
		  # endif
		  assert( new_win->sanity_check() ); 
		  return new_win;
		}

		
        static Window*	create_projected(  Window*                            	  parent
										, const Surface::edge_descriptor& 	  edge
					 	 				, const coord_t&    b0, const coord_t&    b1
										, const distance_t& d0, const distance_t& d1  )
        { 
		  Window* new_win = new Window( parent, edge, b0, b1, d0, d1, parent->ps, parent->subpath() );

		  # ifdef DBG_FLAT_MMP_WINDOW_CONSTRUCTION
		  std::clog << "mmp::Window::create_projected\t| " << *new_win << std::endl;
		  # endif
            
		  assert( new_win->sanity_check() ); 
		  return new_win; 
		}
		
        template< side_t PSVertexSide >  
        static Window* create_side_lobe( Window* 								 parent
                                       , const Surface::edge_handle& 		     edge
						               , const coord_t&    b0, const coord_t&    b1
						               , const distance_t& d0, const distance_t& d1 
                                       , const vertex_descriptor& 				 psvertex )
        { 
          assert( PSVertexSide == LEFT 
		          ? psvertex == Surface::edge_handle(parent->edge, edge.mesh()).source().descriptor() 
		          : psvertex == Surface::edge_handle(parent->edge, edge.mesh()).target().descriptor() );

		  assert( parent );
		  
		  const distance_t pspath = parent->source_distance< PSVertexSide >();
		  
		  Window* new_win = new Window( parent, edge.descriptor(), b0, b1, d0, d1, psvertex, pspath);

          assert( is_sidelobe( *new_win ) );

          assert( !is_inner_sidelobe( *new_win ) || pspath == new_win->source_distance<PSVertexSide>() );
          
		  # ifdef DBG_FLAT_MMP_WINDOW_CONSTRUCTION
		  std::clog << "mmp::Window::create_side_lobe\t|"
					<< " " << side_traits< PSVertexSide >::string()
					<< " (" << ( is_inner_sidelobe( *new_win ) ? "inside " : "outside") << ')'
					<< " " << *new_win
					<< std::endl;
		  # endif
                    
		  assert( new_win->sanity_check() );
                    
		  return new_win; 
		}

		// Note: sidelobes have to be inserted before projected windows 

        ~Window()
        {
          # ifdef DBG_FLAT_MMP_WINDOW_DESTRUCTION
          std::clog << "mmp::Window::~Window\t\t| id " << id << std::endl;
          # endif
        }
                                               
        operator const std::pair<coord_t,coord_t>& ()   const  
        { return m_bounds; }

        const std::pair< coord_t, coord_t >& 
                                    bounds()        const 
                                    { return m_bounds; }
                                    
		template< side_t Side >  
		coord_t&  					bound()
	   								{ return get< Side > ( m_bounds ); }
									
		template< side_t Side >  
		const coord_t&  			bound()			const
									{ return get< Side > ( m_bounds ); }

		const std::pair< distance_t, distance_t >& 
                                    ps_distances() const
                                    { return m_distances; }

        std::pair< distance_t, distance_t > 
                                    source_distances() const
                                    { return std::make_pair( ps_distances().first + subpath(), ps_distances().second + subpath() ); }
                                    
		template< side_t Side >  
		distance_t&  				ps_distance()
	   								{ return get< Side > ( m_distances ); }
									
		template< side_t Side >
		const distance_t&  			ps_distance()	const
									{ return get< Side > ( m_distances ); }
        template< side_t Side >  
        bool                        has_ps_vertex()  const
                                    { return ps_distance<Side>() == 0; }

        bool                        has_ps_vertex()  const
                                    { return has_ps_vertex< LEFT >() || has_ps_vertex< RIGHT >(); }
                                    
        Window*                     predeccessor()  const
                                    { return parent; }
        
        coord_t                     length()        const
                                    { return bound<RIGHT>() - bound<LEFT>(); }

        const distance_t&           subpath()       const   
                                    { return d; }

		template< side_t Side >
        void                        set(const coord_t& newbound, const ps_t& ps) 
                                    { 
									  bound<Side>() = newbound;
                                      ps_distance<Side>() = pseudosource_distance( newbound, ps );
                                    }
		template< side_t Side >
        void                        set( const std::pair< coord_t, distance_t >& bound_pair ) 
									{
									  get<Side>( m_bounds )    = bound_pair.first;
                                      get<Side>( m_distances ) = bound_pair.second;
                                    }

        void                        invalidate()                                { get<  LEFT >( m_bounds ) = 0;
                                                                                  get< RIGHT >( m_bounds ) = 0;
                                                                                }
                                    
        bool                        is_valid()                          const   { return bound<LEFT>() < bound<RIGHT>(); }
                                    
    	ps_t        				pseudosource()						const	
                                    { 
                                      if( has_ps_vertex<LEFT>() ) 
                                      { 
                                        assert( bound<LEFT>() == coord_t(0.) );
                                        assert( utk::close_ulps( ps_distance<RIGHT>(), length() ) ); 
                                        return ps_t( bound<LEFT>() , 0 ); 
                                      }

									  if( has_ps_vertex<RIGHT>() ) 
                                      { assert( utk::close_ulps( ps_distance<LEFT>(), length() ) ); 
                                        return ps_t( bound<RIGHT>() , 0 ); 
                                      }

									  const ps_coord_t xs  = b0_xps();
                                      const ps_coord_t ysq = std::max( ps_coord_t(0.), utk::sqr<ps_coord_t>( ps_distance<LEFT>() ) - utk::sqr<ps_coord_t>( xs ) );
                                      //std::cerr<<"x "<<xs<<" ysq1 "<<ysq<<" ysq2 "<< (utk::sqr(d1)-utk::sqr(xs-b1)) <<std::endl;
									  return ps_t( xs + bound<LEFT>(), sqrt( ysq ) );
                                    }

        ps_coord_t                  ps_error()                          const   
                                    { 
									  if( is_inner_sidelobe( *this ) ) return ps_coord_t(0.);

									  const ps_coord_t b0psx = b0_xps();
                                      const ps_t ps( b0psx, std::sqrt( utk::sqr<ps_coord_t>( ps_distance<LEFT>() ) - utk::sqr(b0psx) ) ); // TODO optimize
                                      const ps_coord_t      l  = length();        

                                      const ps_coord_t dl  = 1;//l;
                                      const ps_coord_t dd0 = 1;//d0;
                                      const ps_coord_t dd1 = 1;//d1;
                                      const ps_coord_t dpx_dl  =   1 - ps[0] / l;
                                      const ps_coord_t dpx_dd0 =   ps_distance< LEFT>() / l;
                                      const ps_coord_t dpx_dd1 = - ps_distance<RIGHT>() / l;

                                      const ps_coord_t dpx     = std::fabs(dpx_dl * dl) + std::fabs(dpx_dd0 * dd0) + std::fabs(dpx_dd1 * dd1);
                                      const ps_coord_t pyi     = 1 / ps[1];
                                      const ps_coord_t dpy_dpx = ps[0] * pyi;
                                      const ps_coord_t dpy_dd0 = ps_distance<LEFT>() * pyi;

                                      const ps_coord_t dpy     = dpy_dpx * dpx + dpy_dd0 * dd0 ;

                                      //std::clog << dpx << " + " << dpy << " = ";
                                      return std::hypot( dpx, dpy );
                                    }

        coord_t                     frontier_point( const ps_t& ps )	const   
                                    { return std::min( bound<RIGHT>(), std::max( bound<LEFT>(), ps.x() ) ); }

        coord_t                     frontier_point()                    const   
                                    { return frontier_point( pseudosource() ); }

		// returns the distance of the pseudosource to a point (on the windows edge)
		distance_t                  pseudosource_distance( const coord_t& p, const ps_t& ps )	const	
                                    { return std::hypot( p - ps[0], ps[1] ); }

		// returns the distance of the source to a point (on the windows edge)
		distance_t	                source_distance(const coord_t& p, const ps_t& ps )  const	
                                    { return pseudosource_distance( p, ps ) + subpath(); }

		// returns the distance of the source to the endpoints
		template< side_t Side >
        distance_t                  source_distance()   const	
                                    { return ps_distance<Side>() + subpath(); }
        
        // the result is the frontier point and its distance to the pseudosource
		std::pair< coord_t, distance_t >    min_ps_distance(const ps_t& ps)		const	
        { if(      ps[0] >= bound<RIGHT>() ) return std::make_pair( bound<RIGHT>(), ps_distance<RIGHT>() );
          else if( ps[0] <= bound< LEFT>() ) return std::make_pair( bound< LEFT>(), ps_distance<LEFT>() );
		  return std::make_pair( ps[0], ps[1] );
        }

        // the result is the frontier point and its distance to the source
		std::pair< coord_t, distance_t >    min_source_distance(const ps_t& ps) const	
        { 
          const std::pair<coord_t,distance_t> psd = min_ps_distance(ps);
          return std::make_pair( psd.first, psd.second + subpath() );
        }
        
        std::pair< coord_t, distance_t >    min_source_distance()               const	
        { return min_source_distance( pseudosource() ); }

		// returns the maximal distance from the pseudosource to a point p inside [b0,bound<RIGHT>()]
        // p will be either b0 or bound<RIGHT>()
		std::pair< coord_t, distance_t >    max_ps_distance(const ps_t& ps)   	const	
        { 
          const ps_coord_t fp = frontier_point( ps );

          assert( (fp - bound<LEFT>() ) != ( bound<RIGHT>() - fp ) );

          return ( fp - bound<LEFT>() )  > ( bound<RIGHT>() - fp ) 
                 ? std::make_pair( bound< LEFT>(), ps_distance< LEFT>() ) 
                 : std::make_pair( bound<RIGHT>(), ps_distance<RIGHT>() );
  		}

    	bool	sanity_check()	const;

        types   type()  const   
        { return is_inner_sidelobe( *this ) 
                 ? INNER_SIDELOBE 
                 : ( is_outer_sidelobe( *this ) 
                     ? OUTER_SIDELOBE : PROJECTED );
        }
                                    
    };// of class Window


    template< side_t Side >
    bool is_vertex( const coord_t& pos, const coord_t& edge_length)
    { return Side == LEFT ? pos == 0 : utk::close_ulps( pos, edge_length ); }

    // TODO: optimize sidelobe checks
    
    inline bool is_sidelobe( const Window& window )
    { return is_inner_sidelobe( window ) || is_outer_sidelobe( window ); }
    
    inline bool is_inner_sidelobe( const Window& window )
    { return window.has_ps_vertex(); }  
    
    inline bool is_outer_sidelobe( const Window& window )
    { 
      if( !window.predeccessor() ) return !is_inner_sidelobe( window );
      const Window& pred = *window.predeccessor();
      //std::cout << " path " << ( window.subpath() > pred.subpath() ) << " pred sl " << is_inner_sidelobe( pred ) << " win sl " << is_inner_sidelobe( window ) << std::endl;
      return ( window.subpath() > pred.subpath()  || is_inner_sidelobe( pred ) ) && !is_inner_sidelobe( window ); }
    
    // TODO: remove?
    inline bool is_outer_sidelobe( const Window& window, const Surface::edge_handle& edge )
    {
      assert( window.edge == edge.descriptor() );

      const bool is_outer_sl = window.ps == edge.next().target().descriptor();

      assert( is_outer_sl == is_outer_sidelobe( window ) );
      
      return is_outer_sl;
    }


    // !!! use with care - window must not be trimmed/cut
    template< side_t Side >
    inline std::pair< ps_t, distance_t >   predeccessor_bound_ps( const Window& window, const ps_t& ps )
    {
      if( is_sidelobe( window ) ) return std::make_pair( ps, window.subpath() );

      const utk::ray< ps_coord_t, 2 > ps_ray( ps_t( window.bound< Side >(), 0 ) , ps );
      
      assert( window.predeccessor() );

      const ps_t pre_bound =  ps_ray.at_arc_length( window.ps_distance<Side>() - window.predeccessor()->ps_distance< Side >() );
      
      return std::make_pair( pre_bound, window.predeccessor()->source_distance<Side>() );
    }
    
  }// of namespace mmp
