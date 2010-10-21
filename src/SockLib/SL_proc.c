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

int SL_proc_init ( int proc_id, char *hostname, int port ) 
{
    SL_proc *tproc=NULL;
    int pos;
    int retval;
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

	retval = PAPI_library_init(PAPI_VER_CURRENT);
        if(retval != PAPI_VER_CURRENT)
        {
            printf("\nPAPI library init error!\n");
            exit(-1);
        }


    return SL_SUCCESS;
}

SL_msg_perf* SL_msg_performance_init()
{
	SL_msg_perf *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= PERFBUFSIZE; i++){
                node= (SL_msg_perf *)malloc(sizeof(SL_msg_perf));
                node->pos    = i;
		node->msglen = -1;
                node->time   = -1;
		node->msgtype = -1;
		node->elemid = -1;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
		}
                if(i > 0 && i < PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}

SL_network_perf* SL_network_performance_init()
{
        SL_network_perf *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= PERFBUFSIZE; i++){
                node= (SL_network_perf *)malloc(sizeof(SL_network_perf));
                node->pos   	 = i;
                node->latency	 = -1;
                node->bandwidth  = -1;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
                }
                if(i > 0 && i < PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}

	

void SL_msg_performance_insert(int msglen,double time, int msgtype, int elemid, SL_proc *proc)
{
	/** if pointer reaches to the end of buffer
 	**	Calculate latency and bandwidth using least square method
		using Hockney's formula 

		t(s) =  {l		if s<MTU
			{l + s/b	else

		where t = execution time
		      l = latency
		      s = msglength
		      b = bandwidth
	**	insert it into th circular buffer for that proc
	**/

	SL_msg_perf* tmppt = proc->msgperf;
	double latency = -1.0, bandwidth = -1.0;
	double total_time = 0.0;
	int total_len = 0;
	int i;
	int lcount = 0, bcount = 0;
	
	if (proc->msgperf->pos == PERFBUFSIZE){
		proc->msgperf = proc->msgperf->fwd;
		for(i=0; i<PERFBUFSIZE ; i++){
			tmppt = tmppt->fwd;
			if (tmppt->msglen < MTU){
				total_time = total_time + tmppt->time;
				lcount++;
			}
			else{
				total_len  = total_len  + tmppt->msglen;
				bcount++;
			}
		}	
	if (lcount != 0)
		latency = (total_time/(double)lcount)/1000000.0;

	if (bcount != 0)
		bandwidth = ((double)total_len/(double)(1024L*1024L))/((total_time/1000000.0)- latency);
	

	SL_network_performance_insert( latency, bandwidth,proc);
//	SL_print_msg_performance(proc->msgperf);
	
	}
	else if (msgtype == RECV){
		proc->msgperf->msglen = msglen;
		proc->msgperf->time   = time;
		proc->msgperf->msgtype= msgtype;
		proc->msgperf->elemid = elemid;
		proc->msgperf = proc->msgperf->fwd;

			
	}

/*		double c0, c1, cov00, cov01, cov11, chisq;
		xmsglen[i] = (double)tmppt->msglen/1024.0;
                ytime[i] = tmppt->time/1000000.0        ;
                
                gsl_fit_linear (ytime, 1, xmsglen, 1, PERFBUFSIZE,
                        &c0, &c1, &cov00, &cov01, &cov11,
                        &chisq);
                latency = c0;
                bandwidth = 1/c1;
*/
}

void SL_print_msg_performance(SL_msg_perf *insertpt)
{
	SL_msg_perf *temp;
	FILE *fp;
	
	int i;

	fp = fopen("tmp","wa");
	temp = insertpt;
	printf(" Pos   MSGLEN   TIME		MSGTYPE		\n");
              for(i=0; i<=PERFBUFSIZE ; i++){
                      printf("%d	%d	%g  			%d	\n",temp->pos,temp->msglen, 
				temp->time, temp->msgtype );
			if (i!=0){
				fprintf(fp,"%g %g\n",(double)(temp->msglen)/(double)(1024L),(double)(temp->time)/1000000);
			}
			temp = temp->fwd;
              }

	system("graph -T X -m 2 -C -W 0.001 -X x -Y y< tmp");
	fclose(fp);
}


void SL_network_performance_insert(double latency, double bandwidth, SL_proc *proc)
{
	if (proc->netperf->pos == PERFBUFSIZE){
//		SL_print_net_performance(proc->netperf);
//		proc->netperf = proc->netperf->fwd;

        }
	else{
	proc->netperf->latency   = latency;
	proc->netperf->bandwidth = bandwidth;
	proc->netperf = proc->netperf->fwd;
	}
}

