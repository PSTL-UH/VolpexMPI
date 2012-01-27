#include "mpi.h"
#include "SL_msg.h"

SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_comm_array;
extern int SL_this_procid;
extern char *hostip;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];

extern int Volpex_numcomms;




int Volpex_insert_comm_newproc(int rank, int procid)
{
    int i,j,k;
    Volpex_comm *tcomm,*comm;
    Volpex_proclist *plist;
    int newrank = -1;

    plist = (Volpex_proclist*) malloc (sizeof(Volpex_proclist));

    comm = Volpex_get_comm_byid(MPI_COMM_WORLD);

//Creating a new plist with increased number of procs
//inserting new proc in the plist of each communicator
//inserting new proc int the plist of each process
// inserting plist for new proc
// should add a Isend for this new proc in each existing communicator


/*****Creating a new plist*******/
    plist->num = comm->plist[rank].num+1;
    plist->ids = (int*) malloc (plist->num * sizeof(int));
    plist->SL_ids = (int*) malloc (plist->num * sizeof(int));
    plist->recvpost = (int*) malloc (plist->num * sizeof(int));

    if ( NULL == plist->ids ) {
        return SL_ERR_NO_MEMORY;
    }
    for(i=0; i<plist->num-1; i++){
        plist->ids[i] = comm->plist[rank].ids[i];
        plist->SL_ids[i] = comm->plist[rank].SL_ids[i];
	plist->recvpost[i] = comm->plist[rank].recvpost[i];
    }
        plist->ids[i] = procid;
        plist->SL_ids[i] = procid;
	plist->recvpost[i] = 0;

/**inserting new proc in the plist of each communicator*********/
    for (i=1; i<Volpex_numcomms+1; i++){
        tcomm = Volpex_get_comm_byid(i);
        newrank = -1;
        for(j=0;j<tcomm->size;j++){
                if(tcomm->plist[j].ids[0] == plist->ids[0]){
                        newrank = j;
                        tcomm->plist[j].num = plist->num;
                        if (tcomm->plist !=NULL){
                                free(tcomm->plist[j].ids);
                                free(tcomm->plist[j].SL_ids);
                                free(tcomm->plist[j].recvpost);
				
			}

                        tcomm->plist[j].ids = (int *) malloc (plist->num * sizeof(int));
                        tcomm->plist[j].SL_ids = (int *) malloc (plist->num * sizeof(int));
                        tcomm->plist[j].recvpost = (int *) malloc (plist->num * sizeof(int));
                        for(k=0; k<plist->num; k++){
                                tcomm->plist[j].ids[k] = plist->ids[k] ;
                                tcomm->plist[j].SL_ids[k] = plist->SL_ids[k] ;
                                tcomm->plist[j].recvpost[k] = plist->recvpost[k] ;
                        }

                }
        }
        if (newrank != -1)
                Volpex_init_send_newproc(tcomm,procid,j);

      }




/***inserting new proc int the plist of each process****/
Volpex_proc *proc;
    Volpex_proclist *tplist;

    tplist = (Volpex_proclist*) malloc (sizeof(Volpex_proclist));
    proc = Volpex_get_proc_byid(plist->ids[0]);
    tplist->num = proc->plist->num+1;
    tplist->ids = (int*) malloc (tplist->num * sizeof(int));
    tplist->SL_ids = (int*) malloc (tplist->num * sizeof(int));
    tplist->recvpost = (int*) malloc (tplist->num * sizeof(int));
    if ( NULL == tplist->ids ) {
            return SL_ERR_NO_MEMORY;
    }
    for(j=0; j<tplist->num-1; j++){
        tplist->ids[j] = proc->plist->ids[j];
	tplist->SL_ids[i] = proc->plist->SL_ids[i];
        tplist->recvpost[i] = proc->plist->recvpost[i];
    }
    tplist->ids[j] = procid;
    tplist->SL_ids[j] = procid;
    tplist->recvpost[j] = 0;
    


    for(i=0; i<plist->num; i++){
        proc = Volpex_get_proc_byid(plist->ids[i]);
        proc->plist->num = tplist->num;
        if (proc->plist!= NULL){
                free(proc->plist->ids);
                free(proc->plist->SL_ids);
                free(proc->plist->recvpost);
	}
        proc->plist->ids = (int *) malloc (tplist->num * sizeof(int));
        proc->plist->SL_ids = (int *) malloc (tplist->num * sizeof(int));
        proc->plist->recvpost = (int *) malloc (tplist->num * sizeof(int));
//      printf("\n[%d]:  ", SL_this_procid);
        for(k=0; k<tplist->num; k++){
                proc->plist->ids[k] = tplist->ids[k] ;
                proc->plist->SL_ids[k] = tplist->SL_ids[k] ;
                proc->plist->recvpost[k] = tplist->recvpost[k] ;
		
//              printf("%d  ", tplist->ids[k]);
        }

    }

	if (plist!= NULL){
		free(plist->ids);
		free(plist->SL_ids);
		free(plist->recvpost);
	}

	if (tplist!= NULL){
                free(tplist->ids);
                free(tplist->SL_ids);
                free(tplist->recvpost);
        }


        return MPI_SUCCESS;
}



int Volpex_init_send_newproc(Volpex_comm *tcomm,int procid,int rank)
{
        int istart;
        istart  = Volpex_request_get_counter ( 1 );
        Volpex_request_clean ( istart, 1 );


        reqlist[istart].target = procid;
        reqlist[istart].cktag = CK_TAG; /*for regular buffer check*/
        reqlist[istart].req_type = 0;  /*0 = send*/
        reqlist[istart].in_use = 1;
        reqlist[istart].flag = 0;
        reqlist[istart].send_status = 0;
        reqlist[istart].reqnumber = istart;
        reqlist[istart].header = Volpex_get_msg_header(0, rank,procid, CK_TAG, tcomm->id, 0);

        PRINTF(("[%d] VIinit_send_newproc: Setting Irecv to target->%d cktag: %d comm: %d for reqnumber %d  \n",
                SL_this_procid, reqlist[istart].target,  CK_TAG, tcomm->id, istart));


        SL_recv_post(&reqlist[istart].returnheader, sizeof(Volpex_msg_header), procid,
                           CK_TAG, tcomm->id,SL_ACCEPT_MAX_TIME, &reqlist[istart].request);
        return MPI_SUCCESS;
}

