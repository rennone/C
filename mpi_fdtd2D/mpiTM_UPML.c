#define USE_MATH_DEFINES
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include "mpiTM_UPML.h"
#include "field.h"
#include "models.h"

/* about MPI  */
static int rank;
static int nproc;
static int offsetX, offsetY;
static int ltRank, rtRank, tpRank, bmRank;
static int proc_x, proc_y;
static int SUB_N_X, SUB_N_Y;
static int SUB_N_PX, SUB_N_PY;
static int SUB_N_CELL;
static MPI_Datatype X_DIRECTION_DOUBLE_COMPLEX;

//Ez(i    , j    ) -> Ez[i,j];
//Hx(i    , j+0.5) -> Hx[i,j];
//Hy(i+0.5, j    ) -> Hy[i,j];
static double complex *Ez = NULL;
static double complex *Jz = NULL;
static double complex *Dz = NULL;

static double complex *Hx = NULL;
static double complex *Mx = NULL;
static double complex *Bx = NULL;

static double complex *Hy = NULL;
static double complex *My = NULL;
static double complex *By = NULL;

static double *C_JZ = NULL, *C_MX = NULL, *C_MY = NULL;
static double *C_JZHXHY = NULL, *C_MXEZ = NULL, *C_MYEZ = NULL;
static double *C_DZ = NULL, *C_BX = NULL, *C_BY = NULL;
static double *C_DZJZ0 = NULL, *C_DZJZ1 = NULL;
static double *C_BXMX0 = NULL, *C_BXMX1 = NULL;
static double *C_BYMY0 = NULL, *C_BYMY1 = NULL;

static double *EPS_EZ = NULL, *EPS_HX = NULL, *EPS_HY = NULL;

static void update(void);
static void finish(void);
static void output(void);
static void freeMemories(void);
static void allocateMemories(void);

static void setCoefficient(void);
static void init(void);
static void init_mpi(void);

static inline void calcJD(void);
static inline void calcE(void);
static inline void calcMB(void);
static inline void calcH(void);
  
//:public
inline int getSubNx(void)
{
  return SUB_N_X;
}
inline int getSubNy(void)
{
  return SUB_N_Y;
}
inline int getSubNpx(void)
{
  return SUB_N_PX;
}
inline int getSubNpy(void)
{
  return SUB_N_PY;
}
inline int getSubNcell(void)
{
  return SUB_N_CELL;
}
void (* fdtdTM_upml_getUpdate(void))(void)
{
  return update;
}
void (* fdtdTM_upml_getFinish(void))(void)
{
  return finish;
}
void (* fdtdTM_upml_getInit(void))(void)
{
  return init;
}
double complex* fdtdTM_upml_getHx(void){
  return Hx;
}
double complex* fdtdTM_upml_getHy(void){
  return Hy;
}
double complex* fdtdTM_upml_getEz(void){
  return Ez;
}
double* fdtdTM_upml_getEps(void){
  return EPS_EZ;
}

//:private
//--------------------getter--------------------//
static int subInd(const int *i, const int *j)
{
  return (*i)*SUB_N_PY + (*j);
}
static int subIndv(const int i, const int j)
{
  return (i)*SUB_N_PY + (j);
}
//Initialize
static void init(void)
{  
  init_mpi();
  allocateMemories();
  setCoefficient();  
}
//Finish
static void finish(void)
{
  output();
  freeMemories();
}
static void Connection_ISend_IRecvH(void)
{
  MPI_Status status;
  MPI_Request req1, req2, req3, req4;
  //left & right
  //this needs only Hy[i-1, j] so send to right and recieve from left 
  int ltRecv = subIndv(0,1);
  int rtSend = subIndv(SUB_N_PX-2, 1);  
  MPI_Isend(&Hy[rtSend], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, rtRank, 1, MPI_COMM_WORLD, &req1);
  MPI_Irecv(&Hy[ltRecv], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, ltRank, 1, MPI_COMM_WORLD, &req2);
  
  //this needs only Hy[i,j-1] so send to top and recieve from bottom   
  int bmRecv = subIndv(0,0);
  int tpSend = subIndv(0,SUB_N_PY-2);
  MPI_Isend(&Hx[tpSend], 1, X_DIRECTION_DOUBLE_COMPLEX, tpRank, 1, MPI_COMM_WORLD, &req3);
  MPI_Irecv(&Hx[bmRecv], 1, X_DIRECTION_DOUBLE_COMPLEX, bmRank, 1, MPI_COMM_WORLD, &req4);

  MPI_Wait(&req1, &status);
  MPI_Wait(&req2, &status);
  MPI_Wait(&req3, &status);
  MPI_Wait(&req4, &status); 
}

