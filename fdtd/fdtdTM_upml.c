#define USE_MATH_DEFINES
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fdtdTM_upml.h"
#include "field.h"
#include "models.h"

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
static void init(void);

static inline void calcJD(void);
static inline void calcE(void);


static inline void calcMB(void);
static inline void calcH(void);

//:public
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

//:private
//--------------------getter--------------------//
static inline double complex EZ(const int i, const int j)
{
  return Ez[ind(i,j)];
}

static inline double complex HX(const int i, const int j)
{
  return Hx[ind(i,j)];
}

static inline double complex HY(const int i, const int j)
{
  return Hy[ind(i,j)];
}

static inline double complex DZ(const int i, const int j)
{
  return Dz[ind(i,j)];
}

static inline double complex BX(const int i, const int j)
{
  return Bx[ind(i,j)];
}

static inline double complex BY(const int i, const int j)
{
  return By[ind(i,j)];
}

static inline double complex JZ(const int i, const int j)
{
  return Jz[ind(i,j)];
}

static inline double complex MX(const int i, const int j)
{
  return Mx[ind(i,j)];
}

static inline double complex MY(const int i, const int j)
{
  return My[ind(i,j)];
}

//Coefficient of Jz in calcJ
static inline double CJZ(const int i, const int j)
{
  return C_JZ[ind(i,j)];
}

//Coefficient of Hx and Hy in calcJ
static inline double CJZHXHY(const int i, const int j)
{
  return C_JZHXHY[ind(i,j)];
}

//Coefficient of Dz in calcD
static inline double CDZ(const int i, const int j)
{
  return C_DZ[ind(i,j)];
}

//Coefficient of Jz(n) in calcD
static inline double CDZJZ0(const int i, const int j)
{
  return C_DZJZ0[ind(i,j)];
}

//Coefficient of Jz(n+1) in calcD
static inline double CDZJZ1(const int i, const int j)
{
  return C_DZJZ1[ind(i,j)];
}

//Coefficient of Mx in calcM
static inline double CMX (const int i, const int j)
{
  return  C_MX[ind(i,j)];
}

//Coefficient of Ez in calcM
static inline double CMXEZ(const int i, const int j)
{
  return C_MXEZ [ind(i,j)];
}

//Coefficient of Bx in calcB
static inline double CBX(const int i, const int j)
{
  return C_BX [ind(i,j)];
}

//Coefficient of Mx(n+1) in calcB
static inline double CBXMX1(const int i, const int j)
{
  return C_BXMX1 [ind(i,j)];
}

//Coefficient of Mx(n) in calcB
static inline double CBXMX0(const int i, const int j)
{
  return C_BXMX0[ind(i,j)];
}

//Coefficient of My in calcM
static inline double CMY (const int i, const int j)
{
  return  C_MY[ind(i,j)];
}

//Coefficient of Ez in calcM
static inline double CMYEZ(const int i, const int j)
{
  return C_MYEZ [ind(i,j)];
}

//Coefficient of By in calcB
static inline double CBY(const int i, const int j)
{
  return C_BY[ind(i,j)];
}

//Coefficient of My(n+1) in calcB
static inline double CBYMY1(const int i, const int j)
{
  return C_BYMY1[ind(i,j)];
}

//Coefficient of My(n) in calcB
static inline double CBYMY0(const int i, const int j)
{
  return C_BYMY0[ind(i,j)];
}

//Epsilon of Ez
static inline double EPSEZ(const int i, const int j)
{
  return EPS_EZ[ind(i,j)];
}

//Epsilon of Hx
static inline double EPSHX(const int i, const int j)
{
  return EPS_HX[ind(i,j)];
}

//Epsilon of Hy
static inline double EPSHY(const int i, const int j)
{
  return EPS_HY[ind(i,j)];
}

