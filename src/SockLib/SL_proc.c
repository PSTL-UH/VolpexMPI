#include "SL.h"
#include "SL_proc.h"
#include "SL_array.h"
#include "SL_msgqueue.h"
#include "SL_msg.h"

SL_array_t *SL_proc_array;
int SL_this_procid=0;
int SL_this_procport=25000;
int SL_this_listensock=-1;
int SL_numprocs=2;

int SL_proc_establishing_connection=0;

fd_set SL_send_fdset;
fd_set SL_recv_fdset;
int SL_fdset_lastused=0;

int SL_proc_init ( int proc_id, char *hostname, int port ) 
{
    SL_proc *tproc=NULL;
    int pos;
    char name[32];

    tproc = (SL_proc *) malloc (sizeof( SL_proc ));
    if ( NULL == tproc ) {
	return SL_ERR_NO_MEMORY;
    }

    tproc->id               = proc_id;
    tproc->hostname         = strdup ( hostname );
    tproc->port             = port;
    tproc->sock             = -1;
    tproc->state            = SL_PROC_NOT_CONNECTED;
    tproc->connect_attempts = 0;
    tproc->connect_start_tstamp  = 0;
    tproc->timeout          = SL_ACCEPT_MAX_TIME;

    sprintf(name, "Squeue to proc %d", proc_id);
    tproc->squeue  = SL_msgq_head_init ( name );

    sprintf(name, "Rqueue to proc %d", proc_id);
    tproc->rqueue  = SL_msgq_head_init ( name );

    sprintf(name, "URqueue to proc %d", proc_id);
    tproc->urqueue = SL_msgq_head_init ( name );

    sprintf(name, "SCqueue to proc %d", proc_id);
    tproc->scqueue = SL_msgq_head_init ( name );

    sprintf(name, "RCqueue to proc %d", proc_id);
    tproc->rcqueue = SL_msgq_head_init ( name );

    SL_array_get_next_free_pos ( SL_proc_array, &pos );
    SL_array_set_element ( SL_proc_array, pos, proc_id, tproc );

    tproc->currecvelem = NULL;
    tproc->cursendelem = NULL;
    tproc->recvfunc    = (SL_msg_comm_fnct *)SL_msg_recv_newmsg;
    tproc->sendfunc    = (SL_msg_comm_fnct *)SL_msg_send_newmsg;

    return SL_SUCCESS;
}

SL_proc*  SL_proc_get_byfd ( int fd )
{
    SL_proc *dproc=NULL, *proc=NULL;
    int i, size;
    
    size = SL_array_get_last ( SL_proc_array ) + 1;
    for ( i=0; i< size; i++ ) {
        proc = (SL_proc *) SL_array_get_ptr_by_pos ( SL_proc_array, i );
	if ( NULL == proc ) {
	    continue;
	}
	if ( fd == proc->sock ) {
	    dproc=proc;
	    break;
	}
    }

    return dproc;
}


void  SL_proc_closeall ( void )
{
    SL_proc *dproc=NULL;
    int i, size;
    
    size = SL_array_get_last ( SL_proc_array );
    for ( i=0; i<= size; i++ ) {
	if  ( i == SL_this_procid ) {
	    continue;
	}
        dproc = (SL_proc *) SL_array_get_ptr_by_pos ( SL_proc_array, i );
	if ( -1 != dproc->sock && dproc->state == SL_PROC_CONNECTED ) {
	    SL_proc_close   ( dproc );
	}
    }

    return;
}



void SL_proc_close ( SL_proc * proc )
{
    SL_msg_header *header, header2;

    header = SL_msg_get_header ( SL_MSG_CMD_CLOSE, SL_this_procid, proc->id, 0, 0, 0 );
    
    /* Implement a shutdown handshake in order to give all processes 
       the possibility to remove themselves from the fdsets correctly. 
    */
    if ( proc->id > SL_this_procid ) {
	SL_socket_write ( proc->sock, (char *) header, sizeof  (SL_msg_header), 
			  proc->timeout );
	PRINTF(("Sending CLOSE request to proc %d\n", proc->id ));
	SL_socket_read  ( proc->sock, (char *) &header2, sizeof ( SL_msg_header), 
			  proc->timeout);
	PRINTF(("Got CLOSE reply from  proc %d\n", proc->id ));
    }
    else {
	SL_socket_read  ( proc->sock, (char *) &header2, sizeof ( SL_msg_header), 
			  proc->timeout );
	PRINTF(("Got CLOSE request from  proc %d\n", proc->id ));
	SL_socket_write ( proc->sock, (char *) header, sizeof  (SL_msg_header), 
			  proc->timeout );
	PRINTF(("Sending CLOSE reply to proc %d\n", proc->id ));
    }
    FD_CLR ( proc->sock, &SL_send_fdset );
    FD_CLR ( proc->sock, &SL_recv_fdset );
    proc->state = SL_PROC_NOT_CONNECTED;

    if ( proc->sock > 0 ) {
	SL_socket_close ( proc->sock );
    }
    proc->sock = -1;

    free ( header );
    return;
}
    
    

