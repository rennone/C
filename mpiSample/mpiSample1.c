#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <GL/glut.h>
#include <GL/glew.h>


int main()
{  

  int rank;
  int nproc;
  char hostname[MPI_MAX_PROCESSOR_NAME];
  int namelen;
  
  MPI_Init( 0, 0 );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  printf( "Hello world from process %d of %d\n", rank, nproc );

  const int n = N/nproc+2;
  int array[n], ans[n];
  int start = rank*N/nproc;
  int end = start + N/nproc;
  array[0] = array[n-1] = 0;
  
  for(int i=1; i<n-1; i++){
    array[i] = start+i-1;
  }
  

  MPI_Request req1, req2, req3, req4;
  MPI_Status status;
  int buffer, message = rank;
  int sum = 0;
  int tmp[5];
  
  int lrank = (rank == 0       ? MPI_PROC_NULL : rank-1);
  int rrank = (rank == nproc-1 ? MPI_PROC_NULL : rank+1);
 
  MPI_Isend(&array[n-2], 1, MPI_INT, rrank, 1, MPI_COMM_WORLD, &req1);
  MPI_Irecv(&array[n-1], 1, MPI_INT, rrank, 1, MPI_COMM_WORLD, &req2);

  //ここにwaitを書くと駄目, 右に送って右から受け取るようになってるから, waitしてしまうと左のrankが送れない&送信できない.
  //ここにwaitを書くには,右に送って左から受け取るようにする必要がある.
  MPI_Isend(&array[1], 1, MPI_INT, lrank, 1, MPI_COMM_WORLD, &req3);
  MPI_Irecv(&array[0], 1, MPI_INT, lrank, 1, MPI_COMM_WORLD, &req4);

  MPI_Wait(&req1, &status);
  MPI_Wait(&req2, &status);
  MPI_Wait(&req3, &status);
  MPI_Wait(&req4, &status);

  for(int i=1; i<n-1; i++){
    ans[i] = array[i-1] + array[i] + array[i+1];
  }

  
  //MPI_Reduce(&sum, &tmp, 1, MPI_INT, MPI_PROD, 0, MPI_COMM_WORLD);
  for(int i=1; i<n-1; i++){
    printf("proc %d | ans[%d] = %d \n",rank, start+i-1, ans[i]);
  }  
  
  MPI_Finalize();
}
