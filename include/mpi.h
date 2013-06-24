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
#include "MCFA.h"
#include "mpi_fortran.h"

#define TOTAL_NODES	2000
#define TOTAL_COMMS	20
#define SENDBUFSIZE     20000
#define REQLISTSIZE     20000
#define TAGLISTSIZE	1000
#define CK_TAG        -15000
#define BARRIER_TAG   -45000
#define BCAST_TAG     -60000
#define REDUCE_TAG    -40000
#define ALLTOALL_TAG  -55000
#define GATHER_TAG    -50000
#define CK_LEN  (int)(5*sizeof(int))
#define MAX_MSG 1	
#define MAX_MSG_TIME 10 
#define MAX_MSG_REPEAT 30000000000
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

struct volpex_proc{
  int 	      id;
  int 	   SL_id;
  char     *hostname;
  int 	    port;
  char 	rank[16];  /* fullrank of a process in MPI_COMM_WORLD */
  int       rank_MCW;      
  char      level; 	
  int 	   state;
  int 	numofmsg;
  int 	  msgnum;
  int 	reuseval;
  struct volpex_proclist *plist; 	
  int 	recvpost;
  struct volpex_msg_perf	*msgperf;
  struct volpex_net_perf    *netperf;
  struct volpex_cancel_request		*purgelist;
  struct volpex_target_info *target_info;
};

struct volpex_cancel_request{
  int id;
  SL_msg_header	cancel_request;
  struct volpex_cancel_request *next;
};
typedef struct volpex_cancel_request Volpex_cancel_request;

struct volpex_msg_perf {
  struct volpex_msg_perf   *fwd;
  struct volpex_msg_perf  *back;
  int                    msglen;
  double                   time;
  int                       pos;
};
typedef struct volpex_msg_perf Volpex_msg_perf;

struct volpex_net_perf {
  struct volpex_net_perf   *fwd;
  struct volpex_net_perf  *back;
  double              bandwidth;
  double                latency;
  int                       pos;
};
typedef struct volpex_net_perf Volpex_net_perf;


struct volpex_target_info {
  struct volpex_target_info  *fwd;
  struct volpex_target_info *back;
  int 			   pos;
  int 			  reuse;
  int 			myreuse;
  double			   time;
};
typedef struct volpex_target_info Volpex_target_info;


#define MSGPERF	3	
#define TOTALMSGCOMM	100000
#define SEND		0
#define RECV		1
#define TIMEOUT		10	

typedef struct volpex_proc Volpex_proc;

struct volpex_proclist{
  int 	 num;   // number of replicas for this rank
  int  	 *ids;  // Volpex_id of each replica.
  int 	*SL_ids;// SL_ids of each replica.
  int 	*recvpost;//if send is already posted or not
};

typedef struct volpex_proclist Volpex_proclist;

struct volpex_communicator{
  int 	 	     id;/* id of communicator*/
  int 	           size;/* number of processes in communicator*/
  int 		 myrank;/*rank of process*/
  int 	            pos;/* position of the communicator in the comm-array */ 
  Volpex_proclist	 *plist;/* number of replicas for each rank and their actual id */
};

typedef struct volpex_communicator Volpex_comm;

struct tag_reuse{
  int tag;
  int comm;
  int reuse_count;
};
typedef struct tag_reuse Tag_Reuse;

struct max_tag_reuse{
  struct volpex_msg_header *header;
  struct max_tag_reuse *next;
};
typedef struct max_tag_reuse Max_tag_reuse;

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

struct volpex_msg_header{
  int      len;
  int      src;
  int     dest;
  int      tag;
  int     comm;
  int    reuse;
  float timestamp;
};
typedef struct volpex_msg_header Volpex_msg_header;


struct volpex_returnheader{
  Volpex_msg_header header;
  int              target;
  int     id;
};

typedef struct volpex_returnheader Volpex_returnheader;

struct volpex_returnheaderlist{
  struct volpex_returnheaderlist *next;
  Volpex_returnheader rheader;
  
};

typedef struct volpex_returnheaderlist Volpex_returnheaderlist;


