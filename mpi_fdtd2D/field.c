#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "field.h"
#include "models.h"
#include "function.h"

int N_X;
int N_Y;
int N_CELL;
int H;
int N_PML;
int N_PX;
int N_PY;

static double time;
static double ray_coef; //波をゆっくり入れる為の係数;
static double waveAngle;
static double lambda_s; //波長 
static double k_s;   //波数 
static double w_s;   //角周波数
static double T_s;   //周期 
static void (*defaultWave)(double complex* p, double* eps);
static double maxTime;

static ntffInfo ntff_info;

static void field_setNTFF(int);
//:public------------------------------------//
inline double field_toCellUnit(const double phisycalUnit)
{
  return phisycalUnit/H;   //セル単位に変換 
}

inline double field_toPhisycalUnit(const double cellUnit)
{
  return cellUnit*H;    //物理単位(nm)に変換
}

void setField(const int wid, const int hei, const int _h, const int pml, const double lambda, double maxstep)
{
  H = _h;
  N_PX = wid / _h;
  N_PY = hei / _h;
  N_PML = pml;  
  N_X = N_PX - 2*N_PML;
  N_Y = N_PY - 2*N_PML;
  N_CELL = N_PX * N_PY; //全セル数 
  time = 0;
  maxTime = maxstep;
  
  lambda_s = field_toCellUnit(lambda);
  k_s = 2*M_PI/lambda_s;
  w_s = LIGHT_SPEED_S*k_s;
  T_s = 2*M_PI/w_s;

  ray_coef = 0;  
  waveAngle = 0;


  /* NTFF設定 */
  /*
  ntff_info.top = N_PY - N_PML - 5;
  ntff_info.bottom = N_PML + 5;
  ntff_info.left = N_PML + 5;
  ntff_info.right = N_PX - N_PML - 5;
  
  double len = (ntff_info.top - ntff_info.bottom + 1)/2;
  ntff_info.RFperC = len*2;
  ntff_info.step = maxTime + 4*ntff_info.RFperC;
  */
}  

//-------------------getter-------------------//
double inline field_getK()
{
  return k_s;
}

double inline field_getRayCoef()
{
  return ray_coef;
}

double inline field_getOmega()
{
  return w_s;
}

double inline field_getLambda()
{
  return lambda_s;
}

double inline field_getWaveAngle()
{
  return waveAngle;
}

double inline field_getTime()
{
  return time;
}

double inline field_getMaxTime()
{
  return maxTime;
}

ntffInfo inline field_getNTFFInfo()
{
  return ntff_info;
}
//----------------------------------------//

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
void field_pointLightWave(double complex *p, double *eps)
{
  p[ind(N_PX/2, N_PY/2)] += ray_coef * (cos(w_s*time) + sin(w_s*time)*I);
}


//Standard Scattered Wave
void field_scatteredWave(double complex *p, double *eps){
  double rad = waveAngle*M_PI/180;	//ラジアン変換
  double ks_cos = cos(rad)*k_s, ks_sin = sin(rad)*k_s;	//毎回計算すると時間かかりそうだから,代入しておく
  int i,j;
  for(i=N_PML; i<N_X+N_PML; i++){
    for(j=N_PML; j<N_Y+N_PML; j++){
      double ikx = i*ks_cos + j*ks_sin; //k_s*(i*cos + j*sin)
      p[ind(i,j)] += ray_coef*(EPSILON_0_S/eps[ind(i,j)] - 1)*(cos(ikx-w_s*time) + I*sin(ikx-w_s*time));
    }
  }
}

//Nonstandard Scattered Wave
static void field_NSscatteredWave(double complex *p, double *eps)
{
  double rad = waveAngle*M_PI/180;
  double _cos = cos(rad)*k_s;
  double _sin = sin(rad)*k_s;
  int i,j;
  for(i=N_PML; i<N_X+N_PML; i++){
    for(j=N_PML; j<N_Y+N_PML; j++){
      double _eps = eps[ind(i,j)];
      
      if(_eps == EPSILON_0_S) continue;
      double n  = sqrt(_eps/EPSILON_0_S);				//todo σ = 0が前提の話
      double u0 = sin(w_s/n/2.0)  / sin(  k_s*2.0);
      double u1 = sin(w_s/n/2.0) / sin(n*k_s*2.0);
      double _n = u0/u1;
      double ikx = i*_cos + j*_sin;		//k_s*(i*cos + j*sin)
      p[ind(i,j)] += ray_coef*(1.0/(_n*n) - 1)*(cos(ikx - w_s*time) + I*sin(ikx - w_s*time)					
						);
    }
  }
}

void field_setDefaultIncidence(enum WAVE_MODE wm)
{
  defaultWave = (wm == SCATTER ? field_scatteredWave : field_pointLightWave);
}

void field_defaultIncidence(double complex* p, double *eps)
{
  (*defaultWave)(p, eps);
}

//------------------light method----------------------//
inline void field_nextStep(void){
  time+=1.0;
  ray_coef = 1.0*(1.0 - exp(-0.0001*time*time));
}

inline bool field_isFinish(void){
  return time >= maxTime;
}



//---------------output method---------------//

void field_outputElliptic(const char *fileName, double complex* data)
{
  printf("output start\n");
  //file open
  FILE *fp;

  if( (fp=fopen(fileName, "w") ) == NULL){
    printf("cannot open file %s \n", fileName);
    exit(2);
  }

  for(int ang=180; ang >=0; ang--){
    double rad = ang*M_PI/180.0;
    double x = 1.2*lambda_s*cos(rad)+N_PX/2.0;
    double y = 1.2*lambda_s*sin(rad)+N_PY/2.0;
    double norm = cnorm(cbilinear(data,x,y,N_PX,N_PY));
    fprintf(fp, "%d %lf \n", ang, norm);    
  }
  
  fclose(fp);
  printf("output to %s end\n", fileName);
}

void field_outputAllDataComplex(const char *fileName, double complex *data)
{
   printf("output all data start\n");
  //file open
  FILE *fp;

  if( (fp=fopen(fileName, "w") ) == NULL){
    printf("cannot open file %s \n", fileName);
    exit(2);
  }

  for(int i=0; i<N_PX; i++){
    for(int j=0; j<N_PY; j++){
      fprintf(fp, "%lf \n",cnorm(data[ind(i,j)]));
    }
  }
  
  fclose(fp);
  printf("output all data to %s end\n", fileName);
}


void field_outputAllDataDouble(const char *fileName, double *data)
{
   printf("output all data double start\n");
  //file open
  FILE *fp;

  if( (fp=fopen(fileName, "w") ) == NULL){
    printf("cannot open file %s \n", fileName);
    exit(2);
  }

  for(int i=0; i<N_PX; i++){
    for(int j=0; j<N_PY; j++){
      if(data[ind(i,j)] != 1.0)
        fprintf(fp, "%lf \n",data[ind(i,j)]);
    }
  }
  
  fclose(fp);
  printf("output all data to %s end\n", fileName);
}

 
