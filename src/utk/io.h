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

#ifndef	IO_GENERAL_H
#define	IO_GENERAL_H

#include	<iostream>
#include	<string>
#include        <limits>

#pragma GCC visibility push(default)

namespace utk
{
  static const char	directory_delimiter = '/';
 
  template<class It>
  std::istream&		sequence_from_stream(std::istream& is,It bi,It ei)
  { It it=bi;
    while(it!=ei && is>>*it) it++;
    return is;
  }

  //reads values from input stream and ignore delimiter between values
  template<class It>
  std::istream&		sequence_from_stream(std::istream& is,It bi,It ei,char delim)
  { It it=bi;
    while(it!=ei && is>>*it) 
      if(++it!=ei) is.ignore(std::numeric_limits<std::streamsize>::max(),delim);
    return is;
  }
  
  template<class It>
  std::ostream&		sequence_to_stream(std::ostream& os,It bi,It ei,std::string delim="\n")	
  { 
    It it=bi;
    while(it!=ei && os<<*it)
      if(++it!=ei) os<<delim;   
    return os; 
  }
}

#pragma GCC visibility pop

#endif
