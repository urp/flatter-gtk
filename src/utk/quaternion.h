//libutk - a utility library 
//Copyright (C) 2006-20010  Peter Urban (peter.urban@s2003.tu-chemnitz.de)
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
#include	"vecn.h"
#include	"matn.h"
#include	"geometry.h"
#include	<cmath>
#include	<iostream>

#pragma GCC visibility push(default)

namespace utk
{
  template<class T>
  veca<T,4>			get_axis_angle_from_trackball(const utk::vecn<T,2>& pos1,const utk::vecn<T,2>& pos2,const T& tbsize=T(.8));
  
  template<class T>
  class Quaternion;
  
  template<class T>
  std::ostream&			operator<<(std::ostream&,const Quaternion<T>&);
				  
  template<class T>
  class Quaternion		: public veca<T,4>
  {
    public:
      				Quaternion()
				:veca<T,4>(T(0))			{ vecn<T,4>::w()=1.; }

				Quaternion(T gx,T gy,T gz,T gw)
	  			:veca<T,4>(gx,gy,gz,gw)			{	}
				
				//!make quaternion from rotation about t around n
				Quaternion(const vecn<T,3>& n,const T& t)	
									{ set_axis_angle(n,t); }
				
      explicit			Quaternion(const matn<T,3>& a);

      explicit			Quaternion(const vecn<T,4> quat)
				:veca<T,4>(quat)			{	}
				
				Quaternion(const Quaternion<T>& q)
				:veca<T,4>(q)				{	}
								
				
      Quaternion<T>&		operator= (const Quaternion<T>& q) 	{ veca<T,4>::operator=(q); return *this; }
      
      Quaternion<T>		operator+ (const Quaternion<T>& q)const{ return Quaternion<T>( vecn<T,4>::operator+(q) ); }
      Quaternion<T>		operator- (const Quaternion<T>& q)const{ return Quaternion<T>( vecn<T,4>::operator-(q) ); }
      //! Given two quaternions, multiply them to get a third quaternion.
      //! Multiplying quaternions to get a compound rotation is analagous to adding
      //! the resulting translations to get a compound translation.        
      Quaternion<T>		operator* (const Quaternion<T>& q)const{ return Quaternion<T>( vecn<T,4>::w()*q[0] + vecn<T,4>::x()*q[3] + vecn<T,4>::y()*q[2] - vecn<T,4>::z()*q[1],
											       vecn<T,4>::w()*q[1] + vecn<T,4>::y()*q[3] + vecn<T,4>::z()*q[0] - vecn<T,4>::x()*q[2],
											       vecn<T,4>::w()*q[2] + vecn<T,4>::z()*q[3] + vecn<T,4>::x()*q[1] - vecn<T,4>::y()*q[0],
											       vecn<T,4>::w()*q[3] - vecn<T,4>::x()*q[0] - vecn<T,4>::y()*q[1] - vecn<T,4>::z()*q[2]
											     );
      									}

      Quaternion<T>&		operator+=(const Quaternion<T>& q)	{ return vecn<T,4>::operator+=(q); }
      Quaternion<T>&		operator-=(const Quaternion<T>& q)	{ return vecn<T,4>::operator-=(q); }
      Quaternion<T>&		operator*=(const Quaternion<T>& q)	{ return operator=(operator*(q)); } //optimize
      
      Quaternion<T>		conjugated()			const 	{ return Quaternion(-vecn<T,4>::x(),-vecn<T,4>::y(),-vecn<T,4>::z(),vecn<T,4>::w()); }		  
     
      void			set_axis_angle(const utk::vecn<T,3>& a,const T& t);
      
      veca<T,4>			get_axis_angle()		const	{ T ts=T(1.f)/sqrt(T(1.f)-utk::sqr(vecn<T,4>::w()));
	      								  return veca<T,4>(vecn<T,4>::x()*ts,vecn<T,4>::y()*ts,vecn<T,4>::z()*ts,T(2.f)*acos(vecn<T,4>::w())); 
      									}
  
      mata<T,3>			rot_matrix()			const	{ mata<T,3>	m;
	      								  T	 dx=T(2.f)*vecn<T,4>::x(), dy=T(2.f)*vecn<T,4>::y(), dz=T(2.f)*vecn<T,4>::z(),
	      								  	 wx=dx*vecn<T,4>::w(), xx=dx*vecn<T,4>::x(), yy=dy*vecn<T,4>::y(),          
										 wy=dy*vecn<T,4>::w(), xy=dy*vecn<T,4>::x(), yz=dz*vecn<T,4>::y(),
										 wz=dz*vecn<T,4>::w(), xz=dz*vecn<T,4>::x(), zz=dz*vecn<T,4>::z();

									  m(0,0)=T(1.f)-(yy+zz);  m(1,0)=xy+wz;		  m(2,0)=xz-wy;
									  m(0,1)=xy-wz;		  m(1,1)=T(1.f)-(xx+zz);  m(2,1)=yz+wx;
									  m(0,2)=xz+wy;		  m(1,2)=yz-wx;		  m(2,2)=T(1.f)-(xx+yy);
									  return m;
									}
      void      		normalize()				{ const T len = vecn<T,4>::length();	
									  if( (len-T(1.f))>std::numeric_limits<T>::epsilon() ) vecn<T,4>::operator/=(len);
									}
      friend std::ostream&	operator<< <>(std::ostream&,const Quaternion<T>&);
  };
  
  
  //______________
  //Implementation
  ////////////////
  
