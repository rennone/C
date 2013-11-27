#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

#define TEXSIZE 64

GLfloat bits[TEXSIZE][TEXSIZE][3];
GLuint texName;
int width = 400, height = 300;

void disp( void ) {
  printf("draw\n");
  // カラーバッファとデプスバッファをクリア
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D , texName);
  glTexImage2D(	GL_TEXTURE_2D , 0 , 3 , TEXSIZE , TEXSIZE ,0 , GL_RGB , GL_FLOAT , bits	);

  glBegin(GL_POLYGON);
  glTexCoord2f(0 , 0); glVertex2f(-0.9 , -0.9);
  glTexCoord2f(0 , 1); glVertex2f(-0.9 , 0.9);
  glTexCoord2f(1 , 1); glVertex2f(0.9 , 0.9);
  glTexCoord2f(1 , 0); glVertex2f(0.9 , -0.9);
  glEnd();
  
  glFlush();
}

void timer(int value) {
  glRotatef(1 , 0.5 , 1 , 0.25);
  glutPostRedisplay();
  glutTimerFunc(50 , timer , 0);  
}

int main(int argc , char ** argv) {
  unsigned int i , j;

  for (i = 0 ; i < TEXSIZE ; i++) {
    float r = 1.0*i / TEXSIZE;
    for (j = 0 ; j < TEXSIZE ; j++) {
      bits[i][j][0] = r;
      bits[i][j][1] = 1.0*j/TEXSIZE;
      bits[i][j][2] = 1-r;
    }
  }

  glutInit(&argc , argv);
  glutInitWindowSize(400 , 300);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);

  glutCreateWindow("Kitty on your lap");
  glutDisplayFunc(disp);
  glutTimerFunc(100 , timer , 0);

  if( glewInit() != GLEW_OK){
    printf("error\n");
  }
  printf("%s \n",glewGetString(GLEW_VERSION) );
  
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1 , &texName);
  glBindTexture(GL_TEXTURE_2D , texName);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(	GL_TEXTURE_2D , 0 , 3 , TEXSIZE , TEXSIZE ,0 , GL_RGB , GL_FLOAT, bits	);
  
  glutMainLoop();
  return 0;
}





/*
static int width, height;   // ウィンドウの幅と高さ
 #define FBOWIDTH  512       // フレームバッファオブジェクトの幅
#define FBOHEIGHT 512       // フレームバッファオブジェクトの高さ
 
static GLuint fb;           // フレームバッファオブジェクト
static GLuint cb;           // カラーバッファ用のテクスチャ
static GLuint rb;           // デプスバッファ用のレンダーバッファ

static void init(void)
{
  // GLEW の初期化
  GLenum err = glewInit();

  
  // カラーバッファ用のテクスチャを用意する
  glGenTextures(1, &cb);
  glBindTexture(GL_TEXTURE_2D, cb);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBOWIDTH, FBOHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // デプスバッファ用のレンダーバッファを用意する
  glGenRenderbuffersEXT(1, &rb);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, FBOWIDTH, FBOHEIGHT);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

  // フレームバッファオブジェクトを作成する
  glGenFramebuffersEXT(1, &fb);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
  
  // フレームバッファオブジェクトにカラーバッファとしてテクスチャを結合する
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, cb, 0);
  
  // フレームバッファオブジェクトにデプスバッファとしてレンダーバッファを結合する
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rb);
  
  // フレームバッファオブジェクトの結合を解除する
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  
  // 背景色
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  
  // 光源
  glEnable(GL_LIGHT0);
  
}

static void display(void)
{
    // 透視変換行列の設定
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, (GLdouble)width / (GLdouble)height, 1.0, 10.0);

  // モデルビュー変換行列の設定
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(3.0, 4.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  
  // ビューポートの設定
  glViewport(0, 0, FBOWIDTH, FBOHEIGHT);

  // 隠面消去を有効にする
  glEnable(GL_DEPTH_TEST);

  // 陰影付けを有効にする
  glEnable(GL_LIGHTING);

  // フレームバッファオブジェクトを結合する
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

  // カラーバッファとデプスバッファをクリア
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // シーンの描画
  glutSolidTeapot(1.0);
  glFlush();

  // フレームバッファオブジェクトの結合を解除する
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  // 透視変換行列を単位行列にする
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // モデルビュー変換行列を単位行列にする
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // ビューポートはウィンドウのサイズに合わせる
  glViewport(0, 0, width, height);

  // 陰影付けと隠面消去処理は行わない
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // テクスチャマッピングを有効にする
  glBindTexture(GL_TEXTURE_2D, cb);
  glEnable(GL_TEXTURE_2D);

  // 正方形を描く
  glColor3d(1.0, 1.0, 1.0);
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2d(0.0, 0.0);
  glVertex2d(-1.0, -1.0);
  glTexCoord2d(1.0, 0.0);
  glVertex2d( 1.0, -1.0);
  glTexCoord2d(1.0, 1.0);
  glVertex2d( 1.0,  1.0);
  glTexCoord2d(0.0, 1.0);
  glVertex2d(-1.0,  1.0);
  glEnd();

  // テクスチャマッピングを無効にする
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFlush();
}
 
static void resize(int w, int h)
{
  // ウィンドウのサイズを保存する
  width = w;
  height = h;
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
  glutCreateWindow("FBO Sample");
  glutDisplayFunc(display);
  glutReshapeFunc(resize);

  init();
  glutMainLoop();
  return 0;
}
*/
