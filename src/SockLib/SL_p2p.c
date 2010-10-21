

#include "SL.h"
#include "SL_array.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"
#include "SL_proc.h"
#include "mpi.h"
extern SL_array_t *SL_proc_array;

extern SL_array_t *SL_request_array;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern NODEPTR head, insertpt;

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/


int SL_send_post_self(SL_proc *dproc, SL_msg_header *header, char *buf ,int len)
{
    
    SL_msg_header *nheader;
    SL_qitem *elem ;
    
    elem = SL_msgq_head_check(dproc->rqueue, header);
    if (elem != NULL) {
	PRINTF(("[%d]:SL_send_post_self: dproc->id %d \n",SL_this_procid, dproc->id));
	memcpy (elem->iov[1].iov_base, buf, len );
	/* TODO: adjust the length of the header in the receive queue */
	nheader = (SL_msg_header *) elem->iov[0].iov_base;
	nheader->len = header->len;
	SL_msgq_move(dproc->rqueue, dproc->rcqueue, elem);
	SL_msgq_insert (dproc->scqueue, header, buf, NULL );
    }
    else {
	SL_msgq_insert ( dproc->squeue, header, buf, dproc->scqueue );
    }
    
    
    return SL_SUCCESS;
}

int SL_recv_post_self(SL_proc *dproc, SL_msg_header *header, char *buf,int len)
{
    SL_msg_header *nheader;
    SL_qitem *elem;
    
    elem = SL_msgq_head_check(dproc->squeue, header);
    if (elem != NULL) {
	PRINTF(("[%d]:SL_recv_post_self: \n",SL_this_procid));
	memcpy (buf,elem->iov[1].iov_base, elem->iov[1].iov_len );
	/* TODO: adjust the length of the header in the receive queue */
	nheader = (SL_msg_header *) elem->iov[0].iov_base;
	header->len = nheader->len;
	SL_msgq_move(dproc->squeue, dproc->scqueue, elem);
	SL_msgq_insert (dproc->rcqueue, header, buf, NULL );
    }
    else {
	SL_msgq_insert ( dproc->rqueue, header, buf, dproc->rcqueue );
    }
    
    
    return SL_SUCCESS;
    
}


int SL_recv_post ( void *buf, int len, int src, int tag, int context_id, double timeout,
		   SL_msg_request **req )
{
    SL_msg_header *header=NULL;
    SL_msgq_head *rq=NULL, *rcq=NULL;
    struct SL_proc *dproc=NULL;
    SL_msg_request *treq;
    SL_qitem *elem=NULL;
    int ret;
    
    /* Step 1. Post the message into the recv queue of that proc */
    dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, src );
    rq    = dproc->rqueue;
    rcq   = dproc->rcqueue;

    if ( dproc->state != SL_PROC_CONNECTED ) {
        ret = SL_proc_init_conn_nb ( dproc, timeout );
	if ( SL_SUCCESS != ret ) {
		printf("[%d]:PROBLEM recvpost\n", SL_this_procid);
	    return ret;
	}
        // SL_proc_init_conn ( dproc );
    }

    treq = (SL_msg_request *) malloc ( sizeof (SL_msg_request ));
    if ( NULL == treq ) {
	printf("[%d]:PROBLEM recvpost\n", SL_this_procid);
	return SL_ERR_NO_MEMORY;
    }

    header = SL_msg_get_header ( SL_MSG_CMD_P2P,  /* cmd  */
				 src,             /* from */
				 SL_this_procid,  /* to  */
				 tag,             /* tag */
				 context_id,      /* context */
				 len,            /* msg len */
			         -1,
				 -1);
				 
    PRINTF (("[%d]:SL_recv_post: header from %d to %d tag %d context %d len %d  id %d \n", SL_this_procid,
	    header->from, header->to, header->tag, header->context, header->len, 
	    header->id ));

	if (dproc->id != SL_EVENT_MANAGER)
	 SL_cancelmsg_check(dproc );

    elem = SL_msgq_head_check ( dproc->urqueue, header );
    if ( elem != NULL ) {
	PRINTF(("[%d]:SL_recv_post: found message in unexpected message queue\n",SL_this_procid));
	
	/* 
	** Reset header->len, since the message might be shorter than
	** what we expect 
	*/
	header->len = ((SL_msg_header *)elem->iov[0].iov_base )->len;
	if ( elem->lenpos == header->len ) {
	    /* Data is already received, move to the completion queue */
	    SL_msgq_move ( dproc->urqueue, dproc->rcqueue, elem );
#ifdef QPRINTF
	    SL_msgq_head_debug (dproc->urqueue );
	    SL_msgq_head_debug (dproc->rcqueue );
#endif
	}
	else {
	    /* data is not yet fully received, move to the 
	       head of the working queue */
	    SL_msgq_move_tohead ( dproc->urqueue, dproc->rqueue, elem );
	    elem->move_to = dproc->rcqueue;
#ifdef QPRINTF
	    SL_msgq_head_debug (dproc->urqueue );
	    SL_msgq_head_debug (dproc->rqueue );
#endif
	}

	if ( elem->lenpos > 0 ) {
	    memcpy ( buf, elem->iov[1].iov_base, elem->lenpos );
	}
	free ( elem->iov[0].iov_base );
	if ( NULL != elem->iov[1].iov_base ) {
	    free ( elem->iov[1].iov_base );
	}
	PRINTF(("[%d]:SL_recv_post: copied %d bytes into real buffer\n", SL_this_procid,elem->lenpos ));
	
	elem->iov[0].iov_base = (char *)header;
	elem->iov[1].iov_base = (char *)buf;
	elem->id              = header->id;
    }
    else if(header->from == header->to){
	SL_recv_post_self (dproc, header,(char *)buf ,len);
    }
    
    else {
	elem = SL_msgq_insert ( rq, header, buf, rcq );
    }
    
    treq->proc   = dproc;
    treq->type   = SL_REQ_RECV;
    treq->id     = header->id;
    treq->elem   = elem;
    treq->cqueue = rcq;
    