struct mpi_msg{
  struct mpi_msg *back;
  int counter;
  Volpex_msg_header *header;
  void *buffer;
  int numreqs;
  int *reqnumbers;
  struct mpi_msg *fwd;
};
typedef struct mpi_msg NODE;
typedef NODE *NODEPTR;

struct volpex_target_list{
  int numofmsg;
  int volpex_id;
  int target; 
};
typedef struct volpex_target_list Volpex_target_list;

struct request_list{
  int reqnumber;
  SL_Request request;
  Volpex_msg_header *header;
  Volpex_msg_header returnheader;
  int cktag;
  int target;
  int req_type;
  int in_use;
  int flag;
  int recv_status;
  int send_status;
  void *buffer;
  double time;
  int len;
  int *assoc_recv;
  int numtargets;
  int recv_dup_status;
  void *actualbuf;
  
  NODEPTR insrtbuf;
  
};
typedef struct request_list Request_List;
typedef int Volpex_dest_source_fnct ( int rank, int comm, char *myfullrank, int **target,
                                      int *numoftargets, int msglen, int msgtype );

extern SL_array_t *Volpex_proc_array;

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


#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif
  
  
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
  int  MPI_Test(MPI_Request *, int *flag, MPI_Status *);
  int  MPI_Waitany(int, MPI_Request *, int*, MPI_Status *);
  int  MPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
  int  MPI_Alltoallv(void *, int *, int *, MPI_Datatype, void *, int *, int *, MPI_Datatype, MPI_Comm);
  int  MPI_Comm_dup(MPI_Comm, MPI_Comm *);
  int  MPI_Comm_split(MPI_Comm, int, int, MPI_Comm *);
  int  MPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
  int  MPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
  int  MPI_Unpack( void* inbuf, int insize, int *position, void* outbuf, int outcount,
                   MPI_Datatype datatype, MPI_Comm comm);
  int  MPI_Pack(void* inbuf, int count, MPI_Datatype datatype, void* outbuf,
                int outsize, int *position, MPI_Comm comm);
  
  int  MPI_Type_size ( MPI_Datatype dat, int *size );
  double  MPI_Wtime(void);
  int MPI_rank(int *);
  
  /* Prototypes for the Profiling Interface */
  int  PMPI_Init(int *argc, char ***argv );
  int  PMPI_Finalize(void);
  int  PMPI_Comm_size(MPI_Comm, int *);
  int  PMPI_Comm_rank(MPI_Comm, int *);
  int  PMPI_Send(void *, int, MPI_Datatype, int, int, MPI_Comm);
  int  PMPI_Recv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
  int  PMPI_Bcast(void *, int, MPI_Datatype, int, MPI_Comm);
  int  PMPI_Reduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
  int  PMPI_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
  int  PMPI_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
  int  PMPI_Allreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
  int  PMPI_Barrier(MPI_Comm);
  int  PMPI_Abort(MPI_Comm, int);
  int  PMPI_Wait(MPI_Request *, MPI_Status *);
  int  PMPI_Waitall(int, MPI_Request *, MPI_Status *);
  int  PMPI_Test(MPI_Request *, int *flag, MPI_Status *);
  int  PMPI_Waitany(int, MPI_Request *, int*, MPI_Status *);
  int  PMPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
  int  PMPI_Alltoallv(void *, int *, int *, MPI_Datatype, void *, int *, int *, MPI_Datatype, MPI_Comm);
  int  PMPI_Comm_dup(MPI_Comm, MPI_Comm *);
  int  PMPI_Comm_split(MPI_Comm, int, int, MPI_Comm *);
  int  PMPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
  int  PMPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
  int  PMPI_Unpack( void* inbuf, int insize, int *position, void* outbuf, int outcount,
                    MPI_Datatype datatype, MPI_Comm comm);
  int  PMPI_Pack(void* inbuf, int count, MPI_Datatype datatype, void* outbuf,
                 int outsize, int *position, MPI_Comm comm);
  int  PMPI_Type_size ( MPI_Datatype dat, int *size );
  
#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


