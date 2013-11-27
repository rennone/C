#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "field.h"
#include "fdtdTM.h"
#include "models.h"
//   メンバ変数    //
static double complex *Ezx = NULL;
static double complex *Ezy = NULL;
static double complex *Hx = NULL;
static double complex *Hy = NULL;
static double *C_EZ = NULL, *C_EZLH = NULL, *C_HXLY = NULL, *C_HYLX = NULL;
static double *C_EZX = NULL, *C_EZY = NULL, *C_EZXLX = NULL, *C_EZYLY = NULL;
static double *C_HX = NULL, *C_HY = NULL;
static double *EPS_EZ = NULL, *EPS_HX = NULL, *EPS_HY = NULL;

//  メンバ関数      //
static void update(void);
static void finish(void);
static void init(void);
static inline void calcE(void);
static inline void calcH(void);

//---------------public method---------------//
//---------------getter------------------//
void (* fdtdTM_getUpdate(void))(void)
{
  return update;
}

void (* fdtdTM_getFinish(void))(void)
{
  return finish;
}

void (* fdtdTM_getInit(void))(void)
{
  return init;
}

double complex* fdtdTM_getHx(void){
  return Hx;
}

double complex* fdtdTM_getHy(void){
  return Hy;
}

double complex* fdtdTM_getEzx(void){
  return Ezx;
}

double complex* fdtdTM_getEzy(void){
  return Ezy;
}
//---------------getter------------------//
//---------------public method---------------//


//---------------private getter----------------//
static inline double complex HX(const int i, const int j){
  return Hx[ind(i,j)];
}

static inline double complex HY(const int i, const int j){
  return Hy[ind(i,j)];
}

static inline double complex EZX(const int i, const int j){
  return Ezx[ind(i,j)];
}

static inline double complex EZY(const int i, const int j){
  return Ezy[ind(i,j)];
}

static inline double CEZX(const int i, const int j){
  return C_EZX[ind(i,j)];
}

static inline double CEZY(const int i, const int j){
  return C_EZY[ind(i,j)];
}

static inline double CEZXLX(const int i, const int j){
  return C_EZXLX[ind(i,j)];
}

static inline double CEZYLY(const int i, const int j){
  return C_EZYLY[ind(i,j)];
}

static inline double CEZ(const int i, const int j){
  return C_EZ[ind(i,j)];
}

static inline double CEZLH(const int i, const int j){
  return C_EZLH[ind(i,j)];
}

static inline double CHXLY(const int i, const int j){
  return C_HXLY[ind(i,j)];
}

static inline double CHYLX(const int i, const int j){
  return C_HYLX[ind(i,j)];
}

static inline double CHX(const int i, const int j){
  return C_HX[ind(i,j)];
}

static inline double CHY(const int i, const int j){
  return C_HY[ind(i,j)];
}

static inline double EPSEZ(const int i, const int j){
  return EPS_EZ[ind(i,j)];
}

static inline double EPSHX(const int i, const int j){
  return EPS_HX[ind(i,j)];
}

static inline double EPSHY(const int i, const int j){
  return EPS_HY[ind(i,j)];
}
//----------private getter--------------//


