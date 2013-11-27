#include <stdio.h>

int main(){
  int n[10];
    int i;
  for(i=0; i<10; i++){
    n[i] = i;
  }

  int new_n[10];

  
  for(i=0; i<10; i++){
    new_n[i] = n[(i-1 + 10)%10] + n[i] + n[(i+1)%10];
  }
  
  for(i=0; i<10; i++){
    printf("%d ",n[i]);
  }
  printf("\n");
  
  for(i=0; i<10; i++){
    printf("%d ",new_n[i]);
  }
  printf("\n");
}
