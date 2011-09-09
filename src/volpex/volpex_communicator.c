#include "mpi.h"
#include "SL_msg.h"

SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_comm_array;
extern int SL_this_procid;
extern char *hostip;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];

int Volpex_numcomms = 0;

int Volpex_init_comm(int id, int size)
{
    Volpex_comm *tcomm = NULL;
    int i,pos = 0;
    
    Volpex_numcomms++;
    tcomm = (Volpex_comm*) malloc (sizeof(Volpex_comm));
    if(NULL == tcomm){
	return SL_ERR_NO_MEMORY;
    }
    
    SL_array_get_next_free_pos ( Volpex_comm_array, &pos );
    SL_array_set_element ( Volpex_comm_array, pos, id, tcomm );
    
    tcomm->pos      = pos;
    tcomm->id       = id;
    tcomm->size     = size;
    
    tcomm->plist    = (Volpex_proclist*) malloc(size * sizeof(Volpex_proclist));
    if(NULL == tcomm->plist){
	return SL_ERR_NO_MEMORY;
    }
   for(i=0;i<size;i++)
        tcomm->plist[i].num = 0;
 
    return MPI_SUCCESS;
}

int Volpex_free_comm()
{
	int size, i, j;
	Volpex_comm *tcomm = NULL;

	size = SL_array_get_last(Volpex_comm_array ) + 1;
	for(i=0; i<size; i++){
		tcomm = (Volpex_comm*)SL_array_get_ptr_by_pos ( Volpex_comm_array, i );
		for(j=0;j<tcomm->size;j++){
			free(tcomm->plist[j].ids);
			free(tcomm->plist[j].SL_ids);
			free(tcomm->plist[j].recvpost);
		}
                free(tcomm->plist);
                free(tcomm);
	}
	return MPI_SUCCESS;
}

int Volpex_init_comm_world1 ( int numprocs, int redcy )
{
    Volpex_comm *comm;
    int i, j;
    Volpex_proc *proc;
    int *size;

	size = (int *) malloc (numprocs/redcy*sizeof(int));
    for(i=0;i<numprocs/redcy;i++)
        size[i] = 0;


    Volpex_init_comm(MPI_COMM_WORLD, numprocs/redcy);
    comm = Volpex_get_comm_byid ( MPI_COMM_WORLD );
    for ( i=0; i< comm->size; i++ ) {
	comm->plist[i].num = redcy;
	comm->plist[i].ids = (int *) malloc (redcy * sizeof(int));
	comm->plist[i].SL_ids = (int *) malloc (redcy * sizeof(int));
	comm->plist[i].recvpost = (int *) malloc (redcy * sizeof(int));
	if ( NULL == comm->plist[i].ids || NULL == comm->plist[i].recvpost || NULL == comm->plist[i].SL_ids) {
	    return SL_ERR_NO_MEMORY;
	}
     }
	for ( i=0; i< comm->size; i++ ) {
                for(j=0;j<comm->plist[i].num; j++)
                comm->plist[i].recvpost[j] = 0;
//		comm->plist[i].num = redcy;

        }

	for ( i=0; i< Volpex_numprocs; i++ ) {
	        proc = Volpex_get_proc_byid(i);
        	comm->plist[proc->rank_MCW].ids[size[proc->rank_MCW]]=proc->id ;
	        comm->plist[proc->rank_MCW].SL_ids[size[proc->rank_MCW]]=proc->SL_id ;
                size[proc->rank_MCW]++;

    }



    comm->myrank = Volpex_get_rank();
    hdata[MPI_COMM_WORLD].mysize    = comm->size;
    hdata[MPI_COMM_WORLD].myrank    = comm->myrank;
    hdata[MPI_COMM_WORLD].mybarrier = 0;

   free(size);
    return MPI_SUCCESS;
}