int SL_proc_init_conn ( SL_proc * proc ) 
{


    if ( proc->state == SL_PROC_UNREACHABLE ) {
	return SL_ERR_PROC_UNREACHABLE;
    }

    if ( proc->id < SL_this_procid ) {
        SL_open_socket_conn ( &proc->sock, proc->hostname, proc->port );
	SL_configure_socket_nb ( proc->sock );
	proc->state = SL_PROC_CONNECTED;	
	proc->connect_attempts++;
    }
    else {
        int tmp_handle;
	SL_open_socket_bind ( &tmp_handle, SL_this_procport );
	proc->sock = SL_open_socket_listen  ( tmp_handle );
	SL_configure_socket_nb ( proc->sock );
	proc->state = SL_PROC_CONNECTED;
    }


    /* set the read and write fd sets */
    FD_SET ( proc->sock, &SL_send_fdset );
    FD_SET ( proc->sock, &SL_recv_fdset );
    if ( proc->sock > SL_fdset_lastused ) {
        SL_fdset_lastused = proc->sock;
    }


    return SL_SUCCESS;
}


int SL_proc_init_conn_nb ( SL_proc * proc, double timeout ) 
{
    /* Timeout management:
       - on first connection establishment attempt to a proc, which is
         characterized by the fact, that proc->state == SL_PROC_NOT_CONNECTED
	 we set the timeout to be whatever has been requested.
       - on subsequent connection establishment attempts, we allow to 
         add more constraining requests, e.g. if timeout was set 
	 to INFINITE, it can be overwritten by a finite timeout. 
	 The other way round is not allowed, since it would weaken
	 a stronger request.
    */

    if ( proc->state == SL_PROC_UNREACHABLE ) {
	return SL_ERR_PROC_UNREACHABLE;
    }

    if (proc->state == SL_PROC_NOT_CONNECTED)
    {       

	if ( proc->id < SL_this_procid ) {
	    SL_open_socket_conn_nb ( &proc->sock, proc->hostname, proc->port );
	    proc->sendfunc = ( SL_msg_comm_fnct *) SL_msg_connect_newconn;
	    proc->recvfunc = ( SL_msg_comm_fnct *) SL_msg_connect_newconn;
	    proc->state = SL_PROC_CONNECT;
	    
	    /* set the read and write fd sets */
	    FD_SET ( proc->sock, &SL_send_fdset );
	    FD_SET ( proc->sock, &SL_recv_fdset );
	    if ( proc->sock > SL_fdset_lastused ) {
		SL_fdset_lastused = proc->sock;
	    }
	    
	}
	else {
	    proc->sendfunc = ( SL_msg_comm_fnct *) SL_msg_accept_newconn;
	    proc->recvfunc = ( SL_msg_comm_fnct *) SL_msg_accept_newconn;
	    
	    proc->sock  = SL_this_listensock;
	    proc->state = SL_PROC_ACCEPT;
	}
	if ( proc->connect_attempts == 0 ) {
	    proc->connect_start_tstamp = SL_Wtime();
	}
	
	proc->timeout = timeout;
    }

    proc->connect_attempts++;
    if ( timeout != SL_ACCEPT_INFINITE_TIME ){
        if ( timeout < proc->timeout || 
	     proc->timeout == SL_ACCEPT_INFINITE_TIME ) {
	    proc->timeout = timeout;
	}
	SL_proc_establishing_connection++;
    }

    return SL_SUCCESS;
}