static void Connection_ISend_IRecvE(void)
{
  MPI_Status status;
  MPI_Request req1, req2, req3, req4;

  //left & right
  //this needs only Ez[i+1, j] so send to left and recieve from right 
  int rtRecv = subIndv(SUB_N_PX-1,1);
  int ltSend = subIndv(         1,1);
  MPI_Isend(&Ez[ltSend], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, ltRank, 1, MPI_COMM_WORLD, &req1);
  MPI_Irecv(&Ez[rtRecv], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, rtRank, 1, MPI_COMM_WORLD, &req2);
  
  //bottom & top
//this needs only Ez[i, j+1] so send to bottom and recieve from top
  int bmSend = subIndv(0,1);
  int tpRecv = subIndv(0,SUB_N_PY-1);
  MPI_Isend(&Ez[bmSend], 1, X_DIRECTION_DOUBLE_COMPLEX, bmRank, 1, MPI_COMM_WORLD, &req3);
  MPI_Irecv(&Ez[tpRecv], 1, X_DIRECTION_DOUBLE_COMPLEX, tpRank, 1, MPI_COMM_WORLD, &req4);
  
  MPI_Wait(&req1, &status);
  MPI_Wait(&req2, &status);
  MPI_Wait(&req3, &status);
  MPI_Wait(&req4, &status);
}

static void Connection_SendRecvE(void)
{
  MPI_Status status;
  
  //left & right
  //this needs only Ez[i+1, j] so send to left and recieve from right 
  int rtRecv = subIndv(SUB_N_PX-1,1);
  int ltSend = subIndv(         1,1);
  MPI_Sendrecv(&Ez[ltSend], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, ltRank, 1,
               &Ez[rtRecv], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, rtRank, 1, MPI_COMM_WORLD, &status);
  
  //bottom & top
//this needs only Ez[i, j+1] so send to bottom and recieve from top
  int bmSend = subIndv(1,1);
  int tpRecv = subIndv(1,SUB_N_PY-1);
  MPI_Sendrecv(&Ez[bmSend], 1, X_DIRECTION_DOUBLE_COMPLEX, bmRank, 1,
               &Ez[tpRecv], 1, X_DIRECTION_DOUBLE_COMPLEX, tpRank, 1, MPI_COMM_WORLD, &status);
}

static void Connection_SendRecvH(void)
{
  MPI_Status status;

  //left & right
  //this needs only Hy[i-1, j] so send to right and recieve from left 
  int ltRecv = subIndv(0,1);
  int rtSend = subIndv(SUB_N_PX-2, 1);
  MPI_Sendrecv(&Hy[rtSend], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, rtRank, 1,
               &Hy[ltRecv], SUB_N_Y, MPI_C_DOUBLE_COMPLEX, ltRank, 1,MPI_COMM_WORLD, &status);
  
  //this needs only Hy[i,j-1] so send to top and recieve from bottom   
  int bmRecv = subIndv(1,0);
  int tpSend = subIndv(1,SUB_N_PY-2);
  MPI_Sendrecv(&Hx[tpSend], 1, X_DIRECTION_DOUBLE_COMPLEX, tpRank, 1,
               &Hx[bmRecv], 1, X_DIRECTION_DOUBLE_COMPLEX, bmRank, 1, MPI_COMM_WORLD, &status);
 
}

//Standard Scattered Wave
void scatteredWave(double complex *p, double *eps){
  double time = field_getTime();
  double w_s  = field_getOmega();
  double ray_coef = field_getRayCoef();
  double k_s = field_getK();  
  double rad = field_getWaveAngle()*M_PI/180;	//ラジアン変換
  double ks_cos = cos(rad)*k_s, ks_sin = sin(rad)*k_s;	//毎回計算すると時間かかりそうだから,代入しておく
  
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      int k = subInd(&i,&j);      
      int x = i-1+offsetX;
      int y = j-1+offsetY;
      double ikx = x*ks_cos + y*ks_sin; //k_s*(i*cos + j*sin)
      p[k] += ray_coef*(EPSILON_0_S/eps[k] - 1)*(cos(ikx-w_s*time) + I*sin(ikx-w_s*time));
    }
  }
}

