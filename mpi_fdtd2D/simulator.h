#ifndef _SIMULATOR_H
#define _SIMULATOR_H
#include<complex.h>
#include"bool.h"

enum SOLVER{
  TM_UPML_2D,
  TE_UPML_2D,
};

extern void simulator_init(void);
extern void simulator_calc(void);
extern bool simulator_isFinish(void);
extern void simulator_finish(void);
extern double complex* simulator_getDrawingData();
#endif
