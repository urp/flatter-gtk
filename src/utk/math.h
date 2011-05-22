//libutk - a utility library 
//Copyright (C) 2006-2009  Peter Urban (peter.urban@s2003.tu-chemnitz.de)
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

# pragma once

# include <valarray>
# include <iostream>
# include <iomanip>
# include <numeric>
# include <cmath>
# include <cstdint>
// utk::close_ulps uses boost::math::float_distance
# include <boost/math/special_functions/next.hpp>

# pragma GCC visibility push(default)

namespace utk
{  
  # ifdef _WIN32
  typedef	char			int8_t;
  typedef	short			int16_t;
  typedef	int				int32_t;
  //typedef	int64_t			int64_t;

  typedef	unsigned char	uint8_t;  
  typedef	unsigned short	uint16_t;
  typedef	unsigned int	uint32_t;
  //typedef	uint64_t		uint64_t;
  # else
  typedef	int8_t			int8_t;
  typedef	int16_t			int16_t;
  typedef	int32_t			int32_t;
  typedef	int64_t			int64_t;

  typedef	uint8_t			uint8_t;  
  typedef	uint16_t		uint16_t;
  typedef	uint32_t		uint32_t;
  typedef	uint64_t		uint64_t;
  #endif
  typedef	uint32_t		size_t;

  //containers with variable size
  typedef  	std::valarray<bool>	    vecb;
  typedef  	std::valarray<size_t>	vecs;
  typedef  	std::valarray<int>	    veci;
  typedef  	std::valarray<float>	vecf;
  typedef  	std::valarray<double>	vecd;

  utk::size_t next_power_of_two(const utk::size_t&);

  
  /*depricated*/
  bool almost_equal_2s_complement(const float a, const float b, const int maxulps);

  //TODO: include comparison type in template arguiment (boost???)
  template<class T>
  bool close_ulps(const T& a, const T& b, const int& maxulps = 10)
  { assert( maxulps > 0 );
    return std::abs( boost::math::float_distance(a,b) ) <= maxulps; 
  }

  template<class T>
  bool close_rel(const T& a, const T& b, const T& reltol)
  { assert( reltol > T(0.) );
    return std::abs( a - b ) <= reltol * std::max( std::abs(a), std::abs(b) );
  }

  template<class T>
  bool close_to_zero(const T& a, const T& tol = 1e-5)
  { assert( tol > T(0.) );
    return std::abs(a) <= tol;
  }

  template<typename T>
  struct close
  { 
	bool operator()(const T& a, const T& b)	const
  	{ return close_ulps(a,b); }
  };
  
  template<typename T>
  struct greater_close
  { 
	bool operator()(const T& a, const T& b)	const
  	{ return a>b || close_ulps(a,b); }
  };
  
  template<typename T>     
  struct less_close
  { 
	bool operator()(const T& a, const T& b)	const
    { return a<b || close_ulps(a,b); }
  };

  template<typename T>     
  struct less_not_close
  { 
	bool operator()(const T& a, const T& b)	const
    { return a<b || !close_ulps(a,b); }
  };
  
  //# define DBG_FLAT_SOLVE_QUADRATIC_EQUATION
  template<typename T>
  std::pair<T,T> solve_quadratic_equation(const T& a, const T& b, const T& c)
  { 
    const T discriminant = b*b - T(4.) * a * c;
    # if defined DBG_FLAT_SOLVE_QUADRATIC_EQUATION
    std::clog << "utk::solve_quadratic_equation\t\t|"
              << " a " << a << " b " << b << " c " << c
	  		  << " discriminant " << discriminant
              << std::endl;
    # endif
    //two solution	
    if( discriminant > T(0.) )
    { const T q = - T(.5) * ( b + std::copysign( std::sqrt(discriminant), b ) );
      return std::make_pair( q/a, c/q );
    }

    //single solution (double root)
    if( close_to_zero(discriminant) )
    { 
      const T q = - T(.5) * b;
                    //           v  ?????  v
	  const T r = fabs(a) > fabs(q) ? q/a : c/q;
      # if defined DBG_FLAT_SOLVE_QUADRATIC_EQUATION
      if( discriminant < T(0.) )
        std::clog << "\t\t\t\t\t|" << " WARNING - negative discriminant " << discriminant  << std::endl;
      std::clog << "\t\t\t\t\t|" << "double root " << r
                << " from (" << (q/a) << "," << (c/q) << ")"
                << std::endl;
      # endif
	  return std::make_pair(r,r);
    }

    //no solution
    # if defined DBG_FLAT_SOLVE_QUADRATIC_EQUATION
    std::clog << "\t\t\t\t\t|" << "no solution - returning(NaN,NaN)" << std::endl;
    # endif
    return std::make_pair(  std::numeric_limits<T>::quiet_NaN()
  	  				  	  , std::numeric_limits<T>::quiet_NaN() );
  }
  
  //valarray operators
  template<typename T>
  std::ostream& 	operator<<(std::ostream& os, const std::valarray<T>& v) 
  { for(size_t i=0;i<v.size();i++) 
	  os<<std::setw(10)<<std::setprecision(5)<<v[i]; 
	return os; 
  } 
  