#ifdef QPRINTF
    SL_msgq_head_debug (rq );
#endif
    *req = treq;

    /* SL_msg_progress (); */
    return SL_SUCCESS;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

//int SL_send_post ( void *buf, int len, int dest, int tag, int context_id, double timeout, SL_msg_request **req )
int SL_send_post ( void *buf, int len, int dest, int tag, int context_id, double timeout, int loglength, int reuse,SL_msg_request **req )
{
    SL_msg_header *header=NULL;
    SL_msgq_head *sq=NULL, *scq=NULL;
    struct SL_proc *dproc=NULL;
    SL_msg_request *treq=NULL;
    SL_qitem *elem=NULL;
    int ret;
 //   int loglength = -1;

    /* Step 1. Post the message into the send queue of that proc */
    dproc = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, dest );
    sq    = dproc->squeue;
    scq   = dproc->scqueue;

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
    
    header = SL_msg_get_header ( SL_MSG_CMD_P2P,  /* cmd  */
				 SL_this_procid,  /* from */
				 dest,            /* to  */
				 tag,             /* tag */
				 context_id,      /* context */
				 len,		  /* msg len */
				 loglength,
				 reuse);            
    
    PRINTF(("[%d]:SL_send_post: header from %d to %d tag %d context %d len %d id %d loglength %d\n", SL_this_procid,
	     header->from, header->to, header->tag, header->context, header->len, 
	     header->id, header->loglength ));
    if(header->from == header->to){
	SL_send_post_self (dproc, header,(char *)buf ,len);
    }
    else
	elem = SL_msgq_insert ( sq, header, buf, scq );
    
    treq->proc   = dproc;
    treq->type   = SL_REQ_SEND;
    treq->id     = header->id;
    treq->elem   = elem;
    treq->cqueue = scq;
    
#ifdef QPRINTF
    SL_msgq_head_debug (sq );
#endif
    *req = treq;
    
    /* SL_msg_progress (); */
    return SL_SUCCESS;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
