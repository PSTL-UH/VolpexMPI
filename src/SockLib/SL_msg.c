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
#include "SL_array.h"
#include "SL_proc.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"
#include "SL_event_handling.h"

static int SL_msg_counter=0;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_fdset_lastused;

extern int SL_proc_establishing_connection;

extern SL_array_t *SL_proc_array;

int SL_magic=12345;
int SL_event_handle_counter = 0;
int SL_proxy_server_socket = -100;
fd_set send_fdset;
fd_set recv_fdset;
int proxymsg = -100;
int proxymsgsend = -100;
extern int SL_proxy_connected_procs;
void SL_msg_progress ( void )
{
    SL_qitem *elem=NULL;
    SL_proc *dproc=NULL;
    int i, ret, nd=0,j,size;
    struct timeval tout;

    static int SL_event_handle_counter = 0;
    static int event_msg_counter = 0;  
 

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
		if (i == SL_proxy_server_socket && proxymsg>=-2)
			dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, proxymsg );
		else			
			dproc = SL_proc_get_byfd ( i );
		
		if ( NULL == dproc ) {
		    continue;
		}

		ret = dproc->recvfunc ( dproc, i );
		if ( ret != SL_SUCCESS ) {
		    /* Handle the error code */ 
		    PRINTF(("[%d]Error handling for recvfdset dproc=%d ret=%d\n",SL_this_procid,dproc->id, ret));
		    if (dproc->id != SL_this_procid)
		    	SL_proc_handle_error ( dproc, ret,TRUE );
		}
	    }
	}
	
	for ( i=0; i<= SL_fdset_lastused+1; i++ ) {
	    if ( FD_ISSET ( i, &send_fdset )) {

		if (i == SL_proxy_server_socket && proxymsgsend>=-2){
			dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, proxymsgsend );
		}
		else if (i == SL_proxy_server_socket && proxymsgsend<0){
                	size = SL_array_get_last ( SL_proc_array ) + 1;
	                for(j=0;j<size;j++){
        	            dproc = (SL_proc *) SL_array_get_ptr_by_pos ( SL_proc_array, j );
                	    if (dproc->sock == SL_proxy_server_socket
                                        && dproc->squeue->first !=NULL && dproc->id != SL_PROXY_SERVER){
                        	    proxymsgsend = dproc->id;
                                    break;
                             }
                	}
        	}
		else
			dproc = SL_proc_get_byfd ( i );
		if ( NULL == dproc ) {
		    continue;
		}

		elem = dproc->squeue->first;
		if ( NULL != elem ) {
		    ret = dproc->sendfunc ( dproc, i );
		    if ( ret != SL_SUCCESS ) {
			/* Handle the error code */
			PRINTF(("[%d]Error handling for sendfdset dproc=%d\n",SL_this_procid,dproc->id));
			if (dproc->id != SL_this_procid)
				SL_proc_handle_error ( dproc, ret,TRUE );
		    }
		}
//		}
//		}
//		}






	    }
	}
    }


    /* Handle the sitation where we wait for a very long time for 
       a process to connect */
    if ( SL_proc_establishing_connection > 0 ) {
	int listsize = SL_array_get_last ( SL_proc_array ) + 1;
	double current_tstamp = SL_Wtime();
	for ( i=0; i<listsize; i++ ) {
	    dproc = (SL_proc*)SL_array_get_ptr_by_pos ( SL_proc_array, i );
	    if ( NULL == dproc || dproc->timeout == SL_ACCEPT_INFINITE_TIME ) {
		continue;
	    }
	    if ( ((SL_PROC_ACCEPT == dproc->state)    ||
		  (SL_PROC_CONNECT == dproc->state )) &&
		 ((current_tstamp - dproc->connect_start_tstamp) > dproc->timeout) && dproc->id != SL_this_procid ){
		if(SL_this_procid != SL_PROXY_SERVER && dproc->id != SL_EVENT_MANAGER)
			printf("[%d]:Waiting for %lf secs for a connection from proc %d state %d sock %d:%d\n",
				SL_this_procid, (current_tstamp - dproc->connect_start_tstamp), 
				dproc->id, dproc->state, dproc->sock, SL_proxy_server_socket );

/*			if (dproc->id != SL_EVENT_MANAGER && dproc->id != SL_PROXY_SERVER
                                && dproc->sock != SL_proxy_server_socket){
			        	
	                        proc  = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, SL_PROXY_SERVER );
				PRINTF(("[%d]: 1clearing socket:%d\n", SL_this_procid, dproc->sock));
                                FD_CLR ( dproc->sock, &SL_send_fdset );
                                FD_CLR ( dproc->sock, &SL_recv_fdset );
				if ( dproc->sock > 0 && dproc->sock != SL_this_listensock) {
         			   SL_socket_close ( dproc->sock );
        			}
			
				if(proc->state != SL_PROC_CONNECTED){
					dproc->sendfunc = ( SL_msg_comm_fnct *) SL_msg_connect_proxy;
        	                        dproc->recvfunc = ( SL_msg_comm_fnct *) SL_msg_connect_proxy;
                	                dproc->state = SL_PROC_CONNECT;
                        	        dproc->connect_start_tstamp = current_tstamp;
                //              	dproc->timeout+=dproc->timeout;
					SL_proc_init_conn_nb ( proc, proc->timeout );
					SL_proxy_server_socket = proc->sock;
				}
				else{
					dproc->state = SL_PROC_CONNECTED;
				        PRINTF(("[%d]:SL_msg_connect_proxy:already connected  to proc %d on sock %d\n",
                SL_this_procid, dproc->id, dproc->sock));
					FD_SET ( dproc->sock, &SL_send_fdset );
				        FD_SET ( dproc->sock, &SL_recv_fdset );

				        dproc->sendfunc = SL_msg_send_newmsg;
				        dproc->recvfunc = SL_msg_recv_newmsg;
				}

				dproc->sock = proc->sock;
			}
*/

/*			if (dproc->id != SL_EVENT_MANAGER && dproc->id != SL_PROXY_SERVER 
				&& dproc->sock != SL_proxy_server_socket){
				dproc->sendfunc = ( SL_msg_comm_fnct *) SL_msg_connect_proxy;
        	    		dproc->recvfunc = ( SL_msg_comm_fnct *) SL_msg_connect_proxy;
				dproc->state = SL_PROC_CONNECT;
				dproc->connect_start_tstamp = current_tstamp;
		//		dproc->timeout+=dproc->timeout;
				proc  = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, SL_PROXY_SERVER );
    				if(proc->state != SL_PROC_CONNECTED){
		       			SL_proc_init_conn_nb ( proc, proc->timeout );
				}
				if(dproc->sock >0 && dproc->sock != SL_this_listensock){
					PRINTF(("[%d]: 1clearing socket:%d\n", SL_this_procid, dproc->sock));
		      			FD_CLR ( dproc->sock, &SL_send_fdset );
       			 		FD_CLR ( dproc->sock, &SL_recv_fdset );
				}
				dproc->sock = proc->sock;
				if(proc->state == SL_PROC_CONNECTED){
					dproc->state = SL_PROC_CONNECTED;
					PRINTF(("[%d]: Setting socket:%d\n", SL_this_procid, dproc->sock));
//				        FD_SET ( dproc->sock, &SL_send_fdset );
//        				FD_SET ( dproc->sock, &SL_recv_fdset );

				        PRINTF(("[%d]:SL_msg_connect_proxy:already connected  to proc %d on sock %d\n",
                SL_this_procid, dproc->id, dproc->sock));

				        dproc->sendfunc = SL_msg_send_newmsg;
				        dproc->recvfunc = SL_msg_recv_newmsg;
				 	SL_proc_dumpall();	

				}


			}
*/
		
		else if(SL_this_procid != -2 && dproc->sock != SL_proxy_server_socket 
				&& dproc->id == SL_PROXY_SERVER)
		SL_proc_handle_error ( dproc, SL_ERR_PROC_UNREACHABLE,TRUE);
	    }	      
	}
    }
