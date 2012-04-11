#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#include "SL.h"
#include "SL_msgqueue.h"
#include "SL_msg.h"


SL_qitem* SL_msgq_get_first ( SL_msgq_head *head ) 
{
    if ( NULL == head ) {
	return NULL;
    }

    return head->first;
}

SL_qitem* SL_msgq_get_last ( SL_msgq_head *head ) 
{
    if ( NULL == head ) {
	return NULL;
    }

    return head->last;
}

/* Add an entry to the end of the queue */
SL_qitem* SL_msgq_insert ( SL_msgq_head *head, SL_msg_header *header, void *buf,  
			   SL_msgq_head *moveto )
{
    SL_qitem *elem=NULL;

    if ( NULL == head ) {
	return NULL;
    }

    elem = (SL_qitem *) malloc ( sizeof (struct SL_qitem ));
    if ( NULL == elem ) {
      return NULL;
    }

    elem->head       = head;
    elem->id         = header->id;
    elem->iovpos     = 0;
    elem->lenpos     = 0;
    elem->error      = SL_SUCCESS;
    elem->iov[0].iov_base = (char *)header;
    elem->iov[0].iov_len  = sizeof ( SL_msg_header );
    elem->iov[1].iov_base = (char *)buf;
    elem->iov[1].iov_len  = header->len;
    elem->move_to    = moveto;
    elem->next       = NULL;
    elem->prev       = NULL;
    elem->starttime  = SL_Wtime();
    elem->endtime    = -1;
    SL_msgq_append ( head, elem );
    return elem;
}


int SL_msgq_append ( SL_msgq_head *head, SL_qitem *elem )
{

    if ( NULL == head || NULL == elem ) {
//	SL_msgq_delete(head);
	return SL_ERR_GENERAL;
    }

    if ( NULL != head->last ) {
	elem->prev = head->last;
	elem->prev->next = elem;
    }
    else {
	elem->prev = NULL;
    }
    elem->next = NULL;

    head->last = elem;
    head->count++;
    if ( NULL == head->first ) {
	head->first = elem;
    }

    return SL_SUCCESS;
}

int SL_msgq_remove (SL_msgq_head *head, SL_qitem *elem )
{
    SL_qitem *next, *prev;

    if ( NULL == head ) {
	return SL_ERR_GENERAL;
    }

    prev = elem->prev;
    next = elem->next;
    if ( NULL != next ) {
	next->prev = prev;
    }
    if ( NULL != prev ) {
	prev->next = next;
    }
    if ( head->first == elem ) {
	head->first = next;
    }
    if ( head->last == elem ) {
	head->last = prev;
    }

    elem->next = NULL;
    elem->prev = NULL;
    elem->head = NULL;

    head->count--;
    return SL_SUCCESS;
}

/* Delete the entry at the head of the queue */
int SL_msgq_delete (SL_msgq_head *head )
{
    SL_qitem *curr=head->last;

    if ( NULL == head ) {
	return SL_ERR_GENERAL;
    }

    if ( NULL != head->last ) {
	SL_msgq_remove ( head, curr );
	free ( curr );
    }

    return SL_SUCCESS;
}

/* Find an entry based on the id and return the pointer to the according element */
SL_qitem* SL_msgq_find (SL_msgq_head *head, int id )
{
    SL_qitem *curr, *elem=NULL;

    curr = head->first;
    while ( NULL != curr ) {
	if ( curr->id == id ) {
	    elem = curr;
	    break;
	}
	curr = curr->next;
    }

    return elem;
}

int SL_msgq_move ( SL_msgq_head *head1, SL_msgq_head *head2, SL_qitem *elem )
{
/*    PRINTF(("[%d]:SL_msgq_move: moving elem %d from %s to %s len %d time %f\n", SL_this_procid,elem->id, 
	 head1->name, head2->name , elem->iov[1].iov_len, elem->endtime-elem->starttime));*/
		

    SL_msgq_remove ( head1, elem );
	
    if(head2 == NULL && SL_this_procid == SL_PROXY_SERVER){
	if(NULL != elem->iov[0].iov_base )
		free ( elem->iov[0].iov_base );
	if(NULL != elem->iov[1].iov_base )
		free ( elem->iov[1].iov_base );
	if(NULL != elem)
		free(elem);

	return SL_SUCCESS;
    }

	 
    return SL_msgq_append ( head2, elem );
}