/* this is just a temporary routine to test things quickly */
int SL_proc_read_and_set ( char *filename )
{
    FILE *fp;
    int ret, i;
    char host[80];
    int port;
    int rank;
    int red;
    
    fp = fopen ( filename, "r" );
    if ( NULL == fp ) {
		printf ("Could not open configuration file %s\n", filename );
		exit ( -1 );
    }
    
    fscanf ( fp, "%d", &SL_numprocs );
    fscanf ( fp, "%d", &red );
    PRINTF (("SL_proc_read_and_set: number of processes: %d\n", SL_numprocs ));
    
    for ( i=0; i< SL_numprocs; i++ ) {
	ret = fscanf ( fp, "%d %s %d", &rank, host, &port );
	if ( EOF == ret ) {
	    printf("Configuration file does not have the requested number of entries\n");
	    exit ( -1 );
	}
	PRINTF (("SL_proc_read_and_set: id %d host %s port %d\n", rank, host, port ));
	SL_proc_init ( rank, host, port );
    }
    
    fclose ( fp );
    return SL_SUCCESS;
}

void SL_proc_set_connection ( SL_proc *dproc, int sd )
{
    PRINTF(("SL_proc_set_connection: connection established to proc %d on sock %d\n",
	    dproc->id, sd ));
    
    
    dproc->sock  = sd;
    dproc->state = SL_PROC_CONNECTED;
    SL_configure_socket_nb ( sd );
    if ( sd > SL_fdset_lastused ) {
	SL_fdset_lastused = sd;
    }
    
    FD_SET ( dproc->sock, &SL_send_fdset );
    FD_SET ( dproc->sock, &SL_recv_fdset );    
    
    dproc->recvfunc    = (SL_msg_comm_fnct *)SL_msg_recv_newmsg;
    dproc->sendfunc    = (SL_msg_comm_fnct *)SL_msg_send_newmsg;
    
    SL_proc_establishing_connection--;
    
    return;
}

void SL_proc_dumpall ( )
{
    SL_proc *proc;
    int i, size = SL_array_get_last ( SL_proc_array) + 1;

    for ( i=0; i<size; i++ ) {
	proc = (SL_proc*)SL_array_get_ptr_by_pos ( SL_proc_array, i );
	PRINTF(("proc %d host %s port %d sock %d state %d \n", 
		proc->id, proc->hostname, proc->port, proc->sock, 
		proc->state ));
    }

    return;
}


void SL_proc_handle_error ( SL_proc* proc, int err )
{
    int check_message_queues=0; /* false */

    PRINTF(("Handling Error %d for proc %d\n", err, proc->id));

    /* Step 1: clean up the state, socket and fdsets */
    if ( proc->state == SL_PROC_CONNECTED || proc->state == SL_PROC_CONNECT) {
	/* Connection was already established and we seemed to have lost
	   it again  or we are executing the connect() call and the other 
	   side has not reacted for a long time/ a given number of attempts 
	*/
	FD_CLR ( proc->sock, &SL_send_fdset );
	FD_CLR ( proc->sock, &SL_recv_fdset );
	proc->state = SL_PROC_UNREACHABLE;
	
	if ( proc->sock > 0 ) {
	    SL_socket_close ( proc->sock );
	}
	proc->sock = -1;

	check_message_queues = 1;
    }
    else if ( proc->state == SL_PROC_ACCEPT ) {
	/* not allowed to close the socket in this case, 
	   since its the general accept socket */
	proc->state = SL_PROC_UNREACHABLE;
	proc->sock = -1;	
	check_message_queues = 1;
    }	
    
    
    if ( check_message_queues ) {
	/* Step 2: if there are pending operations for this proc, 
	   move them to the according completion queues, but mark 
	   them with the according error 
	*/
	while ( NULL != proc->squeue->first ) {
	    SL_msgq_set_error ( proc->squeue->first, err );
	    SL_msgq_move ( proc->squeue, proc->scqueue, proc->squeue->first );
	}
	
	while ( NULL != proc->rqueue->first ) {
	    SL_msgq_set_error ( proc->rqueue->first, err );
	    SL_msgq_move ( proc->rqueue, proc->rcqueue, proc->rqueue->first );
	}

	while ( NULL !=proc->urqueue->first ) {
	    /* The receive does not know anything about these items. 
	       So we just dump the items in the unexpected receive queue.
	    */
	    SL_qitem *qt = proc->urqueue->first;
	    SL_msgq_remove ( proc->urqueue, qt );
	    free ( qt->iov[0].iov_base );
	    if ( qt->iov[1].iov_base != NULL ) {
		free ( qt->iov[1].iov_base );
	    }
	}
    }

    SL_proc_establishing_connection--;
    
    return;
}