//#if 0 
    if(SL_this_procid != SL_EVENT_MANAGER)
    {	
	if(SL_event_handle_counter >= SL_MAX_EVENT_HANDLE)
	{
	    elem = SL_msgq_get_first(SL_event_recvq);
	    if ((NULL!=elem) && (elem->lenpos == elem->iov[1].iov_len))
	    {
	    	while(NULL != elem)
	    	{
			event_msg_counter++;
			PRINTF(("[%d]:Value of event counter  :%d",SL_this_procid,SL_event_handle_counter));
			SL_event_progress(elem);
			SL_msgq_remove(SL_event_recvq,elem);
			elem = SL_msgq_get_first(SL_event_recvq);
		}
		if (2 == event_msg_counter)
	    		SL_event_handle_counter = 0;
	    }
	}
	else
	    SL_event_handle_counter++;
    }
//#endif
    return;
}
    

SL_msg_header* SL_msg_get_header ( int cmd, int from, int to, int tag, int context,
                                    int len, int loglength, int temp )
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
    header->loglength= loglength;
    header->temp    = temp;

    return header;
}

void SL_msg_header_dump ( SL_msg_header *header )
{
    PRINTF(("[%d]:header: cmd %d from %d to %d tag %d context %d len %d id %d loglength %d\n",SL_this_procid,
	    header->cmd, header->from, header->to, header->tag, 
	    header->context, header->len, header->id, header->loglength));

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

    PRINTF(("[%d]: into recv_knownmsg\n",SL_this_procid));


	PRINTF(("[%d]: 2clearing socket:%d, prod->id:%d\n", SL_this_procid, fd, dproc->id));
	FD_CLR ( fd, &recv_fdset );

    ret = SL_socket_read_nb ( fd, ((char *)elem->iov[1].iov_base + elem->lenpos), 
			      elem->iov[1].iov_len - elem->lenpos, &len );
    if ( SL_SUCCESS == ret) {
	elem->lenpos += len ;
	PRINTF( ("[%d]:SL_msg_recv_knownmsg: read %d bytes from %d\n", SL_this_procid,
		 len, dproc->id ));
	
	if ( elem->lenpos == elem->iov[1].iov_len ) {
	    if ( NULL != elem->move_to  ) {
		if(dproc->rqueue->first != NULL){
               //         SL_msg_header_dump((SL_msg_header*)elem->iov[0].iov_base);
			SL_msgq_move ( dproc->rqueue, elem->move_to, elem );
			if(fd == SL_proxy_server_socket )
				proxymsg = -100;
			
		}
                else if (dproc->urqueue->first != NULL && SL_this_procid == SL_PROXY_SERVER && 
				dproc->id != SL_EVENT_MANAGER){
                        SL_proc *proc;
                        SL_msg_header *pheader;
//                        SL_msg_header_dump((SL_msg_header*)elem->iov[0].iov_base);
                        pheader = (SL_msg_header*)elem->iov[0].iov_base;
                        proc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array,pheader->to);
//                        elem->move_to = proc->scqueue;
                        elem->move_to = NULL;
                        SL_msgq_move ( dproc->urqueue, proc->squeue, elem );
		//	SL_msgq_delete(dproc->urqueue);
                }
	    }


	    dproc->currecvelem = NULL;
	    PRINTF(("[%d]: Setting proc:%d to SL_msg_recv_newmsg\n",SL_this_procid,dproc->id));
	    dproc->recvfunc = SL_msg_recv_newmsg;
	}
    }

    return ret;
}

