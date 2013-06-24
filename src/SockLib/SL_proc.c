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
#include "SL.h"
#include "SL_proc.h"
#include "SL_array.h"
#include "SL_msgqueue.h"
#include "SL_msg.h"
#include "SL_event_handling.h"

SL_array_t *SL_proc_array;
int SL_this_procid=0;
int SL_this_procport=25000;
int SL_this_listensock=-1;
int SL_numprocs=2;
int SL_proc_establishing_connection=0;
fd_set SL_send_fdset;
fd_set SL_recv_fdset;
int SL_fdset_lastused=0;
int SL_init_numprocs = 0;

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
  
  tproc->msgperf     = SL_msg_performance_init();
  tproc->netperf     = SL_network_performance_init();
  return SL_SUCCESS;
}


int SL_proc_finalize(SL_proc *proc)
{
  SL_net_performance_free(proc);
  SL_msg_performance_free(proc);
  SL_msgq_head_finalize(proc->squeue);
  SL_msgq_head_finalize(proc->rqueue);
  SL_msgq_head_finalize(proc->urqueue);
  SL_msgq_head_finalize(proc->scqueue);
  SL_msgq_head_finalize(proc->rcqueue);
  
  if (NULL != proc->hostname)
    free(proc->hostname);
  free(proc);
  
  return SL_SUCCESS;	
}


int SL_init_eventq()
{
  char name[32];
  sprintf(name, "SL_event_recvq");
  SL_event_recvq = SL_msgq_head_init ( name );
  return SL_SUCCESS;
}


int SL_init_internal()
{
  SL_proc *dproc=NULL;
  dproc = (SL_proc*)SL_array_get_ptr_by_id ( SL_proc_array, SL_this_procid );
  SL_this_procport = dproc->port;
  dproc->state = SL_PROC_CONNECTED;
  
  /* Open a listen socket for this process */
  SL_open_socket_listen_nb ( &SL_this_listensock, SL_this_procport );
  FD_SET ( SL_this_listensock, &SL_send_fdset );
  FD_SET ( SL_this_listensock, &SL_recv_fdset );
  if ( SL_this_listensock > SL_fdset_lastused ) {
    SL_fdset_lastused = SL_this_listensock;
  }
  
  dproc->recvfunc    = (SL_msg_comm_fnct *)SL_msg_accept_newconn;
  dproc->sendfunc    = (SL_msg_comm_fnct *)SL_msg_accept_newconn;
  PRINTF(("[%d]: Changing sock val from %d to %d\n", SL_this_procid, 
          dproc->sock,SL_this_listensock));
  dproc->sock        = SL_this_listensock;
  
  SL_init_eventq();
  return SL_SUCCESS;
}


int SL_finalize_eventq()
{
  SL_msgq_head_finalize(SL_event_recvq);
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
    dproc = (SL_proc *) SL_array_get_ptr_by_pos ( SL_proc_array, i );
	if ( -1 != dproc->sock && dproc->state == SL_PROC_CONNECTED && 
	     dproc->id != SL_this_procid ) {
      SL_proc_close   ( dproc );
	}
  }
  return;
}


