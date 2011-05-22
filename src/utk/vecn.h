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

#ifndef	UTK_VECN_H
#define UTK_VECN_H

#include	"math.h"
#include	"io.h"
#include	<map>
#include	<valarray>
#include	<cassert>

#pragma GCC visibility push(default)

namespace utk
{
  template<class T,size_t Cnt>	class vecn;	//forward
  template<class T,size_t Cnt>	class veca;	
  template<class T,size_t Cnt>	class matn;
 
  enum						def_color			{ black=0,white,red,green,blue,grey };

  const vecn<float,4>&		get_default_color(def_color);

  veca<float,4>				get_random_color();
  
  
  template<class T,size_t Cnt>
  veca<T,Cnt>				abs(const vecn<T,Cnt>&);

  template<class T,size_t Cnt>
  veca<T,Cnt>				sgn(const vecn<T,Cnt>&);

  template<class T,size_t Cnt>
  vecn<T,Cnt>&				clamp_min(vecn<T,Cnt>&,const T&);

  template<class T,size_t Cnt>
  vecn<T,Cnt>&				clamp_max(vecn<T,Cnt>&,const T&);

  template<class T,size_t Cnt>
  vecn<T,Cnt>&				clamp_min(vecn<T,Cnt>&,const vecn<T,Cnt>&);

  template<class T,size_t Cnt>
  vecn<T,Cnt>&				clamp_max(vecn<T,Cnt>&,const vecn<T,Cnt>&);

  
  template<class T,size_t Cnt>
  veca<T,Cnt>				floor(const vecn<T,Cnt>&);
    
  template<class T,size_t Cnt>
  veca<T,Cnt>				ceil(const vecn<T,Cnt>&);
  
  template<class T,size_t Cnt>
  T							dot(const vecn<T,Cnt>&,const vecn<T,Cnt>&);

  //euklidian norm
  template<class T,size_t Cnt>  
  T	 						l2norm(const utk::vecn<T,Cnt>& v);

  //euklidian norm
  template<class T,size_t Cnt>  
  T	 						length(const utk::vecn<T,Cnt>& v);
 
  //normalized vector
  template<class T,size_t Cnt> 
  utk::veca<T,Cnt>			normal(const utk::vecn<T,Cnt>& v);


  //normalize vector
  template<class T,size_t Cnt> 
  utk::vecn<T,Cnt>&			normalize(utk::vecn<T,Cnt>& v);
  
  template<class T>
  veca<T,3>					cross(const vecn<T,3>&,const vecn<T,3>&);
	
  template<class T>
  veca<T,3>					cross3(const vecn<T,3>&,const vecn<T,3>&,const vecn<T,3>&);

  template<class T,size_t Cnt>
  std::ostream&				operator<<(std::ostream&,const vecn<T,Cnt>&); 

  template<class T,size_t Cnt>
  std::istream&				operator>>(std::istream&,const vecn<T,Cnt>&);

  template<class T,size_t Cnt>
  T                 	sum(const vecn<T,Cnt>& v);

  template<class T,size_t Cnt>
  veca<T,Cnt>				eval(T (*)(T)			,const vecn<T,Cnt>&);

  template<class T,size_t Cnt>
  veca<T,Cnt>				eval(T (*)(const T&)		,const vecn<T,Cnt>&);
    
  template<class T,class Arg2T,size_t Cnt>
  veca<T,Cnt>				eval(T (*)(T	    ,Arg2T)	,const vecn<T,Cnt>&,Arg2T);

  template<class T,class Arg2T,size_t Cnt>
  veca<T,Cnt>				eval(T (*)(const T&,Arg2T)	,const vecn<T,Cnt>&,Arg2T);
    
  //stl compliant fixed size numeric vector interface with unmanaged pointer to its components 
  template<class T,size_t Cnt>
  class	vecn			 			
  { friend class			matn<T,Cnt>;
    
      T*					val;

							vecn()
							:val(0)						{	}
    public:

      typedef T             value_type;
      typedef T*			iterator;
      typedef T*			const_iterator;

      static vecn<T,Cnt>	invalid;	

      
      explicit				vecn(T* pvec)
							:val(pvec)								{ 	}

