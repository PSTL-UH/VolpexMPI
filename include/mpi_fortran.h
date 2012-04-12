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
#ifndef __MPI_FORTRAN__
#define __MPI_FORTRAN__

/* This file contains the prototypes for the Fortran to C interfaces. */

void mpi_init   ( int * );
void mpi_init_  ( int * );
void mpi_init__ ( int * );
void MPI_INIT   ( int * );

void mpi_finalize   ( int * );
void mpi_finalize_  ( int * );
void mpi_finalize__ ( int * );
void MPI_FINALIZE   ( int * );

void mpi_comm_size   (  int *, int *, int * );
void mpi_comm_size_  (  int *, int *, int * );
void mpi_comm_size__ (  int *, int *, int * );
void MPI_COMM_SIZE   (  int *, int *, int * );

void mpi_comm_rank   (  int *, int *, int * );
void mpi_comm_rank_  (  int *, int *, int * );
void mpi_comm_rank__ (  int *, int *, int * );
void MPI_COMM_RANK   (  int *, int *, int * );

void mpi_send   (void *, int *,  int *, int *, int *,  int *, int * );
void mpi_send_  (void *, int *,  int *, int *, int *,  int *, int * );
void mpi_send__ (void *, int *,  int *, int *, int *,  int *, int * );
void MPI_SEND   (void *, int *,  int *, int *, int *,  int *, int * );

void mpi_recv   (void *, int *,  int *, int *, int *,  int *, int *, int * );
void mpi_recv_  (void *, int *,  int *, int *, int *,  int *, int *, int * );
void mpi_recv__ (void *, int *,  int *, int *, int *,  int *, int *, int * );
void MPI_RECV   (void *, int *,  int *, int *, int *,  int *, int *, int * );

void mpi_bcast   (void *, int *,  int *, int *,  int *, int * );
void mpi_bcast_  (void *, int *,  int *, int *,  int *, int * );
void mpi_bcast__ (void *, int *,  int *, int *,  int *, int * );
void MPI_BCAST   (void *, int *,  int *, int *,  int *, int * );

void mpi_reduce   (void *, void *, int *,  int *,  int *, int *,  int *, int *);
void mpi_reduce_  (void *, void *, int *,  int *,  int *, int *,  int *, int *);
void mpi_reduce__ (void *, void *, int *,  int *,  int *, int *,  int *, int *);
void MPI_REDUCE   (void *, void *, int *,  int *,  int *, int *,  int *, int *);

void mpi_allreduce   (void *, void *, int *,  int *,  int *,  int *, int *);
void mpi_allreduce_  (void *, void *, int *,  int *,  int *,  int *, int *);
void mpi_allreduce__ (void *, void *, int *,  int *,  int *,  int *, int *);
void MPI_ALLREDUCE   (void *, void *, int *,  int *,  int *,  int *, int *);

void mpi_isend   (void *, int *,  int *, int *, int *,  int *, int *, int * );
void mpi_isend_  (void *, int *,  int *, int *, int *,  int *, int *, int * );
void mpi_isend__ (void *, int *,  int *, int *, int *,  int *, int *, int * );
void MPI_ISEND   (void *, int *,  int *, int *, int *,  int *, int *, int * );

void mpi_irecv   (void *, int *,  int *, int *, int *,  int *, int *, int * );
void mpi_irecv_  (void *, int *,  int *, int *, int *,  int *, int *, int * );
void mpi_irecv__ (void *, int *,  int *, int *, int *,  int *, int *, int * );
void MPI_IRECV   (void *, int *,  int *, int *, int *,  int *, int *, int * );

void mpi_waitall   ( int *, int *, int *, int * );
void mpi_waitall_  ( int *, int *, int *, int * );
void mpi_waitall__ ( int *, int *, int *, int * );
void MPI_WAITALL   ( int *, int *, int *, int * );

void mpi_waitany   ( int *, int *, int *, int *, int * );
void mpi_waitany_  ( int *, int *, int *, int *, int * );
void mpi_waitany__ ( int *, int *, int *, int *, int * );
void MPI_WAITANY   ( int *, int *, int *, int *, int * );