int SL_wait ( SL_msg_request **req, SL_Status *status )
{
    SL_qitem *found=NULL;
    SL_msgq_head *q=NULL;
    SL_msg_request *treq = *req;
    int ret=SL_SUCCESS;

    if ( treq == NULL ) {
	return SL_SUCCESS;
    }

    while ( 1 ) {
	SL_msg_progress ();
	
	/* 
	** The assignment for q tells us which queue we 
	** have to check for completion. It has to be inside the 
	** while loop, since its value might change for SL_ANY_SOURCE
	** within the SL_msg_progress function.
	*/
	q = treq->cqueue;
	
	found = SL_msgq_find ( q, treq->id ); 
	if ( NULL != found ){
	    PRINTF(("[%d]:SL_wait: found message %d in completion queue\n", SL_this_procid,treq->id ));
	    ret = found->error;

	    break;
	}
    }

    if ( NULL != status && SL_STATUS_IGNORE != status ) {
	status->SL_SOURCE  = ((SL_msg_header *)found->iov[0].iov_base )->from;
	status->SL_TAG     = ((SL_msg_header *)found->iov[0].iov_base )->tag;
	status->SL_ERROR   = found->error; 
	status->SL_CONTEXT = ((SL_msg_header *)found->iov[0].iov_base )->context;
	status->SL_LEN     = ((SL_msg_header *)found->iov[0].iov_base )->len;
    }


    /* Remove message from completion queue */
    SL_msgq_remove ( q, found );
    free ( found->iov[0].iov_base );
    free ( found);
    free ( treq );
    
    *req = (SL_msg_request *)SL_REQUEST_NULL;
#ifdef QPRINTF
    SL_msgq_head_debug ( q );
#endif
    return ret;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

int SL_test ( SL_msg_request **req, int *flag, SL_Status *status )
{
    SL_qitem *found=NULL;
    SL_msgq_head *q=NULL;
    SL_msg_request *treq = *req;
    int ret=SL_SUCCESS;

    if ( treq == NULL ) {
	return SL_SUCCESS;
    }

    q = treq->cqueue;

    SL_msg_progress ();
    found = SL_msgq_find ( q, treq->id ); 
    if ( NULL != found ){
	PRINTF(("[%d]:SL_test: found message %d in completion queue\n",SL_this_procid, treq->id));
	ret = found->error;
	*flag = 1;

	if ( NULL != status && SL_STATUS_IGNORE != status ) {
	    status->SL_SOURCE  = ((SL_msg_header *)found->iov[0].iov_base )->from;
	    status->SL_TAG     = ((SL_msg_header *)found->iov[0].iov_base )->tag;
	    status->SL_ERROR   = found->error;
	    status->SL_CONTEXT = ((SL_msg_header *)found->iov[0].iov_base )->context;
	    status->SL_LEN     = ((SL_msg_header *)found->iov[0].iov_base )->len;
	}

	/* Remove message from completion queue */
	SL_msgq_remove ( q, found );
	free ( found->iov[0].iov_base );
	free ( found);
	free ( treq );
	*req = (SL_msg_request*)SL_REQUEST_NULL;
    }
    else {
	*flag = 0;
    }

    return ret;
}

int SL_test_nopg ( SL_msg_request **req, int *flag, SL_Status *status, int *loglength )
{
    SL_qitem *found=NULL;
    SL_msgq_head *q=NULL;
    SL_msg_request *treq = *req;
    int ret=SL_SUCCESS;

    if ( treq == NULL ) {
	return SL_SUCCESS;
    }

      if ( SL_REQUEST_NULL == *req ) {
        *flag = 1;
        SL_msg_set_nullstatus ( status );
        return SL_SUCCESS;
    }

    q = treq->cqueue;

    found = SL_msgq_find ( q, treq->id ); 
    if ( NULL != found ){
	PRINTF(("SL_test: found message %d in completion queue\n", treq->id));
	ret = found->error;
	*flag = 1;

	if ( NULL != status && SL_STATUS_IGNORE != status ) {
	    status->SL_SOURCE  = ((SL_msg_header *)found->iov[0].iov_base )->from;
	    status->SL_TAG     = ((SL_msg_header *)found->iov[0].iov_base )->tag;
	    status->SL_ERROR   = found->error;
	    status->SL_CONTEXT = ((SL_msg_header *)found->iov[0].iov_base )->context;
	    status->SL_LEN     = ((SL_msg_header *)found->iov[0].iov_base )->len;
	    *loglength         = ((SL_msg_header *)found->iov[0].iov_base )->loglength;
	}

	/* Remove message from completion queue */
	SL_msgq_remove ( q, found );
	free ( found->iov[0].iov_base );
	free ( found);
	free ( treq );
#ifdef MINGW
      *req = (SL_msg_request *)SL_REQUEST_NULL;
#else
	*req = SL_REQUEST_NULL;
#endif
    }
    else {
	*flag = 0;
    }

    return ret;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
int SL_cancel ( SL_msg_request **req, int *flag )
{
    SL_msg_request *treq = *req;
    struct SL_proc *dproc=NULL;
    SL_msgq_head *q=NULL;
    SL_qitem *found=NULL;
    
    *flag = 0;
    dproc = treq->proc;
    if ( SL_REQ_RECV == treq->type ) {
	q = dproc->rqueue;
    }
    else if ( SL_REQ_SEND == treq->type ) {
	q = dproc->squeue;
    }

    found = SL_msgq_find ( q, treq->id );
    if ( NULL != found ) {
	/* 
	** The entry is still in the send or recv queue, and not 
	** in the completion queues. We have to verify now, whether
	** the data transfer has alread started. We can only cancel an 
	** operation, if it has not started yet at all.
	*/
	if ( 0 == found->iovpos && 0 == found->lenpos ) {
	    SL_msgq_remove ( q, found );
	    free ( found->iov[0].iov_base );
	    free ( found );
	    free ( treq );
	    *req = (SL_msg_request*)SL_REQUEST_NULL;
//	     printf("[%d]:SL_cancel: canceling the request for id: %d\n ", SL_this_procid  ,treq->id);
	    *flag = 1;
	}
    }
	if (*flag == 0)
	 PRINTF(("[%d]:SL_cancel: could not cancel the request for id: %d\n ", SL_this_procid  ,treq->id));
	
    return SL_SUCCESS;
}

int SL_get_loglength(int len, int dest, int tag, int comm)
{
        NODEPTR curr = insertpt;
        int flag;

        int myrank;

	myrank = Volpex_get_rank();


        do {
            if ( NULL != curr->header ) {
                 if ( curr->header->len   >= len     &&
               	      curr->header->src   == myrank  &&
                      curr->header->dest  == dest    &&
                      curr->header->tag   == tag     &&
                      curr->header->comm  == comm ){

                	flag = 1;
			break;
        }
 
                }
//          Volpex_insert_reuseval(header->dest, header->comm);

            curr = curr->back;
        } while ( curr != head );
	
        if(flag == 1)
		return curr->header->reuse;
	else
		return -1;
}

int SL_cancelmsg_check(SL_proc *dproc )
{

    SL_msg_header *qheader=NULL, header ;
    SL_qitem *curr=NULL, *tcurr=NULL;
    Volpex_proc *proc;
    Volpex_cancel_request *currreq = NULL, *tcurrreq=NULL;
   
    proc = Volpex_get_proc_byid(dproc->id);
    currreq = proc->purgelist;
//    if (dproc->urqueue->first != NULL)
	    curr = dproc->urqueue->first;

if (currreq !=NULL && curr !=NULL){
    while(currreq !=NULL){
	header = currreq->cancel_request;
	while ( curr != NULL ) {
	    qheader = (SL_msg_header *) curr->iov[0].iov_base;
	    if ( ( qheader->cmd  == header.cmd)     &&
		 (qheader->from == header.from )    &&
		 ( qheader->to   == header.to )     &&
		 (qheader->tag  == header.tag)      &&
		 (qheader->context == header.context)&&
		 (qheader->len   >= header.len )    &&
		 (qheader->temp == header.temp)){
		if (curr->lenpos == qheader->len){
		    tcurr = curr;
		    curr = curr->next;
		    tcurrreq = currreq;
		    currreq = currreq->next;
			printf("[%d]:Removing message for proc:%d\n", SL_this_procid,dproc->id);
		    Volpex_remove_purgelist(dproc->id, tcurrreq->id);
		    SL_msgq_remove(dproc->urqueue, tcurr);
		    continue;
		}
  
	    }
		
		    curr = curr->next;
	}
	if (currreq == NULL)
		continue;
	currreq = currreq->next;
    }
    }
    return 1;
    
}
