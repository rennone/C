#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <GL/glut.h>
#include <unistd.h>

const int width = 1024;
int N_X;
double *Ey_next, *Ey_curr, *Ey_prev;
int rank;
int rrank, lrank;
int nproc;
char hostname[MPI_MAX_PROCESSOR_NAME];
int namelen;
double _time;
double l, w, k, c;

void update()
{
  MPI_Request req1, req2, req3, req4;
  MPI_Status status;

  MPI_Isend(&Ey_curr[N_X]  , 1, MPI_DOUBLE, rrank, 1, MPI_COMM_WORLD, &req1);
  MPI_Irecv(&Ey_curr[0]    , 1, MPI_DOUBLE, lrank, 1, MPI_COMM_WORLD, &req2);
  MPI_Irecv(&Ey_curr[N_X+1], 1, MPI_DOUBLE, rrank, 1, MPI_COMM_WORLD, &req3);
  MPI_Isend(&Ey_curr[1]    , 1, MPI_DOUBLE, lrank, 1, MPI_COMM_WORLD, &req4);  

  for(int i=2; i<N_X;i++){
    Ey_next[i] = c*(Ey_curr[i+1] + Ey_curr[i-1] - 2*Ey_curr[i]) - Ey_prev[i] + 2*Ey_curr[i];    
  }  
  MPI_Wait(&req1, &status);
  MPI_Wait(&req2, &status);
  MPI_Wait(&req3, &status);
  MPI_Wait(&req4, &status);
  
  Ey_next[1]   = c*(Ey_curr[2] + Ey_curr[0] - 2*Ey_curr[1]) - Ey_prev[1] + 2*Ey_curr[1];
  Ey_next[N_X] = c*(Ey_curr[N_X+1] + Ey_curr[N_X-1] - 2*Ey_curr[N_X]) - Ey_prev[N_X] + 2*Ey_curr[N_X];

  if(rank == 0){
    Ey_next[1] = 1.0*(1.0 - exp(-0.0001*_time*_time))*sin(w*_time);
    _time+=1;    
  }
  
  double *tmp = Ey_next;
  Ey_next = Ey_prev;
  Ey_prev = Ey_curr;
  Ey_curr = tmp;

  usleep(1.0e4);
  
  glutPostRedisplay();  
}

void display()
{
  
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3d(1.0,0.0,0.0);
  
  for(int i=1; i<=N_X; i++){
    glBegin(GL_LINES);
    
    glVertex2d(2.0/N_X*(i-1)-1.0, Ey_curr[i]);
    glVertex2d(2.0/N_X*(i)-1.0  , Ey_curr[i+1]);
    
    glEnd();    
  }
  glFlush();
  glutSwapBuffers();
  
}

void initialize()
{
  _time = 0;
  l    = 60;
  k    = 2*M_PI/l;
  c    = 0.7;
  w    = c*k;
}

int main(int argc, char* argv[])
{  

  initialize();  
  MPI_Init( 0, 0 );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  printf( "Hello world from process %d of %d\n", rank, nproc );
  
  N_X = width/nproc;  
  Ey_curr = (double *)malloc(sizeof(double)*(N_X+2));
  Ey_next = (double *)malloc(sizeof(double)*(N_X+2));
  Ey_prev = (double *)malloc(sizeof(double)*(N_X+2));
  memset(Ey_curr,0, sizeof(double)*(N_X+2));
  memset(Ey_next,0, sizeof(double)*(N_X+2));
  memset(Ey_prev,0, sizeof(double)*(N_X+2));
//  rrank = rank == nproc-1 ? 0 : rank + 1;
//  lrank = rank == 0       ? nproc-1 : rank - 1;  
  rrank = rank == nproc-1 ? MPI_PROC_NULL : rank + 1;
  lrank = rank == 0       ? MPI_PROC_NULL : rank - 1;  
  
  glutInit(&argc, argv);
  glutInitWindowSize(100, 100);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("FDTD-1D");
  glutIdleFunc(update);
  glutDisplayFunc(display);
  
  glClearColor(0.0, 0.0, 1.0, 1.0);

  glutMainLoop();
  
  MPI_Finalize();

  return 0;
  
}

