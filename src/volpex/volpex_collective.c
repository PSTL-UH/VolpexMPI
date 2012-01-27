#include "mpi.h"
#include "SL_msg.h"

extern int SL_this_procid;
extern SL_array_t *SL_proc_array;
extern NODEPTR head, insertpt, curr;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char *hostip;
extern char *hostname;

extern int next_avail_comm;
extern int request_counter;
extern int Volpex_numcomms;
extern Volpex_target_list *Volpex_targets;
int  Volpex_Finalize()
{
    PRINTF(("[%d] Into Volpex_Finalize\n", SL_this_procid));

    Volpex_Barrier ( MPI_COMM_WORLD);

    PRINTF(("[%d] Into VolPEx_Complete_Barrier\n", SL_this_procid));

//    Volpex_Redundancy_Barrier ( MPI_COMM_WORLD, hdata[MPI_COMM_WORLD].myrank ) ;
 Volpex_Complete_Barrier ( MPI_COMM_WORLD);

    PRINTF(("[%d]: Into MCFA_Finalze\n", SL_this_procid));
    MCFA_Finalize();
    Volpex_send_buffer_delete();
#ifdef MINGW
    WSACleanup();
#endif

//    Volpex_print_alltarget_info();
//    Volpex_print_targetlist();
    Volpex_free_targetlist();
    Volpex_free_proclist();
    Volpex_free_comm();
    GM_free_global_data ();



    return MPI_SUCCESS;
}

int  Volpex_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    MPI_Status *status = (MPI_Status *)MPI_STATUS_IGNORE;
    int i;
    
    PRINTF(("[%d] Into Volpex_Bcast and myrank is %d for comm %d and size %d\n", 
	    SL_this_procid, hdata[comm].myrank, comm, hdata[comm].mysize));
    
    for(i = 1; i < hdata[comm].mysize; i++){
	if(hdata[comm].myrank == root){
	    Volpex_Send(buf, count, datatype, i, BCAST_TAG, comm);
	    Volpex_progress();
	}
	if(hdata[comm].myrank == i){
	    Volpex_Recv(buf, count, datatype, root, BCAST_TAG, comm, status);
	    Volpex_progress();
	}
    }
    return MPI_SUCCESS;
}


