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


#ifdef VOLPEXFORTRAN
#pragma weak mpi_init_   = MPI_Init
#pragma weak mpi_init__  = MPI_Init
#pragma weak MPI_INIT    = MPI_Init

int MPI_Init   ( int * );
int mpi_init_  ( int * );
int mpi_init__ ( int * );
int MPI_INIT   ( int * );

int  MPI_Init(int *ierr)
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

	head = insertpt = curr = send_buffer_init();
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
#else
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
	head = insertpt = curr = send_buffer_init();
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
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_finalize_   = MPI_Finalize
#pragma weak mpi_finalize__  = MPI_Finalize
#pragma weak MPI_FINALIZE    = MPI_Finalize

int MPI_Finalize   ( int * );
int mpi_finalize_  ( int * );
int mpi_finalize__ ( int * );
int MPI_FINALIZE   ( int * );

int  MPI_Finalize( int *ierr)
{
	VolPEx_Finalize();
      *ierr = 0;
	return MPI_SUCCESS;
}
#else
int  MPI_Finalize()
{
	VolPEx_Finalize();
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_comm_size_   = MPI_Comm_size
#pragma weak mpi_comm_size__  = MPI_Comm_size
#pragma weak MPI_COMM_SIZE    = MPI_Comm_size

int MPI_Comm_size   ( MPI_Comm *, int *, int * );
int mpi_comm_size_  ( MPI_Comm *, int *, int * );
int mpi_comm_size__ ( MPI_Comm *, int *, int * );
int MPI_COMM_SIZE   ( MPI_Comm *, int *, int * );

int MPI_Comm_size(MPI_Comm *comm, int *size, int *ierr)
{
	VolPEx_Comm_size(*comm, size);
      *ierr = 0;
	return MPI_SUCCESS;
}
#else
int MPI_Comm_size(MPI_Comm comm, int *size)
{
	VolPEx_Comm_size(comm, size);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_comm_rank_   = MPI_Comm_rank
#pragma weak mpi_comm_rank__  = MPI_Comm_rank
#pragma weak MPI_COMM_RANK    = MPI_Comm_rank

int MPI_Comm_rank   ( MPI_Comm *, int *, int * );
int mpi_comm_rank_  ( MPI_Comm *, int *, int * );
int mpi_comm_rank__ ( MPI_Comm *, int *, int * );
int MPI_COMM_RANK   ( MPI_Comm *, int *, int * );

int MPI_Comm_rank(MPI_Comm *comm, int *rank, int *ierr)
{
	VolPEx_Comm_rank(*comm, rank);
	*ierr = 0;
	return MPI_SUCCESS;
}
#else
int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
	VolPEx_Comm_rank(comm, rank);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_send_   = MPI_Send
#pragma weak mpi_send__  = MPI_Send
#pragma weak MPI_SEND    = MPI_Send

int MPI_Send   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int * );
int mpi_send_  (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int * );
int mpi_send__ (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int * );
int MPI_SEND   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int * );

int  MPI_Send(void *buf, int *count, MPI_Datatype *datatype, int *dest, int *tag, MPI_Comm *comm, int *ierr)
{
      VolPEx_Send(buf, *count, *datatype, *dest, *tag, *comm);
      *ierr = 0;
	return MPI_SUCCESS;
}
#else
int  MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
      VolPEx_Send(buf, count, datatype, dest, tag, comm);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_recv_   = MPI_Recv
#pragma weak mpi_recv__  = MPI_Recv
#pragma weak MPI_RECV    = MPI_Recv

int MPI_Recv   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Status *, int * );
int mpi_recv_  (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Status *, int * );
int mpi_recv__ (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Status *, int * );
int MPI_RECV   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Status *, int * );

