
// flatter - gl_tools.h
// Copyright (C) 2006-2011  Peter Urban (peter.urban@s2003.tu-chemnitz.de)
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

#pragma once

#include	"utk/refptr.h"
#include	"utk/numtypes.h"
#include	"utk/inertial.h"

//#define	DB_FLAT_GL_TOOLS
#ifdef	DB_FLAT_GL_TOOLS
#  define	DB_FLAT_GL_TOOLS_MSG(msg)	  std::cerr<<msg<<std::endl;
#else
#  define	DB_FLAT_GL_TOOLS_MSG(msg)	  {	}
#endif

#  ifdef	__APPLE__
#    include 	"OpenGL/gl.h"
#    include 	"OpenGL/glu.h"
#  else
#    include 	"GL/gl.h"
#    include 	"GL/glu.h"
//#    include 	"GL/glew.h" 
#  endif

#include	<limits>

#pragma GCC visibility push(default)

namespace gl
{ 
  template<class  T>
  class gl_types
  { public:
    //typedef			GLvoid		gl_type;
    //static const	GLenum		gl_enum	= 0;
  };//specializations can be found below

  // gl::Color
	
  inline void		Color( GLfloat  r, GLfloat  g, GLfloat  b ) { glColor3f( r, g, b ); }
  inline void		Color( GLdouble r, GLdouble g, GLdouble b ) { glColor3d( r, g, b ); }

  inline void		Color( GLfloat  r, GLfloat  g, GLfloat  b, GLfloat  a ) { glColor4f( r, g, b, a ); }
  inline void		Color( GLdouble r, GLdouble g, GLdouble b, GLdouble a ) { glColor4d( r, g, b, a ); }

  inline void		Color( const utk::vecn< GLfloat , 3 >& c )	{ glColor3fv( c.ptr() ); }
  inline void		Color( const utk::vecn< GLdouble, 3 >& c )	{ glColor3dv( c.ptr() ); }
  
  inline void    	Color( const utk::vecn< GLfloat , 4 >& c )  { glColor4fv( c.ptr() ); }
  inline void    	Color( const utk::vecn< GLdouble, 4 >& c )  { glColor4dv( c.ptr() ); }

  // gl::Normal
	
  inline void    	Normal( GLfloat  x, GLfloat  y, GLfloat  z )	{ glNormal3f( x, y, z ); }
  inline void    	Normal( GLdouble x, GLdouble y, GLdouble z )	{ glNormal3d( x, y, z ); }
  
  inline void    	Normal( const utk::vecn< GLfloat , 3 >& n )	{ glNormal3fv( n.ptr() ); }
  inline void    	Normal( const utk::vecn< GLdouble, 3 >& n )	{ glNormal3dv( n.ptr() ); }

  // gl::TexCoord2
	
  inline void    	TexCoord2( GLfloat  s, GLfloat  t ) { glTexCoord2f( s, t ); }
  inline void    	TexCoord2( GLdouble s, GLdouble t ) { glTexCoord2d( s, t ); }
  
  inline void    	TexCoord2( const utk::vecn< GLfloat , 2 >& tc )	{ glTexCoord2fv( tc.ptr() ); }
  inline void    	TexCoord2( const utk::vecn< GLdouble, 2 >& tc ) { glTexCoord2dv( tc.ptr() ); }
    
  // gl::Vertex
	
  inline void    	Vertex( GLfloat  x, GLfloat  y, GLfloat  z )    { glVertex3f( x, y, z ); }
  inline void    	Vertex( GLdouble x, GLdouble y, GLdouble z )    { glVertex3d( x, y, z ); }
  
  inline void    	Vertex( const utk::vecn< GLfloat , 3 >& v)  { glVertex3fv( v.ptr() ); }
  inline void		Vertex( const utk::vecn< GLdouble, 3 >& v)  { glVertex3dv( v.ptr() ); }

  // gl::Scale
	
  inline void    	Scale( GLfloat  x, GLfloat  y, GLfloat  z)	{ glScalef( x, y, z ); }
  inline void    	Scale( GLdouble x, GLdouble y, GLdouble z)	{ glScaled( x, y, z ); }
  
