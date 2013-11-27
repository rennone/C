#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "drawer.h"
#include "function.h"

typedef struct {
 GLfloat r,g,b;
}colorf;

static const int vertexNum = 4; //頂点数
static const int TEX_NX = 256;
static const int TEX_NY = 256;
static colorf texColor[TEX_NX][TEX_NY]={0};
static GLuint ver_buf, tex_buf;
static GLuint texId;

static double (*colorMode)(double complex);
static void draw();
static void colorTransform(double p, colorf *c);

static GLfloat vertices[] =
  {-1.0f, -1.0f, 0.0f,
   +1.0f, -1.0f, 0.0f, 
   +1.0f, +1.0f, 0.0f, 
   -1.0f, +1.0f, 0.0f};
static GLfloat texCoords[] =
  { 0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f };

//--------------------prototype--------------------//
void drawer_paintImage(int l, int b,int r, int t, int wid ,double complex*,...);
void drawr_paintImage2(int l, int b,int r, int t, int wid,double complex*,double complex*);
//--------------------------------------//


//--------------public Method-----------------//
void (*drawer_getDraw(void))(void)
{
  return draw;
}
//--------------------------------------//

void drawer_init(enum COLOR_MODE cm)
{
  if(cm == CREAL)
    colorMode = creal;
  else
    colorMode = cnorm;//cabs;
  
  glGenBuffers(1, &ver_buf);
  
  glBindBuffer(GL_ARRAY_BUFFER, ver_buf);
  glBufferData(GL_ARRAY_BUFFER, 3*vertexNum*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  
  glGenBuffers(1, &tex_buf);
  glBindBuffer(GL_ARRAY_BUFFER, tex_buf);
  glBufferData(GL_ARRAY_BUFFER, 2*vertexNum*sizeof(GLfloat), texCoords, GL_STATIC_DRAW);
  
  //
  glEnable( GL_TEXTURE_2D );
  glGenTextures( 1, &texId );
  
  glActiveTexture( GL_TEXTURE0 );
  
  glBindTexture( GL_TEXTURE_2D, texId );
  glTexImage2D( GL_TEXTURE_2D, 0, 3, TEX_NX, TEX_NY, 0, GL_RGB, GL_FLOAT, texColor);
  
    //min, maxフィルタ
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );    //min, maxフィルター

}

static void draw()
{  
  glBindBuffer( GL_ARRAY_BUFFER, ver_buf);
  glVertexPointer( 3, GL_FLOAT, 0, 0);

  glBindBuffer( GL_ARRAY_BUFFER, tex_buf);
  glTexCoordPointer(2, GL_FLOAT, 0, 0);
  
  glBindTexture( GL_TEXTURE_2D, texId );

  glTexImage2D( GL_TEXTURE_2D, 0, 3, TEX_NX, TEX_NY, 0, GL_RGB, GL_FLOAT, texColor);  

  glDrawArrays( GL_POLYGON, 0, vertexNum);  
}

//todo 可変長引数を利用して, 複数のデータの平均で色を出すようにするべき?
void drawer_paintImage(int left, int bottom, int right, int top, int height, double complex *phis, ...)
{
  //  const int num = sizeof(phis) / sizeof(double complex*);
  //printf("size = %d",num);
  colorf c;
  double phi;
  for(int x=left, i=0; x<right; x++, i++){
    for(int y=bottom, j=0; y<top ; y++, j++){
      phi = colorMode(phis[x*height+y]);  //colorMode : creal() or cabs()
   
      colorTransform( phi, &c );      
      texColor[i][j] = c;
    }
  }
}

void drawer_paintImage2(int left, int bottom, int right, int top, int height,double complex *phis,double complex *phis2)
{
  colorf c;
  double phi;
  for(int x=left, i=0; x<right; x++, i++){
    for(int y=bottom, j=0; y<top ; y++, j++){
      phi = colorMode(phis[x*height+y] + phis2[x*height+y]);       //colorMode : creal() or cabs()
      colorTransform( phi, &c );      
      texColor[i][j] = c;
    }
  }
}

void drawer_paintTest(void){
  colorf c;
  for(int i=0; i<TEX_NX; i++){
    for(int j=0; j<TEX_NY; j++){
      //      colorTransform((4.0*(i-i%20)/TEX_NX-2.0), &c);
      colorTransform(1.0*i/TEX_NX, &c);
      texColor[i][j] = c;
    }
  }
}

void drawer_finish()
{
  printf("drawer_finish not implemented\n");
}
//--------------------public Method--------------------//




//--------------------Color Trancform---------------------//
static void colorTransform(double phi, colorf *col)
{
  double range = 1.0; //波の振幅
  
  //todo 調査 abs(phi)だと上手く行かない(int型だから).
  double ab_phi = phi < 0 ? -phi : phi;

  //double a = ab_phi < 1 ? (ab_phi <  0.34 ? min(1.0, max(0.0, 3*ab_phi)) : (-1.5*ab_phi+2) ) : 0.5;
  double a = ab_phi < range ? (ab_phi <  range/3.0 ? 3.0/range*ab_phi : (-3.0/4.0/range*ab_phi+1.25) ) : 0.5;
  
  col->r = phi > 0 ? a:0;
  col->b = phi < 0 ? a:0;
  col->g = min(1.0, max(0.0, -3*ab_phi+2));
  /*    
  col->r = min(1.0, max(0.0, phi/range));
  col->g = min(1.0, max(0.0, -ab_phi/range+1));
  col->b = min(1.0, max(0.0, -phi/range));
  */
}