/* Prototypes of the Volpex Counterparts */
int  Volpex_Finalize(void);
int  Volpex_Comm_size(MPI_Comm, int *);
int  Volpex_Comm_rank(MPI_Comm, int *);

int  Volpex_progress(void);

int  Volpex_Send(void *, int, MPI_Datatype, int, int, MPI_Comm);
int  Volpex_Recv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);

int  Volpex_Bcast(void *, int, MPI_Datatype, int, MPI_Comm);
int  Volpex_Reduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);

int  Volpex_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int  Volpex_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int  Volpex_Irecv_ll(void *, int, int, int, MPI_Comm, MPI_Request *, int);
int  Volpex_Wait(MPI_Request *, MPI_Status *);

int  Volpex_Allreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
int  Volpex_Barrier(MPI_Comm);
int  Volpex_Redundancy_Barrier ( MPI_Comm, int);
int  Volpex_Abort(MPI_Comm, int);
int  Volpex_Waitall(int, MPI_Request *, MPI_Status *);
int  Volpex_Test(MPI_Request *, int *flag, MPI_Status *);
int  Volpex_Waitany(int, MPI_Request *, int*, MPI_Status *);
int  Volpex_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int  Volpex_Alltoallv(void *, int *, int *, MPI_Datatype, void *, int *, int *, MPI_Datatype, MPI_Comm);
int  Volpex_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
int  Volpex_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int  Volpex_Unpack( void* inbuf, int insize, int *position, void* outbuf, int outcount,
                    MPI_Datatype datatype, MPI_Comm comm);
int  Volpex_Pack(void* inbuf, int count, MPI_Datatype datatype, void* outbuf,
                 int outsize, int *position, MPI_Comm comm);
int  Volpex_Type_size ( MPI_Datatype dat, int *size );
int  Volpex_Comm_dup(MPI_Comm, MPI_Comm *);
int  Volpex_Comm_split(MPI_Comm, int, int, MPI_Comm *);

int  Volpex_Cancel_byReqnumber(int);
int  Volpex_tag_reuse_check(int, int, int, int);
void GM_tagreuse_init ( void);
void Volpex_reduce_ll(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm, int);
void GM_host_ip(void);
int  GM_print(int);
NODEPTR Volpex_send_buffer_init(void);
NODEPTR Volpex_send_buffer_insert(NODEPTR, Volpex_msg_header*, int *, void *);
NODEPTR Volpex_send_buffer_search(NODEPTR, Volpex_msg_header*, int *, int *, int*);

void Volpex_send_buffer_delete(void);
void Volpex_send_buffer_print(NODEPTR);
int  Volpex_get_len(int, MPI_Datatype);

Volpex_msg_header* Volpex_get_msg_header(int len, int src, int dest, int tag, int comm, int reuse);
Volpex_msg_header* Volpex_init_msg_header();
int Volpex_compare_msg_header(Volpex_msg_header* header1, Volpex_msg_header* header2, int *reuse);
int Volpex_compare_msg_progress(Volpex_msg_header* header1, Volpex_msg_header* header2, int *msgprogress );
void Volpex_print_msg_header ( Volpex_msg_header *header );

int Volpex_init_proc(int id, int SL_id, char *hostname, int port, char *rank);
int Volpex_proc_read_and_set();
int Volpex_proc_read_and_set();
int Volpex_get_procid_fullrank(char *fullrank);
Volpex_proc* Volpex_get_proc_byid(int id);
Volpex_proc* Volpex_get_proc_bySLid(int id);
int Volpex_proc_dump();
int Volpex_get_procrank(int id);
int Volpex_get_fullrank(char *myredrank);
int Volpex_get_rank();
int Volpex_count_numoftargets(int rank, int comm, char mylevel, int *ownteam);
int Volpex_dest_src_locator(int rank, int comm, char *myfullrank, int **target, int *numoftargets,int msglen, int msgtype);
int Volpex_set_state_not_connected(int target);
int Volpex_get_max_rank();