int Volpex_init_comm_world ( int maxrank )
{
    Volpex_comm *comm;
    int i, j;
    Volpex_proc *proc;
    int *size;

        size = (int *) malloc (maxrank*sizeof(int));
    for(i=0;i<maxrank;i++)
        size[i] = 0;


    Volpex_init_comm(MPI_COMM_WORLD, maxrank);
    comm = Volpex_get_comm_byid ( MPI_COMM_WORLD );

//counting number of processes for each rank
//since we can diffrent number of processes of 1 rank

    for ( i=0; i< Volpex_numprocs; i++ ) {
        proc = Volpex_get_proc_byid(i);
        comm->plist[proc->rank_MCW].num++ ;
    }




    for ( i=0; i< comm->size; i++ ) {
        comm->plist[i].ids = (int *) malloc (comm->plist->num * sizeof(int));
        comm->plist[i].SL_ids = (int *) malloc (comm->plist->num * sizeof(int));
        comm->plist[i].recvpost = (int *) malloc (comm->plist->num * sizeof(int));
        if ( NULL == comm->plist[i].ids || NULL == comm->plist[i].recvpost || NULL == comm->plist[i].SL_ids) {
            return SL_ERR_NO_MEMORY;
        }
     }
        for ( i=0; i< comm->size; i++ ) {
                for(j=0;j<comm->plist[i].num; j++)
                comm->plist[i].recvpost[j] = 0;
//              comm->plist[i].num = redcy;

        }

        for ( i=0; i< Volpex_numprocs; i++ ) {
                proc = Volpex_get_proc_byid(i);
                comm->plist[proc->rank_MCW].ids[size[proc->rank_MCW]]=proc->id ;
                comm->plist[proc->rank_MCW].SL_ids[size[proc->rank_MCW]]=proc->SL_id ;
                size[proc->rank_MCW]++;

    }



    comm->myrank = Volpex_get_rank();
    hdata[MPI_COMM_WORLD].mysize    = comm->size;
    hdata[MPI_COMM_WORLD].myrank    = comm->myrank;
    hdata[MPI_COMM_WORLD].mybarrier = 0;

   free(size);
    return MPI_SUCCESS;
}





int Volpex_init_comm_self ( void ) 
{
    Volpex_comm *comm, *commworld;

    Volpex_init_comm ( MPI_COMM_SELF, 1 );
    comm      = Volpex_get_comm_byid ( MPI_COMM_SELF );
    commworld = Volpex_get_comm_byid ( MPI_COMM_WORLD );
    
    Volpex_get_plist_byrank ( commworld->myrank, commworld, &comm->plist[0]);
    comm->myrank = 0;
    
    hdata[MPI_COMM_SELF].mysize = 1;
    hdata[MPI_COMM_SELF].myrank = 0;
    hdata[MPI_COMM_SELF].mybarrier = 0;
    
    return MPI_SUCCESS;
}


int Volpex_comm_copy(Volpex_comm* newcomm, Volpex_comm* oldcomm)
{
    int i, j;

    newcomm->size   = oldcomm->size;
    newcomm->myrank = oldcomm->myrank;
    
    for(i=0;i<newcomm->size;i++){
	newcomm->plist[i].num = oldcomm->plist[i].num;
	newcomm->plist[i].ids = (int *) malloc (newcomm->plist[i].num * 
						sizeof(int));
	newcomm->plist[i].SL_ids = (int *) malloc (newcomm->plist[i].num *
                                                sizeof(int));
	newcomm->plist[i].recvpost = (int *) malloc (newcomm->plist[i].num *
                                                sizeof(int));
	if ( NULL == newcomm->plist[i].ids ) {
	    return SL_ERR_NO_MEMORY;
	}
	
	for(j=0; j<newcomm->plist[i].num;j++) {
	    newcomm->plist[i].ids[j] = oldcomm->plist[i].ids[j];
	    newcomm->plist[i].SL_ids[j] = oldcomm->plist[i].SL_ids[j];
	    newcomm->plist[i].recvpost[j] = 0;
	}
    }

    return MPI_SUCCESS;
}

int Volpex_get_plist_byrank(int rank, Volpex_comm *oldcomm, 
			    Volpex_proclist *plist)
{
    int i;

    plist->num = oldcomm->plist[rank].num;
    plist->ids = (int*) malloc (plist->num * sizeof(int));
    plist->SL_ids = (int*) malloc (plist->num * sizeof(int));
    plist->recvpost = (int*) malloc (plist->num * sizeof(int));
    if ( NULL == plist->ids ) {
	return SL_ERR_NO_MEMORY;
    }
    for(i=0; i<plist->num; i++){
	plist->ids[i] = oldcomm->plist[rank].ids[i];
	plist->SL_ids[i] = oldcomm->plist[rank].SL_ids[i];
	plist->recvpost[i] = 0;
    }

    return MPI_SUCCESS;
}


Volpex_comm* Volpex_get_comm_byid(int id)
{
    Volpex_comm *dcomm = NULL, *comm = NULL;
    int i, size;
    
    size = SL_array_get_last(Volpex_comm_array ) + 1;
    for(i=0; i<size; i++){
	comm = (Volpex_comm*) SL_array_get_ptr_by_pos (Volpex_comm_array, i);
	if ( NULL == comm ) {
	    continue;
	}
	
	if (id == comm->id){
	    dcomm = comm;
	    break;
	}
    }

    return dcomm;
}



