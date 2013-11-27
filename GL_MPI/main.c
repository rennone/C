#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <mpi.h>

#define MASTER 0
#define SLAVE 1

static double spin = 0.0;
static double direction = 0.1;

int rank;
int nproc;

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0, 1.0, 1.0);
  glPushMatrix();
  glutSolidSphere(1.0,2.0,16);
  glRotatef((GLfloat)spin,0.0,1.0,0.0);
  glTranslatef(rank,0.0,0.0);
  glutSolidSphere(0.2,10,8);
  glPopMatrix();
  glutSwapBuffers();
}

void spinDisplay()
{
  if(rank == MASTER){
    spin = (spin + direction);
    spin = spin > 360 ? 360 : spin;
  }

  MPI_Bcast(&spin, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  glutPostRedisplay();
}

void initialize(){
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearAccum(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
}

void reshape(int w, int h){
   double ytheta=60.0;
    double zdist=5.0, xcent=0.0;

    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(ytheta, (GLfloat) w/ (GLfloat) h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if(rank == MASTER)
        xcent=   zdist * tan(M_PI * ytheta/180.0/2.0);
    else
        xcent= - zdist * tan(M_PI * ytheta/180.0/2.0);
    gluLookAt(0.0, 0.0, zdist, xcent, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void mouse(int button, int state, int x, int y)
{
  switch(button){
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN){
      glutIdleFunc(spinDisplay);
      if(direction == 1)
	direction = -1;
      else
	direction = 1;
    }
    break;
    
  case GLUT_MIDDLE_BUTTON:
    If (state == GLUT_DOWN)
      glutIdleFunc(NULL);
    break;
    
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN)
      exit(0);
    break;
    
  default:
    break;
  }
}

int main(int argc, char *argv[]){
  const int num = 32;
  int arr[num];

  for(int i=0; i<num; i++) arr[i] = i;
  
  
  MPI_Init( 0, 0 );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  
  printf( "Hello world from process %d of %d\n", rank, nproc );

  glutInit(&argc, argv);
  glutInitWindowSize(200, 200);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("GLUT & MPI");
  
  initialize();
  
  if(rank == MASTER)
    glutMouseFunc(mouse);
  else
    glutDisplayFunc(display);
  glutReshapeFunc(reshape);


  
  glutIdleFunc(spinDisplay);

  glutMainLoop();

  int st = rank*(num/nproc)+1;
  int en = st + (num/nproc-1);
  
  for(int i=st; i<=en; i++){
    
  }
  
  MPI_Finalize();
  
  return 0;
}