Volpex_msg_perf* Volpex_msg_performance_init();
void Volpex_msg_performance_insert(double time, int msglen, int src);
Volpex_net_perf* Volpex_net_performance_init();
void Volpex_net_performance_insert(double latency, double bandwidth, Volpex_proc *proc);
int Volpex_compare_perf(Volpex_proclist *plist,int pos);
int Volpex_dest_src_locator1(int rank, int comm, char *myfullrank, int **target, int *numoftargets, int msglen, int msgtype);
int Volpex_dest_src_locator2(int rank, int comm, char *myfullrank, int **target, int *numoftargets, int msglen, int msgtype);
int Volpex_compare_loglength(Volpex_proclist *plist);
int Volpex_insert_reuseval(int procid,int reuseval);

void GM_allocate_global_data ( void ); 
void GM_free_global_data (void);
int Volpex_buffer_remove_ref  ( NODEPTR elem,  int reqid );
void Volpex_request_update_counter ( int num );
int Volpex_request_clean ( int start, int red );
int Volpex_request_get_counter ( int red );


int Volpex_init_comm(int id, int size);
int Volpex_init_comm_world(int size);
int Volpex_init_comm_self(void);
int Volpex_get_plist_byrank(int rank, Volpex_comm *oldcomm, Volpex_proclist *plist);
int Volpex_add_proc(Volpex_proclist *proclist, int id);
Volpex_comm* Volpex_get_comm_byid(int id);
void Volpex_print_comm( int commid);
int Volpex_comm_copy(Volpex_comm* comm1, Volpex_comm* comm2);
int Volpex_proc_dumpall();
int Volpex_searchproc_comm(int commid, int procid);
int Volpex_init_procplist();

int Volpex_msg_performance_free(Volpex_proc *tproc);
int Volpex_net_performance_free(Volpex_proc *tproc);
int Volpex_free_proclist();
int Volpex_free_comm();

int Volpex_insert_purgelist(int proc, Request_List req, int id);
int Volpex_remove_purgelist(int proc, int reqid);
int Volpex_init_purgelist(Volpex_cancel_request **list);
int Volpex_change_target(int rank, int comm);

int Volpex_search_maxreuse(Max_tag_reuse *maxtagreuse,Volpex_msg_header header);
int Volpex_insert_maxreuse(Max_tag_reuse *maxtagreuse,Volpex_msg_header *header);
int Volpex_add_maxreuse(Max_tag_reuse **maxtagreuse, Volpex_msg_header *header);
int Volpex_init_maxreuse(Max_tag_reuse **maxtagreuse);

int Volpex_insert_returnheader(Volpex_returnheaderlist **returnheaderList, Volpex_msg_header header, int target);
int Volpex_init_returnheader( Volpex_returnheaderlist **returnheaderList);
Volpex_returnheaderlist* Volpex_remove_returnheader(Volpex_returnheaderlist **returnheaderList, int id);
int Volpex_free_returnheader();

void Volpex_set_recvpost(int commid,int procid);
int Volpex_check_recvpost(int commid,int procid);

int Volpex_init_targetlist();
int Volpex_print_targetlist();
Volpex_target_info* Volpex_target_info_init();
int Volpex_target_info_insert(double time, int reuse, int myreuse, int src);
int Volpex_compare_target_info(int target);
int Volpex_target_info_free(Volpex_proc *tproc);
void Volpex_print_target_info(int source);
void Volpex_print_alltarget_info();

void Volpex_free_targetlist();
int Volpex_free_request(int i);
int Volpex_set_newtarget(int newtarget, int rank, int comm);

int Volpex_get_volpexid(int SL_id);

void Volpex_print_procplist();
int Volpex_numoftargets(int rank, int comm, int target);
int Volpex_insert_comm_newproc(int rank, int procid);

int Volpex_init_send(int commid);
int Volpex_init_send_newproc(Volpex_comm *tcomm,int procid,int rank);
int Volpex_set_primarytarget(Volpex_proc *proc, int newtarget);
int Volpex_set_target();
int Volpex_Complete_Barrier ( MPI_Comm);
extern Global_Map **GM;
extern Tag_Reuse **sendtagreuse;
extern Tag_Reuse **recvtagreuse;
extern Hidden_Data *hdata;
extern Request_List *reqlist;

#endif	/* MPI_H_INCLUDED */
