#include "SL.h"
#include "SL_array.h"
#include "SL_proc.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"

static int SL_msg_counter=0;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_fdset_lastused;

extern int SL_proc_establishing_connection;

extern SL_array_t *SL_proc_array;

int SL_magic=12345;

void SL_msg_progress ( void )
{
    SL_qitem *elem=NULL;
    SL_proc *dproc=NULL;
    int i, ret, nd=0;
    struct timeval tout;

    fd_set send_fdset;
    fd_set recv_fdset;

    send_fdset=SL_send_fdset;
    recv_fdset=SL_recv_fdset;

    tout.tv_sec=0;
    tout.tv_usec=500;
    
    /* reset the fdsets before the select call */
    nd = select ( SL_fdset_lastused+1,  &recv_fdset, &send_fdset, NULL, &tout );
    if ( nd > 0 ) {
	/* 
	** We always have to check for recvs, since we might have 
	** unexpected messages 
	*/
	for ( i=0; i<= SL_fdset_lastused+1; i++ ) {
	    if ( FD_ISSET ( i, &recv_fdset )) {
		dproc = SL_proc_get_byfd ( i );
		if ( NULL == dproc ) {
		    continue;
		}
		ret = dproc->recvfunc ( dproc, i );
		// FD_CLR ( dproc->sock, &recv_fdset );
		if ( ret == SL_MSG_DONE ) {
		    dproc->recvfunc = SL_msg_recv_newmsg;
		}
		else if ( ret == SL_MSG_STARTED ) {
		    dproc->recvfunc = SL_msg_recv_knownmsg;
		}
		else if ( ret == SL_MSG_CLOSED ) {
		    dproc->recvfunc = SL_msg_closed;
		    dproc->sendfunc = SL_msg_closed;
		}
		else if ( ret != SL_SUCCESS ) {
		    /* Handle the error code */ 
		    SL_proc_handle_error ( dproc, ret );
		}
	    }
	}
	
	for ( i=0; i<= SL_fdset_lastused+1; i++ ) {
	    if ( FD_ISSET ( i, &send_fdset )) {
		dproc = SL_proc_get_byfd ( i );
		if ( NULL == dproc ) {
		    continue;
		}

		elem = dproc->squeue->first;
		if ( NULL != elem ) {
		    ret = dproc->sendfunc ( dproc, i );
		    FD_CLR ( dproc->sock, &send_fdset );
		    if ( ret == SL_MSG_DONE ) {
			dproc->sendfunc = SL_msg_send_newmsg;
		    }
		    else if ( ret == SL_MSG_STARTED ) {
			dproc->sendfunc = SL_msg_send_knownmsg;
		    }
		    else if ( ret != SL_SUCCESS ) {
			/* Handle the error code */
			SL_proc_handle_error ( dproc, ret );
		    }
		}
	    }
	}
    }


    /* Handle the sitation where we wait for a very long time for 
       a process to connect */
    if ( SL_proc_establishing_connection > 0 ) {
	int listsize = SL_array_get_last ( SL_proc_array ) + 1;
	double current_tstamp = SL_Wtime();
	for ( i=0; i<listsize; i++ ) {
#ifdef MINGW
		dproc = (SL_proc*)SL_array_get_ptr_by_pos ( SL_proc_array, i );
#else
	     	dproc = (SL_proc*)SL_array_get_ptr_by_pos ( SL_proc_array, i );
#endif
	   
	    if ( NULL == dproc || dproc->timeout == SL_ACCEPT_INFINITE_TIME ) {
		continue;
	    }
	    if ( ((SL_PROC_ACCEPT == dproc->state)    ||
		  (SL_PROC_CONNECT == dproc->state )) &&
		 ((current_tstamp - dproc->connect_start_tstamp) > dproc->timeout) ){
		PRINTF(("Waiting for %lf secs for a connection from proc %d\n",
			(current_tstamp - dproc->connect_start_tstamp), dproc->id ));
		SL_proc_handle_error ( dproc, SL_ERR_PROC_UNREACHABLE);
	    }	      
	}
    }

    return;
}
    
