//libutk - a utility library 
//Copyright (C) 2006-2009 Peter Urban (peter.urban@s2003.tu-chemnitz.de)
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

#pragma once

#include	"quaternion.h"
#include 	"cache.h"

#include	<iostream>

#pragma GCC visibility push(default)

namespace utk
{
  template<class T>
  class inertial;

  template<class T>
  std::ostream&			    operator<<(std::ostream&,const inertial<T>&);

  template<class T>  
  class location
  {   
      typedef T                     value_type;
      typedef utk::veca<T,3>        point_type;
      typedef utk::vecn<T,3>&       point_reference;
      typedef const utk::vecn<T,3>& const_point_reference;
    
      point_type    		m_pos;		//pivot coords
    public:
      				        location()
				            : m_pos( T(0.) )
                            {  	}

            				location(const location<T>& o)
			            	: m_pos( o.pos() )				
                            { 	}

            				location(const_point_reference pos)
			            	: m_pos( pos )				
                            { 	}

      virtual			    ~location()
                            { 	}
      
      point_reference		position()
                            { return m_pos; }
      
      const_point_reference	position()	const	
                            { return m_pos; }
  };


  //3d space orientation representation
  template<class T>  
  class orientation		
  {
      //unit quaternion representing orientation
      Quaternion<T>		        m_quat;		
      Cache<utk::mata<T,3> >	m_cache;
      Cache<utk::mata<T,3> >	m_inv_cache;
      
    public:
          				    orientation()
				            : m_quat(), m_cache(), m_inv_cache()	 	
					        { assert(std::numeric_limits<T>::is_integer!=true); }
				
				            orientation(const orientation& o)
				            : m_quat(o.quat()), m_cache(), m_inv_cache()	
					        { assert(std::numeric_limits<T>::is_integer!=true); }
					 
      virtual		        ~orientation()
                            { 	 }
      
      orientation<T>&	    operator=(const orientation<T>& o)		
                            { m_quat = o.quat(); 
						      return *this; 
						    }
  
      void			        rot(const T& x,const T& y,const T& z,const T& t)
        					{ if( t != T(0) && ( x != T(0) || y != T(0) || z != T(0) ) )
						      { m_quat *= Quaternion<T>(x,y,z,t);
						        m_cache.invalidate();
							    m_inv_cache.invalidate();
						      }  
      						}
      
      void			        rot(const vecn<T,3>& n,const T& t)	
                            { if( t != T(0) && ( n.x() != T(0) || n.y() != T(0) || n.z() != T(0) ) )
						      { //assert(n.length());
	          				    m_quat *= Quaternion<T>(n,t);
	          				    m_cache.invalidate();
	          				    m_inv_cache.invalidate();
						      }  
          					}
      
      veca<T,3>			    rot_vec(const vecn<T,3>& v)	const	
                            { veca<T,3> qres( m_quat.conjugated() * Quaternion<T>(v.x(),v.y(),v.z(),T(0)) * m_quat );
						      return qres;
						    }
      
      veca<T,3>			    inv_rot_vec(const vecn<T,3>& v)	const
                            { veca<T,3> qres( m_quat * Quaternion<T>(v.x(),v.y(),v.z(),T(0)) * m_quat.conjugated() );
						      return qres;
						    }  
      
      const matn<T,3>&	    get_mat3()	const	
                            { if( !m_cache.is_valid() )
						        m_cache = m_quat.rot_matrix(); 
						      return m_cache; 
        					}
      
      const matn<T,3>&	    get_inverse_mat3()	const
                            { if( !m_inv_cache.is_valid() )
						        m_inv_cache = m_quat.conjugated().rot_matrix(); 
						      return m_inv_cache; 
    					    }
      
      Quaternion<T>&	    quat()
                            { m_cache.invalidate();
						      m_inv_cache.invalidate(); 
							  return m_quat; 
							}
      
      const Quaternion<T>&	quat()	const	
                            { return m_quat; }
      
  };

  template<class T>
  class inertial    : public location<T>, public orientation<T>
  {
    public:
      typedef inertial<T>*  parent_type;

      typedef std::vector< parent_type >        children_type;
      typedef const std::vector< parent_type >& const_children_reference;
    private:
      parent_type	        m_parent;
      children_type     	m_children;
    public:
            				inertial()
			                : location<T>(), orientation<T>(), m_parent(0), m_children()	
							{	}

	                        inertial(const inertial<T>& o)
	                        : location<T>(o), orientation<T>(o), m_parent( o.parent() ), m_children( o.children() )
	                        { 	}

      void			        look_at(vecn<T,3> dir,vecn<T,3> up)	
      						{ std::cout<<"utk::inertial::look_at\t|dir\t"<<dir<<"\t|up\t"<<up<<std::endl;
							  dir.normalize();
							  up.normalize();
							  //assert(fabs(dot(dir,up))<std::numeric_limits<T>::epsilon()*10.);
							  
							  veca<T,3> right(cross(dir,up));
							  mata<T,3> m;
							  m[0]=right;
							  m[1]=up;
							  m[2]=-dir;//you look along the z-axis in negative direction
							  std::clog << "utk::inertial::look_at\t|"
                                        << " " << Quaternion<T>(m)
                                        << std::endl;
                                
							  std::clog << "utk::inertial::look_at\t|"
                                        << " x " << m[0] 
                                        << " y " << m[1]
                                        << " z " << m[2]
                                        << std::endl;
                                
							  orientation<T>::quat() = Quaternion<T>(m);
							}
      
      mata<T,4> 			get_mat4()	const	
                            { mata<T,4> m( orientation<T>::get_mat3() );
  							  m[3]	= location<T>::position(); 
  							  m(0,3)= T(0);
							  m(1,3)= T(0);
							  m(2,3)= T(0);
							  m(3,3)= T(1);
							  return m;
      						}
      
      mata<T,4>	    		get_inverse_mat4()	const	
                            { mata<T,4> m(orientation<T>::get_inverse_mat3()); 
							  m[3]	= - location<T>::position(); 
							  m(0,3)= T(0);
							  m(1,3)= T(0);
							  m(2,3)= T(0);
							  m(3,3)= T(1);
							  return m;
  							}
      
      void			        assign_parent( parent_type p)		
                            { assert( p != 0 );
							  m_parent = p; 
    						}
      
      parent_type		    parent()	const
                            { return m_parent; }
      
      const_children_reference		
	              			children()	const
                            { return m_children; }
      
      void			        append( parent_type c )
                            { assert( c!=0 );
	     					  m_children.push_back( c ); 
      						}
      
      void			        remove( parent_type i )
                            { assert( i != 0 );
	      					  m_children.remove( i ); 
      						}
      
      inertial<T>		    inverse()	const
                            { inertial<T> inv( *this );
	      					  inv.position() *= T(-1); 
							  inv.quat()      = inv.quat().inverse();
							  return inv;	  
      						}
      
      bool			        is_child(parent_type i)    const
                            { return std::find( m_children.begin(), m_children.end(), i ) != m_children.end(); };
      
      parent_type		    root_inertial()				
                            { return m_parent ? m_parent->root_inertial() : this; }

      friend std::ostream&	operator<< <>(std::ostream&,const inertial<T>&);
  };

  template<class T>
  std::ostream&	operator<<(std::ostream& os,const inertial<T>& i)
  { 
    os << "inertial"
       << " pos " << i.position()
       << " quat " << i.quat()
       << std::endl;
    return os;
  }
  
}				

#pragma GCC visibility pop
