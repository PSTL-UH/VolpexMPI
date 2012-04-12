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

extern SL_array_t *SL_proc_array;
extern SL_array_t *Volpex_proc_array;
extern SL_array_t *Volpex_comm_array;
extern int SL_this_procid;

extern NODEPTR head, insertpt, curr;

extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char *hostip;
extern char *hostname;
extern int next_avail_comm;
extern int request_counter;



#pragma weak mpi_init_   = mpi_init
#pragma weak mpi_init__  = mpi_init
#pragma weak MPI_INIT    = mpi_init

void mpi_init(int *ierr)
{
    int argc=1;
    char ***argv=NULL;

    *ierr = MPI_Init (&argc, argv);
    return;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_finalize_   = mpi_finalize
#pragma weak mpi_finalize__  = mpi_finalize
#pragma weak MPI_FINALIZE    = mpi_finalize

void mpi_finalize( int *ierr)
{
    *ierr = MPI_Finalize();
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_size_   = mpi_comm_size
#pragma weak mpi_comm_size__  = mpi_comm_size
#pragma weak MPI_COMM_SIZE    = mpi_comm_size


void mpi_comm_size( int *comm, int *size, int *ierr)
{
    *ierr = MPI_Comm_size(*comm, size);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_rank_   = mpi_comm_rank
#pragma weak mpi_comm_rank__  = mpi_comm_rank
#pragma weak MPI_COMM_RANK    = mpi_comm_rank

void mpi_comm_rank( int *comm, int *rank, int *ierr)
{
    *ierr = MPI_Comm_rank(*comm, rank);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_send_   = mpi_send
#pragma weak mpi_send__  = mpi_send
#pragma weak MPI_SEND    = mpi_send

void mpi_send(void *buf, int *count,  int *datatype, int *dest, int *tag, 
	      int *comm, int *ierr)
{
    *ierr = MPI_Send(buf, *count, *datatype, *dest, *tag, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_recv_   = mpi_recv
#pragma weak mpi_recv__  = mpi_recv
#pragma weak MPI_RECV    = mpi_recv

void mpi_recv(void *buf, int *count,  int *datatype, int *source, int *tag, 
	      int *comm, int *status, int *ierr)
{
    *ierr = MPI_Recv(buf, *count, *datatype, *source, *tag, *comm, (MPI_Status *)status);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_bcast_   = mpi_bcast
#pragma weak mpi_bcast__  = mpi_bcast
#pragma weak MPI_BCAST    = mpi_bcast

void mpi_bcast(void *buf, int *count,  int *datatype, int *root, 
	       int *comm, int *ierr)
{
    *ierr = MPI_Bcast(buf, *count, *datatype, *root, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_reduce_   = mpi_reduce
#pragma weak mpi_reduce__  = mpi_reduce
#pragma weak MPI_REDUCE    = mpi_reduce

void mpi_reduce(void *sendbuf, void *recvbuf, int *count,  int *datatype, 
	        int *op, int *root,  int *comm, int *ierr)
{
    *ierr = MPI_Reduce(sendbuf, recvbuf, *count, *datatype, *op, *root, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_allreduce_   = mpi_allreduce
#pragma weak mpi_allreduce__  = mpi_allreduce
#pragma weak MPI_ALLREDUCE    = mpi_allreduce

void mpi_allreduce(void *sendbuf, void *recvbuf, int *count,  int *datatype, 
		   int *op,  int *comm, int *ierr)
{
    *ierr = MPI_Allreduce(sendbuf, recvbuf, *count, *datatype, *op, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_isend_   = mpi_isend
#pragma weak mpi_isend__  = mpi_isend
#pragma weak MPI_ISEND    = mpi_isend

void mpi_isend(void *buf, int *count,  int *datatype, int *dest, int *tag, 
	       int *comm, int *request, int *ierr)
{
    *ierr = MPI_Isend(buf, *count, *datatype, *dest, *tag, *comm, request);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_irecv_   = mpi_irecv
#pragma weak mpi_irecv__  = mpi_irecv
#pragma weak MPI_IRECV    = mpi_irecv

void mpi_irecv(void *buf, int *count,  int *datatype, int *source, int *tag, 
	       int *comm, int *request, int *ierr)
{
    *ierr = MPI_Irecv(buf, *count, *datatype, *source, *tag, *comm, request);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_waitall_   = mpi_waitall
#pragma weak mpi_waitall__  = mpi_waitall
#pragma weak MPI_WAITALL    = mpi_waitall

void  mpi_waitall(int *count, int *request, int *status, int *ierr)
{
    *ierr = MPI_Waitall(*count, request, (MPI_Status *) status);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_waitany_   = mpi_waitany
#pragma weak mpi_waitany__  = mpi_waitany
#pragma weak MPI_WAITANY    = mpi_waitany

void  mpi_waitany (int *count, int *request, int *index, int *status, int *ierr)
{
    int c_index;

    *ierr = MPI_Waitany(*count, request, &c_index, (MPI_Status *)status);
    *index = c_index + 1; /* Fortran counts from one not from zero */
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_wait_   = mpi_wait
#pragma weak mpi_wait__  = mpi_wait
#pragma weak MPI_WAIT    = mpi_wait

void mpi_wait(int *request, int *status, int *ierr)
{
    *ierr = MPI_Wait(request, (MPI_Status *)status);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_test_   = mpi_test
#pragma weak mpi_test__  = mpi_test
#pragma weak MPI_TEST    = mpi_test

void mpi_test(int *request, int *flag, int *status, int *ierr)
{
    *ierr = MPI_Test(request, flag, (MPI_Status *) status);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_barrier_   = mpi_barrier
#pragma weak mpi_barrier__  = mpi_barrier
#pragma weak MPI_BARRIER    = mpi_barrier

void mpi_barrier( int *comm, int *ierr)
{
    *ierr = MPI_Barrier(*comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_abort_   = mpi_abort
#pragma weak mpi_abort__  = mpi_abort
#pragma weak MPI_ABORT    = mpi_abort

void mpi_abort( int *comm, int *errorcode, int *ierr)
{
    printf("Error %d occured on context_id %d. Aborting.\n", *errorcode, *comm);
    *ierr = 0;
    exit(*errorcode);
    return;
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
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_alltoall_   = mpi_alltoall
#pragma weak mpi_alltoall__  = mpi_alltoall
#pragma weak MPI_ALLTOALL    = mpi_alltoall

void  mpi_alltoall(void *sendbuf, int *sendcount,  int *sendtype, void *recvbuf, 
		  int *recvcount,  int *recvtype,  int *comm, int *ierr)
{
    *ierr = MPI_Alltoall(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, *recvtype, *comm);
    return;      
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_alltoallv_   = mpi_alltoallv
#pragma weak mpi_alltoallv__  = mpi_alltoallv
#pragma weak MPI_ALLTOALLV    = mpi_alltoallv

void mpi_alltoallv(void *sendbuf, int *sendcount, int *sdispls,  int *sendtype, 
		  void *recvbuf, int *recvcount, int *rdispls,  int *recvtype,
		   int *comm, int *ierr)
{
    *ierr = MPI_Alltoallv(sendbuf, sendcount, sdispls, *sendtype, recvbuf, recvcount, 
			     rdispls, *recvtype, *comm);
    return;      
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_dup_   = mpi_comm_dup
#pragma weak mpi_comm_dup__  = mpi_comm_dup
#pragma weak MPI_COMM_DUP    = mpi_comm_dup

void mpi_comm_dup( int *comm,  int *newcomm, int *ierr)
{
    *ierr = MPI_Comm_dup(*comm, newcomm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_comm_split_   = mpi_comm_split
#pragma weak mpi_comm_split__  = mpi_comm_split
#pragma weak MPI_COMM_SPLIT    = mpi_comm_split

void mpi_comm_split( int *comm, int *color, int *key,  int *newcomm, 
		   int *ierr)
{
    *ierr = MPI_Comm_split(*comm, *color, *key, newcomm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_gather_   = mpi_gather
#pragma weak mpi_gather__  = mpi_gather
#pragma weak MPI_GATHER    = mpi_gather

void mpi_gather(void *sendbuf, int *sendcnt,  int *sendtype, void *recvbuf, 
	       int *recvcnt,  int *recvtype, int *root,  int *comm, 
	       int *ierr)
{
    *ierr = MPI_Gather(sendbuf, *sendcnt, *sendtype, recvbuf, *recvcnt, *recvtype, *root, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_allgather_   = mpi_allgather
#pragma weak mpi_allgather__  = mpi_allgather
#pragma weak MPI_ALLGATHER    = mpi_allgather

void mpi_allgather(void *sendbuf, int *sendcount,  int *sendtype, void *recvbuf, 
		  int *recvcount,  int *recvtype,  int *comm, int *ierr)
{
    *ierr = MPI_Allgather(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, *recvtype, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_pack_   = mpi_pack
#pragma weak mpi_pack__  = mpi_pack
#pragma weak MPI_PACK    = mpi_pack

void mpi_pack(void *inbuf, int *count,  int * datatype, void *outbuf, 
		  int *outsize,  int *position,  int *comm, int *ierr)
{
    *ierr = MPI_Pack(inbuf, *count, *datatype, outbuf, *outsize, position, *comm);
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_unpack_   = mpi_unpack
#pragma weak mpi_unpack__  = mpi_unpack
#pragma weak MPI_UNPACK    = mpi_unpack

void mpi_unpack(void *inbuf, int *insize,  int *position, void *outbuf, 
		  int *outcount,  int *datatype,  int *comm, int *ierr)
{
    *ierr = MPI_Unpack(inbuf, *insize, position, outbuf, *outcount, *datatype, *comm);
    return;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak mpi_type_size_   = mpi_type_size
#pragma weak mpi_type_size__  = mpi_type_size
#pragma weak MPI_TYPE_SIZE    = mpi_type_size

void mpi_type_size ( int *datatype, int *size, int *ierr )
{

    *ierr = MPI_Type_size ( *datatype, size );
    return;
}

#pragma weak mpi_rank_   = mpi_rank
#pragma weak mpi_rank__  = mpi_rank
#pragma weak MPI_RANK    = mpi_rank

void mpi_rank(int *actualrank, int *ierr)
{
	*ierr = MPI_rank(actualrank);
	return;
}