  template<class T>
  std::valarray<T>	cross(const std::valarray<T>& a,const std::valarray<T>& b)			
  { assert(a.size()==3 && b.size()==3);
    std::valarray<T> r(3);
    r[0]=a[1]*b[2]-b[1]*a[2];
    r[1]=a[2]*b[0]-b[2]*a[0];
    r[2]=a[0]*b[1]-b[0]*a[1];
    return r;
  }
  
  template<class T>
  std::valarray<T>	cross3(const std::valarray<T>& a,const std::valarray<T>& b,const std::valarray<T>& c)			
  										{ assert(a.size()==3 && b.size()==3 && c.size()==3);
										  return b*std::inner_product(&a[0],&a[3],&c[0],T())-c*std::inner_product(&a[0],&a[3],&b[0],T());
  										}  


  template<size_t N>
  struct factorial
  {
    enum { value = N * factorial<N-1>::value };  
  };

  template<>
  struct factorial<0>
  {
    enum { value = 1 };  
  };
  
  template<int Base,size_t Exponent>
  struct integer_power
  {
    enum { value = Base * integer_power<Base,Exponent-1>::value };  
  };

  template<int Base>
  struct integer_power<Base,0>
  {
    enum { value = 1 };  
  };

  //power with positive integer exponent
  template<class T,size_t Exp>
  const T&		ipow(const T& base)
  { T res=T(1);
    for(size_t i=0;i<Exp;i++) res*=base;
    return res;  
  };
  
  //power with positive integer exponent
  template<class T>
  T			ipow(T base,size_t exp)
  { T res=T(1);
    for(size_t i=0;i<exp;i++) res*=base;
    return res;  
  }

  //sign function
  template<class T>  
  T			sgn(const T& a)
  { return a>T(0) ? T(1) : (a<T(0) ? T(-1) : T(0)); }

  
  //simple square
  template<class T>  
  T			sqr(const T& a)
  { return a*a; }
  
  //add constant unsigned int to input
  template<class TO,class TI,size_t Inc>
  TO			increment(TI input)
  { return TO(input+=TI(Inc)); }
  
  //substract constant unsigned int from input
  template<class TO,class TI,size_t Dec>
  TO			decrement(TI input)
  { return TO(input-=TI(Dec)); }
  
  //clamp scalar to [min,max]
  template< typename T >
  T&			clamp( T& value, const T& min = T(0), const T& max = T(1) )
  { if( value < min ) return value = min; 
    if( value > max ) return value = max;
    return value;
  }
  
  //clamp scalar to [min,max]
  template< typename  T>
  T&			clamp( T& value, const std::pair< T, T >& interval = std::pair< T, T >{ T(0), T(1) } )
  { 
    return clamp( value, interval );
  }
  
  //clamp scalar to lower boundary

  inline float&			clamp_min(float& val,const float& min=0.f)
  { if(val<min) return val=min; 
    return val;
  }
  inline double&		clamp_min(double& val,const double& min=0.)
  { if(val<min) return val=min; 
    return val;
  }
  inline int&			clamp_min(int& val,const int& min=0)
  { if(val<min) return val=min; 
    return val;
  }
  inline unsigned int&	clamp_min(unsigned int& val,const unsigned int& min=0u)
  { if(val<min) return val=min; 
    return val;
  }
  
  //clamp scalar to upper boundary

  inline float&			clamp_max(float& val,const float& max=1.f)
  { if(val>max) return val=max; 
    return val;
  }
  inline double&		clamp_max(double& val,const double& max=1.)
  { if(val>max) return val=max; 
    return val;
  }
  inline int&			clamp_max(int& val,const int& max=1)
  { if(val>max) return val=max; 
    return val;
  }
  inline unsigned int&	clamp_max(unsigned int& val,const unsigned int& max=1u)
  { if(val>max) return val=max; 
    return val;
  }

//euklidian norm
//  template<class T,int Cnt,template<class,int> class C >  
//  T	 		l2norm(const C<T,Cnt>& v)
//  { return std::sqrt( v.apply(sqr).sum() ); } 
//  
//  //euklidian norm
//  template<class T,template<class> class C >  
//  T	 		length(const C<T>& v)
//  { return l2norm(v); } 
// 
//  //normalized vector
//  template<class T,template<class> class C>  
//  C<T>			normal(const C<T>& v)
//  { assert(length(v)>0.);
//    return v/length(v);
//  }
//
//  //normalize vector
//  template<class T,template<class> class C>  
//  C<T>&			normalize(C<T>& v)
//  { assert(length(v)>0.);
//    return v/=length(v);
//  }

  //explicit per value container conversion
  template<class A,class B,template<class> class C>  
  void			convert_container(C<B>& b,const C<A>& a) 	
  { typename C<A>::iterator	ita	=a.begin();
    typename C<A>::iterator	itae	=a.end();
    typename C<B>::iterator	itb	=b.begin();
    typename C<B>::iterator	itbe	=b.end();
    while(ita!=itae)
    { assert(itb!=itbe);
      *itb++ = B(*ita++);
    }
  }
} 

#pragma GCC visibility pop


