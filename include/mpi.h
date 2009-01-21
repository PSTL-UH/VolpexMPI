#ifndef MPI_H_INCLUDED
#define MPI_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <complex.h>

#ifdef MINGW
#include <winsock2.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <pwd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <pthread.h>
#endif

#include "SL.h"

#define TOTAL_NODES	2000
#define TOTAL_COMMS	20
#define SENDBUFSIZE     500
#define REQLISTSIZE     10000
#define TAGLISTSIZE	10000

#define CK_TAG        -15000
#define BARRIER_TAG   -45000
#define BCAST_TAG     -60000
#define REDUCE_TAG    -40000
#define ALLTOALL_TAG  -55000
#define GATHER_TAG    -50000

#define CK_LEN  (int)(5*sizeof(int))

#define VOLPEX_PROC_CONNECTED     1
#define VOLPEX_PROC_NOT_CONNECTED 0

struct global_map{
      int id;
      char host[32];
      int port;
      char rank[16];
	  int state;
};
typedef struct global_map Global_Map;

struct tag_reuse{
	int tag;
	int reuse_count;
};
typedef struct tag_reuse Tag_Reuse;

struct cktag_reuse{
	int id;
	int cktag;
};
typedef struct cktag_reuse CkTag;

struct hidden_data{
    int mysize;
    int myrank;
    int mybarrier;     
};
typedef struct hidden_data Hidden_Data;

struct request_list{
    int reqnumber;
	SL_Request request;
	int header[5];
	int returnheader[5];
	int cktag;
	int target;
	int req_type;
	int in_use;
	int flag;
	int recv_status;
	int send_status;
	void *buffer;
};
typedef struct request_list Request_List;

struct mpi_msg{
	struct mpi_msg *back;
	int counter;
	int header[5];
	void *buffer;
	int reqnumbers[3];
   	struct mpi_msg *fwd;
};
typedef struct mpi_msg NODE;
typedef NODE *NODEPTR;

#define MPI_VERSION	1
#define MPI_SUBVERSION	2

#define MPI_BOTTOM	((MPI_Aint)0)
#define MPI_STATUS_IGNORE (MPI_Status *)0

typedef long		MPI_Aint;
typedef int	        MPI_Request;
typedef unsigned int	MPI_Group;
typedef unsigned int	MPI_Comm;
typedef unsigned int	MPI_Errhandler;
typedef unsigned int	MPI_Op;
typedef unsigned int	MPI_Datatype;

/* #define MPI_Request        SL_Request */
#define MPI_Status         SL_Status
#define MPI_SOURCE         SL_SOURCE
#define MPI_TAG            SL_TAG
#define MPI_ERROR          SL_ERROR

enum {
	MPI_COMM_NULL		= 0,
	MPI_COMM_WORLD		= 1,
	MPI_COMM_SELF		= 2
};

enum {
	MPI_ERRHANDLER_NULL	= 0,
	MPI_ERRORS_ARE_FATAL	= 1,
	MPI_ERRORS_RETURN	= 2
};

enum {
	MPI_GROUP_NULL		= 0,
	MPI_GROUP_EMPTY		= 1
};

enum {
        MPI_REQUEST_NULL	= -1
};

enum {
	MPI_OP_NULL		= 0,
	MPI_MAX			= 1,
	MPI_MIN			= 2,
	MPI_SUM			= 3,
	MPI_PROD		= 4,
	MPI_LAND		= 5,
	MPI_BAND 		= 6,
	MPI_LOR			= 7,
	MPI_BOR			= 8,
	MPI_LXOR		= 9,
	MPI_BXOR		= 10,
	MPI_MAXLOC		= 11,
	MPI_MINLOC		= 12
};

enum {
	MPI_DATATYPE_NULL	= 0,
	MPI_CHAR		= 1,
	MPI_SHORT		= 2,
	MPI_INT			= 3,
	MPI_LONG		= 4,
	MPI_UNSIGNED_CHAR	= 5,
	MPI_UNSIGNED_SHORT	= 6,
	MPI_UNSIGNED		= 7,
	MPI_UNSIGNED_LONG	= 8,
	MPI_FLOAT		= 9,
	MPI_DOUBLE		= 10,
	MPI_LONG_DOUBLE		= 11,
	MPI_LONG_LONG		= 12,
	MPI_INTEGER		= 13,
	MPI_REAL		= 14,
	MPI_DOUBLE_PRECISION	= 15,
	MPI_COMPLEX		= 16,
	MPI_DOUBLE_COMPLEX	= 17,
	MPI_LOGICAL		= 18,
	MPI_CHARACTER		= 19,
	MPI_INTEGER1		= 20,
	MPI_INTEGER2		= 21,
	MPI_INTEGER4		= 22,
	MPI_INTEGER8		= 23,
	MPI_REAL4		= 24,
	MPI_REAL8		= 25,
	MPI_REAL16		= 26,
	MPI_BYTE		= 27,
	MPI_PACKED		= 28,
	MPI_UB			= 29,
	MPI_LB			= 30,
	MPI_FLOAT_INT		= 31,
	MPI_DOUBLE_INT		= 32,
	MPI_LONG_INT		= 33,
	MPI_2INT		= 34,
	MPI_SHORT_INT		= 35,
	MPI_LONG_DOUBLE_INT	= 36,
	MPI_2REAL		= 37,
	MPI_2DOUBLE_PRECISION	= 38,
	MPI_2INTEGER		= 39
};

