

#include "SL.h"
#include "SL_array.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"
#include "SL_proc.h"

extern SL_array_t *SL_proc_array;

extern SL_array_t *SL_request_array;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

int SL_send_post_self(SL_proc *dproc, SL_msg_header *header, char *buf ,int len)
{
    
    SL_msg_header *nheader;
    SL_qitem *elem ;

    elem = SL_msgq_head_check(dproc->rqueue, header);
    if (elem != NULL) {
	PRINTF(("SL_send_post_self: \n"));
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
	PRINTF(("SL_recv_post_self: \n"));
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
    dproc = (SL_proc*)SL_array_get_ptr_by_id ( SL_proc_array, src );
    
    rq    = dproc->rqueue;
    rcq   = dproc->rcqueue;

    if ( dproc->state != SL_PROC_CONNECTED ) {
        ret = SL_proc_init_conn_nb ( dproc, timeout );
	if ( SL_SUCCESS != ret ) {
	    return ret;
	}
        // SL_proc_init_conn ( dproc );
    }

    treq = (SL_msg_request *) malloc ( sizeof (SL_msg_request ));
    if ( NULL == treq ) {
	return SL_ERR_NO_MEMORY;
    }

    header = SL_msg_get_header ( SL_MSG_CMD_P2P,  /* cmd  */
				 src,             /* from */
				 SL_this_procid,  /* to  */
				 tag,             /* tag */
				 context_id,      /* context */
				 len );           /* msg len */
				 
    PRINTF (("SL_recv_post: header from %d to %d tag %d context %d len %d  id %d \n", 
	    header->from, header->to, header->tag, header->context, header->len, 
	    header->id ));


    elem = SL_msgq_head_check ( dproc->urqueue, header );
    if ( elem != NULL ) {
	PRINTF(("SL_recv_post: found message in unexpected message queue\n"));
	
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
	PRINTF(("SL_recv_post: copied %d bytes into real buffer\n", elem->lenpos ));
#ifdef MINGW
	elem->iov[0].iov_base = (char*) header;
	elem->iov[1].iov_base = (char *)buf;
#else
	elem->iov[0].iov_base = header;
	elem->iov[1].iov_base = buf;
	
#endif
	elem->id = header->id;
    }
    else  {
      if(header->from == header->to){
#ifdef MINGW
	SL_recv_post_self (dproc, header,(char *)buf ,len);
#else
	SL_recv_post_self (dproc, header,buf ,len);
#endif
      }
      else {
	elem = SL_msgq_insert ( rq, header, buf, rcq );
      }
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

int SL_send_post ( void *buf, int len, int dest, int tag, int context_id, double timeout, SL_msg_request **req )
{
    SL_msg_header *header=NULL;
    SL_msgq_head *sq=NULL, *scq=NULL;
    struct SL_proc *dproc=NULL;
    SL_msg_request *treq=NULL;
    SL_qitem *elem=NULL;
    int ret;

    /* Step 1. Post the message into the send queue of that proc */
#ifdef MINGW
    dproc = (SL_proc *)SL_array_get_ptr_by_id ( SL_proc_array, dest );
#else
    dproc = SL_array_get_ptr_by_id ( SL_proc_array, dest );
#endif
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
				 len);            /* msg len */
    
    PRINTF (("SL_send_post: header from %d to %d tag %d context %d len %d id %d\n", 
	     header->from, header->to, header->tag, header->context, header->len, 
	     header->id ));
    if(header->from == header->to){
#ifdef MINGW
	SL_send_post_self (dproc, header,(char *)buf ,len);
#else
	SL_send_post_self (dproc, header,buf ,len);
#endif
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
	    PRINTF(("SL_wait: found message %d in completion queue\n", treq->id ));
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

#ifdef MINGW
    *req = (SL_msg_request *)SL_REQUEST_NULL;
#else  
    *req = SL_REQUEST_NULL;
#endif

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
	PRINTF(("SL_test: found message %d in completion queue\n", treq->id));
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

int SL_test_nopg ( SL_msg_request **req, int *flag, SL_Status *status )
{
    SL_qitem *found=NULL;
    SL_msgq_head *q=NULL;
    SL_msg_request *treq = *req;
    int ret=SL_SUCCESS;

    if ( treq == NULL ) {
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
#ifdef MINGW
		*req = (SL_msg_request*)SL_REQUEST_NULL;
#else
	*req = SL_REQUEST_NULL;
#endif
	    
	    *flag = 1;
	}
    }
    
    return SL_SUCCESS;
}
