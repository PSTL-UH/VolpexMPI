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
#ifndef __SL_PROC__
#define __SL_PROC__

#include "SL_internal.h"
#include "SL_array.h"

#define SL_PROC_CONNECTED     2000
#define SL_PROC_CONNECT       2001
#define SL_PROC_ACCEPT        2002
#define SL_PROC_NOT_CONNECTED 2003
#define SL_PROC_UNREACHABLE   2004
#define SL_PROC_CONNECT_STAGE2 2005
#define SL_PROC_ACCEPT_STAGE2 2006

#define TRUE                    1
#define FALSE			0 

extern SL_array_t *SL_proc_array;
extern int SL_this_procid;
extern int SL_this_procport;
extern int SL_numprocs;

extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_fdset_lastused;

int SL_proc_init ( int proc_id, char *hostname, int port );
int SL_proc_finalize(SL_proc *proc);
SL_proc*  SL_proc_get_byfd ( int fd );
int SL_proc_init_conn    ( SL_proc * proc ); 
int SL_proc_init_conn_nb ( SL_proc * proc, double timeout ); 
int SL_proc_read_and_set ( char *filename );
void SL_proc_closeall ( void );
void SL_proc_close ( SL_proc *proc );
void SL_proc_set_connection ( SL_proc *dproc, int sd );
void SL_proc_dumpall ( void );
void SL_proc_handle_error ( SL_proc *proc, int err, int flag );
int SL_init_eventq();
int SL_finalize_eventq();

int SL_proc_id_generate(int flag);
int SL_proc_port_generate();
SL_msg_perf* SL_msg_performance_init();
void SL_msg_performance_insert(int msglen, double time, int msgtype, int elemid, SL_proc *proc);
void SL_print_msg_performance(SL_msg_perf* insertpt);


SL_network_perf* SL_network_performance_init();
void SL_network_performance_insert(double latency, double bandwidth, SL_proc *proc);
void SL_print_net_performance(SL_network_perf *insertpt);
int SL_net_performance_free(SL_proc *tproc);
int SL_msg_performance_free(SL_proc *tproc);

#endif
