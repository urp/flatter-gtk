/***************************************************************************
 *            mds-solver.h
 *
 *  Fri Jan  8 15:05:03 2010
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

# include "common.h"

# include "surface.h"
# include "solver.h"


//# define FLAT_MDS_GENERAL_SOLVER_USE_INVERSE

#pragma GCC visibility push(default)

namespace mds
{
  using namespace flat;
  
  using boost::numeric::ublas::matrix;
  using boost::numeric::ublas::symmetric_matrix;
  using boost::numeric::ublas::scalar_matrix;
  using boost::numeric::ublas::triangular_matrix;
  using boost::numeric::ublas::identity_matrix;
  using boost::numeric::ublas::upper;
  using boost::numeric::ublas::lower;
  using boost::numeric::ublas::lower_tag;

  class Solver : public flat::Solver 
  {
    protected:
     
      matrix<coord_t> X;
    
	  virtual void	mds_step() = 0;

    public:  
      
      static std::string    class_name()    { return "MDSSolver (equal weights)"; }
      
	  Solver( const std::shared_ptr< Surface >& surface )
	  : flat::Solver( surface )
      { 
        X.resize( surface->num_vertices(), 2, false ); 
      }

      virtual void set_surface( const std::shared_ptr< Surface >& surface )
      {
        flat::Solver::set_surface( surface );
        X.resize( surface->num_vertices(), 2, false );
      }
      
	  void	step();
  };
  
  class EqualWeightSolver : public Solver
  {
    protected:

      void	mds_step();
  
    public:  
      
      static std::string    class_name()    { return "MDSSolver (equal weights)"; }
      
	  EqualWeightSolver( const std::shared_ptr< Surface >& surface )
	  : Solver( surface )
      { 
        surface->initial_distances.compute_distances( surface, Surface::distance_function::ALL );
      }

      virtual void set_surface( const std::shared_ptr< Surface >& surface )
      {
        Solver::set_surface( surface );
        
        surface->initial_distances.compute_distances( surface, Surface::distance_function::ALL );
      }
  };

  class GeneralSolver : public Solver
  {
    protected:

      // weight matrix
      symmetric_matrix< coord_t, upper > W;

      symmetric_matrix< coord_t, upper > create_V() const;
      
      # if defined FLAT_MDS_GENERAL_SOLVER_USE_INVERSE
      matrix<coord_t> Vinv;      // pseudo inverse of V matrix 
      # else
      triangular_matrix< coord_t, lower > L; // cholesky decomposition of V
      # endif
      
	  void mds_step();

      void initialize_weights();
      void initialize_static_matrices();
      
    public:
     
      typedef coord_t weight_type;

      static std::string    class_name()    { return "MDSSolver (general weights)"; }
      
      GeneralSolver( const std::shared_ptr< Surface >& surface )
	  : Solver( surface )
      { 
        initialize_static_matrices(); 
      }

      virtual void set_surface( const std::shared_ptr< Surface >& surface )
      {
        Solver::set_surface( surface );

        initialize_static_matrices();
      }
  };  
}
#pragma GCC visibility pop
