
#ifndef __SL_MSG__
#define __SL_MSG__

#include "SL_internal.h"

#define SL_MSG_CMD_P2P     1000
#define SL_MSG_CMD_CLOSE   1001

#define SL_REQ_SEND  100
#define SL_REQ_RECV  101

#define SL_MSG_DONE    -1000
#define SL_MSG_STARTED -2000
#define SL_MSG_CLOSED  -3000

void SL_msg_progress ( void );
SL_msg_header * SL_msg_get_header ( int cmd, int from, int to, int tag, int context, int len );
int SL_send_post ( void *buf, int len, int dest, int tag, int context_id, double timeout, SL_msg_request **req );
int SL_recv_post ( void *buf, int len, int src, int tag, int context_id, double timeout, SL_msg_request **req );
int SL_send_post_self(SL_proc *dproc, SL_msg_header *header, char *buf ,int len);
int SL_recv_post_self(SL_proc *dproc, SL_msg_header *header, char *buf,int len);
int SL_wait ( SL_msg_request **req, SL_Status *status );
int SL_test ( SL_msg_request **req, int *flag, SL_Status *status );
int SL_test_nopg ( SL_msg_request **req, int *flag, SL_Status *status );
int SL_cancel ( SL_msg_request **req, int *flag );
SL_qitem* SL_msgq_head_check ( SL_msgq_head *head, SL_msg_header *header );
int SL_msg_recv_knownmsg ( SL_proc *dproc, int fd );
int SL_msg_recv_newmsg   ( SL_proc *dproc, int fd );
int SL_msg_send_knownmsg ( SL_proc *dproc, int fd );
int SL_msg_send_newmsg   ( SL_proc *dproc, int fd );
int SL_msg_accept_newconn( SL_proc *dproc, int fd );
int SL_msg_connect_newconn( SL_proc *dproc, int fd );
int SL_msg_closed         ( SL_proc *dproc, int fd );
void SL_msg_set_nullstatus ( SL_Status *status );
void SL_msg_header_dump ( SL_msg_header *header );

#endif

