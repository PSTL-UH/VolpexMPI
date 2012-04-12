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
#include "SL_proc.h"
#include "SL_msg.h"

SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_comm_array;
SL_array_t *Volpex_rank_array;
extern int SL_this_procid;
extern char *hostip;
extern char *hostname;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
extern Volpex_target_list *Volpex_targets;
extern Volpex_dest_source_fnct *Volpex_dest_source_select;
int Volpex_set_target()
{
//      #define MAX_LEN (1024L * 64L)
        #define MAX_LEN (1024L * 1024L )

        int numprocs,i,j;
        char *buf;
        double stime, etime;
        numprocs = Volpex_numprocs/redundancy;
        buf = malloc(MAX_LEN);
        double *timeval;

        timeval = (double*) malloc (Volpex_numprocs * (sizeof(double)));
        Volpex_proc *proc;
        int k, newtarget=0;
        float tottime, min;



    int step;
    int size = Volpex_numprocs;
    int rank = SL_this_procid;


int sendrecv;
//      printf("\n  [%d]:",SL_this_procid);
        for ( step=0; step< size; step++ ) {
                sendrecv = (size-rank+step)%size;
        tottime =0.0;
//      printf ("  %d  ",sendrecv);
//printf("\n\n[%d]---------------PROCESS: %d----------------------\n", SL_this_procid,sendrecv);
        for(k=0;k<10;k++){
                stime = SL_papi_time();
                if (SL_this_procid == sendrecv)
                        continue;
                if (SL_this_procid<sendrecv){
                        SL_Send (buf, MAX_LEN, sendrecv, 0, 0 );
                        SL_Recv (buf, MAX_LEN, sendrecv, 0, 0, SL_STATUS_IGNORE);
                }
                else{
                        SL_Recv (buf, MAX_LEN, sendrecv, 0, 0, SL_STATUS_IGNORE);
                        SL_Send (buf, MAX_LEN, sendrecv, 0, 0 );
                }
                etime = SL_papi_time();
//            printf("[%d]:Iteration:%d = %f \n", SL_this_procid,k,etime-stime);
                if (k>1)
                        tottime += etime-stime;

      }
//        printf("[%d]:Recv time for proc:%d = %f \n", SL_this_procid, sendrecv,tottime);

        timeval[sendrecv] = tottime;
    }

 for(i=0;i<numprocs;i++){
        min = 9999999.0;
        proc = Volpex_get_proc_byid(i);
        for (j=0; j<proc->plist->num;j++){
                if(timeval[proc->plist->ids[j]]<min){
                        min = timeval[proc->plist->ids[j]];
                        newtarget = proc->plist->ids[j];
                }
        }
                printf("[%d]: Newtarget for rank %d=%d\n", SL_this_procid, i, newtarget);
                Volpex_set_primarytarget(proc,newtarget);

        }

        Volpex_dest_source_select = Volpex_dest_src_locator;
        return MPI_SUCCESS;
}

int Volpex_set_primarytarget(Volpex_proc *proc, int newtarget)
{
    Volpex_proc *tproc;
    int i,j,id;
    id = proc->plist->ids[0];
    for (i=1; i<proc->plist->num; i++)
    {
        if (proc->plist->ids[i] == newtarget)
                proc->plist->ids[i] = id ;
    }
    proc->plist->ids[0] = newtarget;

    for(i=0;i<proc->plist->num;i++){
        tproc = Volpex_get_proc_byid(proc->plist->ids[i]);
        for(j=0; j<proc->plist->num;j++){
                tproc->plist->ids[j] = proc->plist->ids[j];
        }
    }
/*
        for(i=0;i<Volpex_numprocs;i++){
                proc = Volpex_get_proc_byid(i);
                printf("[%d]:%d-> ",SL_this_procid,proc->id);
                for (j=0; j<proc->plist->num;j++)
                        printf("%d ", proc->plist->ids[j]);

                printf("\n");
        }
*/
        return MPI_SUCCESS;
}

