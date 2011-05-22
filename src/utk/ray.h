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

#ifndef	UTK_RAY_H
#define UTK_RAY_H

#include 	"math.h"
#include 	"vecn.h"
#include	<iostream>

#pragma GCC visibility push(default)

namespace utk
{
  //forward
  template<class T>
  class inertial;	

  //forward
  template< class T, size_t Dim >
  class	ray; 

  template< class T, size_t Dim >
  std::ostream&		    operator<<(std::ostream& os,const ray<T,Dim>& r);
  
  template< class T, size_t Dim=3 >
  class	ray
  {
    public:

      typedef T                    parameter_type;
      
      typedef veca<T, Dim>         point_type;
      typedef vecn<T, Dim>&        point_reference;
      typedef const vecn<T, Dim>&  const_point_reference;
      
      typedef veca<T, Dim>         direction_type;
      typedef vecn<T, Dim>&        direction_reference;
      typedef const vecn<T, Dim>&  const_direction_reference;

      friend 
      std::ostream& 	operator<< <>(std::ostream&,const ray<T>&);
      
    private:
      
      point_type	    m_src;  //source
      direction_type	m_dir;  //direction
    
    public:
          	            ray( const_point_reference gsource, const_point_reference gtarget )
	                    : m_src( gsource ), m_dir( gtarget - gsource ) {	}
			
			            ray(const ray<T,Dim>& r)
			            : m_src( r.source() ), m_dir( r.direction() )  {	}
			
      point_reference   source()                                   				
                        { return m_src; }
      
      const_point_reference	
                        source()                                        const	
                        { return m_src; }
      
      direction_reference
                        direction()					                            
                        { return m_dir; }
      
      const_direction_reference	
                        direction()			                            const   
                        { return m_dir; }

      point_type        target()                                        const   
                        { return source() + direction(); }
      
      point_type	    at(parameter_type t)	                        const	
                        { return source() + direction() * t; }
      
      point_type	    at_arc_length( const parameter_type& t )               const	
                        { return source() + direction() * ( t / length() ); }
      
      const_point_reference	
                        move_source_to( const parameter_type& t )                      
                        { return source() += direction() * t; }
      
      const_point_reference
                        move_source_to_arc_length( const parameter_type& t)	const   
                        { return source() += direction() * ( t / length() ); }
      
      parameter_type    length()				                        const   
                        { return utk::length( direction() ); }
        
      ray<T,Dim>	    trafo_to(const inertial<T>& i)                  const 	
                        { return ray<T>( i.inv_rot_vec( source() - i.pos() ), i.inv_rot_vec( target() - i.pos() ) ); }
      
      ray<T,Dim>	    trafo_from(const inertial<T>& i)                const 	
                        { return ray<T>( i.rot_vec( source() ) + i.pos(), i.rot_vec( target() ) + i.pos() ); }
     
  };
  
  template< class T, size_t Dim >
  std::ostream& 	operator<<(std::ostream& os,const ray<T,Dim>& r) 
  { os << "ray"
       << " src " << r.source() 
       << " dir " << r.direction() 
       << " trg " << r.target() 
       << " len " << r.length();
	return os;
  }
}

#pragma GCC visibility pop

#endif