void Volpex_reduce_ll(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, 
		      MPI_Op op, int root, MPI_Comm comm, int tag)
{
    MPI_Status *status = (MPI_Status *)MPI_STATUS_IGNORE;
    int j=0, k=0;
    int *tempIntArray1, *tempIntArray2;
    float *tempFloatArray1, *tempFloatArray2;
    double *tempDoubleArray1, *tempDoubleArray2;
    double _Complex *tempDoubleComplexArray1, *tempDoubleComplexArray2;
    
    PRINTF(("[%d] Into Volpex_reduce_ll\n", SL_this_procid));
    if(hdata[comm].myrank != root) {
	Volpex_Send(sendbuf, count, datatype, root, tag, comm);
    }

    if(hdata[comm].myrank == root){
        if(datatype == MPI_INT || datatype == MPI_INTEGER){
            tempIntArray2 = (int *)malloc(count * sizeof(int));
            tempIntArray1 = (int *)malloc(count * sizeof(int));
	    memcpy(tempIntArray2, sendbuf, count * sizeof(int));
	    
	    for(j = 0; j < hdata[comm].mysize; j++){
		if ( hdata[comm].myrank == j ) {
		    continue;
		}
		Volpex_Recv(tempIntArray1, count, datatype, j, tag, comm, status);
                for(k = 0; k < count; k++){
                    if(op == MPI_MAX){
			PRINTF(("[%d]:Comparing tempIntArray1[%d]=%d to tempIntArray2[%d]=%d\n", 
				SL_this_procid,k, tempIntArray1[k], k, tempIntArray2[k]));
			if(tempIntArray1[k] > tempIntArray2[k])
                            tempIntArray2[k] = tempIntArray1[k];
                    }
                    if(op == MPI_MIN){
                        if(tempIntArray1[k] < tempIntArray2[k])
                            tempIntArray2[k] = tempIntArray1[k];
                    }
                    if(op == MPI_SUM){
                        tempIntArray2[k] = tempIntArray2[k] + tempIntArray1[k];
                    }
                    if(op == MPI_PROD){
                        tempIntArray2[k] = tempIntArray2[k] * tempIntArray1[k];
                    }
                }
            }
	    memcpy(recvbuf, tempIntArray2, count * sizeof(int));
	    free(tempIntArray2);
            free(tempIntArray1);
        }

        if(datatype == MPI_FLOAT || datatype == MPI_REAL){
            tempFloatArray2 = (float *)malloc(count * sizeof(float));
            tempFloatArray1 = (float *)malloc(count * sizeof(float));
	    memcpy(tempFloatArray2, sendbuf, count * sizeof(float));

            for(j = 0; j < hdata[comm].mysize; j++){
		if ( hdata[comm].myrank == j ) {
		    continue;
		}
                Volpex_Recv(tempFloatArray1, count, datatype, j, tag, comm, status);
                for(k = 0; k < count; k++){
                    if(op == MPI_MAX){
                        if(tempFloatArray1[k] > tempFloatArray2[k])
                            tempFloatArray2[k] = tempFloatArray1[k];
                    }
                    if(op == MPI_MIN){
                        if(tempFloatArray1[k] < tempFloatArray2[k])
                            tempFloatArray2[k] = tempFloatArray1[k];
                    }
                    if(op == MPI_SUM){
                        tempFloatArray2[k] = tempFloatArray2[k] + tempFloatArray1[k];
                    }
                    if(op == MPI_PROD){
                        tempFloatArray2[k] = tempFloatArray2[k] * tempFloatArray1[k];
                    }
                }
            }
	    memcpy(recvbuf, tempFloatArray2, count * sizeof(float));
        }
	if(datatype == MPI_DOUBLE || datatype == MPI_DOUBLE_PRECISION){
	    tempDoubleArray2 = (double *)malloc(count * sizeof(double));
	    tempDoubleArray1 = (double *)malloc(count * sizeof(double));
	    memcpy(tempDoubleArray2, sendbuf, count * sizeof(double));

	    for(j = 0; j < hdata[comm].mysize; j++){
		if ( hdata[comm].myrank == j ) {
		    continue;
		}
		Volpex_Recv(tempDoubleArray1, count, datatype, j, tag, comm, status);
		for(k = 0; k < count; k++){
		    if(op == MPI_MAX){
			if(tempDoubleArray1[k] > tempDoubleArray2[k])
			    tempDoubleArray2[k] = tempDoubleArray1[k];
		    }
		    if(op == MPI_MIN){
			if(tempDoubleArray1[k] < tempDoubleArray2[k])
			    tempDoubleArray2[k] = tempDoubleArray1[k];
		    }
		    if(op == MPI_SUM){
			tempDoubleArray2[k] = tempDoubleArray2[k] + tempDoubleArray1[k];
		    }
		    if(op == MPI_PROD){
			tempDoubleArray2[k] = tempDoubleArray2[k] * tempDoubleArray1[k];
		    }
		}
	    }
	    memcpy(recvbuf, tempDoubleArray2, count * sizeof(double));
	}
	if(datatype == MPI_DOUBLE_COMPLEX){
	    tempDoubleComplexArray2 = (double _Complex *)malloc(count * sizeof(double _Complex));
	    tempDoubleComplexArray1 = (double _Complex *)malloc(count * sizeof(double _Complex));
	    memcpy(tempDoubleComplexArray2 , sendbuf, count * sizeof(double _Complex));

            for(j = 0; j < hdata[comm].mysize; j++){
		if ( hdata[comm].myrank == j ) {
		    continue;
		}
                Volpex_Recv(tempDoubleComplexArray1, count, datatype, j, tag, comm, status);
                for(k = 0; k < count; k++){
                    if(op == MPI_SUM){
                        tempDoubleComplexArray2[k] = tempDoubleComplexArray2[k]+tempDoubleComplexArray1[k];
                    }
                }
            }
	    memcpy(recvbuf, tempDoubleComplexArray2, count * sizeof(double _Complex));
	}
    }
}

int Volpex_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, 
		  int root, MPI_Comm comm)
{
    PRINTF(("[%d] Into Volpex_Reduce\n", SL_this_procid));
    Volpex_reduce_ll(sendbuf, recvbuf, count, datatype, op, root, comm, REDUCE_TAG);
    return MPI_SUCCESS;
}

int Volpex_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, 
		     MPI_Comm comm)
{
    int root = 0;
    
    PRINTF(("[%d] Into VolpexE_Allreduce\n", SL_this_procid));
    Volpex_reduce_ll(sendbuf, recvbuf, count, datatype, op, root, comm, REDUCE_TAG);
    Volpex_Bcast(recvbuf, count, datatype, root, comm);
    return MPI_SUCCESS;
}

