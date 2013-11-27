
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <mpi.h>
#include "simulator.h"

void display(void);
void idle(void);

void display(void)
{
  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_TEXTURE_COORD_ARRAY );
  
  draw();
    
  glDisableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  glutSwapBuffers();
}

void idle(void)
{
  calc();
  glutPostRedisplay();  //再描画
}

void initialize(){
  init();
}

const int windowX = 100;
const int windowY = 100;
const int windowWidth = 500;
const int windowHeight = 500;

int main( int argc, char *argv[] )
{
    int rank;
    int size;   

    MPI_Init( 0, 0 );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf( "Hello world from process %d of %d\n", rank, size );
    MPI_Finalize();    
    
    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("FDTD Simulator");
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glewInit();

    initialize();
    glutMainLoop();

    /*
    //only calculate mode
    while(1){
      calc(); 
    }
    */
    return 1;
}