void SL_proc_close ( SL_proc * proc )
{
  SL_msg_header *header, header2;
  
  header = SL_msg_get_header ( SL_MSG_CMD_CLOSE, SL_this_procid, proc->id, 0, 0, 0, -1, -1 );
  
  /* Implement a shutdown handshake in order to give all processes 
     the possibility to remove themselves from the fdsets correctly. 
  */
  if ( proc->id < SL_this_procid || proc->id == SL_EVENT_MANAGER) {
	SL_socket_write ( proc->sock, (char *) header, sizeof  (SL_msg_header), 2);
	PRINTF(("[%d]:Sending CLOSE request to proc %d\n", SL_this_procid,proc->id ));
  }
  else {
    SL_socket_read  ( proc->sock, (char *) &header2, sizeof ( SL_msg_header), 2);
    PRINTF(("[%d]:Got CLOSE request from  proc %d\n", SL_this_procid,proc->id ));
    SL_socket_write ( proc->sock, (char *) header, sizeof  (SL_msg_header), 2);
    PRINTF(("[%d]:Sending CLOSE reply to proc %d\n", SL_this_procid,proc->id ));
  }
  
  SL_socket_close ( proc->sock );
  FD_CLR ( proc->sock, &SL_send_fdset );
  FD_CLR ( proc->sock, &SL_recv_fdset );
  proc->state = SL_PROC_NOT_CONNECTED;
  
  SL_proc_finalize(proc);
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


int SL_compare_subnet(SL_proc *proc)
{
  SL_proc *dproc;
  int subnet1[4], subnet2[4];
  dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, SL_this_procid );
  
  sscanf(proc->hostname,"%d.%d.%d.%d",&subnet1[0],&subnet1[1],&subnet1[2],&subnet1[3]);
  sscanf(dproc->hostname,"%d.%d.%d.%d",&subnet2[0],&subnet2[1],&subnet2[2],&subnet2[3]);
  if(subnet1[0]==subnet2[0] && subnet1[1]== subnet2[1]){
    PRINTF(("[%d]:Same subnet Connect directly proc:%d %s:%s\n\n", 
			SL_this_procid, proc->id, proc->hostname, dproc->hostname));
    return 1;
  }
  else{
    PRINTF(("[%d]:different subnet Connect through proxy proc:%d %s:%s\n\n",
			SL_this_procid, proc->id, proc->hostname, dproc->hostname));
    return 0;
  }
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
  
  int ret = SL_SUCCESS;
  PRINTF(("[%d]:SL_proc_init_conn_nb: Into function for process :%d\n",
          SL_this_procid,proc->id));
  if ( proc->state == SL_PROC_UNREACHABLE ) {
	return SL_ERR_PROC_UNREACHABLE;
  }
  
  if(proc->state == SL_PROC_NOT_CONNECTED) {       
    if ( proc->id < SL_this_procid ) {
      ret = SL_open_socket_conn_nb ( &proc->sock, proc->hostname, proc->port );
      proc->sendfunc = ( SL_msg_comm_fnct *) SL_msg_connect_newconn;
      proc->recvfunc = ( SL_msg_comm_fnct *) SL_msg_connect_newconn;
      proc->state = SL_PROC_CONNECT;
      
      PRINTF(("[%d]:1SL_proc_init_conn_nb:Changing socket from to %d proc id:%d ret:%d\n",
              SL_this_procid,proc->sock,proc->id, ret));
      
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
      
      PRINTF(("[%d]Changing socket from %d to listen socket %d\n", 
              SL_this_procid, proc->sock, SL_this_listensock));
      proc->sock  = SL_this_listensock;
      proc->state = SL_PROC_ACCEPT;
    }
    if ( proc->connect_attempts == 0 ) {
      proc->connect_start_tstamp = SL_Wtime();
    }
    proc->timeout = timeout;	
  }
  proc->connect_attempts++;
  if(timeout != SL_ACCEPT_INFINITE_TIME){
	if ( timeout < proc->timeout ||
	     proc->timeout == SL_ACCEPT_INFINITE_TIME ) {
      proc->timeout = timeout;
	}
	SL_proc_establishing_connection++;
  }
  
  SL_proc_dumpall();
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
	printf ("[%d]:Could not open configuration file %s\n", SL_this_procid,filename );
	exit ( -1 );
  }
  fscanf ( fp, "%d", &SL_numprocs );
  fscanf ( fp, "%d", &red );
  PRINTF (("[%d]:SL_proc_read_and_set: number of processes: %d\n", 
           SL_this_procid,SL_numprocs ));
  
  for ( i=0; i< SL_numprocs; i++ ) {
	ret = fscanf ( fp, "%d %s %d", &rank, host, &port );
	if ( EOF == ret ) {
      printf("[%d]:Configuration file does not have the requested number of entries\n",
             SL_this_procid);
      exit ( -1 );
	}
	PRINTF (("[%d]:SL_proc_read_and_set: id %d host %s port %d\n", SL_this_procid,rank, 
             host, port ));
	SL_proc_init ( rank, host, port );
  }
  fclose ( fp );
  return SL_SUCCESS;
}


void SL_proc_set_connection ( SL_proc *dproc, int sd )
{
  PRINTF(("[%d]:SL_proc_set_connection: connection established to proc %d on sock %d time:%lf\n",
          SL_this_procid, dproc->id, sd, SL_Wtime()-dproc->connect_start_tstamp) );
  
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
  
  if(dproc->connect_attempts >0)    
    SL_proc_establishing_connection--;
  
  return;
}


void SL_proc_dumpall ( )
{
  SL_proc *proc;
  int i, size = SL_array_get_last ( SL_proc_array) + 1;
  
  for ( i=0; i<size; i++ ) {
	proc = (SL_proc*)SL_array_get_ptr_by_pos ( SL_proc_array, i );
	PRINTF(("[%d]:id:%d, port:%d, hostname:%s, state:%d sock:%d timeout:%f\n",SL_this_procid, 
            proc->id,proc->port, proc->hostname, proc->state, proc->sock, proc->timeout));
  }
  return;
}


void SL_proc_handle_error ( SL_proc* proc, int err, int flag )
{
  int check_message_queues=0; /* false */
  SL_Request req;
  SL_event_msg_header *header;
  
  if ((flag == TRUE) && (SL_this_procid != SL_EVENT_MANAGER)){
    PRINTF(("[%d]:Handling Event Error %d for proc %d\n",SL_this_procid, err, proc->id));
    if(proc->id == SL_EVENT_MANAGER){
      printf("[%d]: Server is dead no point to continue: Killing myself bye!!!\n", 
             SL_this_procid);
      exit(-1);
    }
    header = (SL_event_msg_header*)malloc(sizeof(SL_event_msg_header));
    header->cmd = SL_CMD_DELETE_PROC;
    header->procid = proc->id;
    header->id = SL_this_procid;
    SL_event_post(header,sizeof(SL_event_msg_header),SL_EVENT_MANAGER, 0,0,&req );
  }
  
  PRINTF(("[%d]:Handling Error %d for proc %d\n", SL_this_procid,err, proc->id));
  
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


int SL_proc_id_generate(int flag)
{
  static int id = -2;
  static int procid = -1;
  if (flag == 0)
    return ++procid;
  else if (flag == -1)
    procid--;
  else 
    return --id;
  
  return SL_SUCCESS;
}


int SL_proc_port_generate()
{
  static int port = 45001;
  return port++;
}


double SL_papi_time()
{
  double time;
  time = SL_Wtime();
  return time;
}