#define MPI_LONG_LONG_INT	MPI_LONG_LONG

enum {
	MPI_SUCCESS		= 0,
	MPI_ERR_BUFFER		= 1,
	MPI_ERR_COUNT		= 2,
	MPI_ERR_TYPE		= 3,
	MPI_ERR_TAG		= 4,
	MPI_ERR_COMM		= 5,
	MPI_ERR_RANK		= 6,
	MPI_ERR_REQUEST		= 7,
	MPI_ERR_ROOT		= 8,
	MPI_ERR_GROUP		= 9,
	MPI_ERR_OP		= 10,
	MPI_ERR_TOPOLOGY	= 11,
	MPI_ERR_DIMS		= 12,
	MPI_ERR_ARG		= 13,
	MPI_ERR_UNKNOWN		= 14,
	MPI_ERR_TRUNCATE	= 15,
	MPI_ERR_OTHER		= 16,
	MPI_ERR_INTERN		= 17,
	MPI_ERR_IN_STATUS	= 18,
	MPI_ERR_PENDING		= 19,
	MPI_ERR_LASTCODE	= 31
};

enum {
	MPI_KEYVAL_INVALID	= 0,
	MPI_TAG_UB		= 1,
	MPI_HOST		= 2,
	MPI_IO			= 3,
	MPI_WTIME_IS_GLOBAL	= 4
};

enum {
	MPI_IDENT		= 0,
	MPI_CONGRUENT		= 1,
	MPI_SIMILAR		= 2,
	MPI_UNEQUAL		= 3
};

enum {
	MPI_GRAPH		= 1,
	MPI_CART		= 2
};

enum {
	MPI_UNDEFINED		= -3,
	MPI_ANY_SOURCE		= -2,
	MPI_PROC_NULL		= -1
};

enum {
	MPI_ANY_TAG		= -1
};

enum {
	MPI_BSEND_OVERHEAD	= 32
};

enum {
	MPI_MAX_PROCESSOR_NAME	= 256
};

enum {
	MPI_MAX_ERROR_STRING	= 256
};

int  MPI_Init(int *argc, char ***argv );
int  MPI_Finalize(void);
int  MPI_Comm_size(MPI_Comm, int *);
int  MPI_Comm_rank(MPI_Comm, int *);
int  MPI_Send(void *, int, MPI_Datatype, int, int, MPI_Comm);
int  MPI_Recv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int  MPI_Bcast(void *, int, MPI_Datatype, int, MPI_Comm);
int  MPI_Reduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int  MPI_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int  MPI_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int  MPI_Wait(MPI_Request *, MPI_Status *);
int  MPI_Allreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
int  MPI_Barrier(MPI_Comm);
int  MPI_Abort(MPI_Comm, int);
int  MPI_Waitall(int, MPI_Request *, MPI_Status *);
int  MPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int  MPI_Alltoallv(void *, int *, int *, MPI_Datatype, void *, int *, int *, MPI_Datatype, MPI_Comm);
int  MPI_Comm_dup(MPI_Comm, MPI_Comm *);
int  MPI_Comm_split(MPI_Comm, int, int, MPI_Comm *);
int  MPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
int  MPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);

double  MPI_Wtime(void);
int  VolPEx_Finalize(void);
int  VolPEx_Comm_size(MPI_Comm, int *);
int  VolPEx_Comm_rank(MPI_Comm, int *);
int  VolPEx_progress(void);
int  VolPEx_Send(void *, int, MPI_Datatype, int, int, MPI_Comm);
int  VolPEx_Recv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int  VolPEx_Bcast(void *, int, MPI_Datatype, int, MPI_Comm);
int  VolPEx_Reduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int  VolPEx_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int  VolPEx_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int  VolPEx_Irecv_ll(void *, int, int, int, MPI_Comm, MPI_Request *, int);
int  VolPEx_Wait(MPI_Request *, MPI_Status *);
int  VolPEx_Allreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
int  VolPEx_Barrier(MPI_Comm);
int  VolPEx_Redundancy_Barrier ( MPI_Comm, int);
int  VolPEx_Abort(MPI_Comm, int);
int  VolPEx_Waitall(int, MPI_Request *, MPI_Status *);
int  VolPEx_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int  VolPEx_Alltoallv(void *, int *, int *, MPI_Datatype, void *, int *, int *, MPI_Datatype, MPI_Comm);
int  VolPEx_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
int  VolPEx_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int  VolPEx_Comm_dup(MPI_Comm, MPI_Comm *);
int  VolPEx_Comm_split(MPI_Comm, int, int, MPI_Comm *);
int  VolPEx_Cancel_byReqnumber(int);
int  tag_reuse_check(int, int);
void VolPEx_reduce_ll(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm, int);
void GM_host_ip();
int  GM_print(int);
int  GM_proc_read_and_set (void);
int  GM_get_fullrank(char *);
int  GM_get_procid_fullrank(char *);
int  GM_dest_src_locator(int, int, char *, int[]);
int  GM_set_state_not_connected(int);
NODEPTR send_buffer_init(void);
NODEPTR send_buffer_insert(NODEPTR, int[], int[], void *);
NODEPTR send_buffer_search(NODEPTR, int [], int *);
void send_buffer_delete(void);
void send_buffer_print(NODEPTR);
int  get_len(int, MPI_Datatype);

#endif	/* MPI_H_INCLUDED */
