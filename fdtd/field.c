#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "field.h"
#include "models.h"

int N_X;
int N_Y;
int N_CELL;
int H;
int N_PML;
int N_PX;
int N_PY;

double time;
static double ray_coef; //波をゆっくり入れる為の係数;
static double waveAngle;
static double lambda_s; //波長 
static double k_s;   //波数 
static double w_s;   //角周波数
static double T_s;   //周期 
static double *EPS;
static void (*defaultWave)(double complex*);
static double maxTime;

//:public------------------------------------//
inline double field_toCellUnit(const double phisycalUnit)
{
  return phisycalUnit/H;   //セル単位に変換 
}

inline double field_toPhisycalUnit(const double cellUnit)
{
  return cellUnit*H;    //物理単位(nm)に変換
}

void setField(const int wid, const int hei, const int _h, const int pml, const double lambda, double step)
{
  H = _h;
  N_X = wid / _h;
  N_Y = hei/_h;
  N_PML = pml;
  N_PX = N_X + 2*N_PML;
  N_PY = N_Y + 2*N_PML;
  N_CELL = N_PX * N_PY; //全セル数 
  time = 0;
  maxTime = step;

  lambda_s = field_toCellUnit(lambda);
  k_s = 2*M_PI/lambda_s;
  w_s = LIGHT_SPEED_S*k_s;
  T_s = 2*M_PI/w_s;

  ray_coef = 0;

  waveAngle = 0;
}

//-------------------getter-------------------//
double field_getLambda()
{
  return lambda_s;
}

double field_sigmaX(const double x, const double y)
{
  const int M = 2;
  if(x<N_PML)
    return pow(1.0*(N_PML-x)/N_PML, M);
  
  else if(N_PML <= x && x < (N_X+N_PML))    
    return 0;
  
  else
    return pow(1.0*(x - (N_PX-N_PML-1))/N_PML, M);
}

double field_sigmaY(const double x, double y)
{
  const int M = 2;
  if(y<N_PML)
    return pow(1.0*(N_PML - y)/N_PML,M);
  
  else if(y>=N_PML && y<(N_Y+N_PML))
    return 0.0;

  else
    return pow(1.0*(y - (N_PY-N_PML-1))/N_PML,M);
}

//pml用の係数のひな形 Δt = 1
//ep_mu εかμ(Eの係数->ε, Hの係数-> μ
//sig  σ
double field_pmlCoef(double ep_mu, double sig)
{
  return (1.0 - sig/ep_mu)/(1.0+sig/ep_mu);
}

double field_pmlCoef_LXY(double ep_mu, double sig)
{
  return 1.0/(ep_mu + sig);
  //  return 1.0/ep_mu/(1.0 + sig/ep_mu);
}

//1次元配列に変換
inline int ind(const int i, const int j)
{
  // return i*N_PY + j;
  return i*N_PY + j;
}
//------------------getter-------------------------//


//------------------light method----------------------//
//点光源を返す
double complex field_pointLight(void)
{
  return ray_coef * (cos(w_s*time) + sin(w_s*time)*I);
}

//点光源を中心に入れる
void field_pointLightWave(double complex *p)
{
  p[ind(N_PX/2, N_PY/2)] += ray_coef * (cos(w_s*time) + sin(w_s*time)*I);
}

//散乱波の計算
void field_scatteredWave(double complex *p)
{
  double rad = waveAngle*M_PI/180.0;
  double _cos = cos(rad), _sin = sin(rad);

  int i,j;
  for(i=N_PML; i<N_X+N_PML; i++){
    for(j=N_PML; j<N_Y+N_PML; j++){
      double _eps = EPS[ind(i,j)];
      if(_eps == EPSILON_0_S)
	continue;

      double ikx = k_s*(i*_cos + j*_sin); //波の方向 
      p[ind(i,j)] += ray_coef*(EPSILON_0_S/_eps - 1) * (
					  cos(ikx-w_s*(time+1)) + I*sin(ikx-w_s*(time+1))
					+ cos(ikx-w_s*(time-1)) + I*sin(ikx-w_s*(time-1))
					- 2*cos(ikx-w_s*time)   - 2*I*sin(ikx-w_s*time)	);
    }
  }
}

void field_setDefaultIncidence(enum WAVE_MODE wm)
{
  defaultWave = (wm == SCATTER ? field_scatteredWave : field_pointLightWave);
}

void field_defaultIncidence(double complex* p)
{
  defaultWave(p);
}

//------------------light method----------------------//
//:this method is not neccesary
void field_setEPS()
{
  EPS = (double *)malloc(sizeof(double)*N_CELL);
  int i,j;
  for(i=0; i<N_PX; i++){
    for(j=0; j<N_PY; j++){
      EPS[ind(i,j)] = models_eps(i, j, D_XY);
    }
  }  
}

int field_nextStep(void){
  time+=1.0;
  ray_coef = 100.0*(1.0 - exp(-0.0001*time*time));
  return time >= maxTime;
}