int SL_msg_recv_newmsg ( SL_proc *dproc, int fd )
{
    SL_msg_header tmpheader, *header=NULL;
    SL_qitem *elem=NULL;
    int len, ret = SL_SUCCESS;
//ret = SL_MSG_STARTED;
    SL_msgq_head *r = NULL;

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

	PRINTF(("[%d]: 3clearing socket:%d\n", SL_this_procid, fd));
	FD_CLR ( fd, &recv_fdset );
    ret = SL_socket_read_nb ( fd, (char *) &tmpheader, sizeof(SL_msg_header), &len);
    if ( SL_SUCCESS != ret ) {
	return ret;
    }

   PRINTF(  ("[%d]:SL_msg_recv_newmsg: read header %d bytes from %d\n", SL_this_procid,
	     len, dproc->id ));

    if ( 0 == len ) {
        dproc->recvfunc = SL_msg_recv_newmsg;
	return SL_SUCCESS;
    }
    if ( len < sizeof (SL_msg_header ) ) {
	int tlen = (sizeof(SL_msg_header) - len);
	char *tbuf = ((char *) &tmpheader) + len ;
	PRINTF( ("[%d]:1SL_msg_recv_newmsg: read header %d bytes from %d \n", SL_this_procid,
             len, dproc->id  ));
	PRINTF(("[%d]:SL_msg_recv_newmsg: tmpheader:%p tbuf:%p\n",SL_this_procid,&tmpheader,tbuf));
	ret = SL_socket_read ( fd, tbuf, tlen, dproc->timeout );
	if ( SL_SUCCESS != ret ) {
	    return ret;
	}
	PRINTF(("[%d]:SL_msg_recv_newmsg: read header %d bytes from %d\n", SL_this_procid,
                 tlen, dproc->id ));
    }

    PRINTF (("[%d]:SL_msg_recv_newmsg: read header from %d expected from %d\n", SL_this_procid,
	    tmpheader.from, dproc->id ));

	if(dproc->sock == SL_proxy_server_socket  )
		proxymsg = dproc->id;

    SL_msg_header_dump(&tmpheader);

    if ( SL_MSG_CMD_CLOSE == tmpheader.cmd) {
	PRINTF(("[%d]:SL_msg_recv_newmsg:Got CLOSE request from  proc %d for proc %d\n", 
		SL_this_procid,tmpheader.from, tmpheader.to ));

	if ( tmpheader.from != dproc->id ) {
            PRINTF((" [%d]:Connection management mixed up? %d %d\n", SL_this_procid,
                    tmpheader.from, dproc->id ));

	    if(SL_this_procid>SL_EVENT_MANAGER)
                dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, tmpheader.from );
//          SL_proc_dumpall ();
//            return SL_ERR_GENERAL;
        }

	if(SL_this_procid == SL_PROXY_SERVER && dproc->id>SL_EVENT_MANAGER && 
			tmpheader.to != SL_PROXY_SERVER){
		dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array,tmpheader.to);
		SL_msg_header *closehead=NULL;
	        closehead = (SL_msg_header *) malloc ( sizeof ( SL_msg_header ) );
        	if ( NULL == closehead ) {
           	 return SL_ERR_NO_MEMORY;
        	}

        	memcpy ( closehead, &tmpheader, sizeof ( SL_msg_header ));
		SL_msgq_insert(dproc->squeue,closehead,NULL,NULL);

/*		SL_socket_write ( dproc->sock, (char *) &tmpheader, sizeof  (SL_msg_header),
            				dproc->timeout );
		
                PRINTF(("[%d]:SL_msg_recv_newmsg:Sending CLOSE reply to proc %d from proc %d\n", 
				SL_this_procid,dproc->id, tmpheader.from ));
*/
		return SL_SUCCESS;
	}

	if(SL_this_procid == SL_PROXY_SERVER && tmpheader.to == SL_PROXY_SERVER){
		SL_proxy_numprocs--;
		
		PRINTF(("[%d]:SL_msg_recv_newmsg: Procs closed :%d for proc:%d \n",
			SL_this_procid, SL_proxy_numprocs, tmpheader.from));


	}
	
	tmpheader.from = SL_this_procid;
        tmpheader.to = dproc->id;
	SL_socket_write ( dproc->sock, (char *) &tmpheader, sizeof  (SL_msg_header), 1 );
	PRINTF(("[%d]:SL_msg_recv_newmsg:Sending CLOSE reply to proc %d\n", SL_this_procid,dproc->id ));
	if(dproc->sock != SL_proxy_server_socket || SL_this_procid == SL_EVENT_MANAGER){
		PRINTF(("[%d]: 4clearing socket:%d\n", SL_this_procid, dproc->sock));
	        FD_CLR ( dproc->sock, &SL_send_fdset );
        	FD_CLR ( dproc->sock, &SL_recv_fdset );
		SL_socket_close ( dproc->sock );
	}

	PRINTF(("[%d]: Setting state as NOT_CONNECTED for proc:%d\n", SL_this_procid, dproc->id));
	dproc->state = SL_PROC_NOT_CONNECTED;
	dproc->sock  = -1;


//	dproc->recvfunc = SL_msg_closed;
//        dproc->sendfunc = SL_msg_closed;
	return SL_SUCCESS;
//	return SL_MSG_CLOSED;
    }

    if ( tmpheader.from != dproc->id ) {
	PRINTF(("[%d]:Recv_newmsg: got a message from the wrong process. Expected: %d[fd=%d]" 
	       "is from %d[fd=%d]\n",SL_this_procid,dproc->id, dproc->sock, tmpheader.from, fd ));
	if (dproc->id == -2)
                SL_proxy_server_socket = fd;

//	SL_proc_dumpall();
	SL_msg_header_dump(&tmpheader);
	if(SL_this_procid > SL_EVENT_MANAGER){
		dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, tmpheader.from );
		if(dproc == NULL){
			printf("[%d] Something went wrong...need to check\n", SL_this_procid);
			exit(-1);
		}
		if (dproc->state == SL_PROC_NOT_CONNECTED){
			dproc->sendfunc = ( SL_msg_comm_fnct *) SL_msg_send_newmsg;
                        dproc->recvfunc = ( SL_msg_comm_fnct *) SL_msg_recv_newmsg;
                        dproc->state    = SL_PROC_CONNECTED;
			dproc->sock     = fd;
			
			PRINTF(("[%d]:SL_msg_connect_proxy:already connected  to proc %d on sock %d %d\n",
                                SL_this_procid, dproc->id, dproc->sock, SL_proxy_server_socket));
		} 
	}

        proxymsg = dproc->id;

    }

    if (tmpheader.cmd == SL_MSG_CMD_EVENT){
	PRINTF(("[%d]:Got event handling request from proc %d\n",SL_this_procid,tmpheader.from));
	r = SL_event_recvq;
        }
    else {
	r = dproc->urqueue;
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
	header->loglength = tmpheader.loglength;
	header->temp	  =tmpheader.temp;
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

	if ( tmpheader.len > 0 ) {
	    tbuf  = (char *) malloc ( tmpheader.len );
	    if ( NULL == tbuf ) {
		return SL_ERR_NO_MEMORY;
	    }
	}
	else {
	    tbuf=NULL;
	}
	PRINTF(("[%d]:SL_recv_newmsg: Inserting into msg queue\n\n\n\n",SL_this_procid));


	SL_msgq_head *moveto=NULL;
	if (SL_this_procid == SL_PROXY_SERVER && dproc->id != SL_EVENT_MANAGER)
		moveto = dproc->squeue;
		
	 
	elem = SL_msgq_insert ( r, thead, tbuf, moveto );
#ifdef QPRINTF
	SL_msgq_head_debug ( r );
#endif	
	if ( tmpheader.len == 0 ) {
		 PRINTF(("[%d]:SL_recv_newmsg: Changing proc:%d recvfunction\n", SL_this_procid, dproc->id));
//	     dproc->recvfunc = SL_msg_recv_newmsg;
	    return SL_SUCCESS;
//	    return SL_MSG_DONE;
	}
    }