  inline void    	Scale( const utk::vecn< GLfloat , 3 >& v)	{ Scale( v[0], v[1], v[2] ); }
  inline void		Scale( const utk::vecn< GLdouble, 3 >& v)	{ Scale( v[0], v[1], v[2] ); }

  // gl::Translate
	
  inline void    	Translate( GLfloat  x, GLfloat  y, GLfloat  z )	{ DB_FLAT_GL_TOOLS_MSG("uv::gl::glTranslate:\t| ("<<x<<", "<<y<<", "<<z<<")")
							                                          glTranslatef( x, y, z ); 
                    								                }
  inline void    	Translate( GLdouble x, GLdouble y, GLdouble z )	{ DB_FLAT_GL_TOOLS_MSG("uv::gl::glTranslate:\t| ("<<x<<", "<<y<<", "<<z<<")")
							                                          glTranslated( x, y, z ); 
                    								                }
    
  inline void    	Translate( const utk::vecn< GLfloat , 3 >& v )  { DB_FLAT_GL_TOOLS_MSG("uv::gl::glTranslate:\t|"<<v)
									                                  glTranslatef( v.x(), v.y(), v.z() ); 
									                                }
  inline void		Translate( const utk::vecn< GLdouble, 3 >& v )	{ glTranslated( v.x(), v.y(), v.z() ); }

	
  inline void    	Trafo( const utk::inertial< GLfloat  >&  i )	{ DB_FLAT_GL_TOOLS_MSG("uv::gl::glTrafo:\t|"<<i)
																      glMultMatrixf( i.get_mat4().ptr() ); 
																    }
  inline void    	Trafo( const utk::inertial< GLdouble >&  i)	    { glMultMatrixd( i.get_mat4().ptr() ); }

	
  void	InvTrafo(const utk::inertial<float>& i); 	
    
  void	InvTrafo(const utk::inertial<double>& i); 	

  void	PrintError();

  // draws a coordinate system of size s x s x s 
  void	    	DrawCoords( GLfloat );		
    
  // draw the 3 edge sequences of a cube along which the intersection points are located
  void	    	DrawBoxPaths( const utk::uint8_t front
                            , const utk::vecn<GLfloat,24>& vert
                            , const GLint* e1
                            , const GLint* e2
                            , const GLint* seq );

  void	    	DrawBoxVertices( const utk::vecn<GLfloat,24>& vert );

  //returns normal of a triangle given by points p1,p2,p3 which have to be in counter-clockwise order 
  template<class T> 
  utk::veca<T,3>  	ccw_normal( const utk::vecn<T,3>& p1,const utk::vecn<T,3>& p2,const utk::vecn<T,3>& p3 ) 
  {     
    utk::veca<T,3>    v12n( p2 );
    utk::veca<T,3> 	v23( p3 );
    v12n -= p1;
    v23  -= p2;
    v12n  = cross(v12n,v23);
    v12n.normalize();
    //DB_UV_GL_TOOLS_MSG("uv::gl::ccw_normal:\t|a "<<p1<<"\t|b "<<p2<<"\t|c "<<p3<<"\t|normal "<<v12n);
    return v12n;
  }
    
  //______________________
  //GL TYPE SPECIALIZATION
  ////////////////////////  

  template<> 
  class gl_types<float>
  { public:
	  typedef	GLfloat		gl_type;
	  static const	GLenum		gl_enum	= GL_FLOAT;
  };
    
  template<> 
  class gl_types<double>
  { public:
	  typedef	GLdouble	gl_type;
      // static const	GLenum		gl_enum	= 0;
  };
    
  template<> 
  class gl_types<int>
  { public:
      typedef	GLint		gl_type;
      static const	GLenum		gl_enum	= GL_INT;
  };
    
  template<> 
  class gl_types<unsigned int>
  { public:
      typedef	GLuint		gl_type;
      static const	GLenum		gl_enum	= GL_UNSIGNED_INT;
  };
    
}//gl

#pragma GCC visibility pop