void mpi_wait   ( int *, int *, int * );
void mpi_wait_  ( int *, int *, int * );
void mpi_wait__ ( int *, int *, int * );
void MPI_WAIT   ( int *, int *, int * );

void mpi_test   ( int *, int *, int *, int* );
void mpi_test_  ( int *, int *, int *, int* );
void mpi_test__ ( int *, int *, int *, int* );
void MPI_TEST   ( int *, int *, int *, int* );


void mpi_barrier   (  int *, int * );
void mpi_barrier_  (  int *, int * );
void mpi_barrier__ (  int *, int * );
void MPI_BARRIER   (  int *, int * );

void mpi_abort   (  int *, int *, int * );
void mpi_abort_  (  int *, int *, int * );
void mpi_abort__ (  int *, int *, int * );
void MPI_ABORT   (  int *, int *, int * );

double mpi_wtime   ( );
double mpi_wtime_  ( );
double mpi_wtime__ ( );
double MPI_WTIME   ( );

void  mpi_alltoall   ( void *, int *,  int *, void *, int *,  int *, 
       int *, int * );
void  mpi_alltoall_  ( void *, int *,  int *, void *, int *,  int *, 
		       int *, int * );
void  mpi_alltoall__ ( void *, int *,  int *, void *, int *,  int *, 
		       int *, int * );
void  MPI_ALLTOALL   ( void *, int *,  int *, void *, int *,  int *, 
		       int *, int * );

void  mpi_alltoallv   ( void *, int *, int *,  int *, void *, int *, int *, 
		        int *,  int *, int * );
void  mpi_alltoallv_  ( void *, int *, int *,  int *, void *, int *, int *, 
		        int *,  int *, int * );
void  mpi_alltoallv__ ( void *, int *, int *,  int *, void *, int *, int *, 
		        int *,  int *, int * );
void  MPI_ALLTOALLV   ( void *, int *, int *,  int *, void *, int *, int *, 
		        int *,  int *, int * );


void mpi_rank(int *, int *);
void mpi_rank_(int *, int *);
void mpi_rank__(int *, int *);
void MPI_RANK(int *, int *);


void mpi_comm_dup   (  int *,  int *, int * );
void mpi_comm_dup_  (  int *,  int *, int * );
void mpi_comm_dup__ (  int *,  int *, int * );
void MPI_COMM_DUP   (  int *,  int *, int * );

void mpi_comm_split   (  int *, int *, int *,  int *, int * );
void mpi_comm_split_  (  int *, int *, int *,  int *, int * );
void mpi_comm_split__ (  int *, int *, int *,  int *, int * );
void MPI_COMM_SPLIT   (  int *, int *, int *,  int *, int * );

void mpi_gather   (void *, int *,  int *, void *, int *,  int *, int *,  int *, int * );
void MPI_gather_  (void *, int *,  int *, void *, int *,  int *, int *,  int *, int * );
void MPI_gather__ (void *, int *,  int *, void *, int *,  int *, int *,  int *, int * );
void MPI_GATHER   (void *, int *,  int *, void *, int *,  int *, int *,  int *, int * );

void mpi_allgather   (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_allgather_  (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_allgather__ (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_ALLGATHER   (void *, int *,  int *, void *, int *,  int *,  int *, int * );

void mpi_pack   (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_pack_  (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_pack__ (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_PACK   (void *, int *,  int *, void *, int *,  int *,  int *, int * );

void mpi_unpack   (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_unpack_  (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_unpack__ (void *, int *,  int *, void *, int *,  int *,  int *, int * );
void MPI_UNPACK   (void *, int *,  int *, void *, int *,  int *,  int *, int * );


void mpi_type_size   ( int*, int*, int* );
void mpi_type_size_  ( int*, int*, int* );
void mpi_type_size__ ( int*, int*, int* );
void MPI_TYPE_SIZE   ( int*, int*, int* );


#endif

