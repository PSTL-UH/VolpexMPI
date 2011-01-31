















#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"

extern int SL_this_listensock;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;
extern SL_array_t *SL_proc_array;
extern int SL_numprocs;
extern int SL_this_procid;


int print_options();

int main(int argc, char *argv[])
{
    
    char *hostname;
    int port;
    SL_proc *dproc = NULL;
    int myid;
    int next,numprocs=1; 
    char *path;
    int flag = 0, hostfileflag=0;
    char hostfile[256] ;
    int jobid, procid=-1;
    int addflag = 0;   

    if(!strcmp(argv[1],"-help")||!strcmp(argv[1],"--help")){
        MCFAcontrol_print_options();
        exit(-1);
    }

 
    if ( argc < 3 ) {
        printf("./mcfaconrol <hostname> <port_number>\n");
        return MCFA_ERROR;
    }
    
    strcpy(hostfile,"");
    
    next = 3;
    hostname = strdup(argv[1]);
    port = atoi(argv[2]);
   


    SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );
    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );
    
    SL_proc_init ( MCFA_MASTER_ID, hostname, port );
    
    
    MCFA_connect(MCFA_CONSTANT_ID);
    myid = MCFA_connect_stage2();

    SL_this_procid = myid;
    SL_this_procport =  port = 25001;
    SL_proc_init(myid,hostname,25001);
    SL_numprocs = 2;
    
    dproc = SL_array_get_ptr_by_id ( SL_proc_array, SL_this_procid );
    
    
    /* Open a listen socket for this process */
    SL_open_socket_listen_nb ( &SL_this_listensock, SL_this_procport );
    FD_SET ( SL_this_listensock, &SL_send_fdset );
    FD_SET ( SL_this_listensock, &SL_recv_fdset );
    if ( SL_this_listensock > SL_fdset_lastused ) {
        SL_fdset_lastused = SL_this_listensock;
    }
    
    printf(" MY ID IS :::::::::::::::::::::::::::::%d\n",SL_this_procid);
    SL_proc_dumpall();
    
// getting full path of the executable
    MCFA_get_path(argv[argc-1],&path);



    /* Parsing startup options */
    while(next<argc)
    {
        if(!strcmp(argv[next],"-addnp")||!strcmp(argv[next],"--addnp"))
        {
            numprocs = atoi(argv[next+1]);
	    addflag = 1;
            next= next+2;
        }
	if(!strcmp(argv[next],"-addjob")||!strcmp(argv[next],"--addjob"))
        {
	    flag = 1;
            numprocs = atoi(argv[next+1]);
            addflag = 1;
            next= next+2;
        }
	else if(!strcmp(argv[next],"-addhostfile")||!strcmp(argv[next],"--addhostfile"))
        {
            hostfileflag = 1;
            strcpy(hostfile,argv[next+1]);
            addflag = 1;
            next=next+2;
	}
        else if(!strcmp(argv[next],"-deletejob")||!strcmp(argv[next],"--deletejob"))
        {
            jobid = atoi(argv[next+1]);
	    MCFAcontrol_deletejob(jobid);
            next=next+2;
        }
	else if(!strcmp(argv[next],"-deleteproc")||!strcmp(argv[next],"--deleteproc"))
        {
//            jobid = atoi(argv[next+1]);
	    procid = atoi(argv[next+1]);
            MCFAcontrol_deleteproc(procid);
            next=next+2;
        }
        else if(!strcmp(argv[next],"-print")||!strcmp(argv[next],"--print"))
        {
	    MCFAcontrol_print();
            next=next+1;
        }
	
	
        else if(!strcmp(argv[next],"-help")||!strcmp(argv[next],"--help"))
        {
            MCFAcontrol_print_options();
            exit(-1);
        }
	else if(!strcmp(argv[next],"-addprocid") || !strcmp(argv[next],"--addprocid"))
	{
		procid = atoi(argv[next+1]);
		addflag = 1;
		numprocs = 1;
		next=next+2;
	}	
	
        else
        {
            //  MCFA_printf(" Invalid Options ");
            //  exit(-1);
	    
            next=next+1;
        }
    }
    
    if (1 == addflag){
	    MCFAcontrol_add(numprocs,path, flag,hostfileflag, hostfile,procid);	
    }
