#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
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
extern int Volpex_this_procid;
extern int SL_proxy_server_socket;
int MCFA_proc_read_init(char *msgbuf,int len);
struct MCFA_init_nodes{
	double timeval;
	int id;
};
typedef struct MCFA_init_nodes MCFA_init_nodes;

int MCFA_time_sort(MCFA_init_nodes *a, int size);
int MCFA_update_slprocid(int oldid, int newid);
int MCFA_Init(int argc, char **argv)
{
    char *hostname;
    char  *msgbuf = NULL;
    int port;
    int	 msglen = 0;
    int id,event_handler_id;
    char *path;		
    char newpath[BUFFERSIZE] = ".";
    int spawn_flag, cluster_flag ;
    int ret;


    path 		= strdup(getenv("MCFA_PATH"));
    hostname 		= strdup(getenv("MCFA_HOSTNAME"));
    port 		= atoi(getenv("MCFA_PORT"));
    id 			= atoi(getenv("MCFA_ID"));
    event_handler_id    = atoi(getenv("MCFA_EVENT_HANDLER"));
    redundancy		= atoi(getenv("MCFA_REDUNDANCY"));
    spawn_flag		= atoi(getenv("MCFA_SPAWN_FLAG"));	
    cluster_flag		= atoi(getenv("MCFA_CLUSTER_FLAG"));	
    
    PRINTF(("path           : %s\n \
        hostname         : %s\n \
        port             : %d\n \
        id               : %d\n \
        event_handler_id : %d\n \
        red              : %d\n \
        flag             : %d\n",
        path, hostname, port, id, event_handler_id,redundancy,spawn_flag));



    if (spawn_flag == SSH || spawn_flag == RANDOM){
	    char *pos;
	    pos = strrchr(path, '/');
	    strncpy(newpath, path, pos - path +1);
	   chdir(newpath);
    }

    SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );
    SL_array_init ( &(Volpex_proc_array), "Volpex_proc_array", 32 );
    SL_array_init ( &(Volpex_comm_array), "Volpex_comm_array", 32 );

    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );


    /* Add the startprocs process to the SL_array */
    SL_proc_init ( MCFA_MASTER_ID, hostname, port );
    SL_this_procid = id;
    PRINTF(("My id is %d\n",SL_this_procid));
    PRINTF(("My hostname is %s redundancy:%d, flag:%d\n",hostname,redundancy, cluster_flag));

	char myhostname[512];
	int myid;
	char *hname;
	struct SL_event_msg_header header;
        SL_msg_request *req;


	/*CONDOR*/
	if(spawn_flag == 1 || spawn_flag == 2 || spawn_flag == RANDOM){
        	hname = (char*) malloc (256 *sizeof(char));

        	MCFA_get_ip(&hname);
        	strcpy(myhostname,hname);
		ret = MCFA_connect(-64);
		if (ret != SL_SUCCESS){
         	   printf("Could not recieve correct id\n");
	           exit(-1);
        	}

		myid = MCFA_connect_stage2();
		
		SL_this_procid = myid;

		printf("[%d]:My id is %d\n",SL_this_procid,myid);
		printf("[%d]:My hostname is %s\n",SL_this_procid, myhostname);
		if (myid == 104)
			exit(-1);

		header.id = myid;
		header.cmd = MCFA_CMD_GETID;
		header.port = SL_this_procport;
		strcpy(header.hostname, myhostname);

    		SL_event_post(&header, sizeof(struct SL_event_msg_header),MCFA_MASTER_ID,0,0, &req);
		SL_Wait (&req, SL_STATUS_NULL);
//	        printf("Message sent \n");
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

//    SL_proc_init ( MCFA_PROXY_ID, hostname, 6262 );
//    sleep(5);

int i,comm_id;
if(cluster_flag == COMMUNICATION){
for(i=0;i<SL_numprocs;i++){
	SL_Recv (&comm_id , sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);

	SL_start_communication(msgbuf, comm_id);
}

/*int *commarray;
commarray = (int*) malloc (SL_numprocs+1 * sizeof(int));
if(redundancy>1){
	SL_Recv (commarray,SL_numprocs+1 * sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
	SL_Recv (&comm_id , sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);
	
}
*/


}
	SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);

	Volpex_numprocs = MCFA_proc_read_volpex_procs(msgbuf,msglen);

