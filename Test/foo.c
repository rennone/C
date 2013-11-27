#include"foo.h"
#include"aaa.h"
#include<stdio.h>

static void printer(){
  printf("foo %d \n", variable);
}

void (* get(void))(void){
  return printer;
}
