#include "mpi.h"
#include "SL_msg.h"

extern Global_Map **GM;
extern Tag_Reuse *sendtagreuse;
extern Tag_Reuse *recvtagreuse;
extern Hidden_Data *hdata;
extern Request_List *reqlist;

extern NODEPTR head, insertpt, curr;
extern int GM_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char *hostip;
extern char *hostname;
extern int GM_numprocs;
extern int next_avail_comm;
extern int request_counter;

int  VolPEx_Finalize()
{
    PRINTF(("Into VolPEx_Finalize\n"));
    VolPEx_Barrier ( MPI_COMM_WORLD);
    VolPEx_Redundancy_Barrier ( MPI_COMM_WORLD, hdata[MPI_COMM_WORLD].myrank ) ;
    VolPEx_Barrier ( MPI_COMM_WORLD);
    SL_Finalize();
#ifdef MINGW
    WSACleanup();
#endif
    VolPex_send_buffer_delete();
    GM_free_global_data ();
    
    return MPI_SUCCESS;
}

int  VolPEx_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    MPI_Status *status = (MPI_Status *)MPI_STATUS_IGNORE;
    int i;
    
    PRINTF(("Into VolPEx_Bcast and myrank is %d for comm %d and size %d\n", 
	    hdata[comm].myrank, comm, hdata[comm].mysize));
    
    for(i = 1; i < hdata[comm].mysize; i++){
	if(hdata[comm].myrank == root){
	    VolPEx_Send(buf, count, datatype, i, BCAST_TAG, comm);
	    VolPEx_progress();
	}
	if(hdata[comm].myrank == i){
	    VolPEx_Recv(buf, count, datatype, root, BCAST_TAG, comm, status);
	    VolPEx_progress();
	}
    }
    return MPI_SUCCESS;
}