//    free(hostname); 
    SL_Finalize();    
    return 0;
}
int MCFAcontrol_add(int numprocs, char *path, int flag, int hostfileflag,char *hostfile, int tprocid)
{
    
    struct SL_event_msg_header *header;
//    struct MCFA_header header;
    int cmd=-1, jobid = -1, id=-1,  msglen=-1, port=-1, procid=-1;    
    char executable[256], hostname[256];

    strcpy(executable,"");
    strcpy(hostname,"");


    id = SL_this_procid;
    numprocs = numprocs;
    strcpy(executable,path); 
    
    if(flag == 0){
	cmd = MCFA_CMD_ADD_PROCS;
	jobid = MCFA_EXISTING_JOBID;
    }
    else
    {
	cmd = MCFA_CMD_ADD_JOB;
    }
    
    if (tprocid != -1){
	cmd = MCFA_CMD_ADD_PROCID;
    	procid = tprocid;
    }
    
    header = SL_get_msg_header(cmd, id, jobid, procid, numprocs, msglen, port, executable,
                                hostfile, hostname);

    SL_msg_request *req;
    SL_event_post(header, sizeof(SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);

    SL_Wait (&req, SL_STATUS_NULL);
    printf("Message sent \n");
    return MCFA_SUCCESS;
}


int MCFAcontrol_print()
{
   struct SL_event_msg_header header;

   SL_msg_request *req;

   header.cmd = MCFA_CMD_PRINT_PROCS;
   header.id = SL_this_procid;
   SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
   SL_Wait (&req, SL_STATUS_NULL);
   return MCFA_SUCCESS;
}

int MCFAcontrol_deletejob(int jobid)
{
	struct SL_event_msg_header header;

	header.id = SL_this_procid;
        header.jobid = jobid;
	header.cmd = MCFA_CMD_DELETE_JOB;

        SL_msg_request *req;
        SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
	
	SL_Wait (&req, SL_STATUS_NULL);
        printf("Message sent \n");

	return MCFA_SUCCESS;

}

int MCFAcontrol_deleteproc(int procid)
{
	struct SL_event_msg_header header;
	
	header.id = SL_this_procid;
	header.procid = procid;
	header.cmd = MCFA_CMD_DELETE_PROC;
	
	SL_msg_request *req;
	SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
	SL_Wait (&req, SL_STATUS_NULL);
	printf("Message sent \n");
	return MCFA_SUCCESS;

}

	




int MCFAcontrol_print_options()
{
    printf("NAME\n");
    printf("\tmcfa_control - to add or delete required number of processes :\n");
    printf("SYNOPSIS\n");
    printf("\tmcfa_control [options]\n\n");
    printf("DESCRIPTION\n");
    printf("\t the mcfa_add command is used to add or delete required number of processes\n\n");
    
    printf("\t -addnp,  --addnp\n");
    printf("\t\t specify number of processes to be added\n\n");
    printf("\t -addhostfile, --addhostfile [name]\n");
    printf("\t\t specify the name of hostfile used \n\n");
    printf("\t -help, --help\n");
    printf("\t\t displays a list of options supported by mcfa_add\n\n");
    
    
    printf("\t -addjob,  --addjob\n");
    printf("\t\t specify number of processes to be added with differentjobid\n\n");
    
    printf("\t -deletejob,  --deletejob [jobid]\n");
    printf("\t\t specify the job id for which processes are to be deleted\n\n");
    printf("\t -deleteproc,  --deleteproc [procid]\n");
    printf("\t\t specify the job id and process id for which process is to be deleted\n\n");
    printf("\t -addprocid,  --addprocid [procid]\n");
    printf("\t\t specify the process rank for which is to be added\n\n");

    printf("\t\t -print, --print\n");
    printf("\t\t to print all processes\n\n");
    
    return MCFA_SUCCESS;
}