SL_msg_header* SL_msg_get_header ( int cmd, int from, int to, int tag, int context,
				    int len )
{
    SL_msg_header *header;
    
    header = (SL_msg_header *) malloc ( sizeof(SL_msg_header ));
    if ( NULL == header ) {
	return NULL;
    }

    header->cmd     = cmd;
    header->from    = from;
    header->to      = to;
    header->tag     = tag;
    header->context = context;
    header->len     = len;
    header->id      = SL_msg_counter++;

    return header;
}

void SL_msg_header_dump ( SL_msg_header *header )
{
    PRINTF(("header: cmd %d from %d to %d tag %d context %d len %d id %d\n",
	    header->cmd, header->from, header->to, header->tag, 
	    header->context, header->len, header->id));

    return;
}

int SL_msg_recv_knownmsg ( SL_proc *dproc, int fd )
{
    int len, ret=SL_SUCCESS;
    SL_qitem* elem=dproc->currecvelem;

    /* 
    ** We know, that header has been read already, and only
    ** the second element of the iov is of interest 
    */
    ret = SL_socket_read_nb ( fd, ((char *)elem->iov[1].iov_base + elem->lenpos), 
			      elem->iov[1].iov_len - elem->lenpos, &len );
    if ( SL_SUCCESS == ret) {
	elem->lenpos += len ;
	PRINTF (("SL_msg_recv_knownmsg: read %d bytes from %d\n", 
		 len, dproc->id ));
	
	if ( elem->lenpos == elem->iov[1].iov_len ) {
	    if ( NULL != elem->move_to  ) {
		SL_msgq_move ( dproc->rqueue, elem->move_to, elem );
	    }
	    dproc->currecvelem = NULL;
	    ret = SL_MSG_DONE;
	}
    }

    return ret;
}