      template<size_t Cnt2>	vecn(const vecn<T,Cnt2>& o)
							:val(o.ptr())							{ static_assert( Cnt <= Cnt2, "element range larger than target array." ); }

							vecn(const vecn<T,Cnt>& o)
							:val(o.ptr())							{	}
					
      virtual				~vecn()									{  	}

      static size_t			size()									{ return Cnt;	}
    
      void					ref(T* ptr)								{ val=ptr; }
      void					unref()									{ val=0; }

      vecn<T,Cnt>&			shift(const int elements)				{ val+=elements; return *this; }
	  
      template<utk::size_t ResCnt>
      vecn<T,ResCnt>		shifted(const int elements)		const	{ return vecn<T,ResCnt>(val+elements); }
      
      vecn<T,Cnt>&			operator++()							{ val+=Cnt; return *this; }
      vecn<T,Cnt>			operator++(int)							{ val+=Cnt; return utk::vecn<T,Cnt>(val-Cnt); }
      
      vecn<T,Cnt>&			operator= (const T& v)					{ fill(v);
																	  return *this;
																	}
      
      template<size_t Cnt2>
      vecn<T,Cnt>&			operator= (const vecn<T,Cnt2>& a)		{ typename vecn<T,Cnt>::iterator    itv=begin();
																	  typename vecn<T,Cnt2>::iterator   ita=a.begin();
																	  while(itv!=end()&&ita!=a.end()) 
																	  { assert(ita!=a.end());	  
																		*(itv++)= *(ita++);
																	  }
																	  return *this;
																	}
      vecn<T,Cnt>&			operator= (const vecn<T,Cnt>& a)		{ typename vecn<T,Cnt>::iterator    itv=begin();
																	  typename vecn<T,Cnt>::iterator    ita=a.begin();
																	  while(itv!=end())
																	  { assert(ita!=a.end());	  
																		*(itv++)= *(ita++);
																	  }
																	  return *this;
																	}
  
						  //operator bool()					const	{ return val; }
				
      veca<bool,Cnt>		operator==(const vecn<T,Cnt>& m)const	{ veca<bool,Cnt> r;
																	  typename vecn<bool,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator    itv=begin();
																	  typename vecn<T,Cnt>::iterator    itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++) = (*(itv++)) == (*(itm++)); 
																		  return r;
																	}
				
