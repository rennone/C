#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <complex.h>
int nproc;
int rank;
int LRrank[2], BTrank[2];
int wproc, hproc;
int offsetX, offsetY;

void rankSetMPICart()
{
  //MPI_CARTによるrank取得
  int dim_sizes[2];
  int period[2];
  int reorder;
  MPI_Comm grid_comm;
  dim_sizes[0] = wproc, dim_sizes[1] = hproc;
  period[0] = period[1] = 1; //固定端境界
  reorder = 0;
  
  MPI_Cart_create(MPI_COMM_WORLD, 2, dim_sizes, period, reorder, &grid_comm);
  MPI_Cart_shift(grid_comm, 0, 1, &LRrank[0], &LRrank[1]);
  MPI_Cart_shift(grid_comm, 1, 1, &BTrank[0], &BTrank[1]);
}

int main()
{  
  MPI_Init( 0, 0 );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  
  const int wid = 8;
  const int hei = 8;
  const int N = wid*hei;
  wproc = nproc; /* 横の分割数 */
  hproc = 1;     /* 縦の分割数 */
  while(wproc > hproc){
    wproc = wproc>>1;
    hproc = hproc<<1;
  }
  
  MPI_Status status;
  MPI_Request req1, req2, req3, req4;
  
  /* ブロック(互いに離れている領域の数) , ブロックのサイズ(各ブロックの連続した要素の数), 隙間+連続データの個数 */
  /* 2個の連続データを, 4個跳びに, 5個取る場合, MPI_Type_Vector(5, 2, 6, ~)となる */
  

  const int subWid = wid / wproc;
  const int subHei = hei / hproc;
  const int pWid = subWid+2;
  const int pHei = subHei+2;
  
  MPI_Datatype COLUMN_MPI;
  MPI_Type_vector(subWid, subHei, subHei, MPI_C_DOUBLE_COMPLEX, &COLUMN_MPI); 
  MPI_Type_commit(&COLUMN_MPI);
  rankSetMPICart();

  offsetX = rank/wproc;
  offsetY = rank%hproc;
  
  double complex array[pWid*pHei];
  memset(array, 0, sizeof(double complex)*pWid*pHei);
  int i, j;
  for(i=0; i<subWid; i++)
    for(j=0; j<subHei; j++)
      array[(i+1)*pHei+j+1] = rank*subWid*subHei + i*subHei+j;

  int r;
  int x, y;
  if(rank == 0){
    double complex *entire = (double complex*)malloc(sizeof(double complex)*wid*hei);
    memset(entire,0,sizeof(double complex)*wid*hei);

    for(i=1,x=offsetX;i<pWid-1;i++,x++ )
      for(j=1, y=offsetY;j<pHei-1;j++,y++)
        entire[x*hei+y] = array[i*pHei+j];
    
    for(r=1; r<nproc; r++){
      MPI_Recv(&array[0], pWid*pHei, MPI_C_DOUBLE_COMPLEX, r, 1, MPI_COMM_WORLD, &status);
      int offX = r/hproc*subWid;
      int offY = r%hproc*subHei;
      for(i=1, x=offX; i<pWid-1;i++, x++)
        for(j=1, y=offY;j<pHei-1;j++, y++)
            entire[x*hei+y] = array[i*pHei+j];
/*
      printf("%.0lf ", creal(array[i*pHei+j]));
      printf("\n");
*/
    }
    
    for(i=0; i<wid; i++){
      for( j=0; j<hei; j++){
        printf("%3.0lf ", creal(entire[i*hei + j]));
      }
      printf("\n");
    }
    
  }
  else{
    MPI_Send(&array[0], pWid*pHei, MPI_C_DOUBLE_COMPLEX, 0, 1, MPI_COMM_WORLD);
  }
  
  /*
  int i,j, r;
  for(i=0; i<subWid+2; i++)
      for(j=0; j<subHei+2; j++)
        array[i*(subHei+2)+j] = rank*(subWid+2)*(subHei+2) + i*(subHei+2) + j;

  if(rank == 0){
    double complex *entire = (double complex*)malloc(sizeof(double complex)*wid*hei);

    for(i=0;i<subWid;i++ )
      for(j=0;j<subHei;j++)
        entire[i*subHei+j] = array[(i+1)*(subHei+2)+j+1];

    //for(r=1; r<nproc; r++){      
      MPI_Recv(&entire[r*subWid*subHei], 1, COLUMN_MPI, 1, 1, MPI_COMM_WORLD, &status);
      for( i=0; i<subWid*subHei; i++){
        printf("%.0lf ", creal(entire[subWid*subHei+i]));

      }
      printf("\n");
//    }
    
  }
  else{
    MPI_Send(&array[(subHei+2)+1], 1, COLUMN_MPI, 0, 1, MPI_COMM_WORLD);
  }

  if(rank == 1){
    for(i=1; i<subWid+1; i++)
      for(j=1; j<subHei+1; j++){
      }
        // printf("%.0lf ", creal(array[i*(subHei+2)+j]));

  }
  */
  MPI_Type_free(&COLUMN_MPI);
  MPI_Finalize();
}
