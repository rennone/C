#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>


void display(void)
{

}

void idle(void)
{

}

void initialize(){

}

const int windowX = 100;
const int windowY = 100;
const int windowWidth = 256;
const int windowHeight = 256;

int main( int argc, char *argv[] )
{
    int rank;
    int size;
    
    initialize();

    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glewInit();
    glutMainLoop();
    
    return 1;
}
