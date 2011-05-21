/***************************************************************************
 *            view.h
 *
 *  Mon Jan 24 00:22:37 2011
 *  Copyright  2011  Peter Urban
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

# include "drawable.h"

# define DBG_FLAT_GLCANVAS_ADD_DRAWABLE
# define DBG_FLAT_GLCANVAS_REMOVE_DRAWABLE

namespace flat
{
  template< typename DrawableT >
  class View : public boost::signals::trackable
  {
	public: // types

	  typedef DrawableT* value_type;
	  typedef std::vector< value_type > drawable_list_t;

	private:
	  
 	  // a list of drawable objects
	  drawable_list_t m_drawables;

	public:


	  typename drawable_list_t::iterator 		begin()	{ return m_drawables.begin(); }
  	  typename drawable_list_t::const_iterator 	begin()	const { return m_drawables.begin(); }

	  typename drawable_list_t::iterator 		end()	{ return m_drawables.end(); }
  	  typename drawable_list_t::const_iterator 	end()	const { return m_drawables.end(); }
	  
	  void 	add_drawable( DrawableT* drawable )
	  { 
		assert( drawable );
        # if defined DBG_FLAT_GLCANVAS_ADD_DRAWABLE
	    std::clog << "flat::View::add_drawable\t|" << typeid(DrawableT).name() << std::endl;
        # endif
        
		drawable->connect_invalidator( boost::bind( &View< DrawableT >::invalidate, this, drawable ) );
		drawable->connect_remover    ( boost::bind( &View< DrawableT >::remove_drawable, this, drawable ) );
		
		m_drawables.push_back( drawable );
	  }

	  /*void clear()
	  { 
		std::clog << "flat::GLCanvas::clear" << std::endl;
		m_drawables.clear(); // TODO disconnect
	  }*/
	  
	  void	remove_drawable( DrawableT* drawable )
	  { 
		auto it = std::find( m_drawables.begin(), m_drawables.end(), drawable );
		
		if( it != m_drawables.end() )
		{ m_drawables.erase( it );
          # if defined DBG_FLAT_GLCANVAS_REMOVE_DRAWABLE
          std::clog << "flat::View::remove_drawable\t|drawable removed" << std::endl;
          # endif
		} 
        else // TODO: error handling
  		  std::cerr << "flat::View::remove_drawable\t| FAILED - " << drawable << " not found !" << std::endl;
	  }

	  virtual void	invalidate( DrawableT* ) = 0;

      void	invalidate_view() { invalidate( 0 ); }

  }; 
}