#ifdef PROXY	
	int rbuf;
	SL_proc *proc;
	SL_Recv ( &rbuf, sizeof(int), MCFA_PROXY_ID, 0, 0, SL_STATUS_NULL);
	proc  = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, SL_PROXY_SERVER );
	SL_proxy_server_socket = proc->sock;
#endif

    free(msgbuf);	
    return MCFA_SUCCESS;
}

int SL_proxy_connect()
{
	SL_proc *proc;
	int ret;
	int tmp;
	int terr=0;

    	socklen_t len=sizeof(int);

	proc  = (SL_proc *) SL_array_get_ptr_by_id ( SL_proc_array, SL_PROXY_SERVER );
        if(proc->state != SL_PROC_CONNECTED){
        	SL_proc_init_conn_nb ( proc, proc->timeout );
        }

	getsockopt (proc->sock, SOL_SOCKET, SO_ERROR, &terr, &len);
        printf("[%d]:SL_msg_connect_proxy: terr = 0. Trying handshake to proxy %d\n",SL_this_procid,
                proc->id);
        ret = SL_socket_write ( proc->sock, (char *) &SL_this_procid, sizeof(int),
                                proc->timeout );
	PRINTF(("[%d]: SL_msg_connect_proxy: in SL_socket_read socket %d procid %d\n",
                    SL_this_procid, proc->sock, proc->id));
        ret = SL_socket_read ( proc->sock, ( char *) &tmp, sizeof(int),
                               SL_READ_MAX_TIME);

	proc->state = SL_PROC_CONNECTED;
	SL_proxy_server_socket = proc->sock;
	FD_SET ( proc->sock, &SL_send_fdset );
        FD_SET ( proc->sock, &SL_recv_fdset );
	proc->sendfunc = SL_msg_send_newmsg;
        proc->recvfunc = SL_msg_recv_newmsg;

	return MCFA_SUCCESS;
}

int MCFA_Finalize ( void )
{
    
//    MCFA_printf_finalize();	
    SL_Finalize ();
    return MCFA_SUCCESS;
}

int MCFA_proc_read_volpex_procs(char *msgbuf,int len)
{

    int procid,portused,jobid,status,volpex_id;
    int pos = 0;
    char host[MAXHOSTNAMELEN], exec[MAXNAMELEN], fullrank[MAXRANK];
    int proc_count = 0;
    int sock;
    int i=0;
    for(i=0;i<len;)
    {
        proc_count ++ ;

        MCFA_unpack_int(msgbuf, &procid ,1,&pos);
	MCFA_unpack_int(msgbuf, &volpex_id ,1,&pos);
        MCFA_unpack_string(msgbuf, host ,MAXHOSTNAMELEN,&pos);
        MCFA_unpack_int(msgbuf, &portused ,1,&pos);
        MCFA_unpack_int(msgbuf, &jobid ,1,&pos);
        MCFA_unpack_int(msgbuf, &sock ,1,&pos);
        MCFA_unpack_int(msgbuf, &status ,1,&pos);
        MCFA_unpack_string(msgbuf, exec, MAXNAMELEN, &pos);
        MCFA_unpack_string(msgbuf, fullrank, MAXRANK, &pos);
//      PRINTF(("Fullrank = %s\n",fullrank));
        i = pos;
        Volpex_init_proc(volpex_id,procid, host, portused, fullrank);
	if (SL_this_procid == procid)
		Volpex_this_procid = volpex_id;
    }

    return proc_count;

}


int MCFA_proc_read_init(char *msgbuf,int len)
{

    int procid,portused,jobid,status,volpex_id;
    int pos = 0;
    char host[MAXHOSTNAMELEN], exec[MAXNAMELEN], fullrank[MAXRANK];
    int proc_count = 0;
    int sock;
    int i=0;
    for(i=0;i<len;)
    {
	proc_count ++ ;
	
	MCFA_unpack_int(msgbuf, &procid ,1,&pos);
	MCFA_unpack_int(msgbuf, &volpex_id ,1,&pos);
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
//	Volpex_init_proc(procid, host, portused, fullrank);
    }
    
    return proc_count;

}


int SL_add_proc(void *buf, int len)
{
	int numprocs;
	numprocs = MCFA_proc_read_init((char*)buf,len);
        MCFA_proc_read_volpex_procs((char*)buf,len);
	SL_numprocs += numprocs;
	Volpex_numprocs = SL_numprocs;
	printf("[%d] :Added new proc to list \n",SL_this_procid);
        return SL_SUCCESS;
}

