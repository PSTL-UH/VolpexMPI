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





int Volpex_dest_src_locator2(int rank, int comm, char *myfullrank, int **target,
                            int *numoftargets, int msglen, int msgtype)
{

    int i, j=0 ;
    int pos = 0 ;
    int *tar=NULL;
    int numtargets=0;
    Volpex_comm *communicator=NULL;
    Volpex_proc *proc=NULL, *dproc = NULL,*tproc=NULL;

    int newtarget;
    Volpex_proclist *tplist=NULL, *plist=NULL;

    communicator = Volpex_get_comm_byid ( comm );
    tplist        = &communicator->plist[rank];

        tproc = Volpex_get_proc_byid (tplist->ids[0]);
        plist = tproc->plist;

        if (msgtype == RECV ){
        proc = Volpex_get_proc_byid (plist->ids[0]);
        if (proc->reuseval > 20 && proc->numofmsg == -1){
                proc->numofmsg = 1;
                for(i=1; i< plist->num; i++){
                        proc = Volpex_get_proc_byid (plist->ids[i]);
                        proc->numofmsg = 0;
                        printf("[%d]:Entering Volpex_dest_src_locator2 for proc: %d msgtype: %d\n", SL_this_procid, proc->id, msgtype);
                }
        }
        }


        for(i=0; i< plist->num; i++){
                proc = Volpex_get_proc_byid (plist->ids[i]);
                if (proc->state == VOLPEX_PROC_CONNECTED){
                    numtargets ++;
                }

        }

//   printf("[%d]:Entering Volpex_dest_src_locator2 for msgtype: %d\n", SL_this_procid, msgtype);
    for(i=0; i< plist->num; i++){
        proc = Volpex_get_proc_byid (plist->ids[i]);
        if(proc->numofmsg > 0 ){
            pos++;
        }
    }

    if(numtargets > 0){
    tar = (int *) malloc ( numtargets * sizeof(int));
    if ( NULL == tar ) {
        return SL_ERR_NO_MEMORY;
    }
    }
        else{
                printf("[%d]NO TARGETS for %d!!!!!!!!!!!!!!!!!!\n",SL_this_procid, rank);
        }


    if (msgtype == RECV  ){
        if (pos == numtargets){

            newtarget = Volpex_compare_loglength(plist);
        //      printf("New target %d\n",newtarget);
                for (i=0; i<plist->num; i++){
                        proc = Volpex_get_proc_byid (plist->ids[i]);
                        proc->numofmsg = -1;
                }
        Volpex_set_newtarget(newtarget, rank, comm);
        }

        dproc = Volpex_get_proc_byid (plist->ids[0]);

// Swapping to next target
        if (dproc->numofmsg > 0 && hdata[comm].myrank != rank  ){

              printf("[%d] changing %d to %d\n",SL_this_procid,plist->ids[0],plist->ids[1]);
        Volpex_change_target(rank,comm);
    }
}
  for(i=0; i< plist->num; i++){
        proc = Volpex_get_proc_byid (plist->ids[i]);
        if ((i==0 && msgtype == RECV && proc->numofmsg != -1)){
            proc->numofmsg++;
        }
        if(VOLPEX_PROC_CONNECTED == proc->state){
            tar[j] = proc->id;
            j++;
        }
    }
    *target = tar;
    *numoftargets = numtargets;

    return MPI_SUCCESS;
}
int Volpex_compare_loglength(Volpex_proclist *plist)
{
        Volpex_proc *proc = NULL, *retproc = NULL;
        int min = 9999;
        int i;
//        MCFA_printf(" Reuse Values for \n");
        for(i=0; i< plist->num; i++){
            proc = Volpex_get_proc_byid (plist->ids[i]);
            PRINTF(("[%d]:ID : %d, val: %d    \n", SL_this_procid,proc->id, proc->reuseval));
            if (proc->reuseval < min){
                min = proc->reuseval;
                retproc = proc;
            }
        }
        printf("[%d]: Target : %d\n",SL_this_procid, retproc->id);
        return retproc->id;
}


