#ifndef __SL_PROC__
#define __SL_PROC__

#include "SL_internal.h"
#include "SL_array.h"

#define SL_PROC_CONNECTED     2000
#define SL_PROC_CONNECT       2001
#define SL_PROC_ACCEPT        2002
#define SL_PROC_NOT_CONNECTED 2003
#define SL_PROC_UNREACHABLE   2004

extern SL_array_t *SL_proc_array;
extern int SL_this_procid;
extern int SL_this_procport;
extern int SL_numprocs;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_fdset_lastused;

int SL_proc_init ( int proc_id, char *hostname, int port );
SL_proc*  SL_proc_get_byfd ( int fd );
int SL_proc_init_conn    ( SL_proc * proc ); 
int SL_proc_init_conn_nb ( SL_proc * proc, double timeout ); 
int SL_proc_read_and_set ( char *filename );
void SL_proc_closeall ( void );
void SL_proc_close ( SL_proc *proc );
void SL_proc_set_connection ( SL_proc *dproc, int sd );
void SL_proc_dumpall ( void );
void SL_proc_handle_error ( SL_proc *proc, int err );
#endif


