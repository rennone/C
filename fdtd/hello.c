/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <mpi.h>
#include "simulator.h"

void display(void)
{
    printf("draw");
    draw();
}

void idle(void)
{
    printf("calc");
    calc();
}

void initialize(){
    setSolver(TE_2D);
    init(10,10);
}

int main( int argc, char *argv[] )
{
    int rank;
    int size;

    //    printf("before init");
    initialize();
    //    printf("after init");
    
    
    MPI_Init( 0, 0 );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf( "Hello world from process %d of %d\n", rank, size );
    MPI_Finalize();    
    
    //    printf("1");
    glutInit(&argc, argv);
    //printf("2");
    glutCreateWindow(argv[0]);
    //printf("3");
    glutDisplayFunc(display);
    //printf("4");
    glutIdleFunc(idle);
    //printf("5");
    glutMainLoop();
    
    return 1;
}
