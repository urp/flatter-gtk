//libutk - a utility library 
//Copyright (C) 2006-2010  Peter Urban (peter.urban@s2003.tu-chemnitz.de)
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef	UTK_GEOMETRY_H
#define	UTK_GEOMETRY_H

#include	"math.h"
#include	"vecn.h"
#include	"ray.h"
#include	<cmath>
#include	<limits>
#include	<cassert>

#pragma GCC visibility push(default)

namespace utk
{ 

  template< typename T >
  std::pair< T, T > triangulate( const T& base_length, const T& left_distance, const T& right_distance)
  { 
    assert( base_length > T(0) );
    assert( left_distance >= T(0) );
    assert( right_distance >= T(0) );

    if(  left_distance == T(0) ) return std::make_pair( T(0) , T(0) );
    if( right_distance == T(0) ) return std::make_pair( base_length , T(0) );

    const T sqr_left_distance  = sqr(left_distance);
    
    std::pair< T, T > p( T(.5) * ( ( sqr_left_distance + sqr(base_length) ) - sqr(right_distance) ) / base_length, T(0) );

    p.second = std::sqrt( std::max( T(0), sqr_left_distance - sqr(p.first) ) );

    return p;
  }

  
  enum	classification_t	{ INSIDE = -1, ONTO = 0, OUTSIDE = 1 };

  // generalize to a shere??

  template< typename T >
  class circle
  {
    public:
      typedef veca<T, 2>         center_t;
      typedef vecn<T, 2>&        center_reference;
      typedef const vecn<T, 2>&  const_center_reference;

      typedef T                  radius_t;
      typedef T&                 radius_reference;
      typedef const T&           const_radius_reference;
    
    private:

      center_t                  m_center;
      radius_t                  m_radius;
        
    public:
                                circle( const_center_reference center, const_radius_reference radius )
                                : m_center( center ), m_radius( radius )        {   }
      
                                circle(const_center_reference center, const_center_reference circle_point)
                                : m_center( center ), m_radius( length(center - circle_point) )
                                                                                {   }

      center_reference          center()                                        { return m_center; }
      const_center_reference    center()                                const   { return m_center; }

      radius_reference          radius()                                        { return m_radius; }
      const_radius_reference    radius()                                const   { return m_radius; }

  };

  template< typename T >
  std::ostream& 	operator<<(std::ostream& os,const circle<T>& c) 
  { os << "circle"
       << " center " << c.center() 
       << " radius " << c.radius();
	return os;
  }
  
  template< typename T >
  std::pair< T, T > intersection( const ray<T,2u>& ray, const circle<T>& circle )
  { 
    // check for no intersection
    
    const T a = dot( ray.direction(), ray.direction() );
    const T b = T(2.) * sum( ray.direction() * ( ray.source() - circle.center() ) );
    const T c = dot( circle.center(), circle.center() ) 
                + dot( ray.source(), ray.source() ) 
                - T(2.) * dot( ray.source(), circle.center() )
                - sqr( circle.radius() );

    return solve_quadratic_equation(a,b,c);
  }

  template< typename T >
  std::pair< T, T > intersection_with_center_ray( const typename ray<T,2u>::value_type& direction, const circle<T>& twocircle )
  { 
    // check for no intersection

    const T center2 = dot( twocircle.center(), twocircle.center() );
    
    const T a = dot( direction, direction );
    const T c = - sqr( twocircle.radius() );

    return solve_quadratic_equation( a, T(0.), c );
  }
    
  /* enbedd in higher dimensional spaces
  template<class T, size_t Dim>
  class circle
  {
    public:
  }*/

  
  template< typename T, size_t Dim = 3u >
  class plane
  {
      static_assert(Dim > 0, "Dimension must be greater than zero.");
    
      typedef veca<T, Dim+1>        value_type;
      typedef vecn<T, Dim+1>&       value_reference;
      typedef const vecn<T, Dim+1>& const_value_reference;

      typedef veca<T, Dim>          direction_type;
      typedef vecn<T, Dim>&         direction_reference;
      typedef const vecn<T, Dim>&   const_direction_reference;

      typedef vecn<T, Dim>          normal_reference;
    
      typedef T                     distance_type;
      typedef T&                    distance_reference;
      typedef const T&              const_distance_reference;
    
      value_type            m_nd;
    public:
			                plane(const_direction_reference norm, const_distance_reference dist)
			                : m_nd(norm)				
                            { m_nd[Dim] = dist; }
      
			                plane()
			                : m_nd(0)
                            { m_nd[Dim-1] =	T(1); }
      
      // returns the outward pointing normal
      normal_reference      normal()		                            const	
                            { return m_nd; }
					
      distance_reference    distance()					
                            { return m_nd[Dim]; }
      
      const_distance_reference
                            distance()		                            const	
                            { return m_nd[Dim]; }

      // the halfspace containing the normal is defined to be OUTSIDE
      classification_t      classify(const_direction_reference point)	const	
                            { const distance_type d = dot( point , normal() );
	                          if( d == distance() ) 
	                            return ONTO;
	                          if( d < distance() ) 
	                            return INSIDE;
	                          return OUTSIDE; 
	                        }
			
