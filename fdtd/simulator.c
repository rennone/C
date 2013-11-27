#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"
#include "field.h"
#include "fdtdTE.h"
#include "fdtdTE_upml.h"
#include "fdtdTM.h"
#include "fdtdTM_upml.h"
#include "drawer.h"
#include "models.h"

static void (*update)() = NULL;
static void (*present)() = NULL;
static double complex* (*getData)() = NULL;
static double complex* (*getData2)() = NULL;
static void (* finishMethod)() = NULL;
static void (* initMethod)() = NULL;

void setTE(){
  update = fdtdTE_getUpdate();
  initMethod = fdtdTE_getInit();
  finishMethod = fdtdTE_getFinish();
  getData = fdtdTE_getHzx;
  getData2 = fdtdTE_getHzy;
}

void setTM(){
  update = fdtdTM_getUpdate();
  initMethod = fdtdTM_getInit();
  finishMethod = fdtdTM_getFinish();
  getData = fdtdTM_getEzx;
  getData2 = fdtdTM_getEzy;
}

void setTMupml(){
  update = fdtdTM_upml_getUpdate();
  initMethod = fdtdTM_upml_getInit();
  finishMethod = fdtdTM_upml_getFinish();
  getData = fdtdTM_upml_getEz;
  getData2 = fdtdTM_upml_getEz;
}

void setTEupml(){
  update = fdtdTE_upml_getUpdate();
  initMethod = fdtdTE_upml_getInit();
  finishMethod = fdtdTE_upml_getFinish();
  getData = fdtdTE_upml_getHz;
  getData2 = fdtdTE_upml_getHz;
}

void set3D(){
  printf("not implemented \n");
}

void setSolver(enum SOLVER solver)
{
  switch(solver){
  case TE_2D :
    setTE();
    break;
  case TM_2D :
    setTM();
    break;
  case FDTD_3D :
    set3D();
    break;
  case TM_UPML_2D:
    setTMupml();
    break;
  case TE_UPML_2D:
    setTEupml();
    break;
  default :
    break;
  }
  present = drawer_getDraw();
}

void calc(){
  (*update)();
  drawer_paintImage2( N_PML, N_PML, N_X+N_PML, N_Y+N_PML, N_PY, (*getData)(),(*getData2)());   //テクスチャの更新
    //時間を一つ進める
  
  if(field_nextStep() == 1){
    finish();
  }  
}

void draw(){
  (*present)();
}

void init(){
  //横幅(nm), 縦幅(nm), 1セルのサイズ(nm), pmlレイヤの数, 波長(nm), 計算ステップ
  setField(256, 256, 1, 10, 60, 1000); //必ず最初にこれ
  
  /*NO_MODEL. MIE_CYLINDER, SHELF, NONSHELF*/
  setModel(MIE_CYLINDER);  //次にこれ,  モデル(散乱体)を定義

  field_setEPS();  //モデルを定義した後, fieldのEPSを計算(散乱波の計算に必要)

  setSolver(TE_UPML_2D);//Solverの設定
  
  (*initMethod)(); //Solverの初期化, EPS, Coeffの設定

   /*POINT_LIGHT_IN_CENTER: 中心に点光源   SCATTER: 散乱波*/
  field_setDefaultIncidence(SCATTER); //入射波の設定

  /* CREAL : 実部を使って色変換, CABS : 振幅をつかって色変換 */
  drawer_init(CREAL); //drawerの初期化
}

void finish(){
  printf("finish\n");
   (*finishMethod)(); //メモリの解放等
    exit(0);
}


