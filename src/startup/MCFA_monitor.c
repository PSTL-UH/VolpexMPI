#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"

int MCFA_getjobstatus(int jobid );
int MCFA_getprocstatus(int procid );
int MCFA_gethoststatus(char* hostname );
int MCFA_getalljobstatus();
int MCFA_getallhoststatus();
int print_options();
int MCFA_clearprocs();

extern int SL_this_listensock;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;
extern SL_array_t *SL_proc_array;
extern int SL_numprocs;
extern int SL_this_procid;


int main(int argc, char *argv[])
{

    char 	*hostname;
    int 	port;
    SL_proc 	*dproc = NULL;
    int 	myid;
    int 	next = 1;
    int     	jobid;
    int     	procid;
   

 
    if ( argc < 3 ) {
        printf("./mcfaconrol <hostname> <port_number>\n");
        return MCFA_ERROR;
    }
    
    
    
    hostname = strdup(argv[1]);
    port = atoi(argv[2]);
    SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );
    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );

    SL_proc_init ( MCFA_MASTER_ID, hostname, port );


    MCFA_connect(MCFA_CONSTANT_ID);
    myid = MCFA_connect_stage2();

    SL_this_procid = myid;
    SL_this_procport =  port = 25000;
    SL_proc_init(myid,hostname,25000);
    SL_numprocs = 2;

    dproc = SL_array_get_ptr_by_id ( SL_proc_array, SL_this_procid );


    /* Open a listen socket for this process */
/*    SL_open_socket_listen_nb ( &SL_this_listensock, SL_this_procport );
    FD_SET ( SL_this_listensock, &SL_send_fdset );
    FD_SET ( SL_this_listensock, &SL_recv_fdset );
    if ( SL_this_listensock > SL_fdset_lastused ) {
        SL_fdset_lastused = SL_this_listensock;
    }*/
    
    printf(" MY ID IS :::::::::::::::::::::::::::::%d\n",SL_this_procid);
    SL_proc_dumpall();

   next = 3; 

	while(next<argc)
	{
		 if(!strcmp(argv[next],"-getjobstatus")||!strcmp(argv[next],"--getjobstatus"))
                {
                        jobid = atoi(argv[next+1]);
			MCFA_getjobstatus(jobid);
                        next += 2;
                }
                else if(!strcmp(argv[next],"-getprocstatus")||!strcmp(argv[next],"--getprocstatus"))
                {
		//	procjobid = atoi(argv[next+1]);
			procid	= atoi(argv[next+1]);
			MCFA_getprocstatus(procid );
			next += 2; 
                }
                else if(!strcmp(argv[next],"-gethoststatus")||!strcmp(argv[next],"--gethoststatus"))
                {
			strcpy(hostname, argv[next+1]);
			MCFA_gethoststatus(hostname);
			next += 2;
                }
		else if(!strcmp(argv[next],"-getalljobstatus")||!strcmp(argv[next],"--getalljobstatus"))
		{
                        MCFA_getalljobstatus();
			next += 1;
		}	
		else if(!strcmp(argv[next],"-getallhoststatus")||!strcmp(argv[next],"--getallhoststatus"))
                {
                        MCFA_getallhoststatus();
                        next += 1;
                }

		else if(!strcmp(argv[next],"-clearprocs")||!strcmp(argv[next],"--clearprocs"))
		{
			MCFA_clearprocs();
			next += 1;
		}	
		else if(!strcmp(argv[next],"-help")||!strcmp(argv[next],"--help"))
		{
			print_options();
			exit(-1);
		}
		else
		next += 1;

	}	


free(hostname);	

SL_Finalize();
	return 0;
}


int MCFA_getalljobstatus()
{
    char  *msgbuf;
    int  msglen;
    SL_msg_request *req;
    struct SL_event_msg_header header;
    struct MCFA_proc_node* proclist;

    header.cmd = MCFA_CMD_PRINT_ALLJOBSTATUS;
    header.id = SL_this_procid;

    SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");

    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );
	msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
    }

    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
    printf("received data \n");

    proclist = MCFA_unpack_proclist(msgbuf,msglen);
    MCFA_printProclist(proclist);
    free(msgbuf);
    return MCFA_SUCCESS;
}

