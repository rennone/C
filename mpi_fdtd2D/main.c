//#define _USE_OPENGL

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <mpi.h>
#include <complex.h>

#include "simulator.h"
#include "mpiTM_UPML.h"

#ifdef _USE_OPENGL
 #include "drawer.h"
#endif

int windowX = 100;
int windowY = 100;
int windowWidth = 300;
int windowHeight=300;

void display(void)
{
#ifdef _USE_OPENGL
  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_TEXTURE_COORD_ARRAY );

  drawer_paintImage(1,1, getSubNx(), getSubNy(), getSubNpx(), getSubNpy(), simulator_getDrawingData());
  //drawer_paintImage2(1,1, getSubNx(), getSubNy(), getSubNpx(), getSubNpy(), fdtdTM_upml_getEps());
  drawer_draw();
    
  glDisableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  glutSwapBuffers();
#endif
}

void idle(void)
{
  simulator_calc();

  if( simulator_isFinish() ){
    MPI_Barrier(MPI_COMM_WORLD);
    simulator_finish();
    MPI_Finalize();
exit(0);
  }
  glutPostRedisplay();  //再描画
  MPI_Barrier(MPI_COMM_WORLD);
}

int main( int argc, char *argv[] )
{
    MPI_Init( 0, 0 ); 

#ifdef _USE_OPENGL
    glutInit(&argc, argv);
    glutInitWindowPosition(windowX,windowY);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("FDTD Simulator");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glewInit();
    simulator_init();
    drawer_init(CABS);
    glutMainLoop();
    MPI_Finalize();
#endif

#ifndef _USE_OPENGL
    //only calculate mode
    simulator_init();
    while(!simulator_isFinish()){
       simulator_calc();
    }
    simulator_finish();
    MPI_Finalize();
#endif

    return 1;
}
