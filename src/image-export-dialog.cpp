//           image-export-dialog.cpp
//  Fri Sep  4 20:27:54 2009
//  Copyright  2009  Peter Urban
//  <s9peurba@stud.uni-saarland.de>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA

#include "image-export-dialog.h"
#include "surface.h"

gtk::ImageExportDialog::ImageExportDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& uibuilder)
				  : Gtk::FileChooserDialog(cobject)
{			
  // add response buttons
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  uibuilder->get_widget("weighted_radiobutton",weighted_alignment_radiobutton);

  uibuilder->get_widget("regular_radiobutton",regular_alignment_radiobutton);

  uibuilder->get_widget("texel_outline_checkbutton",texel_outline_checkbutton);

  uibuilder->get_widget("block_outline_checkbutton",block_outline_checkbutton);

  uibuilder->get_widget("texel_size_spinbutton",texel_size_spinbutton);

}

void gtk::ImageExportDialog::on_response(int response_id)
{ 
  hide();
}

void gtk::ImageExportDialog::write(const std::shared_ptr<flat::PointCloud>& surface)	const
{
  assert( !Gtk::FileChooserDialog::get_filename().empty() );

  /* TO DO : make dynamic
  assert( dynamic_cast<QuadSurface*>( surface.get() ) );
  std::dynamic_pointer_cast<QuadSurface>(surface)
	->write_png_file( Gtk::FileChooserDialog::get_filename(),
					  weighted_alignment_radiobutton->get_active(),
					  texel_outline_checkbutton->get_active(),
					  block_outline_checkbutton->get_active(),
					  texel_size_spinbutton->get_value()
					 );*/
}
