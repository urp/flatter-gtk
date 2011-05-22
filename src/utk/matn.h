//libutk - a utility library 
//Copyright (C) 2006  Peter Urban (peter.urban@s2003.tu-chemnitz.de)
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

#ifndef	UTK_MATN_H
#define UTK_MATN_H

#include	"math.h"
#include	"vecn.h"
#include	<valarray>

#pragma GCC visibility push(default)

namespace utk
{ 
  template<class T,size_t Cnt> 
  class matn;			//forward
  
  template<class T,size_t Cnt> 
  class mata;			//forward
 
  template<class T,size_t Cnt>
  veca<T,Cnt>	operator* (const vecn<T,Cnt>&,const matn<T,Cnt>&);
  
  template<class T,size_t Cnt>
  veca<T,Cnt>	operator* (const matn<T,Cnt>& m,const vecn<T,Cnt>& v);

  template<class T,size_t Cnt>
  mata<T,Cnt>	operator* (const matn<T,Cnt>&,const matn<T,Cnt>&);
  
  template<class T,size_t Cnt>
  std::ostream&	operator<<(std::ostream&,const matn<T,Cnt>&); 
  

  //numerical matrix with column major layout
  template<class T,size_t Cnt>
  class	matn			 			
  { 
    public: 
      friend  class vecn<T,Cnt>;
      typedef typename vecn<T,Cnt*Cnt>::iterator	iterator;
      typedef vecn<T,Cnt>*							col_iterator;
      typedef typename vecn<T,Cnt>::iterator		row_iterator;
    private:
      vecn<T,Cnt*Cnt>				val;
      vecn<T,Cnt>					colit[Cnt];		//implement with one vecn
      
									matn()
									:val(0)										{ 	}
    protected:
      T&							valref(size_t x,size_t y)					{ return val[x*Cnt+y]; }
      const T&						valref(size_t x,size_t y)			const	{ return val[x*Cnt+y]; }
    public:
     explicit						matn(T* arr)
									:val(arr)									{ assert(arr!=0); 
																				  for(size_t i=0;i<Cnt;i++)
																				  { colit[i].ref( val.ptr()+i*Cnt );
																					for(size_t j=0;j<Cnt;j++)
																					  valref(i,j)=arr[i*Cnt+j];
																				  }
																				}
									matn(const matn<T,Cnt>& o)
									:val(o.ptr())								{ for(size_t i=0;i<Cnt;i++)
																				  { colit[i].ref( val.ptr()+i*Cnt );
																					for(size_t j=0;j<Cnt;j++)
																					  valref(i,j)=o(i,j);
																				  }  
																				}				
      virtual						~matn()										{ 	}
      
      static size_t					dim1()										{ return Cnt; }
      static size_t					dim2()										{ return Cnt; }

      static size_t					size()										{ return Cnt*Cnt; }
				
      matn<T,Cnt>&					shift(const int elements)					{ val+=elements; return *this; }
      matn<T,Cnt>&					operator++()								{ val+=Cnt*Cnt; return *this; }
      matn<T,Cnt>					operator++(int)								{ val+=Cnt*Cnt; return utk::matn<T,Cnt*Cnt>(val-Cnt*Cnt); }
				
      template<size_t Cnt2> 
      matn<T,Cnt>&					operator= (const matn<T,Cnt2>& a)			{ const size_t cnt = std::min(Cnt,Cnt2);
																				  for(size_t i=0;i<cnt;i++)
																					for(size_t j=0;j<cnt;j++)
																					  valref(i,j)=a(i,j);
																				  return *this;
																				}
      matn<T,Cnt>&					operator= (const matn<T,Cnt>& a)			{ std::copy(a.begin(),a.end(),begin()); 
																				  return *this;
																				}
      matn<T,Cnt>&					operator*= (const matn<T,Cnt>& m)			{ iterator itv=begin();
																				  iterator itm=m.begin();
																				  while(itv!=end()) 
																				   *(itv++) *= *(itm++); 
																				  return *this;
																				}
      mata<T,Cnt>					operator* (const T& m)				const	{ mata<T,Cnt> r;
																				  iterator itr=r.begin();
																				  iterator itv=begin();
																				  while(itr!=r.end()) 
																				   *(itr++) = (*(itv++)) * m; 
																				  return r;
																				}      
      mata<T,Cnt>					operator/ (const T& m)				const	{ mata<T,Cnt> r;
																				  iterator itr=r.begin();
																				  iterator itv=begin();
																				  while(itr!=r.end()) 
																				   *(itr++) = (*(itv++)) / m; 
																				  return r;
																				}
      matn<T,Cnt>&					operator*= (const T& m)						{ iterator itv=begin();
																				  while(itv!=end()) 
																				   *(itv++) *= m; 
																				  return *this;
																				}
      matn<T,Cnt>&					operator/= (const T& m)						{ typename matn<T,Cnt>::iterator itv=begin();
																				  while(itv!=end()) 
																				   *(itv++) /= m; 
																				  return *this;
																				}