int Volpex_Barrier(MPI_Comm comm)
{
    MPI_Status status;
    int i = 1;
    int j;
    int root = 0;
    int err;
    int *buffer=NULL;
    MPI_Request *myrequest;

    myrequest = (MPI_Request*) malloc (sizeof (MPI_Request) * hdata[comm].mysize);
    
    PRINTF(("[%d] Into Volpex_Barrier with mybarrier = %d\n", SL_this_procid, hdata[comm].mybarrier));
    Volpex_progress();
    buffer = (int *) calloc ( 1, sizeof(int) * hdata[comm].mysize);
    if  ( NULL == buffer ) {
	return MPI_ERR_OTHER;
    }
    
    if(hdata[comm].myrank != root){
	Volpex_Send(&i, 1, MPI_INT, root, BARRIER_TAG, comm);
//	printf("[%d]: Send msg to root :%d\n",SL_this_procid,root);
    }
    else{
	for (i=1; i<hdata[comm].mysize ; i++){
	    Volpex_Recv(&buffer[i], 1, MPI_INT, i, BARRIER_TAG, comm, &status);
//	    printf("[%d]: Recv msg from proc :%d\n",SL_this_procid,i);
	}
    }
    Volpex_progress();

    if(hdata[comm].myrank == root){
	//j = hdata[comm].mybarrier;
	for (i=1; i<hdata[comm].mysize ; i++){
	    Volpex_Send(&hdata[comm].mybarrier, 1, MPI_INT, i, BARRIER_TAG, comm);
	    Volpex_progress();
	}
    }
    else{
	err = Volpex_Recv(&j, 1, MPI_INT, root, BARRIER_TAG, comm, &status);
	Volpex_progress();
	if ( err != MPI_SUCCESS ) {
		printf("BIG PROBLEM!!!!!!!!!!!!!!!!!\n\n");
	    /* To be determined */
	}	    
	if ( j != hdata[comm].mybarrier ) {
	    printf("[%d]:VBarrierL Mismatch of mybarrier: %d  and j: %d !\n", SL_this_procid,hdata[comm].mybarrier, j );
	}
    }
    hdata[comm].mybarrier = hdata[comm].mybarrier + 1;
    free ( buffer );
    return MPI_SUCCESS;
}
int Volpex_Complete_Barrier ( MPI_Comm comm)
{

    int i;
//    int *targets, numoftargets;
    int *ret;
    int *rbufs;
    SL_Request *reqs;
    SL_Status  *stats;
    int local_comp=0, flag, err;



    ret     = (int*) malloc (2*Volpex_numprocs * sizeof(int));
    reqs    = (SL_Request*) malloc (2*Volpex_numprocs * sizeof(SL_Request));
    stats   = (SL_Status*) malloc (2*Volpex_numprocs * sizeof(SL_Status));
    rbufs   = (int*) malloc (Volpex_numprocs * sizeof(int));

    for ( i=0 ; i < Volpex_numprocs; i++ ) {
            ret[2*i]   = SL_Isend ( &i, sizeof(int), i, 0, 0, &reqs[2*i]);
            if ( ret[2*i] != SL_SUCCESS ) {
                PRINTF(("[%d]: Got an error in Red_barrier for target %d i%d\n", SL_this_procid,
                       i, i ));
                reqs[2*i] = SL_REQUEST_NULL;
                local_comp++;
            }
            ret[2*i+1] = SL_Irecv ( &rbufs[i], sizeof(int), i, 0, 0, &reqs[2*i+1]);
            if ( ret[2*i] != SL_SUCCESS ) {
                reqs[2*i+1] = SL_REQUEST_NULL;
                local_comp++;
            }
    }

	i=0;
    while (local_comp < 2*Volpex_numprocs ) {
        if ( reqs[i] != SL_REQUEST_NULL ) {
            flag = 0;
            err = SL_Test ( &reqs[i], &flag, &stats[i]);
            SL_msg_progress();
            if ( flag ) {
                PRINTF(("[%d]:Volpex_Complete_Barrier: operation to %d finished ret=%d\n", SL_this_procid,i, err ));
                local_comp++;
            }
        }
        Volpex_progress();
        i++;
        if ( i == 2*Volpex_numprocs  ) i=0;
    }

    /* Sanity check whether everybody is alive */
    for ( i=0; i<Volpex_numprocs; i++ ) {
        if ( ret[2*i]              != SL_SUCCESS ||
             ret[2*i+1]            != SL_SUCCESS ||
             stats[2*i].SL_ERROR   != SL_SUCCESS ||
             stats[2*i+1].SL_ERROR != SL_SUCCESS  ) {
            /* Now what? Nothing */
        }
    }

    /* We might need a confirmation step here (i.e.
       second step of the 2-stage commit protocol */

    return MPI_SUCCESS;
}