int SL_msg_recv_newmsg ( SL_proc *dproc, int fd )
{
    SL_msg_header tmpheader, *header=NULL;
    SL_qitem *elem=NULL;
    int len, ret = SL_MSG_STARTED;

    /* Sequence:
    ** - read header
    ** - check expected message queue for first match
    ** - if a match is found :
    **   + recv the first data fragment 
    **   + if last fragment, 
    **      # mv to CRMQ
    **   + else 
    **      # mv the according element to the head of 
    **        the RMQ list
    **      # increase pos accordingly
    ** - else 	   
    **   + generate unexpected message queue entry 
    **   + read the second fragment
    */
    ret = SL_socket_read_nb ( fd, (char *) &tmpheader, sizeof(SL_msg_header), &len);
    if ( SL_SUCCESS != ret ) {
	return ret;
    }

    if ( 0 == len ) {
	return SL_MSG_DONE;
    }
    if ( len < sizeof (SL_msg_header ) ) {
	int tlen = (sizeof(SL_msg_header) - len);
	char *tbuf = ((char *) &tmpheader) + len ;
	ret = SL_socket_read ( fd, tbuf, tlen, dproc->timeout );
	if ( SL_SUCCESS != ret ) {
	    return ret;
	}
    }


    PRINTF (("SL_msg_recv_newmsg: read header from %d expected from %d\n", 
	    tmpheader.from, dproc->id ));

    if ( SL_MSG_CMD_CLOSE == tmpheader.cmd) {
	PRINTF(("Got CLOSE request from  proc %d\n", tmpheader.from ));
	if ( tmpheader.from != dproc->id ) {
	    PRINTF((" Connection management mixed up? %d %d\n", 
		    tmpheader.from, dproc->id ));
	    SL_proc_dumpall ();
	    return SL_ERR_GENERAL;
	}

	SL_socket_write ( dproc->sock, (char *) &tmpheader, sizeof  (SL_msg_header), 
	    dproc->timeout );
	PRINTF(("Sending CLOSE reply to proc %d\n", dproc->id ));
	SL_socket_close ( dproc->sock );

	FD_CLR ( dproc->sock, &SL_send_fdset );
	FD_CLR ( dproc->sock, &SL_recv_fdset );
	dproc->state = SL_PROC_NOT_CONNECTED;
	dproc->sock  = -1;

	return SL_MSG_CLOSED;
    }

    if ( tmpheader.from != dproc->id ) {
	printf("Recv_newmsg: got a message from the wrong process. Expected: %d[fd=%d]" 
	       "is from %d[fd=%d]\n",dproc->id, dproc->sock, tmpheader.from, fd );
	SL_proc_dumpall();
	SL_msg_header_dump(&tmpheader);
    }


    elem = SL_msgq_head_check ( dproc->rqueue, &tmpheader );
    if ( NULL != elem ) {
	/* 
	** update header checking for ANY_SOURCE, ANY_TAG,
	** and ANY_CONTEXT. For ANY_SOURCE, also remove 
	** all the entries of the other procs 
	*/
	header = (SL_msg_header *) elem->iov[0].iov_base;
	header->tag     = tmpheader.tag;
	header->context = tmpheader.context;
	if ( header->from == SL_ANY_SOURCE ) {
	    header->from = tmpheader.from;
	    //	SL_proc_remove_anysource ( header->id, header->from );
	}

	/* Need to adjust the length of the message and of the iov vector*/
	elem->iov[1].iov_len = tmpheader.len;
	header->len          = tmpheader.len;
    }
    else {
	char *tbuf=NULL;
	SL_msg_header *thead=NULL;


	thead = (SL_msg_header *) malloc ( sizeof ( SL_msg_header ) );
	if ( NULL == thead ) {
	    return SL_ERR_NO_MEMORY;
	}
	memcpy ( thead, &tmpheader, sizeof ( SL_msg_header ));

	PRINTF(("SL_msg_recv_newmsg: add element to the unexpected message queue."));
	SL_msg_header_dump ( thead );

	if ( tmpheader.len > 0 ) {
	    tbuf  = (char *) malloc ( tmpheader.len );
	    if ( NULL == tbuf ) {
		return SL_ERR_NO_MEMORY;
	    }
	}
	else {
	    tbuf=NULL;
	}
		 
	elem = SL_msgq_insert ( dproc->urqueue, thead, tbuf, NULL );
#ifdef QPRINTF
	SL_msgq_head_debug ( dproc->urqueue );
#endif	
	if ( tmpheader.len == 0 ) {
	    return SL_MSG_DONE;
	}
    }

    elem->lenpos = 0;
    elem->iovpos = 1;

    ret = SL_socket_read_nb ( fd, elem->iov[1].iov_base, elem->iov[1].iov_len, &len);
    if ( SL_SUCCESS == ret ) {
	elem->lenpos += len ;
	PRINTF (("SL_msg_recv_newmsg: read %d bytes from %d\n", 
		 len, dproc->id ));

	if ( elem->lenpos == elem->iov[1].iov_len ) {
	    if ( NULL != elem->move_to  ) {
		SL_msgq_move ( dproc->rqueue, elem->move_to, elem );
	    }
	    dproc->currecvelem = NULL;
	    ret = SL_MSG_DONE;
	}
	else {
	    /* SL_msgq_move_tohead ( dproc->rqueue, elem ); */
	    dproc->currecvelem = elem;
	    ret = SL_MSG_STARTED;
	}
    }
	
    return ret;
}

int SL_msg_send_knownmsg ( SL_proc *dproc, int fd )
{
    int ret=SL_SUCCESS;
    int len;
    SL_qitem *elem=dproc->cursendelem;
    int iovpos = elem->iovpos;
    int lenpos = elem->lenpos;

    ret = SL_socket_write_nb ( fd, ((char *)elem->iov[iovpos].iov_base + lenpos), 
			       (elem->iov[iovpos].iov_len - lenpos), &len );
    if ( SL_SUCCESS == ret ) {
	elem->lenpos += len;
	PRINTF (("SL_msg_send_knownmsg: wrote %d bytes to %d\n", 
		 len, dproc->id ));
	
	if ( 0 == elem->iovpos  && elem->lenpos == elem->iov[0].iov_len ) {
	    elem->iovpos = 1;
	    elem->lenpos = 0;
	}
	
	if ( 1 == elem->iovpos && elem->iov[1].iov_len == elem->lenpos   &&
	     NULL != elem->move_to ) {
	    SL_msgq_move ( dproc->squeue, elem->move_to, elem );
#ifdef QPRINTF
	    if ( NULL != elem->move_to ) {
		SL_msgq_head_debug ( elem->move_to );
	    }
#endif	
	    ret = SL_MSG_DONE;
	}
    }

    return ret;
}