/*
if (tmpheader.loglength != -1){
            printf("[%d]:RECIEVER elemid:%d\n",SL_this_procid,elem->id);
            SL_msg_header_dump(&tmpheader);
}
*/

    elem->lenpos = 0;
    elem->iovpos = 1;
	SL_proc_dumpall();
    ret = SL_socket_read_nb ( fd, elem->iov[1].iov_base, elem->iov[1].iov_len, &len);

    if ( SL_SUCCESS == ret ) {
	elem->lenpos += len ;
	PRINTF(("[%d]:SL_msg_recv_newmsg: read %d bytes from %d elemid %d\n", SL_this_procid,
		 len, dproc->id, elem->id ));

	if ( elem->lenpos == elem->iov[1].iov_len ) {

	    if ( NULL != elem->move_to  ) {
		if(dproc->rqueue->first != NULL){
//			SL_msg_header_dump((SL_msg_header*)elem->iov[0].iov_base);
			SL_msgq_move ( dproc->rqueue, elem->move_to, elem );
			if(fd == SL_proxy_server_socket )
				proxymsg = -100;
		}
		else if (dproc->urqueue->first != NULL && SL_this_procid == SL_PROXY_SERVER && dproc->id != SL_EVENT_MANAGER){
			SL_proc *proc;
			SL_msg_header *pheader;
//			SL_msg_header_dump((SL_msg_header*)elem->iov[0].iov_base);
			pheader = (SL_msg_header*)elem->iov[0].iov_base;
			proc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array,pheader->to);
		//	elem->move_to = proc->scqueue;
			elem->move_to = NULL;
			SL_msgq_move ( dproc->urqueue, proc->squeue, elem );
		//	SL_msgq_delete(dproc->urqueue);
		}

	    }
	    dproc->currecvelem = NULL;
            dproc->recvfunc = SL_msg_recv_newmsg;
            ret = SL_SUCCESS;
	}
	else {

	    /* SL_msgq_move_tohead ( dproc->rqueue, elem ); */
	    dproc->currecvelem = elem;
            dproc->recvfunc = SL_msg_recv_knownmsg;
	    ret = SL_SUCCESS;
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
   

	PRINTF(("[%d]: 5clearing socket:%d\n", SL_this_procid, fd));
	FD_CLR ( fd, &send_fdset );

    ret = SL_socket_write_nb ( fd, ((char *)elem->iov[iovpos].iov_base + lenpos), 
			       (elem->iov[iovpos].iov_len - lenpos), &len );
    if ( SL_SUCCESS == ret ) {
	elem->lenpos += len;
	PRINTF (("[%d]:SL_msg_send_knownmsg: wrote %d bytes to %d\n", SL_this_procid,
		 len, dproc->id ));
	
	if ( 0 == elem->iovpos  && elem->lenpos == elem->iov[0].iov_len ) {
	    elem->iovpos = 1;
	    elem->lenpos = 0;
	}
	
	if ( 1 == elem->iovpos && elem->iov[1].iov_len == elem->lenpos  ){
// &&	     NULL != elem->move_to ) {
//	   SL_msg_header_dump((SL_msg_header*)elem->iov[0].iov_base);
	    SL_msgq_move ( dproc->squeue, elem->move_to, elem );
//	    SL_msgq_delete(dproc->squeue);
#ifdef QPRINTF
	    if ( NULL != elem->move_to ) {
		SL_msgq_head_debug ( elem->move_to );
	    }
#endif	
//	    ret = SL_MSG_DONE;

		PRINTF(("[%d]: SendSetting function for proc:%d\n", SL_this_procid, dproc->id));
            dproc->sendfunc = SL_msg_send_newmsg;
	    if(fd == SL_proxy_server_socket )
		    proxymsgsend = -100;	
	}
    }

    return ret;
}

int SL_msg_send_newmsg ( SL_proc *dproc, int fd )
{
    int ret=SL_SUCCESS;
    int len;
    SL_qitem *elem = NULL;
    SL_msg_header *theader;

	PRINTF(("[%d]: 6clearing socket:%d\n", SL_this_procid, fd));
	FD_CLR ( fd, &send_fdset );

    elem=dproc->squeue->first;

    theader = (SL_msg_header*)elem->iov[0].iov_base;
//    printf("[%d]:SENDER\n",SL_this_procid);
//    SL_msg_header_dump(theader);
     

    ret = SL_socket_write ( fd, elem->iov[0].iov_base, elem->iov[0].iov_len, dproc->timeout );
    if ( ret != SL_SUCCESS ) {
	return ret;
    }


    PRINTF(("[%d]:SL_msg_send_newmsg: wrote header to %d\n",SL_this_procid, 
	     dproc->id ));
   SL_msg_header_dump((SL_msg_header*)elem->iov[0].iov_base);
    elem->iovpos = 1;
    elem->lenpos = 0;

    if(theader->cmd != SL_MSG_CMD_CLOSE )	    
    ret = SL_socket_write_nb (fd, elem->iov[1].iov_base, elem->iov[1].iov_len, &len );

    if ( SL_SUCCESS == ret) {
	PRINTF (("[%d]:SL_msg_send_newmsg: wrote %d bytes to %d\n",SL_this_procid, 
		 len, dproc->id ));
    
	elem->lenpos += len;
	dproc->cursendelem = elem;
	dproc->sendfunc = SL_msg_send_knownmsg;
	if ( elem->iov[1].iov_len == elem->lenpos){
//   && 	     NULL != elem->move_to ) {
	    SL_msgq_move ( dproc->squeue, elem->move_to, elem );
//	    SL_msgq_delete(dproc->squeue);
	    dproc->cursendelem = NULL;
	    dproc->sendfunc = SL_msg_send_newmsg;
	    if(fd == SL_proxy_server_socket )
		    proxymsgsend = -100;
//	    ret = SL_MSG_DONE;
#ifdef QPRINTF
	    if ( NULL != elem->move_to ) {
		SL_msgq_head_debug ( elem->move_to );
	    }
#endif
	}
	
	
    }
    else
	PRINTF(("[%d]:ERROR!!in SL_socket_write_nb for proc:%d\n",SL_this_procid,dproc->id));

    return ret;
}

