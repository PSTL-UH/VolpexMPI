/*
#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
*/
#include "mpi.h"
#include "SL_msg.h"

extern int SL_this_procid;
extern SL_array_t *Volpex_proc_array;
extern NODEPTR head, insertpt, curr;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
extern int next_avail_comm;
extern int request_counter;

extern int redundancy;
extern char fullrank[16];
extern char *hostip;
extern char *hostname;

/*
** sort-function for MPI_Comm_split 
*/
static int rankkeycompare(const void *, const void *);


int Volpex_Comm_size(MPI_Comm comm, int *size)
{
    Volpex_comm *communicator=NULL;
	
    communicator = Volpex_get_comm_byid(comm);
    *size = communicator->size;

    PRINTF(("MPI_Comm_size = %d\n", *size));
    return MPI_SUCCESS;

}

int Volpex_Comm_rank(MPI_Comm comm, int *rank)
{

    Volpex_comm *communicator=NULL;
    communicator = Volpex_get_comm_byid(comm);
    *rank = communicator->myrank;

    PRINTF(("MPI_Comm_rank = %d\n", *rank));
    return MPI_SUCCESS;
}

int  Volpex_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    int Vnextcomm = next_avail_comm;
    Volpex_comm *communicator = NULL, *newcommunicator = NULL;
    
    PRINTF(("[%d] Into Volpex_Comm_dup\n", SL_this_procid));
    
    // Determine the communicator id of the new communicator
    Volpex_Allreduce(&next_avail_comm, &Vnextcomm, 1, MPI_INT, MPI_MAX, comm);
    PRINTF(("[%d] Vnextcomm = %d\n", SL_this_procid, Vnextcomm));
    *newcomm = Vnextcomm;

    // Allocate the basic communicator structure
    communicator = Volpex_get_comm_byid(comm);
    Volpex_init_comm ( *newcomm, communicator->size);
    newcommunicator = Volpex_get_comm_byid(*newcomm);

    // Copy all relevant fields from the original communicator
    // to the new one.
    Volpex_comm_copy(newcommunicator, communicator);	

    /* Set also the hdata structure. This structure will
    ** go away in a long term, but not before the 
    ** EuroPVM/MPI paper 
    */
    hdata[*newcomm].mysize    = hdata[comm].mysize;
    hdata[*newcomm].myrank    = hdata[comm].myrank;
    hdata[*newcomm].mybarrier = 0;
    next_avail_comm = Vnextcomm + 1;
    return MPI_SUCCESS;       
}

int Volpex_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
    int p[3], *procs=NULL;
    int i, j, nextcomm = 0;
    Volpex_comm *newcommunicator = NULL;
    Volpex_comm *oldcommunicator = NULL;
    Volpex_comm *tcomm;
    int localrank=0, commsize = 0;

    tcomm = Volpex_get_comm_byid ( comm );
    
    PRINTF(("[%d]:Into Volpex_Comm_split with color %d and key %d comm:%d\n", SL_this_procid,color, key, tcomm->id));

    
    Volpex_Allreduce(&next_avail_comm, &nextcomm, 1, MPI_INT, MPI_MAX, comm);
    // printf("Past Allreduce and using nextcomm %d\n", nextcomm);
    next_avail_comm = nextcomm + 1;
    *newcomm = nextcomm;
    
   PRINTF(("[%d]:Volpex_Comm_split: Volpex_Allreduce gave next_avail_comm=%d nextcomm=%d\n", 
				SL_this_procid,next_avail_comm, nextcomm));
   //  create an array of process information for doing the split 
    procs = (int *) malloc(3 * hdata[comm].mysize * sizeof(int));
    
    //   gather all process information at all processes 
    p[0] = color;
    p[1] = key;
    p[2] = hdata[comm].myrank;  //  this is rank from original comm 
    
    Volpex_Allgather(p, 3, MPI_INT, procs, 3, MPI_INT, comm);
    qsort ( (int *)procs, hdata[comm].mysize, sizeof(int)*3, 
	    rankkeycompare );
     

    // locate myself in order to determine my rank in this 
    // new communicator and count the no. of processes having my color 
    for ( j=0, i = 0; i < hdata[comm].mysize; i++){
   	if(procs[i*3] == color){
	    commsize++;
		PRINTF(("[%d]:Into Volpex_Comm_split :%d  procs=%d i=%d commsize=%d+++++++++++++\n", 
				SL_this_procid, tcomm->myrank,procs[i*3+2],i,commsize));
	    if (procs[i*3+2] == hdata[comm].myrank ) {
//	    if (procs[i*3+2] == tcomm->myrank ) {
	    
		localrank = j;
	    }
	    j++;
	}
    }
    
    PRINTF(("[%d]:Size of new comm:%d = %d myrank=%d\n", 
	    SL_this_procid, *newcomm,commsize, localrank ));	
    Volpex_init_comm ( *newcomm, commsize );
    newcommunicator = Volpex_get_comm_byid ( *newcomm );
    oldcommunicator = Volpex_get_comm_byid ( comm );

    newcommunicator->myrank = localrank;
    
    for (j=0, i = 0; i < hdata[comm].mysize; i++){
    	if (procs[i*3] == color){
	    Volpex_get_plist_byrank ( procs[3*i+2], oldcommunicator, 
				      &newcommunicator->plist[j]);
	    j++;
	}
    }	

    hdata[*newcomm].mysize = commsize;
    hdata[*newcomm].myrank = localrank; 
    hdata[*newcomm].mybarrier = 0;
    
    PRINTF(("[%d] VComm_split: rank=%d size=%d for comm_id = %d\n", 
	    SL_this_procid, localrank, commsize, *newcomm));

    if (color == MPI_UNDEFINED) {
	//  free all the resources that have been allocated by this process. 
	*newcomm = MPI_COMM_NULL;
    }

//	Volpex_print_allcomm();

    free ( procs);
    return(MPI_SUCCESS);    
}

/* static functions */
/* 
** rankkeygidcompare() compares a tuple of (rank,key,gid) producing 
** sorted lists that match the rules needed for a MPI_Comm_split 
*/
static int rankkeycompare (const void *p, const void *q)
{
    int *a, *b;
    
    /* ranks at [0] key at [1] */
    /* i.e. we cast and just compare the keys and then the original ranks.. */
    a = (int*)p;
    b = (int*)q;
    
    /* simple tests are those where the keys are different */
    if (a[1] < b[1]) {
        return (-1);
    }
    if (a[1] > b[1]) {
        return (1);
    }
    
    /* ok, if the keys are the same then we check the original ranks */
    if (a[1] == b[1]) {
        if (a[0] < b[0]) {
            return (-1);
        }
        if (a[0] == b[0]) {
            return (0);
        }
        if (a[0] > b[0]) {
            return (1);
        }
    }
    return ( 0 );
}
