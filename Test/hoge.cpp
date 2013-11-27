#include<stdio.h>

class Hoge
{
public:
  Hoge()
  {
    static int a=0;
    a++;
    printf("con %d \n",a);
  };

  ~Hoge()
  {
    static int a=0;
    a++;
    printf("des %d \n",a);
  }

};


int main(int argc, char *argv[])
{
  Hoge *h1 = new Hoge();
  Hoge *h2 = new Hoge();
  delete h1;
  delete h2;
  return 0;
}
