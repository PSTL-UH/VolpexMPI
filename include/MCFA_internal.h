#ifndef __MCFA_INTERNAL__
#define __MCFA_INTERNAL__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include "SL_event_handling.h"
#include "SL_proc.h"
#include "SL_msg.h"
#include "SL_msgqueue.h"


/*#define MCFA_CMD_PRINT_PROCS		1
#define MCFA_CMD_ADD_PROCS 		2
#define MCFA_CMD_ADD_JOB		3
#define MCFA_CMD_DELETE_JOB 		4
#define MCFA_CMD_DELETE_PROC  		5
#define MCFA_CMD_PRINT_JOBSTATUS	6
#define MCFA_CMD_PRINT_PROCSTATUS	7
#define MCFA_CMD_PRINT_HOSTSTATUS	8
#define MCFA_CMD_PRINT_ALLJOBSTATUS	9
#define MCFA_CMD_PRINT_ALLPROCSTATUS	10
#define MCFA_CMD_PRINT_ALLHOSTSTATUS	11*/



#define MCFA_MASTER_ID  	-1
#define SL_STATUS_NULL  	0
#define MCFA_FILE_PER_PROC 	1
#define BUFFERSIZE 		2048
#define MCFA_CONTROL_ID  	-2	
#define MAXPROCSPERNODE		200
#define MCFA_CONSTANT_ID	-32	
#define MCFA_EXISTING_JOBID	-1
#define MAXNODES       		 24
#define MAXPROCS 		 100
#define MAXARGUMENTS   		 13
#define MAXPORTSIZE     	 9
#define MAXRANK			 16
#define BUFFERSIZE     		 2048
#define PROCADDRESS		516     //hostname + id


#define	CONDOR		1
#define SSH		2

char  *MCFA_HOSTNAME;
char  *MCFA_PORT;
char  *MCFA_JOBID;
char  *MCFA_ID;
char  *MCFA_PATH;
char  *MCFA_FULLRANK;
char  *MCFA_REDUNDANCY;
char  *MCFA_EVENT_HANDLER;


struct MCFA_ID{
        int procID;
        int jobID;
	char exec[MAXNAMELEN];
        };

struct MCFA_host{
        char 	hostname[MAXHOSTNAMELEN];
        int 	numofProcs;
        int 	lastPortUsed;
	struct 	MCFA_ID *id;
//        char 	executable[MAXNAMELEN];
        int 	status;
        };


struct MCFA_host_node{
        struct MCFA_host_node *next;
        struct MCFA_host hostdata;
        };

struct MCFA_process{
        int 	id;
	char 	*hostname;
//        char 	hostname[MAXHOSTNAMELEN];
        int 	portnumber;
        int 	jobid;
        int 	sock;
	int 	status;
	char 	*executable;
//	char  	executable[MAXNAMELEN];
	char    fullrank[MAXRANK];
        };


struct MCFA_proc_node{
        struct MCFA_proc_node *next;
        struct MCFA_process procdata;
        };



/*struct MCFA_header{
		int 	cmd; 
		int 	jobid;
		int 	procid;
		int 	numprocs;
		int 	id;
                char  	executable[MAXHOSTNAMELEN];   
		char 	hostfile[MAXHOSTNAMELEN];
		char 	hostname[MAXHOSTNAMELEN];
};*/




//struct MCFA_host_node *hostList;
//struct MCFA_proc_node *procList;


int MCFA_dump_info_host(char *host,struct MCFA_host_node *hostList);
int MCFA_dump_info_jobID(int id, struct MCFA_host_node *hostList);
int MCFA_dump_info_jobID_procID(int jobid, int procid, struct MCFA_proc_node *procList);
int MCFA_print_proc(struct MCFA_process *proc);
int MCFA_print_host(struct MCFA_host *host);

int MCFA_initHostList(struct MCFA_host_node **hostList);
struct MCFA_host* MCFA_search_hostname(struct MCFA_host_node *hostList, char *host);
int MCFA_add_hostname(struct MCFA_host_node **hostList, char *hostname,int numofProcs,int lastPortUsed, int status );
int MCFA_printHostlist(struct MCFA_host_node *hostList);
struct MCFA_host* MCFA_init_hostnode(char *name, int ,int);


int MCFA_pack_int(void *packbuf, int *from, int count, int *pos);
int MCFA_pack_string(void *packbuf, char *from, int count, int *pos);
int MCFA_pack_size(int num_of_ints, int num_of_chars, int *buffer_length);
int MCFA_unpack_int(void *unpackbuf, int *to, int count, int *pos);
int MCFA_unpack_string(void *unpackbuf, char *to, int count, int *pos);

int MCFA_initProcList(struct MCFA_proc_node **procList);
int MCFA_add_proc(struct MCFA_proc_node **procList, int procID ,char *host, int portNumber,int jobID,
			int sock,int status, char *exec, char*fullrank );