//Update
static void update(void)
{
  calcJD();
  calcE();
  scatteredWave(Ez, EPS_EZ);
  MPI_Barrier(MPI_COMM_WORLD);
  //Connection_SendRecvE();
  Connection_ISend_IRecvE();
  calcMB();
  calcH();
  //Connection_SendRecvH();
  Connection_SendRecvH();
}

//calculate J and D
static inline void calcJD()
{
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      const int k = subInd(&i,&j);
      const int k_lft = k-SUB_N_PY; //一つ左
      const int k_btm  = k-1;       //一つ下
      const double complex nowJz = Jz[k];
      Jz[k] = C_JZ[k]*Jz[k] + C_JZHXHY[k]*(+Hy[k] - Hy[k_lft] - Hx[k] + Hx[k_btm]);
      Dz[k] = C_DZ[k]*Dz[k] + C_DZJZ1[k]*Jz[k] - C_DZJZ0[k]*nowJz;
    }
  }
}

//calculate E 
static inline void calcE()
{
  double epsilon = EPSILON_0_S;
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){      
      const int k = subInd(&i,&j);
      Ez[k] = Dz[k]/EPS_EZ[k];
    }
  }
}

//calculate M and B
static inline void calcMB()
{  
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      const int k = subInd(&i,&j);
      const int k_top = k+1; //一つ上
      double complex nowMx = Mx[k];
      Mx[k] = C_MX[k]*Mx[k] - C_MXEZ[k]*(Ez[k_top] - Ez[k]);
      Bx[k] = C_BX[k]*Bx[k] + C_BXMX1[k]*Mx[k] - C_BXMX0[k]*nowMx;
    }
  }
  
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      const int k = subInd(&i,&j);
      const int k_rht = k+SUB_N_PY; //一つ右
      double complex nowMy = My[k];
      My[k] = C_MY[k]*My[k] - C_MYEZ[k]*(-Ez[k_rht] + Ez[k]);
      By[k] = C_BY[k]*By[k] + C_BYMY1[k]*My[k] - C_BYMY0[k]*nowMy;
    }
  }
}

//calculate H
static inline void calcH()
{
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      const int k = subInd(&i,&j);
      Hx[k] = Bx[k]/MU_0_S;
    }
  }  
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      const int k = subInd(&i,&j);
      Hy[k] = By[k]/MU_0_S;
    }
  }
}