int  MPI_Recv(void *buf, int *count, MPI_Datatype *datatype, int *source, int *tag, MPI_Comm *comm, MPI_Status *status, int *ierr)
{
     	VolPEx_Recv(buf, *count, *datatype, *source, *tag, *comm, status);
	*ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
     	VolPEx_Recv(buf, count, datatype, source, tag, comm, status);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_bcast_   = MPI_Bcast
#pragma weak mpi_bcast__  = MPI_Bcast
#pragma weak MPI_BCAST    = MPI_Bcast

int MPI_Bcast   (void *, int *, MPI_Datatype *, int *, MPI_Comm *, int * );
int mpi_bcast_  (void *, int *, MPI_Datatype *, int *, MPI_Comm *, int * );
int mpi_bcast__ (void *, int *, MPI_Datatype *, int *, MPI_Comm *, int * );
int MPI_BCAST   (void *, int *, MPI_Datatype *, int *, MPI_Comm *, int * );

int  MPI_Bcast(void *buf, int *count, MPI_Datatype *datatype, int *root, MPI_Comm *comm, int *ierr)
{
	VolPEx_Bcast(buf, *count, *datatype, *root, *comm);
	*ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
     	VolPEx_Bcast(buf, count, datatype, root, comm);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_reduce_   = MPI_Reduce
#pragma weak mpi_reduce__  = MPI_Reduce
#pragma weak MPI_REDUCE    = MPI_Reduce

int MPI_Reduce   (void *, void *, int *, MPI_Datatype *, MPI_Op *, int *, MPI_Comm *, int *);
int mpi_reduce_  (void *, void *, int *, MPI_Datatype *, MPI_Op *, int *, MPI_Comm *, int *);
int mpi_reduce__ (void *, void *, int *, MPI_Datatype *, MPI_Op *, int *, MPI_Comm *, int *);
int MPI_REDUCE   (void *, void *, int *, MPI_Datatype *, MPI_Op *, int *, MPI_Comm *, int *);

int MPI_Reduce(void *sendbuf, void *recvbuf, int *count, MPI_Datatype *datatype, MPI_Op *op, int *root, MPI_Comm *comm, int *ierr)
{
     	VolPEx_Reduce(sendbuf, recvbuf, *count, *datatype, *op, *root, *comm);
      *ierr = 0;
      return MPI_SUCCESS;
}
#else
int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
     	VolPEx_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_allreduce_   = MPI_Allreduce
#pragma weak mpi_allreduce__  = MPI_Allreduce
#pragma weak MPI_ALLREDUCE    = MPI_Allreduce

int MPI_Allreduce   (void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int *);
int mpi_allreduce_  (void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int *);
int mpi_allreduce__ (void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int *);
int MPI_ALLREDUCE   (void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int *);

int MPI_Allreduce(void *sendbuf, void *recvbuf, int *count, MPI_Datatype *datatype, MPI_Op *op, MPI_Comm *comm, int *ierr)
{
	VolPEx_Allreduce(sendbuf, recvbuf, *count, *datatype, *op, *comm);
      *ierr = 0;
      return MPI_SUCCESS;
}
#else
int MPI_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
	VolPEx_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_isend_   = MPI_Isend
#pragma weak mpi_isend__  = MPI_Isend
#pragma weak MPI_ISEND    = MPI_Isend

int MPI_Isend   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );
int mpi_isend_  (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );
int mpi_isend__ (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );
int MPI_ISEND   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );

int  MPI_Isend(void *buf, int *count, MPI_Datatype *datatype, int *dest, int *tag, MPI_Comm *comm, MPI_Request *request, int *ierr)
{
	VolPEx_Isend(buf, *count, *datatype, *dest, *tag, *comm, request);
      *ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	VolPEx_Isend(buf, count, datatype, dest, tag, comm, request);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_irecv_   = MPI_Irecv
#pragma weak mpi_irecv__  = MPI_Irecv
#pragma weak MPI_IRECV    = MPI_Irecv

int MPI_Irecv   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );
int mpi_irecv_  (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );
int mpi_irecv__ (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );
int MPI_IRECV   (void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, MPI_Request *, int * );

