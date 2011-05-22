/***************************************************************************
 *            image-export-dialog.h
 *
 *  Fri Sep  4 20:27:54 2009
 *  Copyright  2009  Peter Urban
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

#ifndef IMAGE_EXPORT_DIALOG_H
#define IMAGE_EXPORT_DIALOG_H

#include "gtkmm.h"

//#include "utk/refptr.h"

#include "surface.h"

namespace gtk
{
  class ImageExportDialog		: public Gtk::FileChooserDialog 
  {
  	  Gtk::RadioButton*		weighted_alignment_radiobutton;
  	  Gtk::RadioButton*		regular_alignment_radiobutton;

  	  Gtk::CheckButton*		texel_outline_checkbutton;
  	  Gtk::CheckButton*		block_outline_checkbutton;

  	  Gtk::SpinButton*		texel_size_spinbutton;
    public:

	  ImageExportDialog( BaseObjectType*,  const Glib::RefPtr<Gtk::Builder>& );

	  void	set_weighted_alignment()							{ weighted_alignment_radiobutton->set_active();}
	
	  void	set_regular_alignment()								{ regular_alignment_radiobutton->set_active(); }

  	  void	set_texel_outline_visibility( const bool& v )		{ texel_outline_checkbutton->set_active(v); }
	
	  void	set_block_outline_visibility( const bool& v )		{ block_outline_checkbutton->set_active(v); }
	
      void	set_texel_size( const float& size )					{ assert(size>0.f);
																				  return texel_size_spinbutton->set_value(size); 
																				}
      void	on_response( int response_id );

      bool	get_weighted_alignment()					const	{ return weighted_alignment_radiobutton->get_active(); }
      bool	get_regular_alignment()						const	{ return regular_alignment_radiobutton->get_active(); }

      bool	get_texel_outline_visibility()				const	{ return texel_outline_checkbutton; }
      bool	get_block_outline_visibility()				const	{ return block_outline_checkbutton; }

      float	get_texel_size()							const	{ return texel_size_spinbutton->get_value(); }
    
	  void	write( const std::shared_ptr<flat::PointCloud>& )  	const;
  };
}


#endif