int Volpex_Redundancy_Barrier ( MPI_Comm comm, int rank )
{
    
    int i,k;
    int *targets, numoftargets;
    int *ret;
    int *rbufs;
    SL_Request *reqs;
    SL_Status  *stats;
    int local_comp=0, flag, err;
    
    PRINTF(("[%d]:Into Volpex_Redundancy_Barrier\n", SL_this_procid));
    Volpex_dest_src_locator(rank, comm, fullrank, &targets, &numoftargets, sizeof(int), SEND);


    PRINTF(("[%d]: Volpex_Redundancy_Barrier: total number of targets are %d\n", SL_this_procid,numoftargets));
    for(k=0; k<numoftargets; k++)
        PRINTF(("[%d]: Volpex_Redundancy_Barrier: Targets are %d  ", SL_this_procid,targets[k]));
    PRINTF(("\n"));

    
    ret     = (int*) malloc (2*numoftargets * sizeof(int));
    reqs    = (SL_Request*) malloc (2*numoftargets * sizeof(SL_Request));    
    stats   = (SL_Status*) malloc (2*numoftargets * sizeof(SL_Status));
    rbufs   = (int*) malloc (numoftargets * sizeof(int));

    for ( i=0 ; i < numoftargets; i++ ) {
	    ret[2*i]   = SL_Isend ( &i, sizeof(int), targets[i], 0, 0, &reqs[2*i]);	    
	    if ( ret[2*i] != SL_SUCCESS ) {
		PRINTF(("[%d]: Got an error in Red_barrier for target %d i%d\n", SL_this_procid, 
		       targets[i], i ));
		reqs[2*i] = SL_REQUEST_NULL;
		local_comp++;
	    }
	    ret[2*i+1] = SL_Irecv ( &rbufs[i], sizeof(int), targets[i], 0, 0, &reqs[2*i+1]);
	    if ( ret[2*i] != SL_SUCCESS ) {
		reqs[2*i+1] = SL_REQUEST_NULL;
		local_comp++;
	    }
    }
    
    i=0;
    while (local_comp < 2*numoftargets ) {
	if ( reqs[i] != SL_REQUEST_NULL ) {
	    flag = 0;
	    err = SL_Test ( &reqs[i], &flag, &stats[i]);
	    SL_msg_progress();
	    if ( flag ) {
		PRINTF(("[%d]:Redundancy Barrier: operation to %d finished ret=%d\n", SL_this_procid,
			targets[i/2], err ));
		local_comp++;
	    }
	}
	Volpex_progress();
	i++;
	if ( i == 2*numoftargets  ) i=0;
    }
    
    /* Sanity check whether everybody is alive */
    for ( i=0; i<numoftargets; i++ ) {
	if ( ret[2*i]              != SL_SUCCESS ||
	     ret[2*i+1]            != SL_SUCCESS ||
	     stats[2*i].SL_ERROR   != SL_SUCCESS ||
	     stats[2*i+1].SL_ERROR != SL_SUCCESS  ) {
	    /* Now what? Nothing */
	}
    }
    
    /* We might need a confirmation step here (i.e. 
       second step of the 2-stage commit protocol */
    
    return MPI_SUCCESS;
}

int  Volpex_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, 
		     int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    int i, waitcount;
    MPI_Request *req;
    MPI_Status *stats;
    int slen = Volpex_get_len (sendcount, sendtype );
    int rlen = Volpex_get_len (recvcount, recvtype );
    char *sbuf = (char *)sendbuf;
    char *rbuf = (char *)recvbuf; 
    
    PRINTF(("[%d] Into Volpex_Alltoall\n", SL_this_procid ));

    req   = (MPI_Request *)malloc ( 2 * hdata[comm].mysize * sizeof(MPI_Request));
    stats = (MPI_Status *) malloc ( 2 * hdata[comm].mysize * sizeof(MPI_Status));
    
    if( sendtype == recvtype  ) {
	for ( i=0; i < hdata[comm].mysize; i++){
	    if ( hdata[comm].myrank == i){
		memcpy(rbuf+i*rlen, sbuf+i*slen, slen);
		req[2*i]   = MPI_REQUEST_NULL;
		req[2*i+1] = MPI_REQUEST_NULL;
	    }
	    else {
		/* TBD: check for return code. What happens if an Irecv or Isend returns
		   ' no target available ?' we should probably abort the alltoall in 
		   that case as well. */
		Volpex_Irecv(rbuf+i*rlen, recvcount, recvtype, i, ALLTOALL_TAG, comm, &req[2*i]);
		Volpex_Isend(sbuf+i*slen, sendcount, sendtype, i, ALLTOALL_TAG, comm, &req[2*i+1]);
	    }
	}
	waitcount = 2*hdata[comm].mysize;
	Volpex_Waitall(waitcount, req, stats);
    }
    else {
	PRINTF((" VAlltoall: current version cannot handle type mismatch\n"));
    }
    
    free(req);
    free(stats);
    return MPI_SUCCESS;      
}

