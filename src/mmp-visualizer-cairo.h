/***************************************************************************
 *            mmp-visualizer-cairo.h
 *
 *  Tue Jan 18 12:51:25 2011
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

# include "mmp-common.h"
# include "mmp-geodesics.h"

#include <cairomm/context.h>

  namespace mmp
  {
    namespace visualizer
    { 
      namespace cairo
      {

        typedef utk::vec2d          cairo_coord_t;
        
        cairo_coord_t user_unit_distance( const Cairo::RefPtr<Cairo::Context>& );

        void draw_half_arrow( const Cairo::RefPtr<Cairo::Context>&, const cairo_coord_t& posA, const cairo_coord_t& posB );
        
        void draw_centered_text( const Cairo::RefPtr<Cairo::Context>&, const std::string&, const cairo_coord_t& pos = cairo_coord_t(0.,0.) );
        
        void draw_triangle( const Cairo::RefPtr< Cairo::Context>&, const Geodesics::edge_handle& );
      
        void draw_window( const Cairo::RefPtr<Cairo::Context>&, const Window& );
        
        void draw_window( const Cairo::RefPtr<Cairo::Context>&, const Window&, const rgba_color_ref_t& );
        
        template< typename ForwardIterator >
        void draw_windows( const Cairo::RefPtr<Cairo::Context>& cr, ForwardIterator begin, ForwardIterator end )
        {
          for( ForwardIterator it = begin; it != end; ++it ) draw_window( cr, **it );
        }
      
        void draw_face_edge_sequences( const Cairo::RefPtr<Cairo::Context>&, const Geodesics::edge_descriptor&, const Geodesics& );

        void draw_edge_sequence( const Cairo::RefPtr<Cairo::Context>&, const Window&, const Geodesics& );
        
        void draw_edge_sequence( const Cairo::RefPtr<Cairo::Context>&, const Window&, const Geodesics&, const rgba_color_ref_t& );

        template< typename ForwardIterator >
        void draw_edge_sequences( const Cairo::RefPtr<Cairo::Context>& cr, ForwardIterator begin, ForwardIterator end, const Geodesics& geodesics )
        {
          for( ForwardIterator it = begin; it != end; ++it ) draw_edge_sequence( cr, **it, geodesics );
        }
        
        inline std::pair< Cairo::RefPtr<Cairo::Context>, Cairo::RefPtr<Cairo::Surface> >
          setup_cairo_context( const std::string& filename, const size_t& width, const size_t& height, const double& scale)
        {
          std::string path = "logs/" + filename + ".svg";
          std::clog << "mmp::visualizer::setup_cairo_context\t|"
                    << " size (" << width << "," << height << ")"
                    << " scale " << scale
                    << " path " << path
                    << std::endl << std::flush;

          Cairo::RefPtr< Cairo::SvgSurface >  surface = Cairo::SvgSurface::create( path, width, height );
          Cairo::RefPtr< Cairo::Context >     cr = Cairo::Context::create( surface );
          
          // center origin
          cr->translate( width*.5, height*.8 );
          // set pixel size
          cr->scale( scale, -scale );
          // use one-pixel lines per default
          cr->set_line_width( user_unit_distance(cr).max() );
        
          return std::make_pair( cr, surface );
        }
      
        template< typename ForwardIterator >
        void draw_windows_to_file( ForwardIterator begin, ForwardIterator end, const std::string& pathtag)
        {
          assert( begin != end );
          std::ostringstream path;
          path << pathtag << "_windows_on_" << (*begin)->edge;

          auto setup = setup_cairo_context( path.str(), 2000, 2000, 300. );

          draw_windows( setup.first, begin, end );
        
          setup.second->show_page();
        }

        inline void draw_edge_sequence_to_file( const Window& win, const Geodesics& geodesics, const std::string& pathtag)
        {
          std::ostringstream path;
          path << pathtag << "_edge_sequence_of_" << win.id;
        
          auto setup = setup_cairo_context( path.str(), 2000, 2000, 300. );

          draw_edge_sequence( setup.first , win, geodesics );
        
          setup.second->show_page();
        }

        template< typename It >
        inline void draw_edge_sequences_to_file( It begin, It end, const Geodesics& geodesics, const std::string& pathtag)
        { 
          assert( (*begin)->edge == (*(-- It(end)))->edge );
          std::ostringstream path;
          path << pathtag << "_edge_sequences_of_edge_" << (*begin)->edge << "_from_" << (*begin)->id << " to " << (*(--It(end)))->id;
        
          auto setup = setup_cairo_context( path.str(), 2000, 2000, 300. );

		  for( It it = begin; it != end; ++it )
            draw_edge_sequence( setup.first , **it, geodesics );
		
          setup.second->show_page();
        }      
	  
        inline void draw_edge_sequences_to_file( const Window& win, const Geodesics& geodesics, const std::string& pathtag)
	    {
  
          std::ostringstream path;
          path << pathtag << "_edge_sequences_of_edge_" << win.edge << "_trig_" << win.id;
        
          auto setup = setup_cairo_context( path.str(), 2000, 2000, 300. );
		  auto wlist = geodesics.edge_windows( win.edge );

		  for( auto it = wlist.first; it != wlist.second; ++it )
            draw_edge_sequence( setup.first , **it, geodesics );
		
          setup.second->show_page();
        }
	  
        inline void draw_face_edge_sequences_to_file( const Window& win, const Geodesics& geodesics, const std::string& pathtag)
        {
          std::ostringstream path;
          path << pathtag << "_face_sequences_of_" << win.id;
        
          auto setup = setup_cairo_context( path.str(), 2000, 2000, 300. );
        
          draw_face_edge_sequences( setup.first , win.edge, geodesics );
        
          setup.second->show_page();
        }

	    inline void draw_merge_debug_image( Window* left, Window& center, Window* right, Window* merged )
	    {
          std::ostringstream path;
          path << "merge/merge_" << center.id;
        
          auto setup = setup_cairo_context( path.str(), 2000, 2000, 600. );
		
		  if(left) draw_window( setup.first, *left );

		  draw_window( setup.first, center );

		  if(right) draw_window( setup.first, *right );  

		  if(merged) draw_window( setup.first, *merged, rgba_color_t( 0.,0.,0., .2 ) );

		  setup.second->show_page();
	    }
   
      } // of namespace cairo
    } // of namespace visualizer
  } // of namespace mmp