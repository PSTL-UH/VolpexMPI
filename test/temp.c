#include <stdio.h>
#include "mpi.h"


int main ( int argc, char * argv[] )
{
    int mynode, numnode, root;
    int    iarr[2], isum[2], imax[2], iprod[2], imin[2];
    float  farr[2], fsum[2], fmax[2], fprod[2], fmin[2];
    double darr[2], dsum[2], dmax[2], dprod[2], dmin[2];
    int iland[2], ilor[2], ilxor[2];
    int iband[2], ibor[2], ibxor[2];
    
    MPI_Init ( &argc, &argv );
    MPI_Comm_size ( MPI_COMM_WORLD, &numnode );
    MPI_Comm_rank ( MPI_COMM_WORLD, &mynode );
        
    root = 0;
    if ( mynode == 0){
        iarr[0] = 1;
        iarr[1] = 47;
    }  
    
    if ( mynode == 1){
        iarr[0] = 2;
        iarr[1] = 1454;
    }  
    
    if ( mynode == 2) {
        iarr[0] = 3;
        iarr[1] = 123485;
    }  
    
    if ( mynode == 3) {
        iarr[0] = 4;
        iarr[1] = 0;
    }  

    /* Testing MPI_MAX */
    MPI_Allreduce ( iarr, imax, 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD ); 
    
    printf(">>>>>>>>>> imax[0]=%d imax[1]=%d\n", imax[0], imax[1]);  
    printf("Should be: imax[0]=4 imax[1]=123485\n");
    
     
    
    
    
    MPI_Finalize ();
    
    return 0;
}