int SL_msg_send_newmsg ( SL_proc *dproc, int fd )
{
    int ret=SL_MSG_STARTED;
    int len;
    SL_qitem *elem=dproc->squeue->first;

    ret = SL_socket_write ( fd, elem->iov[0].iov_base, elem->iov[0].iov_len, dproc->timeout );
    if ( ret != SL_SUCCESS ) {
	return ret;
    }

    PRINTF (("SL_msg_send_newmsg: wrote header to %d\n", 
	     dproc->id ));
    elem->iovpos = 1;
    elem->lenpos = 0;
    
    ret = SL_socket_write_nb (fd, elem->iov[1].iov_base, elem->iov[1].iov_len, &len );
    if ( SL_SUCCESS == ret) {
	PRINTF (("SL_msg_send_newmsg: wrote %d bytes to %d\n", 
		 len, dproc->id ));
	elem->lenpos += len;
	dproc->cursendelem = elem;
	
	if ( elem->iov[1].iov_len == elem->lenpos   &&
	     NULL != elem->move_to ) {
	    SL_msgq_move ( dproc->squeue, elem->move_to, elem );
	    dproc->cursendelem = NULL;
	    ret = SL_MSG_DONE;
#ifdef QPRINTF
	    if ( NULL != elem->move_to ) {
		SL_msgq_head_debug ( elem->move_to );
	    }
#endif	
	}
    }
    
    return ret;
}

