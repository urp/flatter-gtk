// measure-cli-main.cxx
//
// Copyright (C) 2011 - Peter Urban
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

# include "surface.h"
# include "quad-surface.h"
# include "surface-generators.h"

# include <boost/program_options.hpp>

# define CLI_MEASURE__GL_OUTPUT

# if defined CLI_MEASURE__GL_OUTPUT
#   include "gl-view.h"
#   include "surface-drawable.h"
# endif

int main (int argc, char *argv[])
{
  using namespace flat;

  namespace po = boost::program_options;
  
  std::cout << std::boolalpha;
  std::clog << std::boolalpha;
  std::cerr << std::boolalpha;

  std::cout << "cli-measure - Copyright (C) Peter Urban 2011" << std::endl;

  # if defined CLI_MEASURE__GL_OUTPUT
  // init gtkmm
  Gtk::Main kit(argc, argv);
  // init gtkglextmm.
  Gtk::GL::init(argc, argv);
  # endif
	
  //----| parse commandline
  // Declare the supported options.

  // surface
  const char surface_file_param[]= "surface";
  const char generator_param[]   = "generator";
  const char export_dist_param[] = "export-distances";

  const char session_out_param[]   = "session-out";

  po::options_description desc("Program options");
  desc.add_options()
    ("help", "produce help message")

    (surface_file_param, po::value< std::string >(), "loads surface and texture from the specified file" )
    (generator_param, po::value< std::string >(), "defines a surface generator" )
    (export_dist_param, po::value< std::string >(), "specifies a file to which the distance matrix will be exported" )
    ;
  
  po::variables_map vm;
  po::store( po::parse_command_line( argc, argv, desc ), vm );
  po::notify( vm );    

  //help
  if( vm.count("help") ) { std::cout << desc << std::endl; return 1; }

  
  std::cout << "surface options: ";

  if( vm.count( surface_file_param ) )  { std::cout << surface_file_param << " \"" << vm[ surface_file_param ].as<std::string>() << "\" "; }
  if( vm.count( generator_param ) )     { std::cout << generator_param << " \"" << vm[ generator_param ].as<std::string>() << "\" "; }

  if( vm.count( export_dist_param ) ) { std::cout << export_dist_param << " \"" << vm[ export_dist_param ].as<std::string>() << "\" "; }
  std::cout<<std::endl;


	
  //----| model

  //----| model - surface
  
  typedef flat::QuadSurface	surface_t;

  std::shared_ptr<surface_t> surface;  
  
  if( vm.count( surface_file_param ) )
  {
    std::string path( vm[ surface_file_param ].as< std::string >() );
    flat::PdmFileReader<flat::stride_predicate> generator( path, flat::stride_predicate( {80, 80} ) );
    flat::SimpleRectlinearTriangulator          triangulator( generator.vertex_field_size() );
    flat::CenterRescaleTransform                transform( utk::vec3b(true), utk::vec3b(true) );
    surface = surface_t::create_with_generator( generator, triangulator, transform );
  }else
  if( vm.count( generator_param ) )
  { 
    if( vm[ generator_param ].as< std::string >() == "random" )
    {
      flat::RandomHeightGenerator    	 generator( { 10, 10 }, .1 );
      flat::SimpleRectlinearTriangulator triangulator( generator.vertex_field_size() );
      flat::NoTransform                  transform;
      surface = surface_t::create_with_generator( generator, triangulator, transform );
    } else    
    if( vm[ generator_param ].as< std::string >() == "wave" )
    {
      flat::WaveGenerator         		 generator( { 10, 10 }, .05 );
      flat::SimpleRectlinearTriangulator triangulator( generator.vertex_field_size() );
      flat::NoTransform                  transform;
      surface = surface_t::create_with_generator( generator, triangulator, transform );
    }else
    { std::cerr << "ERROR - unknown surface generator \"" << vm[ generator_param ].as<std::string>() << "\" specified." << std::endl;
      return 0;
    }
  } else
  { std::cerr << "ERROR - no surface specified - use one of the following arguments \"" << surface_file_param << "\", \"" << generator_param << "\" specified." << std::endl;
    return 0;
  }

  surface->initial_distances.compute_distances( surface, Surface::distance_function::ALL );

  //----| export distance matrix
  if( vm.count( export_dist_param ) )
  {
    std::string     path( vm[export_dist_param].as<std::string>() );
    std::ofstream   distfile( path );
    std::clog << "exporting distance matrix to file \"" << path << '\"'<< std::endl;
    distfile << surface->initial_distances;
    distfile.close();
  }

  //----| exit
  return 1;
}	
	
