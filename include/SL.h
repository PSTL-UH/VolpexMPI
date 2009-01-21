#ifndef __SL_H__
#define __SL_H__


#include "SL_internal.h"

/* Structures */
typedef struct SL_msg_request*  SL_Request;
struct SL_status {
  int  SL_SOURCE;
  int  SL_TAG;
  int  SL_ERROR;
  int  SL_CONTEXT;
  int  SL_LEN;
};
typedef struct SL_status SL_Status;


/* Constants */
#define SL_SUCCESS				0
#define SL_ERR_GENERAL			1
#define SL_ERR_NO_MEMORY		2
#define SL_ERR_INTERNAL			3
#define SL_ERR_UNDEFINED		4
#define SL_ERR_PROC_UNREACHABLE	5
#define SL_UNDEFINED			6

#define SL_ANY_SOURCE  -1
#define SL_ANY_TAG     -2
#define SL_ANY_CONTEXT -3

#define SL_STATUS_IGNORE (void *) -4
#define SL_REQUEST_NULL  (void *) -5
#define SL_PROC_NULL              -6

#define SL_PROC_WORLD 0
#define SL_PROC_SELF  1


/* Prototypes */
int SL_Init     ( void );
int SL_Finalize ( void );
int SL_Get_id   ( int context_id, int *rank );
int SL_Get_size ( int context_id, int *size ); 
double SL_Wtime ( void );
int SL_Abort ( int context_id, int errcode );

int SL_Send    ( void *buf, int len, int dest, int tag, int context_id );
int SL_Recv    ( void *buf, int len, int from, int tag, int context_id, SL_Status *status );
int SL_Isend   ( void *buf, int len, int dest, int tag, int context_id, SL_Request *req );
int SL_Irecv   ( void *buf, int len, int dest, int tag, int context_id, SL_Request *req );
int SL_Wait    ( SL_Request *req, SL_Status *status );
int SL_Waitall ( int num, SL_Request * reqs, SL_Status *stats );
int SL_Test    ( SL_Request *req, int *flag, SL_Status * status );

int SL_Cancel  ( SL_Request *req, int *flag );



#endif