int MCFA_getallhoststatus()
{
    char  *msgbuf;
    int  msglen;
    struct SL_event_msg_header header;
    SL_msg_request *req;
    struct MCFA_host_node* hostlist;

    header.cmd = MCFA_CMD_PRINT_ALLHOSTSTATUS;
    header.id = SL_this_procid;

    SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");

    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );
     msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
    }
    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
    printf("received data from \n" );

    hostlist = MCFA_unpack_hostlist(msgbuf,msglen);
    MCFA_printHostlist(hostlist);
    free(msgbuf);
    return MCFA_SUCCESS;
}


int MCFA_getjobstatus(int jobid )
{

    char  *msgbuf;
    int  msglen;
    struct SL_event_msg_header header;
    SL_msg_request *req;
    struct MCFA_proc_node *list;

    header.cmd = MCFA_CMD_PRINT_JOBSTATUS;
    header.jobid = jobid;
    header.id = SL_this_procid;

    SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");
    
    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );
	msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
    }
    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
    printf("received data  \n" );
	
    list = MCFA_unpack_jobstatus(msgbuf, msglen);
    MCFA_printProclist(list);
    free(msgbuf);	
    return MCFA_SUCCESS;
}


int MCFA_getprocstatus(int procid )
{
            
    char  *msgbuf;
    int  msglen;
    struct SL_event_msg_header header;
    SL_msg_request *req;
    struct MCFA_process *proc;

    header.cmd = MCFA_CMD_PRINT_PROCSTATUS;
    header.procid = procid;
    header.id = SL_this_procid;
//    header.jobid = procjobid;

    SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");

    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );
    msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
    }

    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
    printf("received data \n");

    proc = MCFA_unpack_procstatus(msgbuf, msglen);
    MCFA_print_proc(proc);
    free(msgbuf);
   return MCFA_SUCCESS;
}

int MCFA_gethoststatus(char* hostname )
{
    char  *msgbuf;
    int  msglen;
    struct SL_event_msg_header header;
    SL_msg_request *req;
    struct MCFA_host* host;

    header.cmd = MCFA_CMD_PRINT_HOSTSTATUS;
    strcpy(header.hostname,hostname);
    header.id = SL_this_procid;

    SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");

    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );
    msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
    }

    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
    printf("received data from \n");
    
    host = MCFA_unpack_hoststatus(msgbuf, msglen);
    MCFA_print_host(host);
    free(msgbuf);
    return MCFA_SUCCESS;
}

int MCFA_clearprocs()
{
    struct SL_event_msg_header header;
    SL_msg_request *req;
    
    header.cmd = MCFA_CMD_CLEAR_PROCS;
    header.id = SL_this_procid;

    SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");
    return MCFA_SUCCESS;
}

int print_options()
{
	printf("NAME\n");
        printf("\tmcfa_monitor - to monitor the progress of required number of processes :\n");
	printf("SYNOPSIS\n");
        printf("\tmcfa_monitor [options]\n\n");
	printf("DESCRIPTION\n");
	printf("\t the mcfa_monitor command is used to monitor the the progress of specified mcfa_run command\n\n");
	printf("\t -mcfarunhost, --mcfarunhost [hosts]\n");
	printf("\t to specify to which hosts to connect\n\n");
	printf("\t -mcfaport, --mcfaport [port]\n");
	printf("\t to specify through which port that host is connected\n\n");
	printf("\t -getjobstatus, --getjobstatus [jobid]\n");
	printf("\t to print details of jobs on each host according to the jobid\n\n");
	printf("\t -getprocstatus, --getprocstatus [procid]\n");
        printf("\t to print details of the process with given process id\n\n");
	printf("\t -gethoststatus, --gethoststatus [hostname]\n");
        printf("\t to print details of jobs running given host \n\n");
	printf("\t -getalljobstatus, --getalljobstatus \n");
        printf("\t to print details of all jobs running \n\n");
	printf("\t -getallhoststatus, --getallhoststatus [hostname]\n");
        printf("\t to print details of jobs running on all hosts \n\n");

	return MCFA_SUCCESS;
}