static void init(){
  Hx = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Hy = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Ezy = (double complex*)malloc(sizeof(double complex)*N_CELL);
  Ezx = (double complex*)malloc(sizeof(double complex)*N_CELL);

  C_HX = (double *)malloc(sizeof(double)*N_CELL);
  C_HY = (double *)malloc(sizeof(double)*N_CELL);
  C_EZ = (double *)malloc(sizeof(double)*N_CELL);
  C_EZX = (double *)malloc(sizeof(double)*N_CELL);
  C_EZY = (double *)malloc(sizeof(double)*N_CELL);

  C_HXLY = (double *)malloc(sizeof(double)*N_CELL);
  C_HYLX = (double *)malloc(sizeof(double)*N_CELL);
  C_EZLH = (double *)malloc(sizeof(double)*N_CELL);

  C_EZXLX = (double *)malloc(sizeof(double)*N_CELL);
  C_EZYLY = (double *)malloc(sizeof(double)*N_CELL);

  EPS_HY = (double *)malloc(sizeof(double)*N_CELL);
  EPS_HX = (double *)malloc(sizeof(double)*N_CELL);
  EPS_EZ = (double *)malloc(sizeof(double)*N_CELL);
  
  memset(Hx, 0, sizeof(double complex)*N_CELL);
  memset(Hy, 0, sizeof(double complex)*N_CELL);
  memset(Ezx,0, sizeof(double complex)*N_CELL);
  memset(Ezy,0, sizeof(double complex)*N_CELL);
  
  //Ez,, Hx, Hyそれぞれでσx, σx*, σy, σy*が違う(場所が違うから)
  double sig_ez_x, sig_ez_y, sig_ez_xx, sig_ez_yy;
  double sig_hx_x, sig_hx_y, sig_hx_xx, sig_hx_yy;
  double sig_hy_x, sig_hy_y, sig_hy_xx, sig_hy_yy;

  double R = 1.0e-8;
  double M = 2.0;
  const double sig_max = -(M+1.0)*EPSILON_0_S*LIGHT_SPEED_S/2.0/N_PML*log(R);
  for(int i=0; i<N_PX; i++){
    for(int j=0; j<N_PY; j++){
      EPS_EZ[ind(i,j)] = models_eps(i,j, D_XY);     //todo D_X, D_Yにしなくていいのか?
      EPS_HX[ind(i,j)] = models_eps(i,j+0.5, D_XY);
      EPS_HY[ind(i,j)] = models_eps(i+0.5,j, D_XY);
      
      sig_ez_x = sig_max*field_sigmaX(i,j);
      sig_ez_xx = MU_0_S/EPSILON_0_S * sig_ez_x;      
      sig_ez_y = sig_max*field_sigmaY(i,j);
      sig_ez_yy = MU_0_S/EPSILON_0_S * sig_ez_y;

      sig_hx_x = sig_max*field_sigmaX(i,j+0.5);
      sig_hx_xx = MU_0_S/EPSILON_0_S * sig_hx_x;
      sig_hx_y = sig_max*field_sigmaY(i,j+0.5);
      sig_hx_yy = MU_0_S/EPSILON_0_S * sig_hx_y;

      sig_hy_x = sig_max*field_sigmaX(i+0.5,j);
      sig_hy_xx = MU_0_S/EPSILON_0_S * sig_hy_x;      
      sig_hy_y = sig_max*field_sigmaY(i+0.5,j);
      sig_hy_yy = MU_0_S/EPSILON_0_S * sig_hy_y;

      //Δt = 1, μ(i,j) = μ0
      C_EZX[ind(i,j)]   = field_pmlCoef(EPSEZ(i,j), sig_ez_x);
      C_EZXLX[ind(i,j)] = field_pmlCoef_LXY(EPSEZ(i,j), sig_ez_x);

      C_EZY[ind(i,j)]   = field_pmlCoef(EPSEZ(i,j), sig_ez_y);
      C_EZYLY[ind(i,j)] = field_pmlCoef_LXY(EPSEZ(i,j), sig_ez_y);

      C_HX[ind(i,j)]    = field_pmlCoef(MU_0_S, sig_hx_yy);
      C_HXLY[ind(i,j)]  = field_pmlCoef_LXY(MU_0_S, sig_hx_yy);

      C_HY[ind(i,j)]    = field_pmlCoef(MU_0_S, sig_hy_xx);
      C_HYLX[ind(i,j)]  = field_pmlCoef_LXY(MU_0_S, sig_hy_xx);
    }
  }  
}

static void finish(){
  if(Hx != NULL)
    free(Hx);
  if(Hy != NULL)
    free(Hy);
  if(Ezx != NULL)
    free(Ezx);
  if(Ezy != NULL)
    free(Ezy);

  if(C_HY!= NULL)
    free(C_HY);
  if(C_HX!= NULL)
    free(C_HX);
  if(C_HXLY!= NULL)
    free(C_HXLY);
  if(C_HYLX!= NULL)
    free(C_HYLX);
  if(C_EZLH!= NULL)
    free(C_EZLH);
  if(C_EZX != NULL)
    free(C_EZX);
  if(C_EZY != NULL)
    free(C_EZY);
  if(C_EZXLX != NULL)
    free(C_EZXLX);
  if(C_EZYLY != NULL)
    free(C_EZYLY);
  
  if(EPS_HX != NULL)
    free(EPS_HX);
  if(EPS_HY != NULL)
    free(EPS_HY);
  if(EPS_EZ != NULL)
    free(EPS_EZ);  
}

static void update(){
  calcE();
  field_defaultIncidence(Ezx);
  field_defaultIncidence(Ezy);
  calcH();
}

static inline void calcE(){
  for(int i=1; i<N_PX-1; i++)
    for(int j=1; j<N_PY-1; j++)
      Ezx[ind(i,j)] = CEZX(i,j)*EZX(i,j) + CEZXLX(i,j)*(HY(i,j) - HY(i-1,j));

  for(int i=1; i<N_PX-1; i++)
    for(int j=1; j<N_PY-1; j++)
      Ezy[ind(i,j)] = CEZY(i,j)*EZY(i,j) - CEZYLY(i,j)*(HX(i,j) - HX(i,j-1));
}

static inline void calcH(){
  for(int i=1; i<N_PX-1; i++)
    for(int j=1; j<N_PY-1; j++)
      Hx[ind(i,j)] = CHX(i,j)*HX(i,j) - CHXLY(i,j)*( EZX(i,j+1)-EZX(i,j) + EZY(i,j+1)-EZY(i,j));

  for(int i=1; i<N_PX-1; i++)
    for(int j=1; j<N_PY-1; j++)
      Hy[ind(i,j)] = CHY(i,j)*HY(i,j) + CHYLX(i,j)*( EZX(i+1,j)-EZX(i,j) + EZY(i+1,j)-EZY(i,j) );
  
}

