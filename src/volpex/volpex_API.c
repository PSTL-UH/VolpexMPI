#include "mpi.h"

extern int SL_this_procid;
extern Global_Map GM[TOTAL_NODES][TOTAL_COMMS];
extern Tag_Reuse sendtagreuse[TAGLISTSIZE];
extern Tag_Reuse recvtagreuse[TAGLISTSIZE];
extern NODEPTR head, insertpt, curr;
extern Request_List reqlist[REQLISTSIZE];
extern int GM_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char hostip[32];
extern char hostname[512];
extern Hidden_Data hdata[TOTAL_COMMS];
extern int GM_numprocs;
extern int next_avail_comm;
extern int request_counter;


#pragma weak mpi_init_   = mpi_init
#pragma weak mpi_init__  = mpi_init
#pragma weak MPI_INIT    = mpi_init

int mpi_init(int *ierr)
{
    int i;
    next_avail_comm = 3;
    for(i = 0; i < REQLISTSIZE; i++)
	reqlist[i].in_use = 0;
    for(i = 0; i < TAGLISTSIZE; i++){
	sendtagreuse[i].tag = -1;
	recvtagreuse[i].tag = -1;
    }
    GM_host_ip();
    PRINTF(("Hostname: %s\n", hostname));
    PRINTF(("HostIP: %s\n", hostip));
    GM_proc_read_and_set();
    SL_this_procid = GM_get_procid_fullrank(fullrank);
    PRINTF(("My full rank is %s\n", fullrank));
    
    head = insertpt = curr = VolPex_send_buffer_init();
    hdata[1].mybarrier = 0;
    
    SL_Init();
    PRINTF(("Initializing MPI Program\n"));
    *ierr = 0;
    int mynumeric;
    char mylevel;
    for ( i=0; i< GM_numprocs; i++ ){
	if(GM[i][MPI_COMM_WORLD].id == SL_this_procid){
	    sscanf(GM[i][MPI_COMM_WORLD].rank, "%d,%c", &mynumeric, &mylevel);
	    hdata[MPI_COMM_WORLD].myrank = mynumeric;
	}
    }
    VolPEx_Redundancy_Barrier ( MPI_COMM_WORLD, hdata[MPI_COMM_WORLD].myrank );
    return MPI_SUCCESS;
}