struct MCFA_process* MCFA_search_proc(struct MCFA_proc_node *procList, int procID);
int MCFA_printProclist(struct MCFA_proc_node *procList);
struct MCFA_process* MCFA_getlast_proc(struct MCFA_proc_node *procList);
void MCFA_free_hostlist(struct MCFA_host_node *hostList);
void MCFA_free_proclist(struct MCFA_proc_node *procList);
int MCFA_proc_close(struct MCFA_proc_node *proclist, int procid);


void write_string ( int hdl, char *buf, int num );
void read_string ( int hdl, char *buf, int num );
void write_int ( int hdl, int val );
void read_int ( int hdl, int *val );


int MCFA_get_nextjobID();
int MCFA_get_nextID();

char* MCFA_pack_proclist(struct MCFA_proc_node *procList, int *msglen);
struct MCFA_proc_node* MCFA_unpack_proclist(char *buffer, int len);
char* MCFA_pack_hostlist(struct MCFA_host_node *hostList, int *msglen);
struct MCFA_host_node* MCFA_unpack_hostlist(char *buffer, int len);


char* MCFA_pack_jobstatus(struct MCFA_proc_node *procList, int jobid, int *msglen);
struct MCFA_proc_node* MCFA_unpack_jobstatus(char *buffer, int len);
char* MCFA_pack_procstatus(struct MCFA_proc_node *procList, int procid, int *msglen);
struct MCFA_process* MCFA_unpack_procstatus(char *buffer, int len);
char* MCFA_pack_hoststatus(struct MCFA_host_node *hostList,char *host, int *msglen);
struct MCFA_host* MCFA_unpack_hoststatus(char *buffer, int len);



int MCFA_printf_init ( int jobid, int procid );
int MCFA_printf_finalize ( void );

int MCFA_init_env();
int MCFA_set_env(char *path, char *hostname, int port, int jobid, int id, int ehandler, char* rank, int red, int flag);

int MCFA_get_total_hosts(struct MCFA_host_node *hostlist);

struct MCFA_proc_node* MCFA_add_procs(struct SL_event_msg_header *header);
struct MCFA_proc_node* MCFA_delete_proc( struct SL_event_msg_header *header);
struct MCFA_proc_node* MCFA_delete_job( struct SL_event_msg_header *header, int *);
int MCFA_clear_procs(struct MCFA_proc_node *procList);

/*struct MCFA_proc_node* MCFA_add_procs(struct MCFA_header *header);
struct MCFA_proc_node* MCFA_delete_proc( struct MCFA_header *header);
struct MCFA_proc_node* MCFA_delete_job( struct MCFA_header *header, int *);*/
int MCFA_remove_proc(struct MCFA_proc_node **procList, int procid);

int MCFA_add_host(struct MCFA_host_node **hostList,struct MCFA_host *node);


int MCFAcontrol_getid();
int MCFAcontrol_add(int, char*, int, int,char*);
int MCFAcontrol_print_options();
int MCFAcontrol_deletejob(int);
int MCFAcontrol_deleteproc(int);
int MCFAcontrol_print();

int MCFA_connect(int id);
struct MCFA_proc_node* MCFA_spawn_processes(char **hostName, char *path, int port, int jobID, int numprocs,int hostCount,int redundancy, int condor_flag);

void MCFA_get_abs_path(char *arg, char **path);
void MCFA_get_path(char *arg, char **path);
char** MCFA_allocate_func(char fileName[MAXHOSTNAMELEN], int *num);
int MCFA_get_exec_name(char *path, char *filename);

int MCFA_proc_exists(struct MCFA_proc_node *procList,int id);
int MCFA_procjob_exists(struct MCFA_proc_node *procList,int id);
int MCFA_check_proc(struct MCFA_proc_node *procList);




int MCFA_event_addprocs(SL_event_msg_header *header, int numprocs);
int MCFA_event_deletejob(SL_event_msg_header *header, int numprocs, int *num);
int MCFA_event_deleteproc(SL_event_msg_header *header, int numprocs);
int MCFA_event_printjobstatus(SL_event_msg_header *header);
int MCFA_event_printprocstatus(SL_event_msg_header *header);
int MCFA_event_printhoststatus(SL_event_msg_header *header);
int MCFA_event_printalljobstatus(SL_event_msg_header *header);
int MCFA_event_printallhoststatus(SL_event_msg_header *header);



char ** MCFA_set_args(int id,char **hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy, int flag);
void MCFA_set_lists(int id,char **hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy);
void MCFA_create_condordesc(char *exe, int numprocs);

int MCFA_update_proclist(struct MCFA_proc_node *procList, int id, char *hostname, int port);
char* MCFA_pack_proc_address(char *name, int id);
int MCFA_unpack_proc_address(char *buf, char **hostname, int *id);
char ** MCFA_read_argfile();
void MCFA_start_condorjob();




/* MACROS */

#endif