static void output()
{  
  if(rank == 0){
    MPI_Status status;
    double complex *entire = (double complex*)malloc(sizeof(double complex)*N_CELL);
    memset(entire, 0, sizeof(double complex)*N_CELL);
    
    for(int i=1, x=0; i<SUB_N_PX-1; i++, x++)
      for(int j=1, y=0; j<SUB_N_PY-1; j++, y++)
        entire[ind(x,y)] = Ez[subInd(&i, &j)];
     
    for(int i=1; i<nproc; i++){      
      MPI_Recv(Ez, SUB_N_CELL, MPI_C_DOUBLE_COMPLEX, i, 1, MPI_COMM_WORLD, &status);
      int offX = i/proc_y*SUB_N_X;
      int offY = i%proc_y*SUB_N_Y;
      for(int i=1, x=offX; i<SUB_N_PX-1; i++, x++)
        for(int j=1, y=offY; j<SUB_N_PY-1; j++, y++){
          entire[x*N_PY + y] = Ez[subInd(&i, &j)];
        }
    }    
    field_outputElliptic("mpi_mie.txt", entire);
    free(entire);
  }
  else{
    MPI_Send(Ez, SUB_N_CELL, MPI_C_DOUBLE_COMPLEX, 0, 1, MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

//free memories and MPI_TYPE
static void freeMemories(void)
{
  MPI_Type_free(&X_DIRECTION_DOUBLE_COMPLEX);
  
  if(Ez != NULL){   free(Ez); Ez = NULL;  }  
  if(Hx != NULL){   free(Hx); Hx = NULL;  }
  if(Hy != NULL){   free(Hy); Hy = NULL;  }
    
  if(C_JZ != NULL){ free(C_JZ); C_JZ = NULL;  }
  if(C_JZHXHY != NULL){ free(C_JZHXHY); C_JZHXHY = NULL;  }
  
  if(C_DZ != NULL){ free(C_DZ); C_DZ = NULL;  }
  if(C_DZJZ1 != NULL){ free(C_DZJZ1); C_DZJZ1 = NULL;  }
  if(C_DZJZ0 != NULL){ free(C_DZJZ0); C_DZJZ0 = NULL;  }

  if(C_MX != NULL){ free(C_MX); C_MX = NULL;  }
  if(C_MXEZ != NULL){ free(C_MXEZ); C_MXEZ = NULL;  }
  
  if(C_BX != NULL){ free(C_BX); C_BX = NULL;  }
  if(C_BXMX1 != NULL){ free(C_BXMX1); C_BXMX1 = NULL;  }
  if(C_BXMX0 != NULL){ free(C_BXMX0); C_BXMX0 = NULL;  }
  
  if(C_MY != NULL){ free(C_MY); C_MY = NULL;  }
  if(C_MYEZ != NULL){ free(C_MYEZ); C_MYEZ = NULL;  }

  if(C_BY != NULL){ free(C_BY); C_BY = NULL;  }
  if(C_BYMY1 != NULL){ free(C_BYMY1); C_BYMY1 = NULL;  }
  if(C_BYMY0 != NULL){ free(C_BYMY0); C_BYMY0 = NULL;  }
}

//malloc memories and initialize with 0
static void allocateMemories(void)
{
    
  Ez = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  Dz = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  Jz = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  
  Hx = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  Mx = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  Bx = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  
  Hy = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  My = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  By = (double complex*)malloc(sizeof(double complex)*SUB_N_CELL);
  
  C_JZ = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_MX = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_MY = (double *)malloc(sizeof(double)*SUB_N_CELL);

  C_DZ = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_BX = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_BY = (double *)malloc(sizeof(double)*SUB_N_CELL);

  C_JZHXHY = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_MXEZ = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_MYEZ = (double *)malloc(sizeof(double)*SUB_N_CELL);

  C_DZJZ0 = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_DZJZ1 = (double *)malloc(sizeof(double)*SUB_N_CELL);

  C_BXMX1 = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_BXMX0 = (double *)malloc(sizeof(double)*SUB_N_CELL);

  C_BYMY1 = (double *)malloc(sizeof(double)*SUB_N_CELL);
  C_BYMY0 = (double *)malloc(sizeof(double)*SUB_N_CELL);

  EPS_HY = (double *)malloc(sizeof(double)*SUB_N_CELL);
  EPS_HX = (double *)malloc(sizeof(double)*SUB_N_CELL);
  EPS_EZ = (double *)malloc(sizeof(double)*SUB_N_CELL);

  memset(Hx, 0, sizeof(double complex)*SUB_N_CELL);
  memset(Hy, 0, sizeof(double complex)*SUB_N_CELL);
  memset(Ez, 0, sizeof(double complex)*SUB_N_CELL);

  memset(Mx, 0, sizeof(double complex)*SUB_N_CELL);
  memset(My, 0, sizeof(double complex)*SUB_N_CELL);
  memset(Jz, 0, sizeof(double complex)*SUB_N_CELL);

  memset(Bx, 0, sizeof(double complex)*SUB_N_CELL);
  memset(By, 0, sizeof(double complex)*SUB_N_CELL);
  memset(Dz, 0, sizeof(double complex)*SUB_N_CELL);
}

static void setCoefficient(void)
{
  //Ez,, Hx, Hyそれぞれでσx,σyが違う(場所が違うから)
  double sig_ez_x, sig_ez_y;
  double sig_hx_x, sig_hx_y;
  double sig_hy_x, sig_hy_y;  
  const double R = 1.0e-8;
  const double M = 2.0;
  //const double sig_max = -(M+1.0)*EPSILON_0_S*LIGHT_SPEED_S/2.0/N_PML*log(R);
  const double sig_max = -(M+1.0)*EPSILON_0_S*LIGHT_SPEED_S/2.0/N_PML/cos(M_PI/3)*log(R);
  
  for(int i=1; i<SUB_N_PX-1; i++){
    for(int j=1; j<SUB_N_PY-1; j++){
      int k = subInd(&i,&j);
      int x = i-1 + offsetX; //todo
      int y = j-1 + offsetY;
      
      EPS_EZ[k] = models_eps(x,y, D_XY);     
      EPS_HX[k] = models_eps(x,y+0.5, D_Y); //todo D_Y ?     
      EPS_HY[k] = models_eps(x+0.5,y, D_X); //todo D_X ?

      sig_ez_x = sig_max*field_sigmaX(x,y);
      sig_ez_y = sig_max*field_sigmaY(x,y);

      sig_hx_x = sig_max*field_sigmaX(x,y+0.5);
      sig_hx_y = sig_max*field_sigmaY(x,y+0.5);
      
      sig_hy_x = sig_max*field_sigmaX(x+0.5,y);
      sig_hy_y = sig_max*field_sigmaY(x+0.5,y);

      double sig_z = 0; // σz is zero in
      
      //Δt = 1  Κ_i = 1
      double eps = EPSILON_0_S;
      C_JZ[k]     = ( 2*eps - sig_ez_x) / (2*eps + sig_ez_x);
      C_JZHXHY[k] = ( 2*eps ) / (2*eps + sig_ez_x);
      C_DZ[k]     = ( 2*eps - sig_ez_y)  / (2*eps + sig_ez_y);      
      C_DZJZ1[k]  = ( 2*eps + sig_z)/(2*eps + sig_ez_y);
      C_DZJZ0[k]  = ( 2*eps - sig_z)/(2*eps + sig_ez_y);

      C_MX[k]    = (2*eps - sig_hx_y) / (2*eps + sig_hx_y);
      C_MXEZ[k]  = (2*eps) / (2*eps + sig_hx_y);
      C_BX[k]    = (2*eps - sig_z) / (2*eps + sig_z);
      C_BXMX1[k] = (2*eps + sig_hx_x) / (2*eps + sig_z);
      C_BXMX0[k] = (2*eps - sig_hx_x) / (2*eps + sig_z);

      C_MY[k]    = (2*eps - sig_z) / (2*eps + sig_z);
      C_MYEZ[k]  = (2*eps) / (2*eps + sig_z);      
      C_BY[k]    = (2*eps - sig_hy_x) / (2*eps + sig_hy_x);
      C_BYMY1[k] = (2*eps + sig_hy_y) / (2*eps + sig_hy_x);
      C_BYMY0[k] = (2*eps - sig_hy_y) / (2*eps + sig_hy_x);      
    }
  }

}

static void init_mpi(void)
{
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  if(( nproc & (nproc-1)) != 0){
    printf("rank %d: number of proess must be power of 2 !\n", rank);
    exit(0);
  }
  
  int procs[2]; //[0]: x方向の分割数, [1]:y方向の分割数
  procs[0] = nproc;     /* 横の分割数 */  
  procs[1] = 1; /* 縦の分割数 */
  while(procs[0] > procs[1]){
    procs[0] = procs[0]>>1;
    procs[1] = procs[1]<<1;
  }

  int LRrank[2], BTrank[2];
  int period[2] = {0,0};
  MPI_Comm grid_comm;
  MPI_Cart_create(MPI_COMM_WORLD, 2, procs, period, 0, &grid_comm);
  MPI_Cart_shift(grid_comm, 0, 1, &LRrank[0], &LRrank[1]);
  MPI_Cart_shift(grid_comm, 1, 1, &BTrank[0], &BTrank[1]);
  
  ltRank = LRrank[0];  rtRank = LRrank[1];
  bmRank = BTrank[0];  tpRank = BTrank[1];
  proc_x = procs[0];   proc_y = procs[1];

  
  for(int r=0; r<nproc; r++){
    if(rank == r)
      printf("    %3d\n%3d %3d %3d\n    %3d\n", tpRank, ltRank, rank, rtRank, bmRank);
    MPI_Barrier(MPI_COMM_WORLD);
  }

  SUB_N_X = N_PX / proc_x;
  SUB_N_Y = N_PY / proc_y;
  SUB_N_PX = SUB_N_X + 2; //のりしろの分2大きい
  SUB_N_PY = SUB_N_Y + 2; //のりしろの分2大きい
  SUB_N_CELL = SUB_N_PX*SUB_N_PY;
  offsetX = floor(rank/proc_y) * SUB_N_X; //インデックスではなく, 物理的なオフセットなのでSUB_N_Xずれる
  offsetY = floor(rank%proc_y) * SUB_N_Y;
  
/*これだと, 1個のデータをSUB_N_PY跳び(次のデータまでSUB_N_PY-1個隙間がある),SUB_N_X行ぶん取ってくる事になる */
  MPI_Type_vector(SUB_N_X, 1, SUB_N_PY, MPI_C_DOUBLE_COMPLEX, &X_DIRECTION_DOUBLE_COMPLEX); 
  MPI_Type_commit(&X_DIRECTION_DOUBLE_COMPLEX);
}
