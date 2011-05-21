// cli_main.cpp
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

# include "spring-solver.h"
# include "mds-solver.h"

# include <boost/program_options.hpp>

# define CLI_FLATTER__GL_OUTPUT

# if defined CLI_FLATTER__GL_OUTPUT
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

  std::cout << "cli-flatter - Copyright (C) Peter Urban 2011" << std::endl;

  //----| parse commandline
  // Declare the supported options.

  // surface
  const char surface_file_param[]= "surface";
  const char generator_param[]   = "generator";
  const char import_dist_param[] = "import-distances";
  const char export_dist_param[] = "export-distances";
  const char stride_x_param[] = "s_x";	
  const char stride_y_param[] = "s_y";
  typedef    size_t stride_type;

	
  // solver
  const char solver_param[]      = "solver";
  typedef    std::string solver_type;
  
  const char stepsize_param[]    = "h";
  typedef    double stepsize_type;
  const char stiffness_param[]   = "ks";
  const char dampening_param[]   = "kd";
  const char ground_param[]      = "kg";
  const char friction_param[]    = "kf";
  typedef    double coeff_type;  

  // main loop
  const char iterations_param[]  = "max-iterations";
  typedef    size_t iteration_type;

  const char iteration_out_param[] = "iteration-out";
  const char session_out_param[]   = "session-out";

  po::options_description desc("Program options");
  desc.add_options()
    ("help", "produce help message")

    (surface_file_param, po::value< std::string >(), "loads surface and texture from the specified file" )
    (generator_param, po::value< std::string >(), "defines a surface generator" )
    (import_dist_param, po::value< std::string >(), "specifies a file containing a full distance matrix" )
    (export_dist_param, po::value< std::string >(), "specifies a file to which the distance matrix will be exported" )
    (stride_x_param, po::value< stride_type >(), "number of samples to skip during downsampling" )
	(stride_y_param, po::value< stride_type >(), "number of samples to skip during downsampling" )
		
    (solver_param, po::value< solver_type >()->default_value( "spring" ), "determines which solver to use" )
    (stepsize_param, po::value< stepsize_type >(), "step size for spring solvers" )
    (stiffness_param, po::value< coeff_type >(), "stiffness coefficient for spring solvers" )
    (dampening_param, po::value< coeff_type >(), "dampening coefficient for spring solvers" )
    (ground_param, po::value< coeff_type >(), "ground attraction coefficient for spring solvers" )
    (friction_param, po::value< coeff_type >(), "friction coefficient for spring solvers" )

    (iterations_param, po::value< iteration_type >()->default_value(100), "defines the maximum amount of solver iterations before the program is stopped.")
    (iteration_out_param/*, po::value< std::string >()*/, "if this is used iteration information will be written to standard output or an optional file." )
    (session_out_param/*, po::value< std::string >()*/, "if this is used session information will be written to standard output or an optional file." )
    ;
  
  po::variables_map vm;
  po::store( po::parse_command_line( argc, argv, desc ), vm );
  po::notify( vm );    

  //help
  if( vm.count("help") ) { std::cout << desc << std::endl; return 1; }

  
  std::cout << "surface options: ";

  if( vm.count( surface_file_param ) )  { std::cout << surface_file_param << " \"" << vm[ surface_file_param ].as<std::string>() << "\" "; }
  if( vm.count( generator_param ) )     { std::cout << generator_param << " \"" << vm[ generator_param ].as<std::string>() << "\" "; }

  if( vm.count( import_dist_param ) ) { std::cout << import_dist_param << " \"" << vm[ import_dist_param ].as<std::string>() << "\" "; }
  if( vm.count( export_dist_param ) ) { std::cout << export_dist_param << " \"" << vm[ export_dist_param ].as<std::string>() << "\" "; }
  if( vm.count( stride_x_param ) ) 	  { std::cout << stride_x_param << " \"" << vm[ stride_x_param ].as< stride_type >() << "\" "; }
  if( vm.count( stride_y_param ) ) 	  { std::cout << stride_y_param << " \"" << vm[ stride_y_param ].as< stride_type >() << "\" "; }
  std::cout<<std::endl;

  std::cout << "solver options: "; 
  
  if( vm.count( stepsize_param ) ) { std::cout << stepsize_param << ' ' << vm[ stepsize_param ].as<stepsize_type>() << ' '; }
  if( vm.count( stiffness_param ) ) { std::cout << stiffness_param << ' ' << vm[ stiffness_param ].as<coeff_type>() << ' '; }
  if( vm.count( dampening_param ) ) { std::cout << dampening_param << ' ' << vm[ dampening_param ].as<coeff_type>() << ' '; }
  if( vm.count( ground_param ) ) { std::cout << ground_param << ' ' << vm[ ground_param ].as<coeff_type>() << ' '; }
  if( vm.count( friction_param ) ) { std::cout << friction_param << ' ' << vm[ friction_param ].as<coeff_type>() << ' '; }
  std::cout << std::endl;

  std::cout << "main loop options: " << iterations_param << " \"" << vm[ iterations_param ].as<iteration_type>() << "\" ";
  if( vm.count( iteration_out_param ) ) { std::cout << iteration_out_param << " \"" << vm[ iteration_out_param ].as<std::string>() << "\" "; }
  if( vm.count( session_out_param ) ) { std::cout << session_out_param << " \"" << vm[ session_out_param ].as<std::string>() << "\" "; }
  std::cout << std::endl;
  
  //----| model

  //----| model - surface
  
  typedef flat::QuadSurface	surface_t;

  //----| import distance matrix
  Surface::distance_function initial_distances;
  if( vm.count( import_dist_param ) )
  {
    std::string     path( vm[import_dist_param].as<std::string>() );
    std::ifstream   distfile( path );
    std::clog << "reading distance matrix from file \"" << path << '\"' << std::endl;
    distfile >> initial_distances;
    distfile.close();
  }

  //----| create surface
  std::shared_ptr<surface_t> surface;  
  
  if( vm.count( surface_file_param ) )
  {
    const stride_type default_stride = 80;
	stride_type stride_x = vm.count( stride_x_param ) ? vm[stride_x_param].as<stride_type>() : default_stride;
	stride_type stride_y = vm.count( stride_y_param ) ? vm[stride_y_param].as<stride_type>() : default_stride;
	  
    std::string path( vm[ surface_file_param ].as< std::string >() );
    flat::PdmFileReader<flat::stride_predicate> generator( path, flat::stride_predicate( { stride_x, stride_y } ) );
    flat::SimpleRectlinearTriangulator          triangulator( generator.vertex_field_size() );
    flat::CenterRescaleTransform                transform( utk::vec3b(true), utk::vec3b(true) );
    surface = surface_t::create_with_generator( generator, triangulator, transform, initial_distances );
  }else
  if( vm.count( generator_param ) )
  { 
    if( vm[ generator_param ].as< std::string >() == "random" )
    {
      flat::RandomHeightGenerator    	 generator( { 10, 10 }, .01 );
      flat::SimpleRectlinearTriangulator triangulator( generator.vertex_field_size() );
      flat::NoTransform                  transform;
      surface = surface_t::create_with_generator( generator, triangulator, transform, initial_distances );
    } else    
    if( vm[ generator_param ].as< std::string >() == "wave" )
    {
      flat::WaveGenerator         		 generator( { 10, 10 }, .1 );
      flat::SimpleRectlinearTriangulator triangulator( generator.vertex_field_size() );
      flat::NoTransform                  transform;
      surface = surface_t::create_with_generator( generator, triangulator, transform, initial_distances );
    }else
    { std::cerr << "ERROR - unknown surface generator \"" << vm[ generator_param ].as<std::string>() << "\" specified." << std::endl;
      return 0;
    }
  } else
  { std::cerr << "ERROR - no surface specified - use one of the following arguments \"" << surface_file_param << "\", \"" << generator_param << "\" specified." << std::endl;
    return 0;
  }

  //----| model - solver

  std::shared_ptr<flat::Solver> solver;  

  solver_type solver_token = vm[ solver_param ].as<solver_type>();
  std::cout << solver_param << ' ' << solver_token << std::endl;
  
  // spring solver
  if( solver_token.find("spring") != std::string::npos )
  {
    bool no_inertia = solver_token.find("inertia") == std::string::npos;
    bool subspace = solver_token.find("embed") == std::string::npos;

    if( no_inertia && subspace )
    {
      typedef spring::SpringSolver< true, true >	solver_t;
      solver_t* new_solver = new solver_t( surface );

      if( vm.count( stepsize_param ) ) new_solver->integrator().set_stepsize( vm[ stepsize_param ].as<stepsize_type>() );
      if( vm.count( stiffness_param ) )new_solver->force().set_stiffness( vm[ stiffness_param ].as<coeff_type>() );
      
      solver.reset( new_solver );
    }else  
    if( no_inertia )
    {
      typedef spring::SpringSolver< true, false >	solver_t;
      solver_t* new_solver = new solver_t( surface );

      if( vm.count( stepsize_param ) ) new_solver->integrator().set_stepsize( vm[ stepsize_param ].as<stepsize_type>() );
      if( vm.count( stiffness_param ) )new_solver->force().set_stiffness( vm[ stiffness_param ].as<coeff_type>() );
      if( vm.count( ground_param ) )   new_solver->force().set_ground_attraction( vm[ ground_param ].as<coeff_type>() );
      
      solver.reset( new_solver );
    }else
    if( subspace )
    {
      typedef spring::SpringSolver< false, true >	solver_t;
      solver_t* new_solver = new solver_t( surface );

      if( vm.count( stepsize_param ) ) new_solver->integrator().set_stepsize( vm[ stepsize_param ].as<stepsize_type>() );
      if( vm.count( stiffness_param ) )new_solver->force().set_stiffness( vm[ stiffness_param ].as<coeff_type>() );
      if( vm.count( dampening_param ) )new_solver->force().set_dampening( vm[ dampening_param ].as<coeff_type>() );
      if( vm.count( friction_param ) ) new_solver->force().set_friction( vm[ friction_param ].as<coeff_type>() );
      
      solver.reset( new_solver );
    }else
    {
      typedef spring::SpringSolver< false, false >	solver_t;
      solver_t* new_solver = new solver_t( surface );

      if( vm.count( stepsize_param ) ) new_solver->integrator().set_stepsize( vm[ stepsize_param ].as<stepsize_type>() );
      if( vm.count( stiffness_param ) )new_solver->force().set_stiffness( vm[ stiffness_param ].as<coeff_type>() );
      if( vm.count( dampening_param ) )new_solver->force().set_dampening( vm[ dampening_param ].as<coeff_type>() );      
      if( vm.count( ground_param ) )   new_solver->force().set_ground_attraction( vm[ ground_param ].as<coeff_type>() );
      if( vm.count( friction_param ) ) new_solver->force().set_friction( vm[ friction_param ].as<coeff_type>() );
      
      solver.reset( new_solver );
    }
  }else // mds solver
  if( solver_token.find("mds") != std::string::npos )
  {
    bool equal_weights = solver_token.find("equal") != std::string::npos;

    if( equal_weights )
    {
      typedef mds::EqualWeightSolver	solver_t;
      solver_t* new_solver = new solver_t( surface );
      solver.reset( new_solver );
    }else  
    {
      typedef mds::GeneralSolver	solver_t;
      solver_t* new_solver = new solver_t( surface );
      solver.reset( new_solver );
    }
  }else 
  { std::cerr << "ERROR - no or unknown solver \"" << solver_token << "\" specified." << std::endl; 
    return 0; 
  }

  //----| export distance matrix
  if( vm.count( export_dist_param ) )
  {
    std::string     path( vm[export_dist_param].as<std::string>() );
    std::ofstream   distfile( path );
    std::clog << "exporting distance matrix to file \"" << path << '\"'<< std::endl;
    distfile << surface->initial_distances;
    distfile.close();
  }
  
  //----|main loop

  //max_iterations
  iteration_type max_iterations = vm[ iterations_param ].as<iteration_type>();

  const bool iteration_out = vm.count( iteration_out_param );
    
  std::time_t it_start_time = std::clock();

  if( iteration_out )
    for( size_t iteration = 0; iteration < max_iterations; ++iteration )
    {
      auto total_sqr_error = surface->get_squared_distance_error();
      std::cout << iteration << '\t' << total_sqr_error.first << " (" << total_sqr_error.second << ')' << std::endl;
      solver->step();
    }
  else
    for( size_t iteration = 0; iteration < max_iterations; ++iteration )
      solver->step();
    
  double it_time = (std::clock() - it_start_time)/double(CLOCKS_PER_SEC);
    
  if( vm.count( session_out_param ) ) 
  {
    auto total_sqr_error = surface->get_squared_distance_error();
    std::cout << max_iterations <<  '\t' << total_sqr_error.first << " (" << total_sqr_error.second << ')' << '\t' << it_time << std::endl;
  }

  # if defined CLI_FLATTER__GL_OUTPUT
  
  //----| OpenGL output
  
    // init gtkmm
  Gtk::Main kit(argc, argv);

  // init gtkglextmm.
  Gtk::GL::init(argc, argv);

  // query OpenGL extension version.
  int major, minor;
  Gdk::GL::query_version(major, minor);
  std::clog << "OpenGL extension version - "
			<< major << '.' << minor
			<< std::endl;

  Gtk::Window gl_win;
  std::auto_ptr< gtk::GLView > gl_view( gtk::GLView::create() );
  std::auto_ptr< gl::SurfaceDrawable > drawable( new gl::SurfaceDrawable( surface ) ); 

  drawable->set_face_mode( gl::SurfaceDrawable::TEXTURE_FACE_MODE );
  gl_view->add_drawable( drawable.get() );

  gl_win.set_title("OpenGL - result view");
  gl_win.add( *gl_view );

  kit.run(gl_win);

  # endif
  
  return true;
}