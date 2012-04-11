#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#include "SL_internal.h"
 
#define SL_CMD_ADD_PROC			1
#define SL_CMD_DELETE_PROC		2
#define SL_CMD_ADD_EXISTING_PROC        17
#define SL_CMD_START_COMM               18


#define MCFA_CMD_PRINT_PROCS          	3
#define MCFA_CMD_ADD_PROCS              4
#define MCFA_CMD_ADD_JOB                5
#define MCFA_CMD_DELETE_JOB             6
#define MCFA_CMD_DELETE_PROC            7
#define MCFA_CMD_PRINT_JOBSTATUS        8
#define MCFA_CMD_PRINT_PROCSTATUS       9
#define MCFA_CMD_PRINT_HOSTSTATUS       10
#define MCFA_CMD_PRINT_ALLJOBSTATUS     11
#define MCFA_CMD_PRINT_ALLPROCSTATUS    12
#define MCFA_CMD_PRINT_ALLHOSTSTATUS    13
#define MCFA_CMD_CLEAR_PROCS		14
#define MCFA_CMD_GETID			15
#define MCFA_CMD_ADD_PROCID		16

int SL_event_post(void *buf, int len, int dest, int tag, int context_id,SL_msg_request **req);
void SL_event_progress(SL_qitem*);
SL_qitem* SL_get_next_event();

/*struct SL_event_msg_header{
	int cmd;
	int procid;
};*/

struct SL_event_msg_header{
                int     cmd;
		int 	id;
                int     jobid;
                int     procid;
                int     numprocs;
                char    executable[256];
                char    hostfile[256];
                char    hostname[256];
		int 	msglen;
		int 	port;
};




typedef struct SL_event_msg_header SL_event_msg_header;

typedef int SL_event_handle(void *buf, int len);

int SL_add_proc(void *buf, int len);
int SL_delete_proc(void *buf, int len);
int SL_add_existing_proc(void *buf, int len);
int SL_start_communication(void *buf, int id);


SL_event_msg_header* SL_init_msgheader();
SL_event_msg_header* SL_get_msg_header (int cmd, int id, int jobid, int procid, int numprocs, int msglen, int port ,
                             char *executable, char *hostfile, char *hostname);


SL_qitem* SL_get_next_event_noremove();
void SL_remove_event(SL_qitem *elem);
void SL_move_eventtolast(SL_qitem *elem);