int SL_msg_accept_newconn ( SL_proc *dproc, int fd )
{
    /* This function will be registered with the according send/recv function pointer
       and is the counter part of the non-blocking accept. */
    int sd=0;
    int ret=SL_SUCCESS;

//	SL_proc_dumpall();    
    PRINTF(("[%d]: 7clearing socket:%d\n", SL_this_procid, dproc->sock));
    FD_CLR (dproc->sock, &send_fdset);
    FD_CLR (dproc->sock, &recv_fdset);
    
    sd = accept ( dproc->sock, 0, 0 );
    if ( sd > 0 ) {
	PRINTF(("[%d]:SL_msg_accept_newconn: establishing new connection %d errno=%d %s\n", 
		SL_this_procid, sd, errno, strerror(errno)));
	PRINTF (("[%d]:SL_msg_accept_newconn: Trying handshake on connection %d \n",SL_this_procid, sd));


	ret = SL_socket_write ( sd, (char *) &SL_this_procid, sizeof(int), SL_READ_MAX_TIME);
	if ( SL_SUCCESS != ret ) {
	    printf("[%d]: SL_accept_newconn: write in handshake on connection %d returned %d\n", 
		    SL_this_procid, sd, ret);
	    SL_socket_close (sd);
	    return SL_SUCCESS;
	}
	
	dproc->sock = sd;
	dproc->state = SL_PROC_ACCEPT_STAGE2;
	dproc->sendfunc = SL_msg_accept_stage2;
	dproc->recvfunc = SL_msg_accept_stage2;

	if ( sd > SL_fdset_lastused ) {
	    SL_fdset_lastused = sd;
	}

	PRINTF(("[%d]: Setting socket:%d\n", SL_this_procid, dproc->sock));
	FD_SET ( dproc->sock, &SL_send_fdset );
	FD_SET ( dproc->sock, &SL_recv_fdset );    
    }
    
    else{
        if ( EWOULDBLOCK != errno  ||
             ECONNABORTED!= errno  ||
             EPROTO      != errno  ||
             EINTR       != errno ) {
            ret = SL_SUCCESS;
        }
    }
    
    return ret;
}