void SL_print_net_performance(SL_network_perf *insertpt)
{
        SL_network_perf *temp;
	int i;
        temp = insertpt;
	printf("**************************************************************************\n");
	printf(" Pos	Latency		Bandwidth\n");
        for(i=0; i<=PERFBUFSIZE ; i++){
	        printf("%d  %g 		%g \n", temp->pos, temp->latency,temp->bandwidth);
		 temp = temp->fwd;
        }
	printf("**************************************************************************\n");
	


}

int SL_net_performance_free(SL_proc *tproc)
{
        SL_network_perf *tnode = NULL;

        while (tproc->netperf!=NULL){
                tnode = tproc->netperf;
                if(tnode->pos != PERFBUFSIZE){
                        tproc->netperf = tproc->netperf->fwd;
                        free(tnode);
                }
                else
                break;
        }
        return SL_SUCCESS;

}

int SL_msg_performance_free(SL_proc *tproc)
{
        SL_msg_perf *tnode = NULL;

        while (tproc->netperf!=NULL){
                tnode = tproc->msgperf;
                if(tnode->pos != PERFBUFSIZE){
                        tproc->msgperf = tproc->msgperf->fwd;
                        free(tnode);
                }
                else
                break;
        }
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
	dproc->sock        = SL_this_listensock;

	SL_init_eventq();


//        SL_proc_dumpall();
	
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
	SL_socket_write ( proc->sock, (char *) header, sizeof  (SL_msg_header), 1);
//			  proc->timeout );
	PRINTF(("[%d]:Sending CLOSE request to proc %d\n", SL_this_procid,proc->id ));
	SL_socket_read  ( proc->sock, (char *) &header2, sizeof ( SL_msg_header), 1);
//			  proc->timeout);
	PRINTF(("[%d]:Got CLOSE reply from  proc %d\n",SL_this_procid, proc->id ));
    }
    else {
	SL_socket_read  ( proc->sock, (char *) &header2, sizeof ( SL_msg_header), 1);
//			  proc->timeout );
	PRINTF(("[%d]:Got CLOSE request from  proc %d\n", SL_this_procid,proc->id ));
	SL_socket_write ( proc->sock, (char *) header, sizeof  (SL_msg_header), 1);
//			  proc->timeout );
	PRINTF(("[%d]:Sending CLOSE reply to proc %d\n", SL_this_procid,proc->id ));
    }
    FD_CLR ( proc->sock, &SL_send_fdset );
    FD_CLR ( proc->sock, &SL_recv_fdset );
    proc->state = SL_PROC_NOT_CONNECTED;

    if ( proc->sock > 0 ) {
	SL_socket_close ( proc->sock );
    }

    proc->sock = -1;

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

    if(proc->state == SL_PROC_NOT_CONNECTED)
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
    if(timeout != SL_ACCEPT_INFINITE_TIME){
	if ( timeout < proc->timeout ||
	     proc->timeout == SL_ACCEPT_INFINITE_TIME ) {
	    proc->timeout = timeout;
	}
	SL_proc_establishing_connection++;
    }
	PRINTF(("[%d]:SL_proc_init_conn_nb: Into function for process :%d\n",
		SL_this_procid,proc->id));

//    SL_proc_dumpall();
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
    PRINTF(("[%d]:SL_proc_set_connection: connection established to proc %d on sock %d\n",
	    SL_this_procid, dproc->id, sd ));
    
    
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
	printf("[%d]:id:%d, port:%d, hostname:%s, state:%d\n",SL_this_procid, proc->id,proc->port, proc->hostname, proc->state);
}
    return;
}


void SL_proc_handle_error ( SL_proc* proc, int err, int flag )
{
    int check_message_queues=0; /* false */
    SL_Request req;
    SL_event_msg_header *header;
    
   
	if ((flag == TRUE) && (SL_this_procid != SL_EVENT_MANAGER)){
		printf("[%d]:Handling Event Error %d for proc %d\n",SL_this_procid, err, proc->id);
		sleep(1);
		header = (SL_event_msg_header*)malloc(sizeof(SL_event_msg_header));
		header->cmd = SL_CMD_DELETE_PROC;
		header->procid = proc->id;
		header->id = SL_this_procid;
		SL_event_post(header,sizeof(SL_event_msg_header),SL_EVENT_MANAGER, 0,0,&req );
		
	}
	
	
    
    printf("[%d]:Handling Error %d for proc %d\n", SL_this_procid,err, proc->id);

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
	static int id = -1;
	if (flag == 0)
		return ++id;
	else
		return --id;
}

int SL_proc_port_generate()
{
        static int port = 45001;
        return port++;
}

double SL_papi_time()
{
	double time;

	 time = ((double)PAPI_get_real_usec());

	return time;
}