      value_reference	    data()			                            const	
                            { return m_nd; }
  };

  // TODO: plane_from_spanning_vectors

  template< typename T >
  plane<T,2>                plane_from_ray( const ray<T,2>& );

  template< typename T, size_t Dim >
  typename ray<T,Dim>::parameter_type intersection( const ray<T,Dim>& ray, const plane<T,Dim>& plane )
  {
    const T denom = dot( ray.direction(), plane.normal() ); 
    //assert( denom != 0 ); 
    return ( plane.distance() - dot( ray.source() , plane.normal() ) ) / denom; 
  }

  template< typename T >
  typename ray<T,2>::point_type	intersection( const ray<T,2>& ray1, const ray<T,2>& ray2 )
  {
    const typename ray<T,2>::point_type A1 = ray1.source();
    const typename ray<T,2>::point_type B1 = ray1.target(); 
	const typename ray<T,2>::point_type A2 = ray2.source();
    const typename ray<T,2>::point_type B2 = ray2.target(); 

	const T denom = ( A1[0] - B1[0] ) * ( A2[1] - B2[1] ) - ( A1[1] - B1[1] ) * ( A2[0] - B2[0] );
	const T det1  = ( A1[0] * B1[1] - A1[1] * B1[0] );
  	const T det2  = ( A2[0] * B2[1] - A2[1] * B2[0] );

	return typename ray<T,2>::point_type( ( A2 - B2 ) * det1 - ( A1 - B1 ) * det2 ) /= denom; 
  }
	
  template< typename T, size_t Dim >
  std::ostream& 	operator<<( std::ostream& os,const plane<T,Dim>& p ) 
  { os << "plane"
       << " normal " << p.normal() 
       << " distance " << p.distance();
	return os;
  }
  
  template< typename T, size_t Dim = 3 >	
  class box
  {

      veca<T,Dim>		    m_min,
	      		            m_max;
    public:
      		            	box() : m_min(std::numeric_limits<T>::min()),
			                        m_max(std::numeric_limits<T>::max())	
                            { 	}

			                box(const vecn<T,Dim>& min,const vecn<T,Dim>& max)
			                : m_min( min ), m_max( max )			
                            {	}
                      
			                box(const utk::box<T,Dim>& o)
			                : m_min( o.min() ), m_max( o.max() )	
							{	}

      void		            extend( const vecn<T,Dim>& p )
                            { 
                              clamp_max( m_min, p );
                              clamp_min( m_max, p );
  							}

      void		            extend( const box<T,Dim>& b )
                            { 
                              extend( b.min() );
                              extend( b.max() );
  							}
      
      veca<T,Dim>		    vertex( size_t i )  const
                            { 
                              assert( i < (integer_power<2,Dim>::value) );

                              veca<T,Dim> result;
                              typename veca<T,Dim>::iterator rit = result.begin();
                              typename veca<T,Dim>::const_iterator minit = min().begin();
                              typename veca<T,Dim>::const_iterator maxit = max().begin();
                              for( size_t dsize = 1u; dsize < integer_power<2,Dim>::value; dsize *= 2u, ++minit, ++maxit )
                                *rit++ = i & dsize ? *minit : *maxit;
                              return result;
                              /*  
                              switch(i)
							  { case 0: return utk::veca<T,3>(min[0],min[1],min[2]);
							    case 1: return utk::veca<T,3>(max[0],min[1],min[2]);
							    case 2: return utk::veca<T,3>(min[0],max[1],min[2]);
							    case 3: return utk::veca<T,3>(max[0],max[1],min[2]);
							    case 4: return utk::veca<T,3>(min[0],min[1],max[2]);
							    case 5: return utk::veca<T,3>(max[0],min[1],max[2]);
							    case 6: return utk::veca<T,3>(min[0],max[1],max[2]);
							    case 7: return utk::veca<T,3>(max[0],max[1],max[2]);
							    default: assert(false);
							  }*/
							}
			
      utk::veca< bool, integer_power<2,Dim>::value >	    
                            classify(const utk::plane<T>& plane)	
                            { 
                              veca< bool, integer_power<2,Dim>::value >	clssfd;
							  for( utk::size_t i = 0; i < clssfd.size(); i++ )
								clssfd[i] = plane.classify( vertex(i) );
							  return clssfd;
							}

      vecn<T,Dim>&          min()
                            { return m_min; }

      
      const vecn<T,Dim>&    min()   const	
                            { return m_min; }

      vecn<T,Dim>&          max()
                            { return m_max; }

      const vecn<T,Dim>&    max()   const
                            { return m_max; }

      veca<T,Dim>	        extent()    const	
                            { return max() - min();	}
      
      veca<T,Dim>     		center()    const  	
                            { return ( min() + max() ) * T(.5); }

  };

  template< typename T, size_t Dim >
  std::ostream& 	operator<<(std::ostream& os,const box<T,Dim>& b) 
  { os << "box"
       << " min " << b.min() 
       << " max " << b.max();
	return os;
  }
  