int SL_msg_accept_stage2(SL_proc *dproc, int fd)
{
    int sd = dproc->sock;
    int tmp=0;
    int ret=SL_SUCCESS;
    char hostname[SL_MAXHOSTNAMELEN];
    int newid ;
    int port;
    static int SL_tnumprocs=0;
    int killsig=0;
//   SL_proc_dumpall(); 
	PRINTF(("[%d]: SL_accept_stage2: read in handshake on connection %d returned %d\n", 
				SL_this_procid, sd, ret));

    ret = SL_socket_read ( sd, (char *) &tmp, sizeof(int), SL_READ_MAX_TIME );
    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_accept_stage2: read in handshake on connection %d returned %d\n", SL_this_procid, sd, ret);
	
	dproc->sock = SL_this_listensock;
	dproc->state = SL_PROC_ACCEPT;
	dproc->sendfunc = SL_msg_accept_newconn;
	dproc->recvfunc = SL_msg_accept_newconn;

	printf("[%d]: 8clearing socket:%d\n", SL_this_procid, sd);
        FD_CLR ( sd, &SL_send_fdset);
	FD_CLR ( sd, &SL_recv_fdset);
	FD_CLR ( sd, &send_fdset);
	FD_CLR ( sd, &recv_fdset);

	SL_socket_close(sd);
	return SL_SUCCESS;
    }

    if ( tmp != dproc->id ) {
	SL_proc * tproc = (SL_proc *)SL_array_get_ptr_by_id ( SL_proc_array, tmp );

//	PRINTF(("[%d]:accept_stage2: received connection request from "
//                   "unkown proc %d Expected %d state %d socket %d\n", SL_this_procid,tmp,dproc->id,tproc->state,sd ));

	if ( NULL == tproc ) {
	    PRINTF(("[%d]:accept_stage2: received connection request from "
		   "unkown proc %d\n", SL_this_procid,tmp ));
	    
	    if (tmp == SL_CONSTANT_ID || tmp == SL_PROC_ID){
		PRINTF(("[%d]:Assining new ID to process  %d\n\n",SL_this_procid,tmp));
		PRINTF(("[%d]: SL_tnumprocs:%d SL_numprocs:%d\n",SL_this_procid,SL_tnumprocs,SL_numprocs));
		if (tmp == SL_CONSTANT_ID){
		    newid = SL_proc_id_generate(1);
		}
		else if (SL_tnumprocs == SL_numprocs){
             	   newid = -1;
                	killsig = 1;
            	}
            	else{
                	newid = SL_proc_id_generate(0);
                	SL_tnumprocs++;
            	}

		port = SL_proc_port_generate();
		ret = SL_socket_read ( sd, hostname, SL_MAXHOSTNAMELEN, SL_ACCEPT_MAX_TIME );

		if ( SL_SUCCESS != ret ) {
		    return ret;
		}

		if (( strlen(hostname)>20) && killsig != 1 ){
                	newid = -1;
	                SL_proc_id_generate(-1);
        	        SL_tnumprocs--;
			printf("Hostname incorrect:%s\n", hostname);
            	}

		ret = SL_socket_write ( sd, (char *) &newid, sizeof(int), SL_ACCEPT_MAX_TIME);
		if ( SL_SUCCESS != ret ) {
		    return ret;
		}

		if(newid == -1){
             		printf(" We do not need more procs as of now so sending a signal to kill %s\n",
				hostname);
                	dproc->sock = SL_this_listensock;
	                dproc->state = SL_PROC_ACCEPT;
        	        dproc->sendfunc = SL_msg_accept_newconn;
                	dproc->recvfunc = SL_msg_accept_newconn;
	                PRINTF(("[%d]: 9clearing socket:%d\n", SL_this_procid, sd));
        	        FD_CLR ( sd, &SL_send_fdset);
                	FD_CLR ( sd, &SL_recv_fdset);
	                FD_CLR ( sd, &send_fdset);
        	        FD_CLR ( sd, &recv_fdset);
                	SL_socket_close(sd);

                	return SL_SUCCESS;
            	}
		printf("[%d]:Assined ID:%d to host  %s\n\n",SL_this_procid,newid,hostname);

		ret = SL_socket_write ( sd, (char *) &port, sizeof(int), SL_ACCEPT_MAX_TIME);
		if ( SL_SUCCESS != ret ) {
		    return ret;
		}
		SL_proc_init(newid, hostname,port );
		tproc = SL_array_get_ptr_by_id ( SL_proc_array, newid );
	    }
	    else{
		printf("ERROR!!!!!\n");
		return SL_ERR_GENERAL;
	    }
	}
	
	/* 
	** swap the connection information in case tproc has anything else than 
	** NOT_CONNECTED set, such that the according information is not lost.
	*/
//	SL_proc_dumpall();    
	PRINTF(("[%d]: 10clearing socket:%d\n", SL_this_procid, sd));
	FD_CLR ( sd, &send_fdset);
	FD_CLR ( sd, &recv_fdset);

	if ( tproc->state != SL_PROC_NOT_CONNECTED && 
	     tproc->state != SL_PROC_UNREACHABLE ) {
	    int tempsock, tempstate;
	    SL_msg_comm_fnct *tempsendfnc, *temprecvfnc;
	    
	    tempsock    = tproc->sock;
	    tempstate   = tproc->state;
	    tempsendfnc = tproc->sendfunc;
	    temprecvfnc = tproc->recvfunc;
	    PRINTF(("[%d]: accept_stage2: swapping connection between dproc %d sd %d with tproc %d state %d sock %d\n",
		    SL_this_procid, dproc->id, sd, tproc->id, tproc->state, tproc->sock ));
	    
	    if ( tproc->state == SL_PROC_CONNECTED ) {
		printf("[%d]: accept_stage2: thats not good, process %d claims to have already a valid connection." 
		       		       "Do not want to overwrite that.\n", SL_this_procid, tproc->id );
	    }

	    
	    SL_proc_set_connection ( tproc, sd );
	    
	    dproc->sock = tempsock;
	    dproc->state = tempstate;
	    dproc->sendfunc = tempsendfnc;
	    dproc->recvfunc = temprecvfnc;
	}
	else {
	    SL_proc_set_connection ( tproc, sd );
	    
	    if ( dproc->id == SL_this_procid ) {
		dproc->sock     = SL_this_listensock;
		dproc->state    = SL_PROC_CONNECTED;
		dproc->sendfunc = SL_msg_accept_newconn;
		dproc->recvfunc = SL_msg_accept_newconn;
	    }
	    else {

		PRINTF(("[%d]: Setting state as NOT_CONNECTED for proc:%d\n", SL_this_procid, dproc->id));
		dproc->sock	= SL_this_listensock;
		dproc->state    = SL_PROC_ACCEPT;
		dproc->recvfunc  = (SL_msg_comm_fnct *)SL_msg_accept_newconn;
		dproc->sendfunc  = (SL_msg_comm_fnct *)SL_msg_accept_newconn;
//		dproc->sock      = -1;
//		dproc->state     = SL_PROC_NOT_CONNECTED;
//		dproc->recvfunc  = (SL_msg_comm_fnct *)SL_msg_recv_newmsg;
//		dproc->sendfunc  = (SL_msg_comm_fnct *)SL_msg_send_newmsg;
	    }

	}
    }
    else {
	SL_proc_set_connection ( dproc, sd );
//	ret = SL_MSG_DONE;
    }
	
	
	
//	SL_proc_dumpall();    
    return ret;
}
   