int SL_add_existing_proc(void *buf, int len)
{
	Volpex_proc *proc;
	SL_add_proc(buf,len);
	proc = Volpex_get_proc_byid(Volpex_numprocs-1);
	Volpex_insert_comm_newproc(proc->rank_MCW,proc->id);
	return SL_SUCCESS;
}


int SL_delete_proc(void *buf, int len)
{

    struct MCFA_proc_node* proclist;
    struct MCFA_proc_node* curr = NULL;
    SL_proc *dproc;

    proclist = MCFA_unpack_proclist((char*)buf,len);	
    curr = proclist;

    while(curr!=NULL)
    {
	
    	dproc = (SL_proc*) SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata->id  );
	PRINTF(("[%d]:MCFA_API: Handling error for proc %d\n",SL_this_procid,dproc->id));
	if (dproc->id == SL_this_procid){
		printf("[%d]:Bye!!!! I am signing off", SL_this_procid);
//		MCFA_printf_finalize();
		exit(-1);
	}
    	SL_proc_handle_error ( dproc, SL_ERR_PROC_UNREACHABLE,FALSE);	
//	Volpex_set_not_connected(dproc->id);	
	curr = curr->next;
     }
        return SL_SUCCESS;
}

int SL_start_communication(void *buf, int id)

//int SL_start_communication(int id)
{
	#define MAX_LEN (1024L * 1024L )

	MCFA_init_nodes *timeval;
        double tottime, stime, etime;
        char  *buf1, *buffer;
        int i,k,j,buflen=0;
        int *initialnodes;
	int len;
        buf1 = (char*)malloc(MAX_LEN*sizeof(char));;

        if (SL_this_procid == id){
            timeval = (MCFA_init_nodes*) calloc (SL_numprocs , (sizeof(MCFA_init_nodes)));
            for (i=id+1; i<SL_numprocs;i++){
		PRINTF(("[%d]: Communicating with procID:%d\n", SL_this_procid, i));
                tottime = 0.0;
                for(k=0;k<3;k++){
                    stime = SL_papi_time();
                    SL_Send (buf1, MAX_LEN, i, 0, 0 );
                    SL_Recv (buf1, MAX_LEN, i, 0, 0, (SL_Status*) SL_STATUS_IGNORE);
                    etime = SL_papi_time();
                  PRINTF(("[%d]:Time for proc:%d Iteration:%d = %f \n", SL_this_procid, i,k,etime-stime));
                    if (k>1)
                        tottime += etime-stime;

                }
		tottime = tottime*1000000;
                PRINTF(("[%d]:Recv time for proc:%d = %f \n", SL_this_procid, i,tottime));

                timeval[i].timeval = tottime;
                timeval[i].id = (int)tottime;
            }
    //        MCFA_time_sort(timeval, SL_numprocs);
            initialnodes = (int*)malloc (SL_numprocs * sizeof(int));
            MCFA_pack_size(SL_numprocs, 0, &len);
            buffer = (char*)malloc(len*sizeof(char)); 
            for(j=0;j<SL_numprocs;j++){
                PRINTF(("[%d]:Sorted recv time for proc:%d is %d \n", SL_this_procid, j,timeval[j].id));
                initialnodes[j] = timeval[j].id;
//              MCFA_pack_int(buffer, &timeval[j].id, 1, &buflen);
            }
                MCFA_pack_int(buffer, initialnodes, SL_numprocs, &buflen);
                SL_Send (buffer, len, MCFA_MASTER_ID, 0, 0 );
                buflen = 0;
        }
        else if (SL_this_procid > id){
            for(k=0;k<3;k++){
                SL_Recv (buf1, MAX_LEN, id, 0, 0, (SL_Status*)SL_STATUS_IGNORE);
                SL_Send (buf1, MAX_LEN,id, 0, 0 );
            }
        }


        return MCFA_SUCCESS;

}


int MCFA_time_sort(MCFA_init_nodes *a, int size)
{
    int i,pass;
    MCFA_init_nodes hold;
    for ( pass = 1; pass <= size - 1; pass++ )
        for( i = 0; i <= size - 2; i++ )
            if ( a[ i ].timeval > a[ i + 1 ].timeval ) {
                hold = a[ i ];
                a[ i ] = a[ i + 1 ];
                a[ i + 1 ] = hold;
            }
    return MCFA_SUCCESS;
}
