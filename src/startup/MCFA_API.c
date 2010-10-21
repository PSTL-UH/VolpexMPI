#include "mpi.h"
#include "MCFA_internal.h"
#include "SL.h"


extern char* fullrank;
extern int SL_this_listensock;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;
extern SL_array_t *SL_proc_array;
extern int SL_numprocs;		
extern int SL_this_procid;
extern SL_array_t *Volpex_proc_array;
extern SL_array_t *Volpex_comm_array;
extern int Volpex_numprocs;
extern int redundancy;

int MCFA_proc_read_init(char *msgbuf,int len);


int MCFA_Init()
{
    char *hostname;
    char  *msgbuf = NULL;
    int port;
    int	 msglen;
    int id,jobID,event_handler_id;
    char *path;		
    char newpath[BUFFERSIZE] = ".";
    int spawn_flag ;
     


    path 		= strdup(getenv("MCFA_PATH"));
    hostname 		= strdup(getenv("MCFA_HOSTNAME"));
    port 		= atoi(getenv("MCFA_PORT"));
    jobID 		= atoi(getenv("MCFA_JOBID"));
    id 			= atoi(getenv("MCFA_ID"));
    event_handler_id    = atoi(getenv("MCFA_EVENT_HANDLER"));
    fullrank		= strdup(getenv("MCFA_FULLRANK"));
    redundancy		= atoi(getenv("MCFA_REDUNDANCY"));
    spawn_flag		= atoi(getenv("MCFA_SPAWN_FLAG"));	
/*    
    PRINTF(("path           : %s\n \
        hostname         : %s\n \
        port             : %d\n \
        jobID            : %d\n \
        id               : %d\n \
        event_handler_id : %d\n \
        red              : %d\n \
        flag             : %d\n",
        path, hostname, port, jobID, id, event_handler_id,redundancy,spawn_flag));
*/

    char *pos;
    pos = strrchr(path, '/');
    strncpy(newpath, path, pos - path +1);
    chdir(newpath);
	
//    MCFA_printf_init(jobID,id);	   

    SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );
    SL_array_init ( &(Volpex_proc_array), "Volpex_proc_array", 32 );
    SL_array_init ( &(Volpex_comm_array), "Volpex_comm_array", 32 );

    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );


    /* Add the startprocs process to the SL_array */
    SL_proc_init ( MCFA_MASTER_ID, hostname, port );
    SL_this_procid = id;

	char myhostname[512];
	int myid;
	struct SL_event_msg_header header;
    SL_msg_request *req;


	/*CONDOR*/
	if(spawn_flag == 1){
		gethostname(myhostname, 512);
		myid = MCFA_connect(-64);
		SL_this_procid = myid;
//		SL_proc_init(myid,hostname,port);

		printf("My id is %d\n",myid);
		printf("My hostname is %s\n", myhostname);
	

		header.id = myid;
		header.cmd = MCFA_CMD_GETID;
		header.port = SL_this_procport;
		strcpy(header.hostname, myhostname);

    		SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
		SL_Wait (&req, SL_STATUS_NULL);
	        printf("Message sent \n");
	}

    /* Determine the length of the message to be received */
    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );

    /* Receive the buffer containing all the information */
    msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);

    SL_numprocs = MCFA_proc_read_init(msgbuf,msglen);
    Volpex_numprocs = SL_numprocs;


    SL_init_internal();
//    SL_proc_dumpall();
    

//    PRINTF(("My id is-------------------------------->%d\n\n",id));
    
//    SL_Send(&SL_this_listensock, sizeof(int),MCFA_MASTER_ID,0,0);

    free(msgbuf);	
    return MCFA_SUCCESS;
}




int MCFA_Finalize ( void )
{
	
//    MCFA_printf_finalize();	
    SL_Finalize ();
    return MCFA_SUCCESS;
}

int MCFA_proc_read_init(char *msgbuf,int len)
{

    int procid,portused,jobid,status;
    int pos = 0;
    char host[MAXHOSTNAMELEN], exec[MAXNAMELEN], fullrank[MAXRANK];
    int proc_count = 0;
    int sock;
    int i=0;
    for(i=0;i<len;)
    {
	proc_count ++ ;
	
	MCFA_unpack_int(msgbuf, &procid ,1,&pos);
	MCFA_unpack_string(msgbuf, host ,MAXHOSTNAMELEN,&pos);
	MCFA_unpack_int(msgbuf, &portused ,1,&pos);              
	MCFA_unpack_int(msgbuf, &jobid ,1,&pos);
	MCFA_unpack_int(msgbuf, &sock ,1,&pos);	
	MCFA_unpack_int(msgbuf, &status ,1,&pos);
	MCFA_unpack_string(msgbuf, exec, MAXNAMELEN, &pos);
	MCFA_unpack_string(msgbuf, fullrank, MAXRANK, &pos);
//	PRINTF(("Fullrank = %s\n",fullrank));
	i = pos;
	SL_proc_init ( procid, host, portused );
	Volpex_init_proc(procid, host, portused, fullrank);
    }
    
    return proc_count;

}

int SL_add_proc(void *buf, int len)
{
	int numprocs;
	numprocs = MCFA_proc_read_init(buf,len);
	SL_numprocs += numprocs;
	PRINTF(("[%d] :Added new proc to list \n",SL_this_procid));
        return SL_SUCCESS;
}

int SL_delete_proc(void *buf, int len)
{

    struct MCFA_proc_node* proclist;
    struct MCFA_proc_node* curr = NULL;
    SL_proc *dproc;

    proclist = MCFA_unpack_proclist(buf,len);	
    curr = proclist;

    while(curr!=NULL)
    {
	
    	dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata.id  );
	PRINTF(("[%d]:MCFA_API: Handling error for proc %d\n",SL_this_procid,dproc->id));
	if (dproc->id == SL_this_procid){
		printf("[%d]:Bye!!!! I am signing off", SL_this_procid);
		MCFA_printf_finalize();
//		exit(-1);
	}
    	SL_proc_handle_error ( dproc, SL_ERR_PROC_UNREACHABLE,FALSE);	
//	Volpex_set_not_connected(dproc->id);	
	curr = curr->next;
     }
        return SL_SUCCESS;
}