int SL_msg_connect_proxy(SL_proc *dproc, int fd)
{
    int ret=SL_SUCCESS, i;
    SL_proc *proc;
    
    int tmp;
    int terr=0;
    SL_proc_dumpall();
    socklen_t len=sizeof(int);
    proc  = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, SL_PROXY_SERVER );
    if(proc->state != SL_PROC_CONNECTED){
	
//	SL_proc_init_conn_nb ( proc, proc->timeout );
	PRINTF(("[%d]: 11clearing socket:%d\n", SL_this_procid, proc->sock));
	FD_CLR ( proc->sock, &send_fdset);
	FD_CLR ( proc->sock, &recv_fdset);
	getsockopt (proc->sock, SOL_SOCKET, SO_ERROR, &terr, &len);
	printf("[%d]:SL_msg_connect_proxy: terr = 0. Trying handshake to proxy %d\n",SL_this_procid, 
		proc->id);
	ret = SL_socket_write ( proc->sock, (char *) &SL_this_procid, sizeof(int),
				proc->timeout );
	if ( SL_SUCCESS != ret ) {
	
	    PRINTF(("[%d]: 12clearing socket:%d\n", SL_this_procid, proc->sock));
	    FD_CLR ( proc->sock, &SL_send_fdset );
	    FD_CLR ( proc->sock, &SL_recv_fdset );
	    
	    printf("[%d]:SL_msg_connect_proxy: reconnecting %d %s \n", SL_this_procid,ret,
		     strerror ( ret ) );
	    proc->state = SL_PROC_NOT_CONNECTED;
	    SL_socket_close ( proc->sock );
	    ret = SL_proc_init_conn_nb ( proc, proc->timeout );
	    return SL_SUCCESS;
	}
	

PRINTF(("[%d]: SL_msg_connect_proxy: in SL_socket_read socket %d procid %d\n",
                    SL_this_procid, dproc->sock, dproc->id));	
	ret = SL_socket_read ( proc->sock, ( char *) &tmp, sizeof(int),
			       SL_READ_MAX_TIME);
	if ( SL_SUCCESS != ret ) {
	    printf("[%d]: SL_msg_connect_proxy: Error in SL_socket_read socket %d procid %d\n",
		    SL_this_procid, proc->sock, proc->id);
	    PRINTF(("[%d]: 12clearing socket:%d\n", SL_this_procid, proc->sock));
	    FD_CLR ( proc->sock, &SL_send_fdset );
	    FD_CLR ( proc->sock, &SL_recv_fdset );
	    
	    printf("[%d]:SL_msg_connect_proxy: reconnecting %d %s \n", SL_this_procid,ret,
		     strerror ( ret ) );
	    proc->state = SL_PROC_NOT_CONNECTED;
	    SL_socket_close ( proc->sock );
	    ret = SL_proc_init_conn_nb ( proc, proc->timeout );
	    return SL_SUCCESS;
	}
	
	if ( tmp != proc->id ) {
	    printf ("[%d]:SL_msg_connect_proxy: error in exchanging handshake\n",SL_this_procid);
	    return SL_ERR_GENERAL;
	}
	
	dproc->state = SL_PROC_CONNECTED;
	proc->state = SL_PROC_CONNECTED;

/*	FD_CLR ( dproc->sock, &SL_send_fdset );
	FD_CLR ( dproc->sock, &SL_recv_fdset );
	if ( dproc->sock > 0 && dproc->sock != SL_this_listensock) {
            SL_socket_close ( dproc->sock );
        }
	dproc->sock = proc->sock;
*/	
	PRINTF(("[%d]: Setting socket:%d\n", SL_this_procid, dproc->sock));
	FD_SET ( dproc->sock, &SL_send_fdset );
	FD_SET ( dproc->sock, &SL_recv_fdset );
	printf("[%d]:SL_msg_connect_proxy: connection established to proc %d on sock %d\n",
		SL_this_procid, dproc->id, dproc->sock);

	SL_proxy_server_socket = dproc->sock;
	dproc->sendfunc = SL_msg_send_newmsg;
	dproc->recvfunc = SL_msg_recv_newmsg;

/*
          dproc->state    = SL_PROC_CONNECT_STAGE2;
            dproc->sendfunc = SL_msg_connect_proxy2;
            dproc->recvfunc = SL_msg_connect_proxy2;
*/	
    }
//    else {

	 int listsize = SL_array_get_last ( SL_proc_array ) + 1;
	
	 for ( i=0; i<listsize; i++ ) {
            dproc = (SL_proc*)SL_array_get_ptr_by_pos ( SL_proc_array, i );
	    if(dproc->sock !=  SL_proxy_server_socket ||  dproc->state == SL_PROC_CONNECTED)
		continue;
	    if(dproc->id != SL_EVENT_MANAGER && dproc->id != SL_PROXY_SERVER ){
		PRINTF(("[%d]:SL_msg_connect_proxy:already connected  to proc %d on sock %d\n",
                SL_this_procid, dproc->id, dproc->sock));


		dproc->state = SL_PROC_CONNECTED;
		dproc->sendfunc = SL_msg_send_newmsg;
		dproc->recvfunc = SL_msg_recv_newmsg;

/*		FD_CLR ( dproc->sock, &SL_send_fdset );
	        FD_CLR ( dproc->sock, &SL_recv_fdset );
        	if ( dproc->sock > 0 && dproc->sock != SL_this_listensock) {
	            SL_socket_close ( dproc->sock );
        	}
		dproc->sock = proc->sock;

		PRINTF(("[%d]: Setting socket:%d\n", SL_this_procid, dproc->sock));
		FD_SET ( dproc->sock, &SL_send_fdset );
	        FD_SET ( dproc->sock, &SL_recv_fdset );
*/	
		SL_proc_dumpall();	
	}
    }
//}
    
    /* connection not yet established , go on */
    return ret;
    
    
} 

int SL_msg_connect_proxy2(SL_proc *dproc, int fd)
{
        int tmp,ret = SL_SUCCESS;
        SL_proc *proc = NULL;
        proc = (SL_proc*)SL_array_get_ptr_by_id(SL_proc_array,SL_PROXY_SERVER);
               PRINTF(("[%d]: SL_msg_connect_proxy: in SL_socket_read socket %d procid %d\n",
                    SL_this_procid, dproc->sock, dproc->id));
        ret = SL_socket_read ( proc->sock, ( char *) &tmp, sizeof(int),
                               SL_READ_MAX_TIME);
        if ( SL_SUCCESS != ret ) {
            printf("[%d]: SL_msg_connect_proxy: Error in SL_socket_read socket %d procid %d\n",
                    SL_this_procid, dproc->sock, dproc->id);
            PRINTF(("[%d]: 13clearing socket:%d\n", SL_this_procid, proc->sock));
            FD_CLR ( proc->sock, &SL_send_fdset );
            FD_CLR ( proc->sock, &SL_recv_fdset );

            PRINTF (("[%d]:SL_msg_connect_proxy: reconnecting %d %s \n", SL_this_procid,ret,
                     strerror ( ret ) ));
            proc->state = SL_PROC_NOT_CONNECTED;
            SL_socket_close ( proc->sock );
            ret = SL_proc_init_conn_nb ( proc, proc->timeout );
            return SL_SUCCESS;
        }

        if ( tmp != proc->id ) {
            printf ("[%d]:SL_msg_connect_proxy: error in exchanging handshake\n",SL_this_procid);
            return SL_ERR_GENERAL;
        }
       dproc->state = SL_PROC_CONNECTED;
        proc->state = SL_PROC_CONNECTED;
        dproc->sock = proc->sock;

	PRINTF(("[%d]: Setting socket:%d\n", SL_this_procid, dproc->sock));
        FD_SET ( dproc->sock, &SL_send_fdset );
        FD_SET ( dproc->sock, &SL_recv_fdset );
        printf("[%d]:SL_msg_connect_proxy: connection established to proc %d on sock %d\n",
                SL_this_procid, dproc->id, dproc->sock);
        SL_proxy_server_socket = dproc->sock;
        dproc->sendfunc = SL_msg_send_newmsg;
        dproc->recvfunc = SL_msg_recv_newmsg;
	
	return SL_SUCCESS;
}