  template<class T>
  Quaternion<T>::Quaternion(const matn<T,3>& a)		//make quaternion from rotation matrix
                :veca<T,4>()	  
  { T trace = a[0][0] + a[1][1] + a[2][2] + T(1.f);
	  
    if( trace > 1e-6 ) 
    { T s = T(.5f) / sqrt(trace);
      vecn<T,4>::w() = T(.25f) / s;
      vecn<T,4>::x() = ( a[2][1] - a[1][2] ) * s;
      vecn<T,4>::y() = ( a[0][2] - a[2][0] ) * s;
      vecn<T,4>::z() = ( a[1][0] - a[0][1] ) * s;
    }else 
    { if( a[0][0] > a[1][1] && a[0][0] > a[2][2] ) 
      { T s = 2. * sqrtf( T(1.f) + a[0][0] - a[1][1] - a[2][2]);
	vecn<T,4>::x()	= T(.25f) * s;
	vecn<T,4>::y()	= (a[0][1] + a[1][0] ) / s;
	vecn<T,4>::z()	= (a[0][2] + a[2][0] ) / s;
	vecn<T,4>::w()	= (a[1][2] - a[2][1] ) / s;
      }else 
      { if(a[1][1] > a[2][2]) 
	{ T s = T(2.f) * sqrtf( T(1.f) + a[1][1] - a[0][0] - a[2][2]);
	  vecn<T,4>::x() = (a[0][1] + a[1][0] ) / s;
	  vecn<T,4>::y() = T(.25f) * s;
	  vecn<T,4>::z() = (a[1][2] + a[2][1] ) / s;
	  vecn<T,4>::w() = (a[0][2] - a[2][0] ) / s;
	}else 
	{ T s = T(2.f) * sqrtf( T(1.f) + a[2][2] - a[0][0] - a[1][1] );
	  vecn<T,4>::x() = (a[0][2] + a[2][0] ) / s;
	  vecn<T,4>::y() = (a[1][2] + a[2][1] ) / s;
	  vecn<T,4>::z() = T(.25f) * s;
	  vecn<T,4>::w() = (a[0][1] - a[1][0] ) / s;
	}
      }
    }
    /* vecn<T,4>::w()=T(.5)*sqrt(m.get_diag().sum()+T(1));
    vecn<T,4>::x()=(m(2,1)-m(1,2))/(vecn<T,4>::w()*T(4));
    vecn<T,4>::y()=(m(0,2)-m(2,0))/(vecn<T,4>::w()*T(4));
    vecn<T,4>::z()=(m(1,0)-m(0,1))/(vecn<T,4>::w()*T(4));*/
    utk::vecn<T,4>::normalize(); 
  }
  
  template<class T>
  void	  Quaternion<T>::set_axis_angle(const utk::vecn<T,3>& a,const T& t)
  { vecn<T,3> imag=vecn<T,4>::xyz();
    imag=a;
    imag.normalize();
    imag*=sin(T(.5f)*t); 
    vecn<T,4>::w()=cos(T(.5f)*t);
  }

  template<class T>
  veca<T,4>			get_axis_angle_from_trackball(const utk::vecn<T,2>& pos1,const utk::vecn<T,2>& pos2,const T& tbsize)
  {
    utk::veca<T,4>  res;
      
    //std::cerr<<"utk::get_axis_angle_trackball\t|pos1 "<<pos1<<"\t|pos2 "<<pos2<<std::endl; 
    if( ((pos1 == pos2)==true) )
    {
      //Zero rotation
      //std::cerr<<"utk::get_axis_angle_trackball\t|zero rotation"<<std::endl;
      res.xyz()=T(0.f);
      res[3]  =T(1.f);
      return res;
    }
      
    //First, figure out z-coordinates for projection of P1 and P2 to deformed sphere
    utk::veca<T,3>  p1(pos1[0],pos1[1],project_to_sphere(tbsize,pos1));	  //intersection of the first mouse point with the trackball
    utk::veca<T,3>  p2(pos2[0],pos2[1],project_to_sphere(tbsize,pos2));	  //the second one
      
    //rotation - axis angle
    res.xyz() = utk::cross(p2,p1);
      
    //Figure out how much to rotate around that axis.
    p1	-= p2;
    T t  = p1.length()/(T(2.f)*tbsize);
      
    //Avoid problems with out-of-control values...
    utk::clamp(t,T(-1.f),T(1.f));
      
    res[3] = T(2.f) * asin(t);
      
    res.xyz().normalize();
    //std::cerr<<"utk::get_axis_angle_from_trackball\t|p1 "<<p1<<"\t|p2 "<<p2<<"\t|t "<<t<<"\t|axis "<<res.xyz()<<"\t|angle "<<res[3]<<std::endl;
      
    return res;
  }  
  
  template<class T>
  std::ostream&			operator<<(std::ostream& os,const Quaternion<T>& q)
  { os<<"utk::quat:\t| "<<vecn<T,4>(q)<<"\t|aa "<<q.get_axis_angle();
    return os;    
  }
  
}

#pragma GCC visibility pop
