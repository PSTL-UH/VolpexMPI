
#ifndef __SL_INTERNAL__
#define __SL_INTERNAL__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
//#include <gsl/gsl_fit.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
//#include <papi.h>
#ifdef MINGW
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <pwd.h>
#include <sys/utsname.h>
#endif


#include "SL_array.h"

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;

extern int SL_this_procid;
extern int SL_this_procport;
extern int SL_this_listensock;
extern int SL_numprocs;
extern int SL_init_numprocs;
extern int SL_proxy_numprocs;
/* Message header send before any message */
struct SL_msg_header {
    int      cmd; /* what type of message is this */
    int     from; /* id of the src process */
    int       to; /* id of the dest. process */
    int      tag; /* tag of the message */
    int  context; /* context id */
    int      len; /* Message length in bytes */
    int       id; /* Id of the last fragment */
    int loglength;
    int     temp;
};
typedef struct SL_msg_header SL_msg_header;


/* Process structure containing all relevant contact information,
   communication status etc. */
struct SL_proc;
typedef int SL_msg_comm_fnct ( struct SL_proc *dproc, int fd );

struct SL_proc {
    int                         id;
    char                 *hostname;
    int                       port;
    int                       sock;
    int                      state;
    int           connect_attempts; /* number of connect attempts */
    double    connect_start_tstamp; /* time stamp when we started to accept or connect
				       for this proc */
    double                 timeout; /* max time a process should wait before disconnecting */

    struct SL_msgq_head    *squeue; /* Send queue */
    struct SL_msgq_head    *rqueue; /* Recv queue */
    struct SL_msgq_head   *urqueue; /* Unexpected msgs queue */
    struct SL_msgq_head   *scqueue; /* Send complete queue */
    struct SL_msgq_head   *rcqueue; /* Recv complete queue */
    struct SL_qitem   *currecvelem;
    struct SL_qitem   *cursendelem;
    SL_msg_comm_fnct     *recvfunc;
    SL_msg_comm_fnct     *sendfunc;
    struct SL_msg_perf    *msgperf; /*to keep track of time and msglenth for each communication */
    struct SL_msg_perf   *insertpt;
    struct SL_network_perf *netperf;
};

struct SL_msg_perf {
	struct SL_msg_perf  *fwd;
	struct SL_msg_perf  *back;
	int 		   msglen;
	double		     time;
	int 		      pos;
	int 		  msgtype; /*send type(0) or recieve type(1)*/
	int 		   elemid;
        struct SL_proc      *proc;
};
typedef struct SL_msg_perf SL_msg_perf;

struct SL_network_perf {
	struct SL_network_perf	 *fwd;
	struct SL_network_perf	*back;
	double 	              latency;
	double 		    bandwidth;
	int 			  pos;

};
typedef struct SL_network_perf SL_network_perf;

typedef struct SL_proc SL_proc;


#ifdef MINGW
struct iovec {
	char	*iov_base;	/* Base address. */
	size_t	 iov_len;	/* Length. */
};

#define	TCP_MAXSEG		0x02	/* set maximum segment size */
#define 	F_GETFL		3	/* get file->f_flags */
#define 	F_SETFL		4	/* set file->f_flags */
#define 	O_NONBLOCK	 	00004
#endif

/* A  message queue item containing the operation
   it decsribes */
struct SL_msgq_head;
struct SL_qitem {
    int                       id;
    int                   iovpos;
    int                   lenpos;
    int                    error;
    struct iovec          iov[2];
    struct SL_msgq_head *move_to;
    struct SL_msgq_head    *head;
    struct SL_qitem        *next;
    struct SL_qitem        *prev;
    double             starttime;
    double               endtime;
};
typedef struct SL_qitem SL_qitem;

/* A message queue */
struct SL_msgq_head {
    int               count;
    char              *name;
    struct SL_qitem  *first;
    struct SL_qitem   *last;
};
typedef struct SL_msgq_head SL_msgq_head;

/* Request object identifying an ongoing communication */
struct SL_msg_request {
    struct SL_proc        *proc;
    int                    type; /* Send or Recv */
    int                      id;
    struct SL_qitem       *elem;
    struct SL_msgq_head *cqueue; /* completion queue to look for */
};
typedef struct SL_msg_request SL_msg_request;


struct SL_msgq_head     *SL_event_sendq;
struct SL_msgq_head     *SL_event_recvq;
struct SL_msgq_head     *SL_event_sendcq;


/* MACROS */
/*#ifdef PRINTF
  #undef PRINTF
  #define PRINTF(A) printf A 
#else
  #define PRINTF(A)
#endif*/

#define FALSE 0
#define TRUE  1

#define SEND 0
#define RECV 1

#define SL_RECONN_MAX      20
#define SL_ACCEPT_MAX_TIME 30
#define SL_READ_MAX_TIME    5
#define SL_ACCEPT_INFINITE_TIME -1
#define SL_BIND_PORTSHIFT 200
#define SL_SLEEP_TIME       1
#define SL_TCP_BUFFER_SIZE  262142
#define SL_MAX_EVENT_HANDLE     1
#define SL_CONSTANT_ID          -32
#define SL_EVENT_MANAGER        -1
#define SL_PROXY_SERVER		-2
#define PERFBUFSIZE	20
#define MTU		(1024L*4L)	
#define SL_PROC_ID	-64

int SL_socket ( void );
int SL_bind_static ( int handle, int port );
int SL_bind_dynamic (  int handle, int port );
int SL_socket_close ( int handle );
int SL_open_socket_conn ( int *handle, const char *as_host, int port );
int SL_open_socket_bind ( int *handle, int port );
int SL_open_socket_listen ( int sock );
int SL_open_socket_listen_nb ( int *handle, int port );
int SL_open_socket_conn_nb ( int *handle, const char *as_host, int port );
int SL_socket_read ( int hdl, char *buf, int num, double timeout );
int SL_socket_write ( int hdl, char *buf, int num, double timeout );
int SL_socket_write_nb ( int hdl, char *buf, int num, int *numwritten );

int SL_socket_read_nb ( int hdl, char *buf, int num, int* numread );

void SL_print_socket_options ( int fd );
void SL_configure_socket ( int sd );
void SL_configure_socket_nb ( int sd );
int SL_init_internal();
double SL_papi_time();

/* status object t.b.d */

#endif /* __SL_INTERNALL__ */