int SL_msg_connect_newconn ( SL_proc *dproc, int fd )
{
    /* This function will be registered with the according send/recv function pointer
       and is the counter part of the non-blocking connect. */
    int ret=SL_SUCCESS;
//	SL_proc_dumpall();
    						
    PRINTF(("[%d]: 14clearing socket:%d procid:%d\n", SL_this_procid, dproc->sock, dproc->id));
    FD_CLR ( dproc->sock, &send_fdset);
    FD_CLR ( dproc->sock, &recv_fdset);
    
    if ( dproc->state != SL_PROC_CONNECTED ) {
	int terr=0;
	socklen_t len=sizeof(int);
	int errval;
	
#ifdef MINGW
	char *winflag;
	sprintf(winflag, "%d", terr);
	getsockopt (dproc->sock, SOL_SOCKET, SO_ERROR, winflag, &len);
	if (terr == WSAEINPROGRESS || terr == WSAEWOULDBLOCK ) {
#else	
	errval = getsockopt (dproc->sock, SOL_SOCKET, SO_ERROR, &terr, &len);

	if ( EINPROGRESS == terr || EWOULDBLOCK == terr) {
#endif
	    /* connection not yet established , go on */
	    return ret;
	}

	if ( 0 != terr ) {
	    PRINTF(("[%d]: 15clearing socket:%d\n", SL_this_procid, dproc->sock));
	    FD_CLR ( dproc->sock, &SL_send_fdset );
	    FD_CLR ( dproc->sock, &SL_recv_fdset );
	    
	    PRINTF( ("[%d]:SL_msg_connect_newconn: reconnecting %d %s \n", SL_this_procid,terr, 
		     strerror ( terr ) ));
	    dproc->state = SL_PROC_NOT_CONNECTED;
	    SL_socket_close ( dproc->sock );
	    ret = SL_proc_init_conn_nb ( dproc, dproc->timeout );
	    return ret;
	}
	else if ( terr == 0 ) {
	    PRINTF(("[%d]:SL_msg_connect_newconn: terr = 0. Trying handshake to proc %d\n",SL_this_procid, dproc->id));
	    ret = SL_socket_write ( dproc->sock, (char *) &SL_this_procid, sizeof(int), 
				    dproc->timeout );
	    if ( SL_SUCCESS != ret ) {
		PRINTF(("[%d]: 16clearing socket:%d\n", SL_this_procid, dproc->sock));
		FD_CLR ( dproc->sock, &SL_send_fdset );
		FD_CLR ( dproc->sock, &SL_recv_fdset );
		
		printf("[%d]:SL_msg_connect_newconn: reconnecting %d %s \n", SL_this_procid,ret, 
			 strerror ( ret ) );
		dproc->state = SL_PROC_NOT_CONNECTED;
		SL_socket_close ( dproc->sock );
		ret = SL_proc_init_conn_nb ( dproc, dproc->timeout );
		return ret;
	    }
	    dproc->state    = SL_PROC_CONNECT_STAGE2;
	    dproc->sendfunc = SL_msg_connect_stage2;
	    dproc->recvfunc = SL_msg_connect_stage2;
	}
    }

    return ret;
}


int SL_msg_connect_stage2(SL_proc* dproc, int fd)
{   
    
    int tmp;
    int ret = SL_SUCCESS; 

	 PRINTF(("[%d]: SL_msg_connect_stage2: SL_socket_read socket %d procid %d\n",
                SL_this_procid, dproc->sock, dproc->id));

    ret = SL_socket_read ( dproc->sock, ( char *) &tmp, sizeof(int), 
			   SL_READ_MAX_TIME);
    PRINTF(("[%d]: 17clearing socket:%d\n", SL_this_procid, dproc->sock));
    FD_CLR ( dproc->sock, &send_fdset);
    FD_CLR ( dproc->sock, &recv_fdset);

    if ( SL_SUCCESS != ret ) {
	printf("[%d]: SL_msg_connect_stage2: Error in SL_socket_read socket %d procid %d\n", 
		SL_this_procid, dproc->sock, dproc->id);
	PRINTF(("[%d]: 18clearing socket:%d\n", SL_this_procid, dproc->sock));
	FD_CLR ( dproc->sock, &SL_send_fdset );
	FD_CLR ( dproc->sock, &SL_recv_fdset );
	
	PRINTF (("[%d]:SL_msg_connect_stage2: reconnecting %d %s \n", SL_this_procid,ret, 
		 strerror ( ret ) ));
	dproc->state = SL_PROC_NOT_CONNECTED;
	SL_socket_close ( dproc->sock );
	ret = SL_proc_init_conn_nb ( dproc, dproc->timeout );
	return SL_SUCCESS;
    }
    
    if ( tmp != dproc->id ) {
	printf ("[%d]:SL_msg_connect_stage2: error in exchanging handshake\n",SL_this_procid);
	return SL_ERR_GENERAL;
    }
    
    dproc->state = SL_PROC_CONNECTED;
    PRINTF(("[%d]:SL_msg_connect_stage2: connection established to proc %d on sock %d time:%lf\n",
	   SL_this_procid, dproc->id, dproc->sock, SL_Wtime()-dproc->connect_start_tstamp));
   
    if(dproc->id == -2)
	SL_proxy_server_socket = dproc->sock;
 
    dproc->sendfunc = SL_msg_send_newmsg;
    dproc->recvfunc = SL_msg_recv_newmsg;
    
//    SL_proc_dumpall ();
    return SL_SUCCESS;
}

int SL_msg_closed ( SL_proc *dproc, int fd )
{
//	if(SL_proxy_connected_procs <=0 && fd == SL_proxy_server_socket)
//		SL_socket_close(fd);
    PRINTF(("[%d]:SL_msg_closed: connection to %d is marked as closed\n", SL_this_procid,dproc->id ));
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
