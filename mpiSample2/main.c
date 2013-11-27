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
/* 自力でrankセット */
void rankSet()
{
  /*
  LRrank[0] = (rank%wproc == 0       ? MPI_PROC_NULL : rank - 1);
  LRrank[1] = (rank%wproc == wproc-1 ? MPI_PROC_NULL : rank + 1);  
  BTrank[0] = (rank/wproc == 0       ? MPI_PROC_NULL : rank - wproc);
  BTrank[1] = (rank/wproc == hproc-1 ? MPI_PROC_NULL : rank + wproc);
*/
  LRrank[0] = (rank%wproc == 0       ? rank+wproc-1 : rank - 1);
  LRrank[1] = (rank%wproc == wproc-1 ? rank-wproc+1 : rank + 1);  
  BTrank[0] = (rank/wproc == 0       ? rank+wproc*(hproc-1) : rank - wproc);
  BTrank[1] = (rank/wproc == hproc-1 ? rank-wproc*(hproc-1) : rank + wproc);  
}

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
  printf( "Hello world from process %d of %d\n", rank, nproc );
  
  const int wid = 8;
  const int hei = 8;
  const int N = wid*hei;
  wproc = nproc; /* 横の分割数 */
  hproc = 1;     /* 縦の分割数 */
  while(wproc > hproc){
    wproc = wproc>>1;
    hproc = hproc<<1;
  }
  const int subWid = wid / wproc;
  const int subHei = hei / hproc;

  double complex array[(subWid+2)*(subHei+2)];

  MPI_Status status;
  MPI_Request req1, req2, req3, req4;
  //MPI_Type_Vector
  /* ブロック(互いに離れている領域の数) , ブロックのサイズ(各ブロックの連続した要素の数), 隙間+連続データの個数 */
  /* 2個の連続データを, 4個跳びに, 5個取る場合, MPI_Type_Vector(5, 2, 6, ~)となる */
  MPI_Datatype COLUMN_MPI;
  /*
  MPI_Datatype MPI_DOUBLE_COMPLEX;
  MPI_Type_vector(1, 2, 0, MPI_DOUBLE, &MPI_DOUBLE_COMPLEX);
  MPI_Type_commit(&MPI_DOUBLE_COMPLEX);
  */
  /*これだと, 1個のデータをHei-1跳び(次のデータまでHei-1個隙間がある),Wid行ぶん取ってくる事になる */
  MPI_Type_vector(subWid, 1, subHei+2, MPI_C_DOUBLE_COMPLEX, &COLUMN_MPI); 
  MPI_Type_commit(&COLUMN_MPI);

  rankSetMPICart();
  //  rankSet();
//  printf("rank %d, L %d, R %d, T %d, B %d \n", rank, LRrank[0], LRrank[1], BTrank[0], BTrank[1]);

  for(int i=0; i<nproc; i++){
    if(rank == i ){
      printf("    %3d   \n",BTrank[1]);
      printf("%3d %3d %3d\n",LRrank[0], rank, LRrank[1]);
      printf("    %3d   \n",BTrank[0]);
      printf("\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  memset(array, 0, sizeof(array));
  for(int i=1; i<subWid+1; i++)
      for(int j=1; j<subHei+1; j++)
        array[i*(subHei+2)+j] = rank*subWid*subHei + (i-1)*subHei + j-1;  

  //left & right
  MPI_Isend(&array[subWid*(subHei+2) + 1], subHei, MPI_C_DOUBLE_COMPLEX, LRrank[1], 1, MPI_COMM_WORLD, &req1);
  MPI_Irecv(&array[0 + 1]                , subHei, MPI_C_DOUBLE_COMPLEX, LRrank[0], 1, MPI_COMM_WORLD, &req2);
  MPI_Wait(&req1, &status);
  MPI_Wait(&req2, &status);

  //top & bottom
  MPI_Isend(&array[subHei+2 + 1       ], 1, COLUMN_MPI, BTrank[0], 1, MPI_COMM_WORLD, &req3);
  MPI_Irecv(&array[subHei+2 + subHei+1], 1, COLUMN_MPI, BTrank[1], 1, MPI_COMM_WORLD, &req4);
  MPI_Wait(&req3, &status);
  MPI_Wait(&req4, &status);

  //left & right
  MPI_Isend(&array[(subHei+2) + 1]           , subHei, MPI_C_DOUBLE_COMPLEX, LRrank[0], 1, MPI_COMM_WORLD, &req1);
  MPI_Irecv(&array[(subWid+1)*(subHei+2) + 1], subHei, MPI_C_DOUBLE_COMPLEX, LRrank[1], 1, MPI_COMM_WORLD, &req2);
  MPI_Wait(&req1, &status);
  MPI_Wait(&req2, &status);

  //top & bottom
  MPI_Isend(&array[subHei+2 + subHei], 1, COLUMN_MPI, BTrank[1], 1, MPI_COMM_WORLD, &req3);
  MPI_Irecv(&array[subHei+2], 1, COLUMN_MPI, BTrank[0], 1, MPI_COMM_WORLD, &req4);
  MPI_Wait(&req3, &status);
  MPI_Wait(&req4, &status);

  if(rank == 1){
    for(int j=0; j<subHei+2;j++){
      for(int i=0; i<subWid+2; i++){
        printf("%5.1lf ", creal(array[i*(subHei+2) + j]));
      }
      printf("\n");          
    }     
  }

  MPI_Finalize();
}
