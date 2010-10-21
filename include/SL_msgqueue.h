#ifndef __SL_MSGQUEUE__
#define __SL_MSGQUEUE__

#include "SL_internal.h"

SL_msgq_head* SL_msgq_head_init ( char *name );
int SL_msgq_head_finalize ( SL_msgq_head *head );
void SL_msgq_head_debug ( SL_msgq_head *head );

SL_qitem* SL_msgq_get_first ( SL_msgq_head *head );
SL_qitem* SL_msgq_get_last ( SL_msgq_head *head );
SL_qitem* SL_msgq_insert ( SL_msgq_head *head, SL_msg_header *header, void * buf,  
			   SL_msgq_head *moveto );
int SL_msgq_append (SL_msgq_head *head, SL_qitem *elem );
int SL_msgq_remove (SL_msgq_head *head, SL_qitem *elem );
int SL_msgq_delete (SL_msgq_head *head );
SL_qitem*  SL_msgq_find (SL_msgq_head *head, int id );
int SL_msgq_move ( SL_msgq_head *head1, SL_msgq_head *head2, SL_qitem *elem );
int SL_msgq_move_tohead ( SL_msgq_head *head1, SL_msgq_head *head2, SL_qitem *elem );
void SL_msgq_set_error ( SL_qitem *elem, int error );
void SL_msgq_debug ( SL_qitem *elem );
void SL_msgq_move_tolast(SL_msgq_head *head, SL_qitem *elem);
#endif