	  // returns true if and only if all elements equal v 
      bool					operator==(const T& v)			const	{ 
																	  # define UTK_VECN_DISABLE_EQUAL_OPERATOR_WARNING
                                                                      # if !defined UTK_VECN_DISABLE_EQUAL_OPERATOR_WARNING
																	  std::cerr << "Implementation of utk::vecn<T,Cnt>::operator==(const T&) changed its semantics. "
																	     		   "Old behavior returned true if at least one element is equal to the argument." << std::endl;
                                                                      # endif
        
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  while( itv != end() && (*itv) == v ) 
																		++itv;
																	  return itv==end()?true:false;
																	}
      veca<bool,Cnt>		operator< (const vecn<T,Cnt>& m)const	{ veca<bool,Cnt> r;
																	  typename vecn<bool,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) < (*(itm++)); 
																	  return r;
																	}

      bool					operator< (const T& a)			const	{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																		if( *(itv++) >= a)
																		  return false; 
																		  return true;
																		}

      veca<bool,Cnt>		operator<=(const vecn<T,Cnt>& m)const	{ veca<bool,Cnt> r;
																	  typename vecn<bool,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) <= (*(itm++)); 
																	  return r;
																	}

      bool					operator<=(const T& a)			const	{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																		if( *(itv++) > a)
																		  return false; 
																	  return true;
																	}


      veca<bool,Cnt>		operator> (const vecn<T,Cnt>& m)const	{ veca<bool,Cnt> r;
																	  typename veca<bool,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) > (*(itm++)); 
																	  return r;
																	}
      bool					operator> (const T& a)   		 const	{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																		if( *(itv++) <= a)
																		  return false; 
																		  return true;
																	}

      veca<bool,Cnt>		operator>=(const vecn<T,Cnt>& m)const	{ veca<bool,Cnt> r;
																	  typename veca<bool,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) >= (*(itm++)); 
																	  return r;
																	}
      bool					operator>= (const T& a)			const	{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																		if( *(itv++) < a)
																		  return false; 
																		  return true;
																	}

      veca<T,Cnt>			operator- ()					const	{ veca<T,Cnt> n(*this);
																	  return n *= -T(1);
																	}
      
      veca<T,Cnt>			operator+ (const vecn<T,Cnt>& m)const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) + (*(itm++)); 
																		  return r;
																	}    
	  veca<T,Cnt>			operator+ (const T& m)			const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) + m; 
																		  return r;
																	} 
      veca<T,Cnt>			operator- (const vecn<T,Cnt>& m)const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																		*(itr++)=(*(itv++)) - (*(itm++)); 
																	  return r;
																	}      
      veca<T,Cnt>			operator- (const T& m)			const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) - m; 
																	  return r;
																	}
      veca<T,Cnt>			operator* (const vecn<T,Cnt>& m)const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) * (*(itm++)); 
																		  return r;
																	}
      veca<T,Cnt>			operator* (const T& m)			const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) * m; 
																	  return r;
																	}      
      veca<T,Cnt>			operator/ (const vecn<T,Cnt>& m)const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itr!=r.end()) 
																		*(itr++)=(*(itv++)) / (*(itm++)); 
																	  return r;
																	}
      veca<T,Cnt>			operator/ (const T& m)			const	{ veca<T,Cnt> r;
																	  typename vecn<T,Cnt>::iterator itr=r.begin();
																	  typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itr!=r.end()) 
																	   *(itr++)=(*(itv++)) / m; 
																	  return r;
																	}
      vecn<T,Cnt>&			operator+= (const vecn<T,Cnt>& m)		{ typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itv!=end()) 
																	   *(itv++)+= *(itm++); 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator+= (const T& m)					{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																	   *(itv++)+= m; 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator-= (const vecn<T,Cnt>& m)		{ typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itv!=end()) 
																	   *(itv++)-= *(itm++); 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator-= (const T& m)					{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																	   *(itv++)-= m; 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator*= (const vecn<T,Cnt>& m)		{ typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itv!=end()) 
																	   *(itv++)*= *(itm++); 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator*= (const T& m)					{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																	   *(itv++)*= m; 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator/= (const vecn<T,Cnt>& m)		{ typename vecn<T,Cnt>::iterator itv=begin();
																	  typename vecn<T,Cnt>::iterator itm=m.begin();
																	  while(itv!=end()) 
																	   *(itv++)/= *(itm++); 
																	  return *this;
																	}
      vecn<T,Cnt>&			operator/= (const T& m)					{ typename vecn<T,Cnt>::iterator itv=begin();
																	  while(itv!=end()) 
																	   *(itv++)/= m; 
																	  return *this;
																	}

      T&					operator[] (const size_t i)				{ assert(i<Cnt); return val[i];	}
      const T&				operator[] (const size_t i)		const	{ assert(i<Cnt); return val[i];	}

      //T&					operator*  ()							{ assert(val); return *val; }
      //const T&			operator*  ()					const	{ assert(val); return *val; }
      
      //					operator std::valarray<T> ()	const	{ return std::valarray<T>(ptr(),Cnt); }
		
      T&					x()						{ return val[0]; }
      const T&				x()					const	{ return val[0]; }
      T&					r()						{ return val[0]; }
      const T&				r()					const	{ return val[0]; }
      
      T&					y()						{ assert(Cnt>0); return val[1]; }
      const T&				y()					const	{ assert(Cnt>0); return val[1]; }
      T&					g()						{ assert(Cnt>0); return val[1]; }
      const T&				g()					const	{ assert(Cnt>0); return val[1]; }
      
      T&					z()						{ assert(Cnt>1); return val[2]; }
      const T&				z()					const	{ assert(Cnt>1); return val[2]; }
      T&					b()						{ assert(Cnt>1); return val[2]; }
      const T&				b()					const	{ assert(Cnt>1); return val[2]; } 
	      
     
      T&					w()						{ assert(Cnt>2); return val[3]; }
      const T&				w()					const	{ assert(Cnt>2); return val[3]; }
      T&					a()						{ assert(Cnt>2); return val[3]; }
      const T&				a()					const	{ assert(Cnt>2); return val[3]; }


      vecn<T,2>				xy()						{ return vecn<T,2>(val); }
      const vecn<T,2>		xy()					const	{ return vecn<T,2>(val); }
      vecn<T,2>				rg()						{ return vecn<T,2>(val); }
      const vecn<T,2>		rg()					const	{ return vecn<T,2>(val); }
      

      vecn<T,2>				zw()						{ return shifted(+2); }
      const vecn<T,2>		zw()					const	{ return shifted(+2); }
      vecn<T,2>				ba()						{ return shifted(+2); }
      const vecn<T,2>		ba()					const	{ return shifted(+2); }
      
      
      vecn<T,3>				xyz()						{ return vecn<T,3>(val); }
      const vecn<T,3>		xyz()					const	{ return vecn<T,3>(val); }
      vecn<T,3>				rgb()						{ return vecn<T,3>(val); }
      const vecn<T,3>		rgb()					const	{ return vecn<T,3>(val); }
      
      iterator				begin()						{ return val; }
      const_iterator		begin()					const	{ return val; }
      iterator				end()						{ return val+Cnt; }
      const_iterator		end()					const	{ return val+Cnt; }
      
      T*					ptr()					const	{ return val; }
      
      const T&				min()					const	{ return *std::min_element(begin(),end()); }
      const T&				max()					const	{ return *std::max_element(begin(),end()); }

      T	 					length()	 			const	{ return utk::length<T,Cnt>(*this);	}
       
      T						sum()					const	{ iterator it=begin();
															  assert(it!=end());
															  T s = *it;
															  while(++it!=end())
																s+=*it;
															  return s;
															}
      
      veca<T,Cnt>			normal() 				const	{ return utk::normal<T,Cnt>(*this);	}
      
      vecn<T,Cnt>&			normalize() 					{ return utk::normalize<T,Cnt>(*this); }
      
      vecn<T,Cnt>&			flip()							{ const size_t	mid=Cnt/2;
															  for(size_t i=0;i<mid;++i)
															  { const T v=val[i];
																val[i]=val[(Cnt-1)-i];
																val[(Cnt-1)-i]=v;
															  }
															  return *this;
															}
										
      veca<T,Cnt>			flipped()				const	{ const size_t	mid=Cnt/2;
															  veca<T,Cnt>	res;
															  res[mid]=val[mid];
															  for(size_t i=0;i<mid;++i)
															  { res[i]=val[(Cnt-1)-i];
																res[(Cnt-1)-i]=val[i];
															  }
															  return res;
															}

      vecn<T,Cnt>&			apply(T (*func)(const T&))		{ iterator it=begin();
															  while(it!=end())
															  { *it=func(*it);
																it++; 
															  }
															  return *this;    
															}      
      vecn<T,Cnt>&			apply(T (*func)(T))				{ iterator it=begin();
															  while(it!=end())
															  { *it=func(*it);
																it++; 
															  }
															  return *this;    
															}
	  
	  vecn<T,Cnt>&			apply(T (*func)(const T&,const T&),const vecn<T,Cnt>& vec)
							{ iterator it    = begin();
							  iterator vecit = vec.begin();
							  while( it!=end() )
							  { *it = func( *it, *(vecit++) );
								it++; 
							  }
							  return *this;    
							}      

	  vecn<T,Cnt>&			apply(T& (*func)(T&,const T&),const vecn<T,Cnt>& vec)
							{ iterator it    = begin();
							  iterator vecit = vec.begin();
							  while( it != end() )
							  { func( *(it), *(vecit) );
								it++;
								vecit++;
							  }
							  return *this;    
							}      

      //assigns the vector elements by calling index_func with the corresponding index as argument. x(i)=index_func(i)
      void					fill_func_table( T(*index_func)(size_t) )	
							{ for(size_t i=0;i<Cnt;++i)
								val[i]=index_func(i);
							}				

      void					fill(const T& v=T(0))			
                            { iterator it=begin();
							  while(it!=end())
								*it++=v;  
							}
      
      template<class Arg1T>
      veca<T,Cnt>			eval( T (*)(T),
			                      const vecn<T,Cnt>&
			                    );
  
      template<class Arg1T>
      veca<T,Cnt>			eval( T (*)(const T&),
			                      const vecn<T,Cnt>&
			                    );
    
      /*template<class Arg2T>
      veca<T,Cnt>			eval( T (*)(T,Arg2T),
			                      const vecn<T,Cnt>&,
			                      Arg2T
			                    );

      template<class Arg2T>
      veca<T,Cnt>			eval( T (*)(const T&,Arg2T),
			                      const vecn<T,Cnt>&,
			                      Arg2T
			                    );*/
  
      friend std::ostream&	operator<< <>(std::ostream&,const vecn<T,Cnt>&);

      friend std::istream&	operator>> <>(std::istream&,const vecn<T,Cnt>&);
  };
  
  template<class T>
  class	vecn<T,0>		 						
  { 	};

  //array (in stack memory) with vecn interface
  template<class T,size_t Cnt>
  class veca				: public vecn<T,Cnt>
  {
      T						arr[Cnt];
    public:
      typedef T*			iterator;
      typedef T*			const_iterator;
 
							veca()
							:vecn<T,Cnt>(arr)				{ 	}

      explicit		     	veca(const T& scalar)
							:vecn<T,Cnt>(arr)				{ vecn<T,Cnt>::fill(scalar); }

				//set x,y,z and w values explicitly
      explicit				veca(const T& gx,const T& gy,const T& gz=T(0),const T& gw=T(0))
							:vecn<T,Cnt>(arr)				{ if(Cnt>0) vecn<T,Cnt>::x()=gx;
															  if(Cnt>1) vecn<T,Cnt>::y()=gy;
															  if(Cnt>2) vecn<T,Cnt>::z()=gz;
															  if(Cnt>3) vecn<T,Cnt>::w()=gw;
															}
							//fill with default color						
							veca(def_color dc)
							:vecn<T,Cnt>(arr)				{ assert(Cnt>=3);
															  vecn<float,4> c(get_default_color(dc));
															  vecn<T,Cnt>::r()=T(c.r());
															  vecn<T,Cnt>::g()=T(c.g());
															  vecn<T,Cnt>::b()=T(c.b());
															  if(Cnt>3)
																vecn<T,Cnt>::a()=T(c.a());
															}
							//copy						
      explicit				veca(const std::valarray<T>& o)
							: vecn<T,Cnt>(arr)				{ assert(o.size()>0); 
															  for(size_t i=0;i<std::min(Cnt,o.size());++i)
																arr[i]=o[i];
															}

							veca(iterator b,iterator e)
							: vecn<T,Cnt>(arr)				{ utk::size_t i=0;
															  do{ arr[i] = *b; }while(i<Cnt && ++b!=e);
															}
				
							veca(const veca<T,Cnt>& o)
							: vecn<T,Cnt>(arr)				{ std::copy(o.begin(),o.end(),vecn<T,Cnt>::begin()); }
				
      template<utk::size_t Cnt2>
							veca(const vecn<T,Cnt2>& o)
							: vecn<T,Cnt>(arr)				{ std::copy(o.ptr(),o.ptr()+std::min(Cnt,Cnt2),vecn<T,Cnt>::begin()); }
				
      virtual				~veca()							{	}
				
							//fill with return values of a function taking indices to scalars						
      static veca<T,Cnt>    create_func_table( T(*index_func)(size_t) )	
															{ veca<T,Cnt> res;
															  for(size_t i=0;i<Cnt;++i)
																res[i]=index_func(i);
															}				
  };

  //---| invalid vector
  template<class T,size_t Cnt>
  vecn<T,Cnt> vecn<T,Cnt>::invalid = vecn<T,Cnt>(static_cast<T*>(0));

  //---| predifined vector types
  
  typedef	veca<bool,1>		vec1b;
  typedef	veca<bool,2>		vec2b;
  typedef	veca<bool,3>		vec3b;
  typedef	veca<bool,4>		vec4b;
  typedef	veca<size_t,1>		vec1s;
  typedef	veca<size_t,2>		vec2s;
  typedef	veca<size_t,3>		vec3s;
  typedef	veca<size_t,4>		vec4s;
  typedef	veca<int,1>			vec1i;
  typedef	veca<int,2>			vec2i;
  typedef	veca<int,3>			vec3i;
  typedef	veca<int,4>			vec4i;
  typedef	veca<float,1>		vec1f;
  typedef	veca<float,2>		vec2f;
  typedef	veca<float,3>		vec3f;
  typedef	veca<float,4>		vec4f;
  typedef	veca<double,1>		vec1d;
  typedef	veca<double,2>		vec2d;
  typedef	veca<double,3>		vec3d;
  typedef	veca<double,4>		vec4d;

  typedef	vecn<bool,1>		vecn1b;
  typedef	vecn<bool,2>		vecn2b;
  typedef	vecn<bool,3>		vecn3b;
  typedef	vecn<bool,4>		vecn4b;
  typedef	vecn<size_t,1>		vecn1s;
  typedef	vecn<size_t,2>		vecn2s;
  typedef	vecn<size_t,3>		vecn3s;
  typedef	vecn<size_t,4>		vecn4s;
  typedef	vecn<int,1>			vecn1i;
  typedef	vecn<int,2>			vecn2i;
  typedef	vecn<int,3>			vecn3i;
  typedef	vecn<int,4>			vecn4i;
  typedef	vecn<float,1>		vecn1f;
  typedef	vecn<float,2>		vecn2f;
  typedef	vecn<float,3>		vecn3f;
  typedef	vecn<float,4>		vecn4f;
  typedef	vecn<double,1>		vecn1d;
  typedef	vecn<double,2>		vecn2d;
  typedef	vecn<double,3>		vecn3d;
  typedef	vecn<double,4>		vecn4d;

  template<class T,size_t Cnt>
  veca<T,Cnt>			abs(const vecn<T,Cnt>&	v)
  { veca<T,Cnt>	res;  
    for(size_t i=0;i<Cnt;i++)
      res[i]=std::abs(v[i]);
    return res;  
  }

  template<class T,size_t Cnt>
  veca<T,Cnt>			sgn(const vecn<T,Cnt>&	v)
  { utk::veca<T,Cnt> res(v);
    res.apply(utk::sgn);
    return res;  
  }

  template<class T,size_t Cnt>
  vecn<T,Cnt>&			clamp_min(vecn<T,Cnt>&	v,const T& min)
  { 
    return v.apply(utk::clamp_min,min);  
  }

  template<class T,size_t Cnt>
  vecn<T,Cnt>&			clamp_max(vecn<T,Cnt>&	v,const T& max)
  { 
    return v.apply(utk::clamp_max,max);  
  }

  template<class T,size_t Cnt>
  vecn<T,Cnt>&			clamp_min(vecn<T,Cnt>&	v,const vecn<T,Cnt>& min)
  { 
    return v.apply(utk::clamp_min,min);  
  }

  template<class T,size_t Cnt>
  vecn<T,Cnt>&			clamp_max(vecn<T,Cnt>&	v,const vecn<T,Cnt>& max)
  { 
    return v.apply(utk::clamp_max,max);  
  }

  template<class T,size_t Cnt>
  veca<T,Cnt>			floor(const vecn<T,Cnt>& v)
  { veca<T,Cnt>	res;  
    for(size_t i=0;i<Cnt;i++)
      res[i]=std::floor(v[i]);
    return res;  
  }
  template<class T,size_t Cnt>
  veca<T,Cnt>			ceil(const vecn<T,Cnt>& v)
  { veca<T,Cnt>	res;  
    for(size_t i=0;i<Cnt;i++)
      res[i]=std::ceil(v[i]);
    return res;  
  }
  
  template<class T,size_t Cnt>
  T				dot( const vecn<T,Cnt>& a, const vecn<T,Cnt>& b )	
  { return std::inner_product(a.begin(),a.end(),b.begin(),T(0)); }	  
  
  template<class T>
  veca<T,3>		cross( const vecn<T,3>& a, const vecn<T,3>& b )
  { veca<T,3> res;
    res[0]=a[1]*b[2]-b[1]*a[2];
    res[1]=a[2]*b[0]-b[2]*a[0];
    res[2]=a[0]*b[1]-b[0]*a[1];
    return res;
  }
   
  //euklidian norm
  template<class T,size_t Cnt>
  T	 l2norm( const utk::vecn<T,Cnt>& v )
  { return std::sqrt( eval(sqr,v).sum() ); }
  
  //euklidian norm
  template<class T,size_t Cnt>  
  T	 length( const utk::vecn<T,Cnt>& v )
  { return l2norm(v); }

  inline float	length( const utk::vecn<float,2>& v )
  { return std::hypot( v.x(), v.y() ); }

  inline double length( const utk::vecn<double,2>& v )
  { return std::hypot( v.x(), v.y() ); }

  inline long double length( const utk::vecn<long double,2>& v )
  { return std::hypot( v.x(), v.y() ); }

  // causes internal segmentationfault in gcc
  //template<class Ta, class Tb, size_t Dim>
  //auto distance( const vecn<Ta,Dim>& a, const vecn<Tb,Dim>& b ) -> decltype( b[0] - a[0] )

  template<class T, size_t Dim>
  T distance( const vecn<T,Dim>& a, const vecn<T,Dim>& b )
  { return length( b - a ); }
  
  //normalized vector
  template<class T,size_t Cnt> 
  utk::veca<T,Cnt>	normal(const utk::vecn<T,Cnt>& v)
  { assert(length(v)>0.);
    return v/length(v); 
  }

  //normalize vector
  template<class T,size_t Cnt> 
  utk::vecn<T,Cnt>&	normalize(utk::vecn<T,Cnt>& v)
  { assert(length(v)>0.);
    return v/=length(v); 
  }
   
  template<class T>
  veca<T,3>	 cross3(const vecn<T,3>& a,const vecn<T,3>& b,const vecn<T,3>& c)	
  { return b*dot(a,c)-c*dot(a,b); }  
										  
  template<class T,size_t Cnt>
  std::ostream&			operator<<(std::ostream& os,const vecn<T,Cnt>& v)	
  { utk::sequence_to_stream( os<<'(' ,v.begin(),v.end(),",");
    return os<<')';
  }

  template<class T,size_t Cnt>
  std::istream&			operator>>(std::istream& is,vecn<T,Cnt>& v)	
  { utk::sequence_from_stream(is,v.begin(),v.end(),',');
    return is;
  }

  template<class T,size_t Cnt>
  T                 	sum(const vecn<T,Cnt>& v)
  { return std::accumulate( v.begin(), v.end(), T(0) ); }

  template<class T,size_t Cnt>
  veca<T,Cnt>			eval(T (*func)(T),const vecn<T,Cnt>& vec)
  { veca<T,Cnt> res(vec);
	res.apply(func);    
	return res;
  }

  template<class T,size_t Cnt>
  veca<T,Cnt>			eval(T (*func)(const T&),const vecn<T,Cnt>& vec)
  { veca<T,Cnt> res(vec);
	res.apply(func);
    return res;
  }
    
  /*template<class T,class Arg2T,size_t Cnt>
  veca<T,Cnt>			eval(T (*func)(T,Arg2T),const vecn<T,Cnt>& vec,Arg2T arg2)
  { veca<T,Cnt> res;
    typename veca<T,Cnt>::iterator	  rit = res.begin();
    typename vecn<T,Cnt>::const_iterator vit = vec.begin();
    while(rit!=res.end())
    { *(rit++) = func(*(vit++),arg2); }
    return res;
  }

  template<class T,class Arg2T,size_t Cnt>
  veca<T,Cnt>			eval(T (*func)(const T&,Arg2T),const vecn<T,Cnt>& vec,Arg2T arg2)
  { veca<T,Cnt> res;
    typename veca<T,Cnt>::iterator	  rit = res.begin();
    typename vecn<T,Cnt>::const_iterator vit = vec.begin();
    while(rit!=res.end())
    { *(rit++) = func(*(vit++),arg2); }
    return res;
  }*/

}

#pragma GCC visibility pop

#endif