static inline void swap(double *a, double *b)
{
  double tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

//Initialize
static void init(void)
{
  Ez = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Dz = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Jz = (double complex*)malloc(sizeof(double complex)*N_CELL);
  
  Hx = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Mx = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Bx = (double complex*)malloc(sizeof(double complex)*N_CELL);
  
  Hy = (double complex*)malloc(sizeof(double complex)*N_CELL);
  My = (double complex*)malloc(sizeof(double complex)*N_CELL);
  By = (double complex*)malloc(sizeof(double complex)*N_CELL);

  C_JZ = (double *)malloc(sizeof(double)*N_CELL);
  C_MX = (double *)malloc(sizeof(double)*N_CELL);
  C_MY = (double *)malloc(sizeof(double)*N_CELL);

  C_DZ = (double *)malloc(sizeof(double)*N_CELL);
  C_BX = (double *)malloc(sizeof(double)*N_CELL);
  C_BY = (double *)malloc(sizeof(double)*N_CELL);

  C_JZHXHY = (double *)malloc(sizeof(double)*N_CELL);
  C_MXEZ = (double *)malloc(sizeof(double)*N_CELL);
  C_MYEZ = (double *)malloc(sizeof(double)*N_CELL);

  C_DZJZ0 = (double *)malloc(sizeof(double)*N_CELL);
  C_DZJZ1 = (double *)malloc(sizeof(double)*N_CELL);

  C_BXMX1 = (double *)malloc(sizeof(double)*N_CELL);
  C_BXMX0 = (double *)malloc(sizeof(double)*N_CELL);

  C_BYMY1 = (double *)malloc(sizeof(double)*N_CELL);
  C_BYMY0 = (double *)malloc(sizeof(double)*N_CELL);

  EPS_HY = (double *)malloc(sizeof(double)*N_CELL);
  EPS_HX = (double *)malloc(sizeof(double)*N_CELL);
  EPS_EZ = (double *)malloc(sizeof(double)*N_CELL);

  memset(Hx, 0, sizeof(double complex)*N_CELL);
  memset(Hy, 0, sizeof(double complex)*N_CELL);
  memset(Ez, 0, sizeof(double complex)*N_CELL);

  memset(Mx, 0, sizeof(double complex)*N_CELL);
  memset(My, 0, sizeof(double complex)*N_CELL);
  memset(Jz, 0, sizeof(double complex)*N_CELL);

  memset(Bx, 0, sizeof(double complex)*N_CELL);
  memset(By, 0, sizeof(double complex)*N_CELL);
  memset(Dz, 0, sizeof(double complex)*N_CELL);

  //Ez,, Hx, Hyそれぞれでσx,σyが違う(場所が違うから)
  double sig_ez_x, sig_ez_y;
  double sig_hx_x, sig_hx_y;
  double sig_hy_x, sig_hy_y;
  
  const double R = 1.0e-8;
  const double M = 2.0;
  //  const double sig_max = -(M+1.0)*EPSILON_0_S*LIGHT_SPEED_S/2.0/N_PML*log(R);
  const double sig_max = -(M+1.0)*EPSILON_0_S*LIGHT_SPEED_S/2.0/N_PML/cos(M_PI/3)*log(R);
  for(int i=0; i<N_PX; i++){
    for(int j=0; j<N_PY; j++){
      EPS_EZ[ind(i,j)] = models_eps(i,j, D_XY);     //todo D_X, D_Yにしなくていいのか?
      EPS_HX[ind(i,j)] = models_eps(i,j+0.5, D_XY);
      EPS_HY[ind(i,j)] = models_eps(i+0.5,j, D_XY);
      
      sig_ez_x = sig_max*field_sigmaX(i,j);
      sig_ez_y = sig_max*field_sigmaY(i,j);

      sig_hx_x = sig_max*field_sigmaX(i,j+0.5);
      sig_hx_y = sig_max*field_sigmaY(i,j+0.5);
      
      sig_hy_x = sig_max*field_sigmaX(i+0.5,j);
      sig_hy_y = sig_max*field_sigmaY(i+0.5,j);

      double sig_z = 0; // σz is zero in 2D ? todo
      
      //Δt = 1  Κ_i = 1
      double eps = EPSILON_0_S;
      
      C_JZ[ind(i,j)]     = ( 2*eps - sig_ez_x) / (2*eps + sig_ez_x);
      C_JZHXHY[ind(i,j)] = ( 2*eps ) / (2*eps + sig_ez_x);
      C_DZ[ind(i,j)]     = ( 2*eps - sig_ez_y)  / (2*eps + sig_ez_y);      
      C_DZJZ1[ind(i,j)]  = ( 2*eps + sig_z)/(2*eps + sig_ez_y);
      C_DZJZ0[ind(i,j)]  = ( 2*eps - sig_z)/(2*eps + sig_ez_y);

      C_MX[ind(i,j)]    = (2*eps - sig_hx_y) / (2*eps + sig_hx_y);
      C_MXEZ[ind(i,j)]  = (2*eps) / (2*eps + sig_hx_y);
      C_BX[ind(i,j)]    = (2*eps - sig_z) / (2*eps + sig_z);
      C_BXMX1[ind(i,j)] = (2*eps + sig_hx_x) / (2*eps + sig_z);
      C_BXMX0[ind(i,j)] = (2*eps - sig_hx_x) / (2*eps + sig_z);
      
      C_MY[ind(i,j)]    = (2*eps - sig_z) / (2*eps + sig_z);
      C_MYEZ[ind(i,j)]  = (2*eps) / (2*eps + sig_z);      
      C_BY[ind(i,j)]    = (2*eps - sig_hy_x) / (2*eps + sig_hy_x);
      C_BYMY1[ind(i,j)] = (2*eps + sig_hy_y) / (2*eps + sig_hy_x);
      C_BYMY0[ind(i,j)] = (2*eps - sig_hy_y) / (2*eps + sig_hy_x);
      
    }
  }
}

//Finish
static void finish(void)
{
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

//Update
static void update(void)
{

  calcJD();
  calcE();
  field_defaultIncidence(Ez);
  calcMB();  
  calcH();
}

//calculate J and D
static inline void calcJD()
{
  for(int i=1; i<N_PX-1; i++){
    for(int j=1; j<N_PY-1; j++){
      double complex nowJz = JZ(i,j);
      Jz[ind(i,j)] = CJZ(i,j)*JZ(i,j) + CJZHXHY(i,j)*(+HY(i,j) - HY(i-1,j) - HX(i,j) + HX(i,j-1) );
      Dz[ind(i,j)] = CDZ(i,j)*DZ(i,j) + CDZJZ1(i,j)*JZ(i,j) - CDZJZ0(i,j)*nowJz;
    }
  }
}

//calculate E 
static inline void calcE()
{
  double epsilon = EPSILON_0_S;
  for(int i=1; i<N_PX-1; i++)
    for(int j=1; j<N_PY-1; j++)
      Ez[ind(i,j)] = DZ(i,j)/EPSEZ(i,j);
}

//calculate M and B
static inline void calcMB()
{
  //todo code of EZ(i, j+1) and EZ(i,j) may be reverse ? 
  for(int i=1; i<N_PX-1; i++){
    for(int j=1; j<N_PY-1; j++){
      double complex nowMx = MX(i,j);
      Mx[ind(i,j)] = CMX(i,j)*MX(i,j) - CMXEZ(i,j)*(EZ(i,j+1) - EZ(i,j));
      Bx[ind(i,j)] = CBX(i,j)*BX(i,j) + CBXMX1(i,j)*MX(i,j) - CBXMX0(i,j)*nowMx;
    }
  }
  
  //todo code of EZ(i+1, j) and EZ(i,j)
  for(int i=1; i<N_PX-1; i++){
    for(int j=1; j<N_PY-1; j++){
      double complex nowMy = MY(i,j);
      My[ind(i,j)] = CMY(i,j)*MY(i,j) - CMYEZ(i,j)*(-EZ(i+1,j) + EZ(i,j));
      By[ind(i,j)] = CBY(i,j)*BY(i,j) + CBYMY1(i,j)*MY(i,j) - CBYMY0(i,j)*nowMy;
    }
  }
}

//calculate H
static inline void calcH()
{
  //todo mu = mu0 ?  
  for(int i=1; i<N_PX-1; i++){
    for(int j=1; j<N_PY-1; j++){      
      Hx[ind(i,j)] = BX(i,j)/MU_0_S;  
    }
  }
  
  for(int i=1; i<N_PX-1; i++){
    for(int j=1; j<N_PY-1; j++){
      Hy[ind(i,j)] = BY(i,j)/MU_0_S;
    }
  }
}







