/***************************************************************************
 *            mmp-queue.h
 *
 *  Fri Apr  9 14:58:17 2010
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

//# define DBG_FLAT_MMP_RRIORITY_QUEUE_VALUES
//# define DBG_FLAT_MMP_RRIORITY_QUEUE_SORTING

# include <list>
# include <functional>
# include <iostream>
# include <algorithm>


  namespace mmp
  {
    // priority queue which allows deletion of elements
    template<typename T, typename Compare = std::less<T> >
	class PriorityQueue
	{
	  public:
		typedef std::list<T>    					    container_t;
		typedef typename container_t::value_type		value_type;

        typedef typename container_t::iterator			iterator;
		typedef typename container_t::const_iterator    const_iterator;
   		typedef typename container_t::reverse_iterator	reverse_iterator;
   		typedef typename container_t::const_reverse_iterator	const_reverse_iterator;

		typedef typename container_t::reference			reference;
   		typedef typename container_t::const_reference	const_reference;
		typedef typename container_t::pointer           pointer;
   		typedef typename container_t::difference_type   difference_type;
        typedef typename container_t::size_type         size_type;
        
	  private:
		container_t					list;
		bool						sortd;
        Compare                     compare;
        
		void						do_sort()									{ 
                                                                                  # if defined DBG_FLAT_MMP_RRIORITY_QUEUE_SORTING
                                                                                  std::clog << "mmp::PriorityQueue::do_sort\t| sorting" << std::endl;
																				  # endif
		  																		  list.sort( compare );
                                                                                  sortd = true;
                                                                                }

	  public:
		  							PriorityQueue( const Compare& comp = Compare() )
									: list(), sortd( true ), compare( comp )	{	}
		
		//iterators
		
		iterator					begin()										{ /*sort();*/ sortd = false; return list.begin(); }
		const_iterator				begin()								const	{ /*sort();*/ return list.begin(); }
		
		iterator					end()										{ /*sort();*/ sortd = false; return list.end(); }
		const_iterator				end()								const	{ /*sort();*/ return list.end(); }

		reverse_iterator			rbegin()									{ /*sort();*/ sortd = false; return list.rbegin(); }
		const_reverse_iterator		rbegin()							const	{ /*sort();*/ return list.rbegin(); }
		
		reverse_iterator			rend()										{ /*sort();*/ sortd = false; return list.rend(); }
		const_reverse_iterator		rend()								const	{ /*sort();*/ return list.rend(); }
        
		const bool					empty()								const	{ return list.empty(); }

		const size_t				size()								const	{ return list.size(); }
		
		const bool&					sorted()							const	{ return sortd; }
		
		void						sort()										{ if( ! sorted() ) do_sort(); }

        reference                   top()                                       { assert( !empty() );
                                                                                  sort(); 
                                                                                  sortd = false;
                                                                                  return list.front();
                                                                                }

        const_reference             top()                               const   { assert( !empty() ); return * std::min_element( begin(), end(), compare ); }

        reference                   bottom()                                    { sort(); 
                                                                                  sortd = false;
                                                                                  assert( !empty() );
                                                                                  return list.back(); 
                                                                                }
        const_reference             bottom()                            const   { assert( !empty() ); return * std::max_element( begin(), end(), compare ); }        
        
        //iterator		            top_iterator()   	                	    { sort(); assert( !empty() ); return list.begin(); }
        //const_iterator		    top_iterator()   	                const   { sort(); assert( !empty() ); return list.begin(); }
		
        value_type					pop()										{ const value_type v = top(); 
		  																		  list.pop_front(); 
																				  return v;
																				}
        
		const_reference				push( const_reference v ) 					{              
                                                                                  # if defined DBG_FLAT_MMP_RRIORITY_QUEUE_VALUES
                                                                                  std::clog << "mmp::PriorityQueue::push\t\t|"
                                                                                            << " value " << v << std::endl;
                                                                                  # endif
                                                                                  list.push_back( v ); 
		   																		  sortd = false;
		  																		  return v;
																				}
        // to be used with std::front_inserter TODO: replace by own InsertIterator
        void                        push_front( const_reference v )             { push(v); }
        

		iterator					insert( iterator loc, const_reference v )   { 
                                                                                  # if defined DBG_FLAT_MMP_RRIORITY_QUEUE_VALUES
                                                                                  std::clog << "mmp::PriorityQueue::insert\t|"
                                                                                            << " value " << v << std::endl;
                                                                                  # endif
		  																		  list.insert(loc,v); sortd = false; return loc; 
																				}


		void						remove( const_reference v )					{ 
																				  # if defined DBG_FLAT_MMP_RRIORITY_QUEUE_VALUES
                                                                                  std::clog << "mmp::PriorityQueue::remove\t\t|"
                                                                                            << " value " << v << std::endl;
                                                                                  # endif
		  																		  list.remove(v); 
																				}
        
		template<class Pred>		
		void						remove_if( const Pred& p )					{ list.remove_if(p); }

		iterator					erase( iterator loc )		                { 
                                                                                  # if defined DBG_FLAT_MMP_RRIORITY_QUEUE_VALUES
                                                                                  std::clog << "mmp::PriorityQueue::erase\t\t|"
                                                                                            << " value " << *loc << std::endl;
                                                                                  # endif
                                                                                  return list.erase(loc); 
                                                                                }
		//iterator					erase( iterator first, iterator last )		{ return list.erase(first, last); }
	};
  }