int SL_msg_accept_newconn ( SL_proc *dproc, int fd )
{
    	/* This function will be registered with the according send/recv function pointer
       and is the counter part of the non-blocking connect. */
    	int tmp=0;
    	int sd=0;
    	int ret=SL_SUCCESS;

    	sd = accept ( dproc->sock, 0, 0 );
    	if ( sd > 0 ) {
		PRINTF (("SL_msg_accept_newconn: established new connection %d errno=%d %s\n", 
		 sd, errno, strerror(errno)));

		PRINTF (("SL_msg_accept_newconn: Trying handshake \n"));
		ret = SL_socket_write ( sd, (char *) &SL_this_procid, sizeof(int), SL_ACCEPT_MAX_TIME);
		if ( SL_SUCCESS != ret ) {
	    		return ret;
		}
		ret = SL_socket_read ( sd, (char *) &tmp, sizeof(int), SL_ACCEPT_MAX_TIME );
		if ( SL_SUCCESS != ret ) {
	    		return ret;
		}
	
		SL_configure_socket_nb ( sd );
		if ( tmp != dproc->id ) {
#ifdef MINGW
			SL_proc * tproc = (SL_proc*)SL_array_get_ptr_by_id ( SL_proc_array, tmp );
#else
	    		SL_proc * tproc = (SL_proc*)SL_array_get_ptr_by_id ( SL_proc_array, tmp );
#endif
	    		if ( NULL == tproc ) {
				PRINTF(("accept_newconn: received connection request from "
				"unkown proc %d\n", tmp ));
				return SL_ERR_GENERAL;
			}
	    		SL_proc_set_connection ( tproc, sd );
		}
		else {
	    		SL_proc_set_connection ( dproc, sd );
	    		ret = SL_MSG_DONE;
		}
		SL_proc_dumpall ();
    	}
    	else {
#ifdef MINGW
      if ( errno != WSAEWOULDBLOCK  ||
	     errno != WSAECONNABORTED ||
	     errno != WSAEINTR       ||
	     errno != WSAEINTR ) {
#else
	if ( EWOULDBLOCK != errno  ||
	     ECONNABORTED!= errno  ||
	     EPROTO      != errno  ||
	     EINTR       != errno ) {
#endif
	    ret = errno;
	}
    }

    return ret;

}


int SL_msg_connect_newconn ( SL_proc *dproc, int fd )
{
  /* This function will be registered with the according send/recv function pointer
     and is the counter part of the non-blocking connect. */
 
    if ( dproc->state != SL_PROC_CONNECTED ) {
	int ret=SL_SUCCESS;
	int tmp=0;
	int terr=0;
	socklen_t len=0;
	
#ifdef MINGW
	char *winflag;
	sprintf(winflag, "%d", terr);
	getsockopt (dproc->sock, SOL_SOCKET, SO_ERROR, winflag, &len);
      if (terr == WSAEINPROGRESS || terr == WSAEWOULDBLOCK ) {
#else
	getsockopt (dproc->sock, SOL_SOCKET, SO_ERROR, &terr, &len);
	if ( EINPROGRESS == terr || EWOULDBLOCK == terr) {
#endif

	    /* connection not yet established , go on */
	    return ret;
	}
	if ( 0 != terr ) {
	    FD_CLR ( dproc->sock, &SL_send_fdset );
	    FD_CLR ( dproc->sock, &SL_recv_fdset );
	    
	    PRINTF (("SL_msg_connect_newconn: reconnecting %d %s \n", terr, 
		     strerror ( terr ) ));
	    SL_socket_close ( dproc->sock );
	    ret = SL_proc_init_conn_nb ( dproc, dproc->timeout );
	    return ret;
	}
	else if ( terr == 0 ) {
	    PRINTF (("SL_msg_connect_newconn: terr = 0. Trying handshake \n"));
	    dproc->state = SL_PROC_CONNECTED;
	    
	    ret = SL_socket_read ( dproc->sock, ( char *) &tmp, sizeof(int), 
				   dproc->timeout);
	    if ( SL_ERR_PROC_UNREACHABLE == ret ) {
		FD_CLR ( dproc->sock, &SL_send_fdset );
		FD_CLR ( dproc->sock, &SL_recv_fdset );
		
		PRINTF (("SL_msg_connect_newconn: connection timed out %d %lf \n", ret, 
			 dproc->timeout));
		SL_socket_close ( dproc->sock );
		return ret;
	    }		
	    else if ( SL_SUCCESS != ret ) {
		FD_CLR ( dproc->sock, &SL_send_fdset );
		FD_CLR ( dproc->sock, &SL_recv_fdset );
		
		PRINTF (("SL_msg_connect_newconn: reconnecting %d %s \n", ret, 
			 strerror ( ret ) ));
		SL_socket_close ( dproc->sock );
		ret = SL_proc_init_conn_nb ( dproc, dproc->timeout );
		return ret;
	    }
	    
	    if ( tmp != dproc->id ) {
		PRINTF (("SL_msg_connect_newconn: error in exchanging handshake\n"));
	    }
	    ret = SL_socket_write ( dproc->sock, (char *) &SL_this_procid, sizeof(int), 
				    dproc->timeout );
	    if ( SL_SUCCESS != ret ) {
		return ret;
	    }
	    
	    SL_proc_dumpall ();
	    return SL_MSG_DONE;
	}
	
	return ret;
    }
    
    return SL_MSG_DONE;
}

int SL_msg_closed ( SL_proc *dproc, int fd )
{
    PRINTF(("SL_msg_closed: connection to %d is marked as closed\n", dproc->id ));
    return SL_MSG_CLOSED;
}

void SL_msg_set_nullstatus ( SL_Status *status )
{
    if ( NULL != status && SL_STATUS_IGNORE != status ) {
        status->SL_SOURCE  = SL_PROC_NULL;
	status->SL_TAG     = SL_ANY_TAG;
	status->SL_ERROR   = SL_SUCCESS;
	status->SL_CONTEXT = SL_ANY_CONTEXT;
	status->SL_LEN     = 0;
    }

  return;
}