      vecn<T,Cnt>&					operator[] (const size_t i)					{ assert(i<Cnt); return colit[i]; }
      const vecn<T,Cnt>&			operator[] (const size_t i)			const	{ assert(i<Cnt); return colit[i]; }
  
      T&							operator() (const size_t x,const size_t y)	{ assert(x<Cnt && y<Cnt); return valref(x,y);}
      const T&						operator() (const size_t x,const size_t y)
																		const	{ assert(x<Cnt && y<Cnt); return valref(x,y);}
      
      iterator						begin()								const	{ return val.begin(); }
      iterator						end()								const	{ return val.end(); }
    
      col_iterator					col_begin()							const	{ return colit; }
      col_iterator					col_end()							const	{ return colit+Cnt; }

      row_iterator					row_begin(size_t col)				const	{ assert(col<Cnt); return colit[col].begin(); }
      row_iterator					row_end(size_t col)					const	{ assert(col<Cnt); return colit[col].end(); }
      
      matn<T,Cnt>					transposed()						const	{ matn<T,Cnt> res;
																				  for(size_t i=0;i<Cnt;i++)
																					for(size_t j=0;j<Cnt;j++)
                                                                                      res(i,j)=valref(j,i);
																				  return res;
																				}
      vecn<T,Cnt>					col_sums()							const	{ vecn<T,Cnt> s;
																				  for(size_t i=0;i<Cnt;i++)
																					s[i]+=colit[i].sum();
																				  return s;
																				}
      veca<T,Cnt>					get_diag()							const	{ veca<T,Cnt> d;	
																				  for(size_t i=0;i<Cnt;i++)
                                                                                    d[i]=valref(i,i);
																				  return d;
																				}
      void							fill(const T& d=T(0),const T& nd=T(0)) 		{ for(size_t i=0;i<Cnt;i++)
																					for(size_t j=0;j<Cnt;j++)
                                                                                      valref(i,j)= i==j ? d : nd;
																				}
      void							unity()										{ fill(T(1),T(0)); }
      
      T*							ptr()								const	{ return val.ptr(); }
      vecn<T,Cnt*Cnt>&				vec()										{ return val; }
      const vecn<T,Cnt*Cnt>&		vec()								const	{ return val; }
      
      friend veca<T,Cnt>	utk::operator*  <>(const matn<T,Cnt>&, const vecn<T,Cnt>&);
      friend veca<T,Cnt>	utk::operator*  <>(const vecn<T,Cnt>&, const matn<T,Cnt>&);	
      friend mata<T,Cnt>	utk::operator*  <>(const matn<T,Cnt>&, const matn<T,Cnt>&);	
      friend std::ostream&	utk::operator<< <>(std::ostream&     , const matn<T,Cnt>&);
  };
  
  template<class T>
  class	matn<T,0>
  {	};
  
  template<class T,size_t Cnt>
  class	mata						: public matn<T,Cnt>
  { 
      T	      						arr[Cnt*Cnt]; 
    public: 
      friend  class vecn<T,Cnt>;
      typedef typename vecn<T,Cnt*Cnt>::iterator	iterator;
      typedef vecn<T,Cnt>*							col_iterator;
      typedef typename vecn<T,Cnt>::iterator		row_iterator;
    public:
									mata()
									:matn<T,Cnt>(arr)							{ matn<T,Cnt>::vec().fill(); }
				
									//fill diagonal elements
      explicit 						mata(const T& de,const T& nde=T(0))
									: matn<T,Cnt>(arr)							{ fill(de,nde); }
      