int  MPI_Init( int *argc, char ***argv )
{
    int i;
    
    next_avail_comm = 3;
    for(i = 0; i < REQLISTSIZE; i++)
	reqlist[i].in_use = 0;
    for(i = 0; i < TAGLISTSIZE; i++){
	sendtagreuse[i].tag = -1;
	recvtagreuse[i].tag = -1;
    }
    GM_host_ip();
    PRINTF(("Hostname: %s\n", hostname));
    PRINTF(("HostIP: %s\n", hostip));
    GM_proc_read_and_set();
    if ( *argc > 1 ) {
	SL_this_procid = atoi ( (*argv)[1] );
	GM_get_fullrank(fullrank);
    }
    else
	SL_this_procid = GM_get_procid_fullrank(fullrank);
    PRINTF(("My full rank is %s\n", fullrank));
    head = insertpt = curr = VolPex_send_buffer_init();
    hdata[1].mybarrier = 0;
    SL_Init();
    PRINTF(("Initializing MPI Program\n"));
    int mynumeric;
    char mylevel;
    for ( i=0; i< GM_numprocs; i++ ){
	if(GM[i][MPI_COMM_WORLD].id == SL_this_procid){
	    sscanf(GM[i][MPI_COMM_WORLD].rank, "%d,%c", &mynumeric, &mylevel);
	    hdata[MPI_COMM_WORLD].myrank = mynumeric;
	}
    }
    VolPEx_Redundancy_Barrier ( MPI_COMM_WORLD, hdata[MPI_COMM_WORLD].myrank ) ;
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_finalize_   = mpi_finalize
#pragma weak mpi_finalize__  = mpi_finalize
#pragma weak MPI_FINALIZE    = mpi_finalize

int mpi_finalize( int *ierr)
{
    VolPEx_Finalize();
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Finalize()
{
    VolPEx_Finalize();
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_size_   = mpi_comm_size
#pragma weak mpi_comm_size__  = mpi_comm_size
#pragma weak MPI_COMM_SIZE    = mpi_comm_size


int mpi_comm_size(unsigned int *comm, int *size, int *ierr)
{
    VolPEx_Comm_size(*comm, size);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int *size)
{
    VolPEx_Comm_size(comm, size);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_rank_   = mpi_comm_rank
#pragma weak mpi_comm_rank__  = mpi_comm_rank
#pragma weak MPI_COMM_RANK    = mpi_comm_rank

int mpi_comm_rank(unsigned int *comm, int *rank, int *ierr)
{
    VolPEx_Comm_rank(*comm, rank);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
    VolPEx_Comm_rank(comm, rank);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_send_   = mpi_send
#pragma weak mpi_send__  = mpi_send
#pragma weak MPI_SEND    = mpi_send

int mpi_send(void *buf, int *count, unsigned int *datatype, int *dest, int *tag, 
	     unsigned int *comm, int *ierr)
{
    VolPEx_Send(buf, *count, *datatype, *dest, *tag, *comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm)
{
    VolPEx_Send(buf, count, datatype, dest, tag, comm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_recv_   = mpi_recv
#pragma weak mpi_recv__  = mpi_recv
#pragma weak MPI_RECV    = mpi_recv

int mpi_recv(void *buf, int *count, unsigned int *datatype, int *source, int *tag, 
	     unsigned int *comm, int *status, int *ierr)
{
    VolPEx_Recv(buf, *count, *datatype, *source, *tag, *comm, status);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
	      MPI_Comm comm, MPI_Status *status)
{
    VolPEx_Recv(buf, count, datatype, source, tag, comm, status);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_bcast_   = mpi_bcast
#pragma weak mpi_bcast__  = mpi_bcast
#pragma weak MPI_BCAST    = mpi_bcast

int mpi_bcast(void *buf, int *count, unsigned int *datatype, int *root, 
	      unsigned int *comm, int *ierr)
{
    VolPEx_Bcast(buf, *count, *datatype, *root, *comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, 
	       MPI_Comm comm)
{
    VolPEx_Bcast(buf, count, datatype, root, comm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_reduce_   = mpi_reduce
#pragma weak mpi_reduce__  = mpi_reduce
#pragma weak MPI_REDUCE    = mpi_reduce

int mpi_reduce(void *sendbuf, void *recvbuf, int *count, unsigned int *datatype, 
	       unsigned int *op, int *root, unsigned int *comm, int *ierr)
{
    VolPEx_Reduce(sendbuf, recvbuf, *count, *datatype, *op, *root, *comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, 
	       MPI_Op op, int root, MPI_Comm comm)
{
    VolPEx_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_allreduce_   = mpi_allreduce
#pragma weak mpi_allreduce__  = mpi_allreduce
#pragma weak MPI_ALLREDUCE    = mpi_allreduce

int mpi_allreduce(void *sendbuf, void *recvbuf, int *count, unsigned int *datatype, 
		  unsigned int *op, unsigned int *comm, int *ierr)
{
    VolPEx_Allreduce(sendbuf, recvbuf, *count, *datatype, *op, *comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, 
		  MPI_Op op, MPI_Comm comm)
{
    VolPEx_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_isend_   = mpi_isend
#pragma weak mpi_isend__  = mpi_isend
#pragma weak MPI_ISEND    = mpi_isend


int mpi_isend(void *buf, int *count, unsigned int *datatype, int *dest, int *tag, 
	      unsigned int *comm, int *request, int *ierr)
{
    VolPEx_Isend(buf, *count, *datatype, *dest, *tag, *comm, request);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, 
	       MPI_Comm comm, MPI_Request *request)
{
    VolPEx_Isend(buf, count, datatype, dest, tag, comm, request);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_irecv_   = mpi_irecv
#pragma weak mpi_irecv__  = mpi_irecv
#pragma weak MPI_IRECV    = mpi_irecv

int mpi_irecv(void *buf, int *count, unsigned int *datatype, int *source, int *tag, 
	      unsigned int *comm, int *request, int *ierr)
{
    VolPEx_Irecv(buf, *count, *datatype, *source, *tag, *comm, request);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
	       MPI_Comm comm, MPI_Request *request)
{
    VolPEx_Irecv(buf, count, datatype, source, tag, comm, request);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_waitall_   = mpi_waitall
#pragma weak mpi_waitall__  = mpi_waitall
#pragma weak MPI_WAITALL    = mpi_waitall

int  mpi_waitall(int *count, int *request, int *status, int *ierr)
{
    VolPEx_Waitall(*count, request, status);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Waitall(int count, MPI_Request request[], MPI_Status status[])
{
    VolPEx_Waitall(count, request, status);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_wait_   = mpi_wait
#pragma weak mpi_wait__  = mpi_wait
#pragma weak MPI_WAIT    = mpi_wait

int mpi_wait(int *request, int *status, int *ierr)
{
    VolPEx_Wait(request, status);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Wait(MPI_Request *request, MPI_Status *status)
{
    VolPEx_Wait(request, status);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_barrier_   = mpi_barrier
#pragma weak mpi_barrier__  = mpi_barrier
#pragma weak MPI_BARRIER    = mpi_barrier

int mpi_barrier(unsigned int *comm, int *ierr)
{
    VolPEx_Barrier(*comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Barrier(MPI_Comm comm)
{
    VolPEx_Barrier(comm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_abort_   = mpi_abort
#pragma weak mpi_abort__  = mpi_abort
#pragma weak MPI_ABORT    = mpi_abort

int mpi_abort(unsigned int *comm, int *errorcode, int *ierr)
{
    printf("Error %d occured on context_id %d. Aborting.\n", *errorcode, *comm);
    *ierr = 0;
    exit(*errorcode);
    return MPI_SUCCESS;
}

int MPI_Abort(MPI_Comm comm, int errorcode)
{
    printf("Error %d occured on context_id %d. Aborting.\n", errorcode, comm);
    exit(errorcode);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_wtime_   = mpi_wtime
#pragma weak mpi_wtime__  = mpi_wtime
#pragma weak MPI_WTIME    = mpi_wtime

double mpi_wtime()
{
    return MPI_Wtime();
}

double MPI_Wtime()
{
    struct timeval tp;
    double sec=0.0;
    double psec=0.0;
    
    gettimeofday( &tp, NULL );
    sec = (double)tp.tv_sec;
    psec = ((double)tp.tv_usec)/((double)1000000.0);
    return (sec+psec);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_alltoall_   = mpi_alltoall
#pragma weak mpi_alltoall__  = mpi_alltoall
#pragma weak MPI_ALLTOALL    = mpi_alltoall

int  mpi_alltoall(void *sendbuf, int *sendcount, unsigned int *sendtype, void *recvbuf, 
		  int *recvcount, unsigned int *recvtype, unsigned int *comm, int *ierr)
{
    VolPEx_Alltoall(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, *recvtype, *comm);
    *ierr = 0;
    return MPI_SUCCESS;      
}

int  MPI_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, 
		  int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    VolPEx_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
    return MPI_SUCCESS;      
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_alltoallv_   = mpi_alltoallv
#pragma weak mpi_alltoallv__  = mpi_alltoallv
#pragma weak MPI_ALLTOALLV    = mpi_alltoallv

int mpi_alltoallv(void *sendbuf, int *sendcount, int *sdispls, unsigned int *sendtype, 
		  void *recvbuf, int *recvcount, int *rdispls, unsigned int *recvtype,
		  unsigned int *comm, int *ierr)
{
    VolPEx_Alltoallv(sendbuf, sendcount, sdispls, *sendtype, recvbuf, recvcount, 
		     rdispls, *recvtype, *comm);
    *ierr = 0;
    return MPI_SUCCESS;      
}

int  MPI_Alltoallv(void *sendbuf, int *sendcount, int *sdispls, MPI_Datatype sendtype, 
		   void *recvbuf, int *recvcount, int *rdispls, MPI_Datatype recvtype, 
		   MPI_Comm comm)
{
    VolPEx_Alltoallv(sendbuf, sendcount, sdispls, sendtype, recvbuf, recvcount, rdispls,
		     recvtype, comm);
    return MPI_SUCCESS;      
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_dup_   = mpi_comm_dup
#pragma weak mpi_comm_dup__  = mpi_comm_dup
#pragma weak MPI_COMM_DUP    = mpi_comm_dup

int mpi_comm_dup(unsigned int *comm, unsigned int *newcomm, int *ierr)
{
    VolPEx_Comm_dup(*comm, newcomm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    VolPEx_Comm_dup(comm, newcomm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_split_   = mpi_comm_split
#pragma weak mpi_comm_split__  = mpi_comm_split
#pragma weak MPI_COMM_SPLIT    = mpi_comm_split

int mpi_comm_split(unsigned int *comm, int *color, int *key, unsigned int *newcomm, 
		   int *ierr)
{
    VolPEx_Comm_split(*comm, *color, *key, newcomm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
    VolPEx_Comm_split(comm, color, key, newcomm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_gather_   = mpi_gather
#pragma weak mpi_gather__  = mpi_gather
#pragma weak MPI_GATHER    = mpi_gather

int mpi_gather(void *sendbuf, int *sendcnt, unsigned int *sendtype, void *recvbuf, 
	       int *recvcnt, unsigned int *recvtype, int *root, unsigned int *comm, 
	       int *ierr)
{
    VolPEx_Gather(sendbuf, *sendcnt, *sendtype, recvbuf, *recvcnt, *recvtype, *root, *comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int  MPI_Gather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, 
		int recvcnt, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    VolPEx_Gather(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_allgather_   = mpi_allgather
#pragma weak mpi_allgather__  = mpi_allgather
#pragma weak MPI_ALLGATHER    = mpi_allgather

int mpi_allgather(void *sendbuf, int *sendcount, unsigned int *sendtype, void *recvbuf, 
		  int *recvcount, unsigned int *recvtype, unsigned int *comm, int *ierr)
{
    VolPEx_Allgather(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, *recvtype, *comm);
    *ierr = 0;
    return MPI_SUCCESS;
}

int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
		  int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    VolPEx_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
    return MPI_SUCCESS;
}
