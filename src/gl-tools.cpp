// flatter - gl_tools.cpp 
// Copyright (C) 2006  Peter Urban (peter.urban@s2003.tu-chemnitz.de)
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

#include	"gl-tools.h"

void	gl::InvTrafo(const utk::inertial<float>& i) 	
{ DB_FLAT_GL_TOOLS_MSG("uv::gl::InvTrafo\t|"<<i)
  utk::mata<float,4> m = i.get_inverse_mat3();
  m(0,3)=0.; m(1,3)=0.; m(2,3)=0.; m(3,0)=0.; m(3,1)=0.; m(3,2)=0.; m(3,3)=1.;
  glMultMatrixf( m.ptr() ); 
  glTranslatef( -i.position().x(), -i.position().y(), -i.position().z() );
}

void	gl::InvTrafo(const utk::inertial<double>& i) 	
{ DB_FLAT_GL_TOOLS_MSG("uv::gl::InvTrafo\t|"<<i)
  utk::mata<double,4> m = i.get_inverse_mat3();
  m(0,3)=0.; m(1,3)=0.; m(2,3)=0.; m(3,0)=0.; m(3,1)=0.; m(3,2)=0.; m(3,3)=1.;
  DB_FLAT_GL_TOOLS_MSG("uv::gl::InvTrafo\t| matrix"<<std::endl<<m)
  glMultMatrixd( m.ptr() );
  Translate( -i.position().x(), -i.position().y(), -i.position().z() );
}

void	gl::PrintError()
{ GLenum code = glGetError();
  if(code != GL_NO_ERROR)
    std::cerr << "flat::gl::PrintError\t|"
			  << "WARNING: an OpenGL error has occured. "
			  << "message: " << gluErrorString(code) << std::endl;
}

void	gl::DrawCoords( GLfloat size )
{ 
  //std::cerr<<"uv::gl::glDrawCoords\t|axis length "<<s<<std::endl;
  glBegin(GL_LINES);
  { 
    Color ( 1.f , 0.f, 0.f );
    Vertex( 0.f , 0.f, 0.f );
    Vertex( size, 0.f, 0.f );
    
    Color ( 0.f, 1.f , 0.f );
    Vertex( 0.f, 0.f , 0.f );
    Vertex( 0.f, size, 0.f );
    
    Color ( 0.f, 0.f, 1.f );
    Vertex( 0.f, 0.f, 0.f );
    Vertex( 0.f, 0.f, size );
  }
  glEnd();
}

void	gl::DrawBoxPaths(const utk::uint8_t front,const utk::vecn<GLfloat,24>& vert,const GLint* e1,const GLint* e2,const GLint* seq)
{ 
  const utk::uint8_t  start=front*8;
  utk::veca<GLfloat,3>  col(1.f);
  //std::cerr<<"uv::gl::glDrawBoxPaths\t|front "<<int(front)<<std::endl;
  
  for(utk::uint8_t path=0;path<5;path+=2)
  { col.fill();
    col[path/2]=1.;
    Color(col);
    for(int e=0;e<4;++e)
    { int   vid1    = seq[start+e1[path*4+e]];
      int   vid2    = seq[start+e2[path*4+e]];  
      
      //std::cerr<<"\t|vid1 "<<vid1<<"\t|vid2 "<<vid2<<"\t|..."<<std::endl;
      //std::cerr<<"\t|v1 "<<vert[vid1*3]<<"\t"<<vert[vid1*3+1]<<"\t"<<vert[vid1*3+2]<<std::endl;
      //std::cerr<<"\t|v2 "<<vert[vid2*3]<<"\t"<<vert[vid2*3+1]<<"\t"<<vert[vid2*3+2]<<std::endl;
      
      glBegin(GL_LINES);
      {   
	    Vertex(vert[vid1*3],vert[vid1*3+1],vert[vid1*3+2]); 
	    Vertex(vert[vid2*3],vert[vid2*3+1],vert[vid2*3+2]); 
      }	
      glEnd();
    }
  }
  glDisable(GL_DEPTH_TEST);
  glPointSize(3.);
  Color(utk::veca<GLfloat,3>(.9f));
  glBegin(GL_POINTS);
  glVertex3fv(&vert[seq[start+e1[0]]*3]/*,vert[seq[start+e1[0]]*3+1],vert[seq[start+e1[0]]*3+2]*/); 
  glEnd();
}

void	gl::DrawBoxVertices(const utk::vecn<GLfloat,24>& vert)
{ //std::cerr<<"uv::gl::glDrawBoxVertices\t|..."<<std::endl;
  glPointSize(5.);
  for(utk::uint8_t i=0;i<8;++i)
  { utk::veca<GLfloat,3> pos(vert.shifted<3>(i*3));
    glBegin(GL_POINTS);
    Color( (pos+utk::veca<GLfloat,3>(3.,2.,1.))/utk::veca<GLfloat,3>(6.,4.,2.) );
    Vertex( pos );
    glEnd();
  }
}