      explicit						mata(const vecn<T,Cnt> diag)
									: matn<T,Cnt>(arr)							{ for(size_t i=0;i<Cnt;i++) 
																					for(size_t j=0;j<Cnt;j++) 
																					  matn<T,Cnt>::valref(i,i)= i==j? diag[i] : T(0) ;
																				}
     
      explicit						mata(const std::valarray<T>& diag)
									: matn<T,Cnt>(arr)							{ assert(diag.size()==Cnt); 
																				  for(size_t i=0;i<Cnt;i++)
																					for(size_t j=0;j<Cnt;j++)
																					  matn<T,Cnt>::valref(i,i)= i==j? diag[i] : T(0);
																				}
      explicit						mata(T* array)
									: matn<T,Cnt>(arr)							{ assert(array!=0); 
																				  for(size_t i=0;i<Cnt;i++)
																					for(size_t j=0;j<Cnt;j++)
																					  matn<T,Cnt>::valref(i,j)=array[i*Cnt+j];
																				}
      template<size_t Cnt2> 		mata(const matn<T,Cnt2>& o)
									: matn<T,Cnt>(arr)							{ const size_t cnt=std::min(Cnt,Cnt2);
																				  for(size_t i=0;i<cnt;i++)
																					for(size_t j=0;j<cnt;j++)
																					  matn<T,Cnt>::valref(i,j)=o(i,j);
																				}
									mata(const matn<T,Cnt>& o)
									: matn<T,Cnt>(arr)							{ std::copy(o.vec().begin(),o.vec().end(),mata<T,Cnt>::vec().begin()); }
  };
  
  //----| predefined vector types
  
  typedef	mata<size_t,2>		mat2s;
  typedef	mata<size_t,3>		mat3s;
  typedef	mata<size_t,4>		mat4s;
  typedef	mata<int,2>			mat2i;
  typedef	mata<int,3>			mat3i;
  typedef	mata<int,4>			mat4i;
  typedef	mata<float,2>		mat2f;
  typedef	mata<float,3>		mat3f;
  typedef	mata<float,4>		mat4f;
  typedef	mata<double,2>		mat2d;
  typedef	mata<double,3>		mat3d;
  typedef	mata<double,4>		mat4d;  
  
  typedef	matn<size_t,2>		matn2s;
  typedef	matn<size_t,3>		matn3s;
  typedef	matn<size_t,4>		matn4s;
  typedef	matn<int,2>			matn2i;
  typedef	matn<int,3>			matn3i;
  typedef	matn<int,4>			matn4i;
  typedef	matn<float,2>		matn2f;
  typedef	matn<float,3>		matn3f;
  typedef	matn<float,4>		matn4f;
  typedef	matn<double,2>		matn2d;
  typedef	matn<double,3>		matn3d;
  typedef	matn<double,4>		matn4d;  
  
  template<class T,size_t Cnt>
  veca<T,Cnt>						operator*(const matn<T,Cnt>& m,const vecn<T,Cnt>& v)	
  { veca<T,Cnt> res(T(0));
	for(size_t i=0;i<Cnt;i++)	
	  res+=m[i]*v[i];
	return res;
  }
  
  template<class T,size_t Cnt>
  veca<T,Cnt>						operator*(const vecn<T,Cnt>& v,const matn<T,Cnt>& m)	
  { veca<T,Cnt> res;
	for(size_t i=0;i<Cnt;i++)	
	  res[i]=dot(v,m[i]);
	return res;
  }
  
  template<class T,size_t Cnt>
  mata<T,Cnt>						operator*(const matn<T,Cnt>& m1,const matn<T,Cnt>& m2)	
  { mata<T,Cnt> res;
	for(size_t i=0;i<Cnt;i++)	
	  for(size_t j=0;j<Cnt;j++)	
		res[i]+=m1[j]*m2[j][i];
	return res;
  }
  
  template<class T,size_t Cnt>
  std::ostream&						operator<<(std::ostream& os,const matn<T,Cnt>& m)	
  { for(size_t i=0;i<Cnt;i++)
	{ for(size_t j=0;j<Cnt;j++)	
		os<<m(j,i)<<"\t";
	   os<<std::endl;
	}   
	return os;
  }
  
}

#pragma GCC visibility pop

#endif