int  Volpex_Alltoallv(void *sendbuf, int *sendcount, int *sdispls, MPI_Datatype sendtype,
                      void *recvbuf, int *recvcount, int *rdispls, MPI_Datatype recvtype, 
		      MPI_Comm comm)
    
{
    int i, waitcount;
    MPI_Request *req;
    MPI_Status *stats;
    char *sbuf = (char *) sendbuf;
    char *rbuf = (char *) recvbuf; 
    int slen = Volpex_get_len (1, sendtype );    
    int rlen = Volpex_get_len (1, recvtype );

    PRINTF(("[%d] Into Volpex_Alltoallv\n", SL_this_procid ));

    req   = (MPI_Request *)malloc ( 2 * hdata[comm].mysize * sizeof(MPI_Request));
    stats = (MPI_Status *) malloc ( 2 * hdata[comm].mysize * sizeof(MPI_Status));
    
    if( sendtype == recvtype  ) {
	for ( i=0; i < hdata[comm].mysize; i++){
	    req[2*i]   = MPI_REQUEST_NULL;
	    req[2*i+1] = MPI_REQUEST_NULL;
	    if ( hdata[comm].myrank == i){
		memcpy(rbuf+rdispls[i]*rlen, sbuf+sdispls[i]*slen, 
		       sendcount[i]*slen);
	    }
	    else {
		/* TBD: check for return code. What happens if an Irecv or Isend returns
		   ' no target available ?' we should probably abort the alltoall in 
		   that case as well. */
		if ( recvcount[i] > 0 ) {
		    Volpex_Irecv(rbuf+rdispls[i]*rlen, recvcount[i], recvtype, i, 
				 ALLTOALL_TAG, comm, &req[2*i]);
		}
		if ( sendcount[i] > 0 ) {
		    Volpex_Isend(sbuf+sdispls[i]*slen, sendcount[i], sendtype, i, 
				 ALLTOALL_TAG, comm, &req[2*i+1]);
		}
	    }
	}
	waitcount = 2*hdata[comm].mysize;
	Volpex_Waitall(waitcount, req, stats);
    }
    else {
	PRINTF((" VAlltoallv: current version cannot handle type mismatch\n"));
    }
    
    free(req);
//    free(stats);
    return MPI_SUCCESS;      
}

int Volpex_Gather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt, 
		  MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    int j;
    MPI_Status *status = (MPI_Status *)MPI_STATUS_IGNORE;
    int rlen = Volpex_get_len(recvcnt, recvtype);
    char *rbuf = (char *)recvbuf; 
    
    PRINTF(("[%d] Into Volpex_Gather\n", SL_this_procid ));
    if(hdata[comm].myrank != root)
	Volpex_Send(sendbuf, sendcnt, sendtype, root, GATHER_TAG, comm);
    
    if(hdata[comm].myrank == root){
	for(j = 0; j < hdata[comm].mysize; j++){
	    if ( hdata[comm].myrank == j ) {
		memcpy ( rbuf+j*rlen, sendbuf, rlen );
	    }
	    else {
		PRINTF(("Now root will recv gather recvbuf\n"));
		Volpex_Recv(rbuf+j*rlen, recvcnt, recvtype, j, GATHER_TAG, comm, status);
	    }
	}
    }
    return MPI_SUCCESS;
}

int Volpex_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
		     void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    int root = 0;
    int count = recvcount*hdata[comm].mysize;
    
    PRINTF(("[%d] Into Volpex_Allgather with count %d\n", SL_this_procid, count));
    Volpex_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    Volpex_Bcast(recvbuf, count, recvtype, root, comm);

    return MPI_SUCCESS;
}


