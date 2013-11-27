#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main(){
  double* hoge[360];

  int i,j;
  for(i=0;i<360;i++)
    hoge[i] = (double*)malloc(sizeof(double)*10);

  for(i=0; i<360;i++)
    for(j=0; j<10;j++)
      hoge[i][j] = i*10 + j;

  for(i=0; i<360;i++)
    for(j=0; j<10;j++)
      printf("%lf \n",hoge[i][j]);

  return 0;
}