  vecn3s		            box3_vertex(uint8_t); 		//coordinates corresponding to vertex index i 
  vec3i			            box3_nb6(uint8_t); 		    //the 6 voxel neighbourhood in relative coordinates 
  vec3i			            box3_nb26(uint8_t); 		//the 26 voxel  	  -''-
  veca<uint8_t,3>	        box3_vertex_nb(uint8_t);	//the 3 neighbourhood vertices

  template<class T>	
  std::valarray<T>	        ray_box_is(const ray<T>&,const box<T>&);	//ray-box intersection test (centered box with dimensions 2*dist )

  template<class T>
  std::vector< veca<T,3> >  plane_box_is(const plane<T>& plane,const box<T>& box);

  template<class T>
  T		                    project_to_sphere(const T& r,const vecn<T,2>& pos);

}

//______________
//Implementation
////////////////
inline utk::vec3i	utk::box3_nb26(uint8_t i)
{ veca<int,3> bp;
  assert(i<26);	
  if(i>=13) ++i;	  
  bp.z()=i/9-1;
  bp.y()=(i-(bp.z()+1)*9)/3-1;
  bp.x()=i-(bp.z()+1)*9-(bp.y()+1)*3-1;
  return bp;
}

template< typename T >
utk::plane<T,2>  utk::plane_from_ray(const ray<T,2>& r)
{
  //std::clog << "utk::plane_from_ray\t\t\t| " << r <<std::endl;
  
  const typename ray<T,2>::parameter_type      rlen = r.length();
  typename ray<T,2>::const_direction_reference rdir = r.direction();
  const typename ray<T,2>::direction_type      normal( -rdir[1] / rlen, rdir[0] / rlen );
    
  return plane<T,2>( normal , dot( normal, r.source() ) );
}


template<class T>	
std::valarray<T>	utk::ray_box_is(const ray<T>& r,const box<T>& b)	//ray-box intersection test (centered box with dimensions 2*dist )
{ 
  std::valarray<T>	t(2);
  size_t k=0;
  T		 t1, t2, tu, tv;
 
  for(size_t i=0;i<3;i++)
  {
	t1 = ( b.get_box_min[i] - r.source()[i] )/r.direction()[i],
    t2 = ( b.get_box_max[i] - r.source()[i] )/r.direction()[i];
    size_t 	u= (i+1) % 3,	
 			v= (i+2) % 3;
    if(t1>=0)	
    { tu = r.source()[u] + t1 * r.direction()[u];
      tv = r.source()[v] + t1 * r.direction()[v];	    
      if( tu >= b.get_box_min()[u] && tu <= b.get_box_max()[u] && tv >= b.get_box_min()[v] && tv >= b.get_box_min()[v] ) 
		t[k++]=t1; 
    }
    if(t2>=0)
    { tu = r.source()[u] + t2 * r.direction()[u];
      tv = r.source()[v] + t2 * r.direction()[v];
      if( tu >= b.get_box_min()[u] && tu <= b.get_box_max()[u] && tv >= b.get_box_min()[v] && tv >= b.get_box_min()[v] ) 
		t[k++]=t2; 
    }
    if(k>1) break;
  } 
  if(k==2) 
  { if(t[0]>t[1])
    { double tb = t[0];
      t[0] = t[1];
      t[1] = tb; 
    }
    return t;
  }
  return std::valarray<double>(0);
}

template<class T>
std::vector< utk::veca<T,3> > utk::plane_box_is(const plane<T>& plane,const box<T>& b)
{ 
  std::vector< veca<T,3> >	ispnts(6); //result
  std::vector< uint8_t >	inpnts(6); //inside plane points
  veca<uint8_t,3>	        nbpnts(6); //neighbours
  
  for(uint8_t i=0;i<8;i++)
    if( plane.classify( b.vertex(i) ) == INSIDE )
      inpnts.push_back(i);
  // ????
  for( std::vector<uint8_t>::iterator it = inpnts.begin(); it != inpnts.end(); it++ )  
  { nbpnts = box3_vertex_nb( *it );        
    for( uint8_t i=0; i<3; i++ )
      if( std::find( inpnts.begin(), inpnts.end(), nbpnts[i] ) == inpnts.end() )
      { ray<T> r( b.vertex(*it), b.vertex(nbpnts[i]) );
        r.direction() -= r.source();
	    if( dot( r, plane.normal() ) )
          ispnts.push_back( ray_plane_is( r, plane ) );
      }  
  }
  return ispnts;
}

// the code of the function below is based on the sgi trackball example under the following license: 
/* (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */
 
//! orthogonal project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
//! if we are away from the center of the sphere.
template<class T>
T     utk::project_to_sphere(const T& r,const vecn<T,2>& pos)
{
  const T d = pos.length();
  
  if (d < r * 0.70710678118654752440) 
  { //Inside sphere
    //std::cerr<<"utk::project_to_sphere\t|inside sphere"<<std::endl;
    return sqrt(r*r - d*d);
  }else 
  { //On hyperbola
    //std::cerr<<"utk::project_to_sphere\t|on hyperbolia"<<std::endl;
    const T t = r / 1.41421356237309504880;
    return t*t/d;
  }
}

#pragma GCC visibility pop

#endif

