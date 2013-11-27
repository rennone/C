#ifndef _SIMULATOR_H
#define _SIMULATOR_H

enum SOLVER{
  TE_2D,
  TM_2D,
  TM_UPML_2D,
  TE_UPML_2D,
  FDTD_3D
};

extern void init(void);
extern void calc(void);
extern void draw(void);
extern void finish(void);
#endif