void VolPEx_reduce_ll(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, 
		      MPI_Op op, int root, MPI_Comm comm, int tag)
{
    MPI_Status *status = (MPI_Status *)MPI_STATUS_IGNORE;
    int j=0, k=0;
    int *tempIntArray1, *tempIntArray2;
    float *tempFloatArray1, *tempFloatArray2;
    double *tempDoubleArray1, *tempDoubleArray2;
    double _Complex *tempDoubleComplexArray1, *tempDoubleComplexArray2;
    
    PRINTF(("Into VolPEx_reduce_ll\n"));
    if(hdata[comm].myrank != root)
	VolPEx_Send(sendbuf, count, datatype, root, tag, comm);
    
    if(hdata[comm].myrank == root){
        if(datatype == MPI_INT || datatype == MPI_INTEGER){
            tempIntArray2 = (int *)malloc(count * sizeof(int));
            tempIntArray1 = (int *)malloc(count * sizeof(int));
	    memcpy(tempIntArray2, sendbuf, count * sizeof(int));
	    
	    for(j = 0; j < hdata[comm].mysize; j++){
		if ( hdata[comm].myrank == j ) {
		    continue;
		}
		VolPEx_Recv(tempIntArray1, count, datatype, j, tag, comm, status);
                for(k = 0; k < count; k++){
                    if(op == MPI_MAX){
			PRINTF(("Comparing tempIntArray1[%d]=%d to tempIntArray2[%d]=%d\n", 
				k, tempIntArray1[k], k, tempIntArray2[k]));
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
            for(j = 1; j < hdata[comm].mysize; j++){
                VolPEx_Recv(tempFloatArray1, count, datatype, j, tag, comm, status);
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
	    for(j = 1; j < hdata[comm].mysize; j++){
		VolPEx_Recv(tempDoubleArray1, count, datatype, j, tag, comm, status);
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
            for(j = 1; j < hdata[comm].mysize; j++){
                VolPEx_Recv(tempDoubleComplexArray1, count, datatype, j, tag, comm, status);
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

int VolPEx_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, 
		  int root, MPI_Comm comm)
{
    PRINTF(("Into VolPEx_Reduce\n"));
    VolPEx_reduce_ll(sendbuf, recvbuf, count, datatype, op, root, comm, REDUCE_TAG);
    return MPI_SUCCESS;
}

int VolPEx_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, 
		     MPI_Comm comm)
{
    int root = 0;
    
    PRINTF(("Into VolPEx_Allreduce\n"));
    VolPEx_reduce_ll(sendbuf, recvbuf, count, datatype, op, root, comm, REDUCE_TAG);
    VolPEx_Bcast(recvbuf, count, datatype, root, comm);
    return MPI_SUCCESS;
}

int VolPEx_Barrier(MPI_Comm comm)
{
    MPI_Status status;
    int i = 1;
    int j;
    int root = 0;
    int err;
    int *buffer=NULL;
    
    PRINTF(("Into VolPEx_Barrier with mybarrier = %d\n", hdata[comm].mybarrier));
    VolPEx_progress();
    buffer = (int *) calloc ( 1, sizeof(int) * hdata[comm].mysize);
    if  ( NULL == buffer ) {
	return MPI_ERR_OTHER;
    }
    
    if(hdata[comm].myrank != root){
	VolPEx_Send(&i, 1, MPI_INT, root, BARRIER_TAG, comm);
    }
    else{
	for (i=1; i<hdata[comm].mysize ; i++){
	    VolPEx_Recv(&buffer[i], 1, MPI_INT, i, BARRIER_TAG, comm, &status);
	}
    }
    VolPEx_progress();
    if(hdata[comm].myrank == root){
	//j = hdata[comm].mybarrier;
	for (i=1; i<hdata[comm].mysize ; i++){
	    VolPEx_Send(&hdata[comm].mybarrier, 1, MPI_INT, i, BARRIER_TAG, comm);
	    VolPEx_progress();
	}
    }
    else{
	err = VolPEx_Recv(&j, 1, MPI_INT, root, BARRIER_TAG, comm, &status);
	VolPEx_progress();
	if ( err != MPI_SUCCESS ) {
	    /* To be determined */
	}	    
	if ( j != hdata[comm].mybarrier ) {
	    printf("VBarrierL Mismatch of mybarrier: %d  and j: %d !\n", hdata[comm].mybarrier, j );
	}
    }
    hdata[comm].mybarrier = hdata[comm].mybarrier + 1;
    free ( buffer );
    return MPI_SUCCESS;
}

int VolPEx_Redundancy_Barrier ( MPI_Comm comm, int rank )
{
    
    int i;
    int targets[3] = {-1,-1,-1};
    int ret[6];
    int rbufs[3];
    SL_Request reqs[6];
    SL_Status  stats[6];
    int local_comp=0, flag, err;
    
    PRINTF(("Into VolPEx_Redundancy_Barrier\n"));
    GM_dest_src_locator(rank, comm, fullrank, targets);
    PRINTF(("Targets are %d %d %d\n", targets[0], targets[1], targets[2]));
    
    for ( i=0 ; i < 3; i++ ) {
	if ( targets[i] != -1 ) {
	    ret[2*i]   = SL_Isend ( &i, sizeof(int), targets[i], 0, 0, &reqs[2*i]);
	    if ( ret[2*i] != SL_SUCCESS ) {
		local_comp++;
	    }
	    ret[2*i+1] = SL_Irecv ( &rbufs[i], sizeof(int), targets[i], 0, 0, &reqs[2*i+1]);
	    if ( ret[2*i] != SL_SUCCESS ) {
		local_comp++;
	    }
	}
	else {
	    ret[2*i]     = SL_SUCCESS;
	    ret[2*i+1]   = SL_SUCCESS;
	    reqs[2*i]    = SL_REQUEST_NULL; 
	    reqs[2*i+1]  = SL_REQUEST_NULL; 
	    local_comp   += 2;
	}
    }
    
    //  SL_Waitall ( 6, reqs, stats );
    i=0;
    while (local_comp < 6 ) {
	if ( reqs[i] != SL_REQUEST_NULL ) {
	    flag = 0;
	    err = SL_Test ( &reqs[i], &flag, &stats[i]);
	    SL_msg_progress();
	    if ( flag ) {
		PRINTF(("Redundancy Barrier: operation to %d finished ret=%d\n", 
			targets[i/2], err ));
		local_comp++;
	    }
	}
	VolPEx_progress();
	i++;
	if ( i == 6 ) i=0;
    }
    
    /* Sanity check whether everybody is alive */
    for ( i=0; i<3; i++ ) {
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

int  VolPEx_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, 
		     int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    int i, waitcount;
    MPI_Request *req;
    MPI_Status *stats;
    int slen = VolPex_get_len (sendcount, sendtype );
    int rlen = VolPex_get_len (recvcount, recvtype );
    char *sbuf = (char *)sendbuf;
    char *rbuf = (char *)recvbuf; 
    
    PRINTF(("Into VolPEx_Alltoall\n"));
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
		VolPEx_Irecv(rbuf+i*rlen, recvcount, recvtype, i, ALLTOALL_TAG, comm, &req[2*i]);
		VolPEx_Isend(sbuf+i*slen, sendcount, sendtype, i, ALLTOALL_TAG, comm, &req[2*i+1]);
	    }
	}
	waitcount = 2*hdata[comm].mysize;
	VolPEx_Waitall(waitcount, req, stats);
    }
    else {
	PRINTF((" VAlltoall: current version cannot handle type mismatch\n"));
    }
    
    free(req);
    free(stats);
    return MPI_SUCCESS;      
}

int  VolPEx_Alltoallv(void *sendbuf, int *sendcount, int *sdispls, MPI_Datatype sendtype,
                      void *recvbuf, int *recvcount, int *rdispls, MPI_Datatype recvtype, 
		      MPI_Comm comm)
    
{
    int i, waitcount;
    MPI_Request *req;
    MPI_Status *stats;
    char *sbuf = (char *) sendbuf;
    char *rbuf = (char *) recvbuf; 
    
    PRINTF(("Into VolPEx_Alltoallv\n"));
    req   = (MPI_Request *)malloc ( 2 * hdata[comm].mysize * sizeof(MPI_Request));
    stats = (MPI_Status *) malloc ( 2 * hdata[comm].mysize * sizeof(MPI_Status));
    
    if( sendtype == recvtype  ) {
	for ( i=0; i < hdata[comm].mysize; i++){
	    if ( hdata[comm].myrank == i){
		memcpy(rbuf+rdispls[i]*sizeof(recvtype), sbuf+sdispls[i]*sizeof(sendtype), 
		       sendcount[i]*sizeof(sendtype));
		req[2*i]   = MPI_REQUEST_NULL;
		req[2*i+1] = MPI_REQUEST_NULL;
	    }
	    else {
		/* TBD: check for return code. What happens if an Irecv or Isend returns
		   ' no target available ?' we should probably abort the alltoall in 
		   that case as well. */
		VolPEx_Irecv(rbuf+rdispls[i]*sizeof(recvtype), recvcount[i], recvtype, i, 
			     ALLTOALL_TAG, comm, &req[2*i]);
		VolPEx_Isend(sbuf+sdispls[i]*sizeof(sendtype), sendcount[i], sendtype, i, 
			     ALLTOALL_TAG, comm, &req[2*i+1]);
	    }
	}
	waitcount = 2*hdata[comm].mysize;
	VolPEx_Waitall(waitcount, req, stats);
    }
    else {
	PRINTF((" VAlltoallv: current version cannot handle type mismatch\n"));
    }
    
    free(req);
    free(stats);
    return MPI_SUCCESS;      
}

int VolPEx_Gather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt, 
		  MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    int j;
    MPI_Status *status = (MPI_Status *)MPI_STATUS_IGNORE;
    int rlen = VolPex_get_len(recvcnt, recvtype);
    char *rbuf = (char *)recvbuf; 
    
    PRINTF(("Into VolPEx_Gather\n"));
    if(hdata[comm].myrank != root)
	VolPEx_Send(sendbuf, sendcnt, sendtype, root, GATHER_TAG, comm);
    
    if(hdata[comm].myrank == root){
	for(j = 0; j < hdata[comm].mysize; j++){
	    if ( hdata[comm].myrank == j ) {
		memcpy ( rbuf+j*rlen, sendbuf, rlen );
	    }
	    else {
		PRINTF(("Now root will recv gather recvbuf\n"));
		VolPEx_Recv(rbuf+j*rlen, recvcnt, recvtype, j, GATHER_TAG, comm, status);
	    }
	}
    }
    return MPI_SUCCESS;
}

int VolPEx_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
		     void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    int root = 0;
    int count = recvcount*hdata[comm].mysize;
    
    PRINTF(("Into VolPEx_Allgather with count %d\n", count));
    VolPEx_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    VolPEx_Bcast(recvbuf, count, recvtype, root, comm);

    return MPI_SUCCESS;
}


