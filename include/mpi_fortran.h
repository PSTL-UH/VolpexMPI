#ifndef __MPI_FORTRAN__
#define __MPI_FORTRAN__

/* This file contains the prototypes for the Fortran to C interfaces. */

int mpi_init   ( int * );
int mpi_init_  ( int * );
int mpi_init__ ( int * );
int MPI_INIT   ( int * );

int mpi_finalize   ( int * );
int mpi_finalize_  ( int * );
int mpi_finalize__ ( int * );
int MPI_FINALIZE   ( int * );

int mpi_comm_size   ( unsigned int *, int *, int * );
int mpi_comm_size_  ( unsigned int *, int *, int * );
int mpi_comm_size__ ( unsigned int *, int *, int * );
int MPI_COMM_SIZE   ( unsigned int *, int *, int * );

int mpi_comm_rank   ( unsigned int *, int *, int * );
int mpi_comm_rank_  ( unsigned int *, int *, int * );
int mpi_comm_rank__ ( unsigned int *, int *, int * );
int MPI_COMM_RANK   ( unsigned int *, int *, int * );

int mpi_send   (void *, int *, unsigned int *, int *, int *, unsigned int *, int * );
int mpi_send_  (void *, int *, unsigned int *, int *, int *, unsigned int *, int * );
int mpi_send__ (void *, int *, unsigned int *, int *, int *, unsigned int *, int * );
int MPI_SEND   (void *, int *, unsigned int *, int *, int *, unsigned int *, int * );

int mpi_recv   (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int mpi_recv_  (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int mpi_recv__ (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int MPI_RECV   (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );

int mpi_bcast   (void *, int *, unsigned int *, int *, unsigned int *, int * );
int mpi_bcast_  (void *, int *, unsigned int *, int *, unsigned int *, int * );
int mpi_bcast__ (void *, int *, unsigned int *, int *, unsigned int *, int * );
int MPI_BCAST   (void *, int *, unsigned int *, int *, unsigned int *, int * );

int mpi_reduce   (void *, void *, int *, unsigned int *, unsigned int *, int *, unsigned int *, int *);
int mpi_reduce_  (void *, void *, int *, unsigned int *, unsigned int *, int *, unsigned int *, int *);
int mpi_reduce__ (void *, void *, int *, unsigned int *, unsigned int *, int *, unsigned int *, int *);
int MPI_REDUCE   (void *, void *, int *, unsigned int *, unsigned int *, int *, unsigned int *, int *);

int mpi_allreduce   (void *, void *, int *, unsigned int *, unsigned int *, unsigned int *, int *);
int mpi_allreduce_  (void *, void *, int *, unsigned int *, unsigned int *, unsigned int *, int *);
int mpi_allreduce__ (void *, void *, int *, unsigned int *, unsigned int *, unsigned int *, int *);
int MPI_ALLREDUCE   (void *, void *, int *, unsigned int *, unsigned int *, unsigned int *, int *);

int mpi_isend   (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int mpi_isend_  (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int mpi_isend__ (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int MPI_ISEND   (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );

int mpi_irecv   (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int mpi_irecv_  (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int mpi_irecv__ (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );
int MPI_IRECV   (void *, int *, unsigned int *, int *, int *, unsigned int *, int *, int * );

int mpi_Waitall   ( int *, int *, int *, int * );
int mpi_waitall_  ( int *, int *, int *, int * );
int mpi_waitall__ ( int *, int *, int *, int * );
int MPI_WAITALL   ( int *, int *, int *, int * );

int mpi_wait   ( int *, int *, int * );
int mpi_wait_  ( int *, int *, int * );
int mpi_wait__ ( int *, int *, int * );
int MPI_WAIT   ( int *, int *, int * );

int mpi_barrier   ( unsigned int *, int * );
int mpi_barrier_  ( unsigned int *, int * );
int mpi_barrier__ ( unsigned int *, int * );
int MPI_BARRIER   ( unsigned int *, int * );

int mpi_abort   ( unsigned int *, int *, int * );
int mpi_abort_  ( unsigned int *, int *, int * );
int mpi_abort__ ( unsigned int *, int *, int * );
int MPI_ABORT   ( unsigned int *, int *, int * );

double mpi_wtime   ( );
double mpi_wtime_  ( );
double mpi_wtime__ ( );
double MPI_WTIME   ( );

int  mpi_alltoall   ( void *, int *, unsigned int *, void *, int *, unsigned int *, 
		      unsigned int *, int * );
int  mpi_alltoall_  ( void *, int *, unsigned int *, void *, int *, unsigned int *, 
		      unsigned int *, int * );
int  mpi_alltoall__ ( void *, int *, unsigned int *, void *, int *, unsigned int *, 
		      unsigned int *, int * );
int  MPI_ALLTOALL   ( void *, int *, unsigned int *, void *, int *, unsigned int *, 
		      unsigned int *, int * );

int  mpi_alltoallv   ( void *, int *, int *, unsigned int *, void *, int *, int *, 
		       unsigned int *, unsigned int *, int * );
int  mpi_alltoallv_  ( void *, int *, int *, unsigned int *, void *, int *, int *, 
		       unsigned int *, unsigned int *, int * );
int  mpi_alltoallv__ ( void *, int *, int *, unsigned int *, void *, int *, int *, 
		       unsigned int *, unsigned int *, int * );
int  MPI_ALLTOALLV   ( void *, int *, int *, unsigned int *, void *, int *, int *, 
		       unsigned int *, unsigned int *, int * );

int mpi_comm_dup   ( unsigned int *, unsigned int *, int * );
int mpi_comm_dup_  ( unsigned int *, unsigned int *, int * );
int mpi_comm_dup__ ( unsigned int *, unsigned int *, int * );
int MPI_COMM_DUP   ( unsigned int *, unsigned int *, int * );

int mpi_comm_split   ( unsigned int *, int *, int *, unsigned int *, int * );
int mpi_comm_split_  ( unsigned int *, int *, int *, unsigned int *, int * );
int mpi_comm_split__ ( unsigned int *, int *, int *, unsigned int *, int * );
int MPI_COMM_SPLIT   ( unsigned int *, int *, int *, unsigned int *, int * );

int mpi_gather   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, unsigned int *, int * );
int MPI_gather_  (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, unsigned int *, int * );
int MPI_gather__ (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, unsigned int *, int * );
int MPI_GATHER   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, unsigned int *, int * );

int mpi_allgather   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, unsigned int *, int * );
int MPI_allgather_  (void *, int, MPI_Datatype, void *, int, MPI_Datatype, unsigned int *, int * );
int MPI_allgather__ (void *, int, MPI_Datatype, void *, int, MPI_Datatype, unsigned int *, int * );
int MPI_ALLGATHER   (void *, int, MPI_Datatype, void *, int, MPI_Datatype, unsigned int *, int * );




#endif