int  MPI_Irecv(void *buf, int *count, MPI_Datatype *datatype, int *source, int *tag, MPI_Comm *comm, MPI_Request *request, int *ierr)
{
     	VolPEx_Irecv(buf, *count, *datatype, *source, *tag, *comm, request);
	*ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
     	VolPEx_Irecv(buf, count, datatype, source, tag, comm, request);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_waitall_   = MPI_Waitall
#pragma weak mpi_waitall__  = MPI_Waitall
#pragma weak MPI_WAITALL    = MPI_Waitall

int MPI_Waitall   ( int *, MPI_Request *, MPI_Status *, int * );
int mpi_waitall_  ( int *, MPI_Request *, MPI_Status *, int * );
int mpi_waitall__ ( int *, MPI_Request *, MPI_Status *, int * );
int MPI_WAITALL   ( int *, MPI_Request *, MPI_Status *, int * );

int  MPI_Waitall(int *count, MPI_Request request[], MPI_Status status[], int *ierr)
{
	VolPEx_Waitall(*count, request, status);
	*ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Waitall(int count, MPI_Request request[], MPI_Status status[])
{
	VolPEx_Waitall(count, request, status);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_wait_   = MPI_Wait
#pragma weak mpi_wait__  = MPI_Wait
#pragma weak MPI_WAIT    = MPI_Wait

int MPI_Wait   ( MPI_Request *, MPI_Status *, int * );
int mpi_wait_  ( MPI_Request *, MPI_Status *, int * );
int mpi_wait__ ( MPI_Request *, MPI_Status *, int * );
int MPI_WAIT   ( MPI_Request *, MPI_Status *, int * );

int  MPI_Wait(MPI_Request *request, MPI_Status *status, int *ierr)
{
     	VolPEx_Wait(request, status);
      *ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Wait(MPI_Request *request, MPI_Status *status)
{
     	VolPEx_Wait(request, status);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_barrier_   = MPI_Barrier
#pragma weak mpi_barrier__  = MPI_Barrier
#pragma weak MPI_BARRIER    = MPI_Barrier

int MPI_Barrier   ( MPI_Comm *, int * );
int mpi_barrier_  ( MPI_Comm *, int * );
int mpi_barrier__ ( MPI_Comm *, int * );
int MPI_BARRIER   ( MPI_Comm *, int * );

int MPI_Barrier(MPI_Comm *comm, int *ierr)
{
     	VolPEx_Barrier(*comm);
      *ierr = 0;
	return MPI_SUCCESS;
}
#else
int MPI_Barrier(MPI_Comm comm)
{
     	VolPEx_Barrier(comm);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_abort_   = MPI_Abort
#pragma weak mpi_abort__  = MPI_Abort
#pragma weak MPI_ABORT    = MPI_Abort

int MPI_Abort   ( MPI_Comm *, int *, int * );
int mpi_abort_  ( MPI_Comm *, int *, int * );
int mpi_abort__ ( MPI_Comm *, int *, int * );
int MPI_ABORT   ( MPI_Comm *, int *, int * );

int  MPI_Abort(MPI_Comm *comm, int *errorcode, int *ierr)
{
	printf("Error %d occured on context_id %d. Aborting.\n", *errorcode, *comm);
  	*ierr = 0;
	exit(*errorcode);
	return MPI_SUCCESS;
}
#else
int  MPI_Abort(MPI_Comm comm, int errorcode)
{
	printf("Error %d occured on context_id %d. Aborting.\n", errorcode, comm);
	exit(errorcode);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_wtime_   = MPI_Wtime
#pragma weak mpi_wtime__  = MPI_Wtime
#pragma weak MPI_WTIME    = MPI_Wtime

double MPI_Wtime   ( );
double mpi_wtime_  ( );
double mpi_wtime__ ( );
double MPI_WTIME   ( );

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
#else
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
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_alltoall_   = MPI_Alltoall
#pragma weak mpi_alltoall__  = MPI_Alltoall
#pragma weak MPI_ALLTOALL    = MPI_Alltoall

int  MPI_Alltoall   ( void *, int *, MPI_Datatype *, void *, int *, MPI_Datatype *, MPI_Comm *, int * );
int  mpi_alltoall_  ( void *, int *, MPI_Datatype *, void *, int *, MPI_Datatype *, MPI_Comm *, int * );
int  mpi_alltoall__ ( void *, int *, MPI_Datatype *, void *, int *, MPI_Datatype *, MPI_Comm *, int * );
int  MPI_ALLTOALL   ( void *, int *, MPI_Datatype *, void *, int *, MPI_Datatype *, MPI_Comm *, int * );

int  MPI_Alltoall(void *sendbuf, int *sendcount, MPI_Datatype *sendtype, void *recvbuf, int *recvcount, MPI_Datatype *recvtype, MPI_Comm *comm, int *ierr)
{
	VolPEx_Alltoall(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, *recvtype, *comm);
      *ierr = 0;
      return MPI_SUCCESS;      
}
#else
int  MPI_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
	VolPEx_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
      return MPI_SUCCESS;      
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_alltoallv_   = MPI_Alltoallv
#pragma weak mpi_alltoallv__  = MPI_Alltoallv
#pragma weak MPI_ALLTOALLV    = MPI_Alltoallv

int  MPI_Alltoallv   ( void *, int *, int *, MPI_Datatype *, void *, int *, int *, MPI_Datatype *, MPI_Comm *, int * );
int  mpi_alltoallv_  ( void *, int *, int *, MPI_Datatype *, void *, int *, int *, MPI_Datatype *, MPI_Comm *, int * );
int  mpi_alltoallv__ ( void *, int *, int *, MPI_Datatype *, void *, int *, int *, MPI_Datatype *, MPI_Comm *, int * );
int  MPI_ALLTOALLV   ( void *, int *, int *, MPI_Datatype *, void *, int *, int *, MPI_Datatype *, MPI_Comm *, int * );

int  MPI_Alltoallv(void *sendbuf, int *sendcount, int *sdispls, MPI_Datatype *sendtype, void *recvbuf, int *recvcount, int *rdispls, MPI_Datatype *recvtype, MPI_Comm *comm, int *ierr)
{
     	VolPEx_Alltoallv(sendbuf, sendcount, sdispls, *sendtype, recvbuf, recvcount, rdispls, *recvtype, *comm);
      *ierr = 0;
      return MPI_SUCCESS;      
}
#else
int  MPI_Alltoallv(void *sendbuf, int *sendcount, int *sdispls, MPI_Datatype sendtype, void *recvbuf, int *recvcount, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm)
{
     	VolPEx_Alltoallv(sendbuf, sendcount, sdispls, sendtype, recvbuf, recvcount, rdispls, recvtype, comm);
      return MPI_SUCCESS;      
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_comm_dup_   = MPI_Comm_dup
#pragma weak mpi_comm_dup__  = MPI_Comm_dup
#pragma weak MPI_COMM_DUP    = MPI_Comm_dup

int MPI_Comm_dup   ( MPI_Comm *, MPI_Comm *, int * );
int mpi_comm_dup_  ( MPI_Comm *, MPI_Comm *, int * );
int mpi_comm_dup__ ( MPI_Comm *, MPI_Comm *, int * );
int MPI_COMM_DUP   ( MPI_Comm *, MPI_Comm *, int * );

int  MPI_Comm_dup(MPI_Comm *comm, MPI_Comm *newcomm, int *ierr)
{
      VolPEx_Comm_dup(*comm, newcomm);
	*ierr = 0;
      return MPI_SUCCESS;
}
#else
int  MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
      VolPEx_Comm_dup(comm, newcomm);
      return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_comm_split_   = MPI_Comm_split
#pragma weak mpi_comm_split__  = MPI_Comm_split
#pragma weak MPI_COMM_SPLIT    = MPI_Comm_split

int MPI_Comm_split   ( MPI_Comm *, int *, int *, MPI_Comm *, int * );
int mpi_comm_split_  ( MPI_Comm *, int *, int *, MPI_Comm *, int * );
int mpi_comm_split__ ( MPI_Comm *, int *, int *, MPI_Comm *, int * );
int MPI_COMM_SPLIT   ( MPI_Comm *, int *, int *, MPI_Comm *, int * );

int  MPI_Comm_split(MPI_Comm *comm, int *color, int *key, MPI_Comm *newcomm, int *ierr)
{
	VolPEx_Comm_split(*comm, *color, *key, newcomm);
	*ierr = 0;
	return MPI_SUCCESS;
}
#else
int  MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
	VolPEx_Comm_split(comm, color, key, newcomm);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_gather_   = MPI_Gather
#pragma weak mpi_gather__  = MPI_Gather
#pragma weak MPI_GATHER    = MPI_Gather

int MPI_Gather   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, int * );
int MPI_gather_  (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, int * );
int MPI_gather__ (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, int * );
int MPI_GATHER   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, int * );

int  MPI_Gather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt, MPI_Datatype recvtype, int root, MPI_Comm comm, int *ierr)
{
	VolPEx_Gather(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
	*ierr = 0;
	return MPI_SUCCESS;
}
#else
int  MPI_Gather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
	VolPEx_Gather(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
	return MPI_SUCCESS;
}
#endif

#ifdef VOLPEXFORTRAN
#pragma weak mpi_allgather_   = MPI_Allgather
#pragma weak mpi_allgather__  = MPI_Allgather
#pragma weak MPI_ALLGATHER    = MPI_Allgather

int MPI_Allgather   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm, int * );
int MPI_allgather_  (void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm, int * );
int MPI_allgather__ (void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm, int * );
int MPI_ALLGATHER   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm, int * );

int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, int *ierr)
{
	VolPEx_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
	*ierr = 0;
	return MPI_SUCCESS;
}
#else
int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
	VolPEx_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
	return MPI_SUCCESS;
}
#endif