int SL_msgq_move_tohead ( SL_msgq_head *head1, SL_msgq_head *head2, SL_qitem *elem )
{
    SL_qitem *tfirst=NULL;

    PRINTF(("[%d]:SL_msgq_move: moving elem %p from %s to %s \n", SL_this_procid,elem, 
	 head1->name, head2->name ));
    SL_msgq_remove ( head1, elem );

    tfirst = head2->first;
    head2->first = elem;
    head2->count++;
    elem->next = tfirst;
    
    if ( NULL != tfirst ) {
	tfirst->prev = elem;
    }

    if ( NULL == head2->last ) {
	head2->last = elem;
    }

    return SL_SUCCESS;
}


void SL_msgq_set_error ( SL_qitem *elem, int error ) 
{
    elem->error = error;
    return;
}


SL_msgq_head* SL_msgq_head_init ( char *name )
{
    SL_msgq_head* head = (SL_msgq_head *) malloc ( sizeof (struct SL_msgq_head ));
    if ( NULL == head ) {
	return NULL;
    }

    head->count = 0;
    head->first = NULL;
    head->last  = NULL;
    head->name  = strdup ( name );
    return head;
}

int SL_msgq_head_finalize ( SL_msgq_head *head )
{
    SL_qitem *curr=NULL, *prev=NULL;

    curr = head->last;
    while ( NULL != curr ) {
	prev = curr->prev;
	SL_msgq_delete ( head  );
	curr = prev;
    }
    
    free ( head->name );
    free ( head );
    return SL_SUCCESS;
}

SL_qitem* SL_msgq_head_check ( SL_msgq_head *head, SL_msg_header *header )
{
    SL_msg_header *qheader=NULL;
    SL_qitem *curr=NULL;

    if ( NULL == head || NULL == header ) {
	return NULL;
    }
    
    curr = head->first;
    while ( curr != NULL ) {
	qheader = (SL_msg_header *) curr->iov[0].iov_base;
	if ( ( qheader->cmd  == header->cmd)     && 
	     ((qheader->from == header->from )  ||
	      (qheader->from == SL_ANY_SOURCE )) &&
	     ( qheader->to   == header->to )    &&
	     ((qheader->tag  == header->tag)    ||
	      (qheader->tag  == SL_ANY_TAG ))   && 
	     ((qheader->context == header->context) ||
	      (qheader->context == SL_ANY_CONTEXT )) &&
	     (qheader->len   >= header->len )) {
	     break;
	}
	curr = curr->next;
    }
    
    return curr;
}


 void SL_msgq_head_debug ( SL_msgq_head *head )
{
    int i;
    SL_qitem *curr=NULL;

    printf("%s: count=%d first=%p last=%p\n", 
	   head->name, head->count, head->first, 
	   head->last );

    curr = head->first;
    for ( i=0; i<head->count; i++) {
	SL_msgq_debug ( curr);
	curr = curr->next;
    }
    return;
}
 

 void SL_msgq_debug ( SL_qitem *elem )
{
    int i;

    PRINTF(("   id=%d addr=%p head=%p next=%p prev=%p iovpos=%d lenpos=%d iov=%p \n", 
	   elem->id, elem, elem->head, elem->next, elem->prev, 
	   elem->iovpos, elem->lenpos, elem->iov ));
    for ( i=0; i< 2; i++ ) {
	PRINTF(("     iov[%d].iov_base %p iov[%d].iov_len = %d\n", 
	       i, elem->iov[i].iov_base, i, (int) elem->iov[i].iov_len ));
    }
    
}
   
void SL_msgq_move_tolast(SL_msgq_head *head, SL_qitem *elem)
{
        SL_msgq_remove(head,elem);
        SL_msgq_append(head,elem);
}