void Volpex_print_comm( int commid)
{
    Volpex_comm *comm = NULL;
    int j, k;

    comm = Volpex_get_comm_byid ( commid );
    printf("[%d] commid: %d, size: %d rank: %d\n", SL_this_procid, 
	   comm->id, comm->size, comm->myrank);
    for(j=0;j<comm->size;j++){
        printf("rank=%d plist->  ",j);
        for(k=0;k<comm->plist[j].num;k++){
            printf("%d   ", comm->plist[j].ids[k]);
        }
	printf("SL_ids->  ");
        for(k=0;k<comm->plist[j].num;k++){
            printf("%d   ", comm->plist[j].SL_ids[k]);
        }

	printf("\n");
    }

    printf("\n");

    return;
}

void Volpex_print_allcomm()
{
        int i;

     for (i=1;i<Volpex_numcomms+1;i++)
        Volpex_print_comm(i);
}


int Volpex_init_send(int commid)
{
	Volpex_comm *comm = NULL;
    int j, k;
    int istart,i,ret;

    comm = Volpex_get_comm_byid ( commid );
    istart = Volpex_request_get_counter(comm->size*redundancy);
	Volpex_request_clean(istart,comm->size*redundancy);

    i=istart;
    for(j=0;j<comm->size;j++){
	for(k=0;k<comm->plist[j].num;k++){

		reqlist[i].target = comm->plist[j].ids[k];
	        reqlist[i].cktag = CK_TAG; /*for regular buffer check*/
        	reqlist[i].req_type = 0;  /*0 = send*/
	        reqlist[i].in_use = 1;
        	reqlist[i].flag = 0;
//	        reqlist[i].header = Volpex_get_msg_header(len, hdata[comm].myrank, dest, tag, comm, reuse);
        	reqlist[i].send_status = 0;
	        reqlist[i].reqnumber = i;
		
		
		PRINTF(("[%d] VIsend: Setting Irecv to %d cktag: %d comm: %d for reqnumber %d  \n",
                SL_this_procid, comm->plist[j].ids[k],  CK_TAG, commid, i));

		ret = SL_recv_post(&reqlist[i].returnheader, sizeof(Volpex_msg_header), comm->plist[j].ids[k],
                           CK_TAG, commid, SL_ACCEPT_INFINITE_TIME, &reqlist[i].request);
		if(ret != SL_SUCCESS){
	            printf("[%d] VIsend Error: After SL_recv_post in Volpex_Send, setting "
                    "VOLPEX_PROC_STATE_NOT_CONNECTED for process : %d\n", SL_this_procid,comm->plist[j].ids[k]);
        	    Volpex_set_state_not_connected(comm->plist[j].ids[k]);
        }

		i++;
	}
    }
	Volpex_request_update_counter(comm->size*redundancy);
	return MPI_SUCCESS;
}

void Volpex_set_recvpost(int commid,int procid)
{
        Volpex_comm *comm;
        int i,j;
        comm = Volpex_get_comm_byid(commid);


        for (i=0;i<comm->size;i++){
                for(j=0; j<comm->plist[i].num; j++){
                        if(comm->plist[i].SL_ids[j] == procid){
                                comm->plist[i].recvpost[j] = 1;
                                break;
                        }
                }
        }



}
int Volpex_check_recvpost(int commid,int procid)
{
        Volpex_comm *comm;
        int i,j,ret = 1;
        comm = Volpex_get_comm_byid(commid);

        for (i=0;i<comm->size;i++){
                for(j=0; j<comm->plist[i].num; j++){
                        if(comm->plist[i].SL_ids[j] == procid){
                                ret = comm->plist[i].recvpost[j] ;
                                break;
                        }
                }
        }

        return ret;
}

/*int Volpex_add_proc_tocomm(int rank, int commid)
{
	Volpex_comm *dcomm=NULL;
	dcomm = Volpex_get_comm_byid(commid);
	dcomm->size++;

	
	
}

int Volpex_searchproc_comm(int commid, int procid)
{
	Volpex_comm *comm = NULL;
        Volpex_proclist *plist=NULL;
        int j, k;

	comm = Volpex_get_comm_byid ( commid );

	for(j=0;j<comm->size;j++){
		plist = &comm->plist[j];		
        for(k=0;k<plist->num;k++){
		if (plist->ids[k] == procid)
			return j;			
        }
    	}
	return -1;

}


*/
