#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#include "mpi.h"
#include "SL_proc.h"


SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_comm_array;
extern int SL_this_procid;
extern char *hostip;
extern char *hostname;
extern int Volpex_numprocs;
extern int redundancy;
extern int Volpex_numcomms;



int Volpex_dest_src_locator3(int rank, int comm, char *myfullrank, int **target,
                            int *numoftargets, int msglen, int msgtype)
{
    int i, j=0 ;
    int *tar=NULL;
    int numtargets=0;
    Volpex_comm *communicator=NULL;
    Volpex_proc *proc=NULL;

    Volpex_proclist *plist=NULL;

    communicator = Volpex_get_comm_byid ( comm );
    plist        = &communicator->plist[rank];


//    printf("[%d]: com  %d plist->     ", SL_this_procid,comm);
    for(i=0; i< plist->num; i++){
        proc = Volpex_get_proc_byid (plist->ids[i]);
//      printf(" %d , ", plist->ids[i]);
        if (proc->state == VOLPEX_PROC_CONNECTED){
            numtargets ++;
        }
    }
//      printf("\n");

    tar = (int *) malloc ( numtargets * sizeof(int));
    if ( NULL == tar ) {
        return SL_ERR_NO_MEMORY;
    }
    for(i=0; i< plist->num; i++){
        proc = Volpex_get_proc_byid (plist->ids[i]);

        if(VOLPEX_PROC_CONNECTED == proc->state){
            tar[j] = proc->id;
            j++;
        }
    }


//      MCFA_printf("[%d]: rank %d is %d\n", SL_this_procid, rank, tar[0]);
    *target = tar;
    *numoftargets = numtargets;

    return MPI_SUCCESS;
}
int Volpex_count_numoftargets(int rank, int comm, char mylevel, int *ownteam)
{
    Volpex_comm *communicator=NULL;
    Volpex_proc *proc=NULL;
    Volpex_proclist *plist=NULL;
    int numoftargets=0;
    int i, numeric;
    char level;

    *ownteam = 0;
    communicator = Volpex_get_comm_byid ( comm );
    plist        = &communicator->plist[rank];

    for ( i=0; i< plist->num; i++ ) {
        proc = Volpex_get_proc_byid (plist->ids[i]);
        if( VOLPEX_PROC_CONNECTED == proc->state){
            sscanf(proc->rank, "%d,%c", &numeric, &level);
            if ( level == mylevel ) {
                *ownteam=1;
            }
            numoftargets++;
        }
    }

    return numoftargets;
}

