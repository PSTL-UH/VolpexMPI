
#include "SL.h"
#include "SL_array.h"
#include "SL_proc.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"
#include "SL_event_handling.h"

//static int SL_msg_counter=0;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_fdset_lastused;

extern int SL_proc_establishing_connection;

extern SL_array_t *SL_proc_array;


//extern int SL_this_proc_id;


int SL_event_post(void *buf, int len, int dest, int tag, int context_id,SL_msg_request **req)
{
	SL_msg_header *header = NULL;
        struct SL_proc *dproc=NULL;        
        SL_msg_request *treq=NULL; 
        SL_qitem *elem=NULL;   
	double timeout = SL_ACCEPT_MAX_TIME;
        int ret;                 

  
  /* Step 1. Post the message into the send_event queue */
    dproc = SL_array_get_ptr_by_id ( SL_proc_array, dest );

    if ( dproc->state != SL_PROC_CONNECTED ) {
        ret = SL_proc_init_conn_nb ( dproc, timeout );
        if ( SL_SUCCESS != ret ) {
            return ret;
        }
     }

    treq = (SL_msg_request *) malloc ( sizeof (SL_msg_request ));
    if ( NULL == treq ) {
        return SL_ERR_NO_MEMORY;
    }

    header = SL_msg_get_header ( SL_MSG_CMD_EVENT,	/* cmd  */
                                 SL_this_procid,	/* from */
                                 dest,		   	/* to  */
                                 tag,             	/* tag */
                                 context_id,      	/* context */
                                 len,            	/* msg len */
				 -1,
				 -1);

    PRINTF(("[%d]:SL_event_post: header from %d to %d tag %d context %d len %d id %d\n",SL_this_procid,
             header->from, header->to, header->tag, header->context, header->len,
             header->id ));

    // for later: add 'special msgq_insert which moves the 
    // event as close to the beginnig of the queue as possible, 
    // e.g. the after the first message which has not been started
    // yet.
    elem = SL_msgq_insert (dproc->squeue , header, buf, dproc->scqueue );


    treq->proc   = dproc;
    treq->type   = SL_REQ_SEND;
    treq->id     = header->id;
    treq->elem   = elem;
    treq->cqueue = dproc->scqueue;

    *req = treq;

//	SL_msg_send_event(dproc);
    /* SL_msg_progress (); */
    return SL_SUCCESS;
  
}


/*void SL_event_progress(SL_qitem *elem)
{
	static SL_event_msg_header *tevent;
	static int status = 0;
	static SL_event_handle *SL_event;
	void *buf = NULL;	
	static int len =0;
    	printf("[%d]:ENTERED INTO EVENT HANDLER !!!!!!!!!!!!:::%d\n\n",SL_this_procid,elem->id);
	if(status == 0){
		tevent = (SL_event_msg_header*) elem->iov[1].iov_base;
		PRINTF(("[%d]:new event is %d\n",SL_this_procid,tevent->cmd));
		status = 1;
		if (tevent->cmd == SL_CMD_ADD_PROC ){
			SL_event = SL_add_proc;

                }
                else if(tevent->cmd == SL_CMD_DELETE_PROC ){
			SL_event = SL_delete_proc;
                }

	}
	else if(status == 1){
		len = *(int*)elem->iov[1].iov_base;
		PRINTF(("[%d]:Length of recived msg is %d\n",SL_this_procid,len));
		status = 2;
	}
	else{
		buf = elem->iov[1].iov_base;
		PRINTF(("[%d]: Handling event %d",SL_this_procid, tevent->cmd));
	 	SL_event(buf,len);
		status = 0;
		len = 0;
	 }
		
}*/



void SL_event_progress(SL_qitem *elem)
{
        static SL_event_msg_header *tevent;
        static int status = 0;
        static SL_event_handle *SL_event;
        void *buf = NULL;
        static int len =0;
        PRINTF(("[%d]:ENTERED INTO EVENT HANDLER !!!!!!!!!!!!:::%d\n\n",SL_this_procid,elem->id));
        if(status == 0){
                tevent = (SL_event_msg_header*) elem->iov[1].iov_base;
                PRINTF(("[%d]:new event is %d\n",SL_this_procid,tevent->cmd));
                status = 1;
                if (tevent->cmd == SL_CMD_ADD_PROC ){
                        SL_event = SL_add_proc;

                }
                else if(tevent->cmd == SL_CMD_DELETE_PROC ){
                        SL_event = SL_delete_proc;
                }
		else if(tevent->cmd == SL_CMD_ADD_EXISTING_PROC ){
                        SL_event = SL_add_existing_proc;
                }
                else if (tevent->cmd == SL_CMD_START_COMM){
                        SL_event = SL_start_communication;
                }


                len = tevent->msglen;
                PRINTF(("[%d]:Length of recived msg is %d\n",SL_this_procid,len));
                
        }
        else{
                buf = elem->iov[1].iov_base;
                PRINTF(("[%d]: Handling event %d",SL_this_procid, tevent->cmd));
                SL_event(buf,len);
                status = 0;
                len = 0;
         }

}

SL_qitem* SL_get_next_event()
{
    SL_qitem *elem = NULL;	
    elem = SL_msgq_get_first(SL_event_recvq);
    if (NULL !=elem){
	if(elem->lenpos == elem->iov[1].iov_len){
		SL_msgq_remove(SL_event_recvq,elem);
	        return elem;
	}
    }

    return NULL;
	
}

void SL_create_event_header()
{
}


SL_qitem* SL_get_next_event_noremove()
{
    SL_qitem *elem = NULL;
    elem = SL_msgq_get_first(SL_event_recvq);
    if (NULL !=elem){
        if(elem->lenpos == elem->iov[1].iov_len){
                return elem;
        }
    }

    return NULL;

}

void SL_remove_event(SL_qitem *elem)
{
    if (NULL !=elem){
        if(elem->lenpos == elem->iov[1].iov_len){
                SL_msgq_remove(SL_event_recvq,elem);
        }
    }

    return ;
}

void SL_move_eventtolast(SL_qitem *elem)
{
        SL_msgq_move_tolast(SL_event_recvq,elem);
}



