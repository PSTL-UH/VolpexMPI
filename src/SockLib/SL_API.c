
#include "SL.h"
#include "SL_array.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"
#include "SL_proc.h"

extern SL_array_t *SL_proc_array;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;

extern int SL_this_procid;
extern int SL_this_procport;
extern int SL_this_listensock;
extern int SL_numprocs;


int SL_Init (void )
{
#ifdef MINGW
    char hostname[512];
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested,&wsaData)) 
    {
     	  printf("\nWSAStartup() failed");
     	  exit(1);
    }
#endif
    SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );


    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );

    /* 
    ** to do:
    ** - initialize the proc structures of all processes 
    ** - set SL_this_procid, SL_this_procport
    ** - for the my own proc structures mark the status 
    **   as SL_PROC_CONNECTED
    */

        SL_proc_read_and_set ( "SL.config");
	SL_init_internal();

    return SL_SUCCESS;
}

int SL_Finalize ()
{
    SL_proc_closeall();

    SL_array_free ( &(SL_proc_array ));
#ifdef MINGW
    WSACleanup();
#endif

    return SL_SUCCESS;
}

int SL_Get_id ( int context_id, int *rank )
{
    *rank = SL_this_procid;
    return SL_SUCCESS;
}

int SL_Get_size ( int context_id, int *size )
{

    *size = SL_numprocs;
    return SL_SUCCESS;
}

int SL_Send ( void *buf, int len, int dest, int tag, int context_id )
{
    SL_msg_request *req;
    double timeout = SL_ACCEPT_MAX_TIME;
    int ret;
    int loglength = -1;
    int reuse = -1;

    if ( SL_PROC_NULL == dest ) {
      return SL_SUCCESS;
    }


    ret = SL_send_post ( buf, len, dest, tag, context_id, timeout, loglength, reuse, &req );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Send - error in SL_post_send %d\n", SL_this_procid, ret );
	return ret;
    }

    ret = SL_wait ( &req, (SL_Status *)SL_STATUS_IGNORE );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Send - error in SL_wait %d\n", SL_this_procid, ret );
	return ret;
    }
    
    /* Initiate the chain of error handlers */
    return ret;
}

int SL_Isend ( void *buf, int len, int dest, int tag, int context_id, SL_Request *req )
{
    int ret;
    double timeout = SL_ACCEPT_MAX_TIME;
    int loglength = -1;
    int reuse = -1;

    *req = (SL_msg_request *) SL_REQUEST_NULL;

    if ( SL_PROC_NULL == dest ) {
        return SL_SUCCESS;
    }

    ret = SL_send_post ( buf, len, dest, tag, context_id, timeout, loglength, reuse, req );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Isend - error in SL_post_send to proc %d code %d\n", SL_this_procid, dest, ret );
	return ret;
    }
    
    /* Initiate the chain of error handlers */
    return ret;
}
    
int SL_Recv ( void *buf, int len, int src, int tag, int context_id, SL_Status *status )
{
    SL_msg_request *req;
    double timeout = SL_ACCEPT_MAX_TIME;
    int ret;

    if ( SL_PROC_NULL == src ) {
        return SL_SUCCESS;
    }

    ret = SL_recv_post ( buf, len, src, tag, context_id, timeout, &req );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Recv - error in SL_recv_post %d\n", SL_this_procid, ret );
	return ret;
    }

    ret = SL_wait ( &req, status );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Recv - error in SL_wait %d\n", SL_this_procid, ret );
	return ret;
    }

    /* Initiate the chain of error handlers */
    return ret;
}

int SL_Irecv ( void *buf, int len, int src, int tag, int context_id, 
	       SL_Request *req )
{
    int ret;
    double timeout = SL_ACCEPT_MAX_TIME;


    *req = (SL_msg_request *) SL_REQUEST_NULL;

    if ( SL_PROC_NULL == src ) {
        return SL_SUCCESS;
    }
    ret = SL_recv_post ( buf, len, src, tag, context_id, timeout, req );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Irecv - error in SL_irecv_post %d\n", SL_this_procid, ret );
	return ret;
    }
    /* Initiate the chain of error handlers */
    return ret;
}


int SL_Wait ( SL_Request *req, SL_Status *status )
{
    int ret;

    if ( SL_REQUEST_NULL == *req ) {
        SL_msg_set_nullstatus ( status );
        return SL_SUCCESS;
    }

    ret = SL_wait ( req, status );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Wait - error in SL_wait %d\n", SL_this_procid, ret );
	return ret;
    }

    /* Initiate the chain of error handlers */
    return ret;
}

int SL_Waitall ( int num, SL_Request *reqs, SL_Status *stats )
{
    int ret = 0;
    int i;

    for ( i=0; i<num; i++ ) { 
      if ( NULL != reqs[i] && SL_REQUEST_NULL != reqs[i] ) {
	  if ( NULL != stats && SL_STATUS_IGNORE != stats ) {
	      ret = SL_wait ( &(reqs[i]), &(stats[i]) );
	  }
	  else {
	      ret = SL_wait ( &(reqs[i]), (SL_Status *)SL_STATUS_IGNORE );
	  }
	  if ( SL_SUCCESS != ret ) {
	      printf("[%d]: SL_Waitall - error in SL_wait %d for reques %d\n", 
		     SL_this_procid, ret, i );
	      return ret;
	  }
      }
      else {
	  SL_msg_set_nullstatus ( &stats[i] );
      }
    }

    /* Initiate the chain of error handlers */
    return ret;
}

int SL_Test ( SL_Request *req, int *flag, SL_Status *status )
{
    int ret;

    if ( SL_REQUEST_NULL == *req ) {
        *flag = 1;
        SL_msg_set_nullstatus ( status );
        return SL_SUCCESS;
    }

    ret = SL_test ( req, flag, status );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Test - error in SL_test %d\n", SL_this_procid, ret );
	return ret;
    }

    /* Initiate the chain of error handlers */
    return ret;
}

int SL_Cancel ( SL_Request *req, int *flag ) 
{
    int ret;
    
    if ( SL_REQUEST_NULL == *req || NULL == *req ) {
	*flag = 1; /* true. no operations available anyway */
	return SL_SUCCESS;
    }
    
    
    ret = SL_cancel ( req, flag );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_Cancel - error in SL_cancel %d\n", SL_this_procid, ret );
	return ret;
    }

    /* Initiate the chain of error handlers */
    return ret;
}

    

double SL_Wtime (void)
{
    double sec=0.0;
    struct timeval tp;
    double psec=0.0;
    
    gettimeofday( &tp, NULL );
    sec = (double)tp.tv_sec;
    psec = ((double)tp.tv_usec)/((double)1000000.0);
    
    return (sec+psec);

}

int SL_Abort ( int context_id, int errcode )
{
    printf("%d: Error %d occured on context_id %d. Aborting.\n", 
	   SL_this_procid, errcode, context_id );
    exit ( errcode );
    return SL_SUCCESS;
}
