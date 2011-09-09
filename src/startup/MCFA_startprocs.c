//extern "C"{
#include "MCFA.h"
#include "MCFA_internal.h"
//}


extern int SL_this_listensock;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;
//extern int SL_init_numprocs;

void print_Options();
void MCFA_gethostname ( char *hostname, int len );

int MCFA_add_host(struct MCFA_host_node **hostList,struct MCFA_host *node);
char ** MCFA_set_args1(struct MCFA_host *host, char *path, int port, int redundancy, int condor_flag);

struct      MCFA_host_node *hostList=NULL;
struct      MCFA_proc_node *procList=NULL;
char        hostname[MAXHOSTNAMELEN];
static int 	id=-1;
MCFA_set_lists_func *MCFA_set_lists;

int main(int argc, char *argv[])
{
    int		i=0;			//intializing loop iterators
    int 	flag=0;				//intializing value to false for -hostfile option
    int 	pathflag=0;			//intializing value to false for -wdir option
    int 	jobID=1; 
//    int 	maxprocspernode = MAXPROCSPERNODE;		//maximum number of processes assigned to each hosts
    int 	numprocs=-1;			//total number of clients
    char 	**hostName=NULL;		//contains all names of all hosts which are available
    char 	*hostFile=NULL;
    int 	hostCount = 0;			//contains total number of hosts
    int 	next=1;	
//    char 	path[BUFFERSIZE]=".";
    char 	*path;
    int 	port,len=MAXHOSTNAMELEN;
    int 	redundancy = 1;
    int         condor_flag = 0;

    /* getting the path of the file to be executed */
 //   path = strdup(argv[argc-1]);
    path = (char*) malloc (MAXNAMELEN * sizeof(char));
    strcpy(path,argv[argc-1]);  

    port = SL_this_procport = 45000;
 //   MCFA_printf_init1(0,-1);
    jobID = MCFA_get_nextjobID();

    /*Server code */

    /* Determine our own hostname. Need to print that on the screen later */
    if ( gethostname(hostname, len ) != 0 ) {
        MCFA_printf("SERVER: could not determine my own hostname \n" );
    }

    MCFA_set_lists =  MCFA_set_liststraight;
/* Parsing startup options */
    while(next<argc)
    {
	if(!strcmp(argv[next],"-np")||!strcmp(argv[next],"--np")){
	    numprocs = atoi(argv[next+1]);	
	    next= next+2;
	}
	else if(!strcmp(argv[next],"-redundancy")||!strcmp(argv[next],"--redundancy")){
	    redundancy = atoi(argv[next+1]);
/*	    if (numprocs % redundancy != 0 ){
	        printf("Total node vs. redundancy is not correct!\n");
                exit(0);
            }*/
            next= next+2;
        }
	else if(!strcmp(argv[next],"-hostfile")||!strcmp(argv[next],"--hostfile")){
	    flag = 1;
	    hostFile = (char *) malloc (MAXHOSTNAMELEN * sizeof(char));
	    strcpy(hostFile, argv[next+1]);
//	    hostFile = strdup(argv[next+1]);
	    next=next+2;
	}
	else if(!strcmp(argv[next], "-condor")||!strcmp(argv[next], "--condor")){
            condor_flag = 1;
            next = next +1;
        }

    else if(!strcmp(argv[next], "-boinc")||!strcmp(argv[next], "--boinc")){
            condor_flag = 2;
            next = next +1;
    }

	else if(!strcmp(argv[next],"-help")||!strcmp(argv[next],"--help")){
	    print_Options();
	    exit(-1);
	}
	else if(!strcmp(argv[next],"-wdir")||!strcmp(argv[next],"--wdir")){	
	    	pathflag = 1;
		MCFA_get_abs_path(argv[next+1],&path); /* getting the path of the file to be executed */
		next=next+2;
	}
	else if(!strcmp(argv[next],"-allocation")||!strcmp(argv[next],"--allocation")){
                if(!strcmp(argv[next+1],"straight"))
			MCFA_set_lists = MCFA_set_liststraight;
		else if (!strcmp(argv[next+1],"concentrate"))
			MCFA_set_lists = MCFA_set_listsconcentrate;
		else
			MCFA_set_lists = MCFA_set_listsroundrobin;

                MCFA_get_abs_path(argv[next+1],&path); /* getting the path of the file to be executed */
                next=next+2;
	}
	
	else{
	    next=next+1;
	}
    }/* end while */
    
    if (pathflag != 1)
	MCFA_get_path(argv[argc-1],&path);

    if(condor_flag == 2){
/*        char *hname;
        hname = (char*) malloc (256 *sizeof(char));

        MCFA_get_ip(&hname);
        strcpy(hostname,hname);
*/
    printf("%s\n\n", hostname);        
//    exit(-1);
    }


    MCFA_initHostList(&hostList);				//initialing a hostlist
    MCFA_initProcList(&procList);
    
	MCFA_set_hostlist(hostFile,hostname, numprocs, port, &hostCount, &hostName);
//	hostName = MCFA_get_hostarray( hostList, hostList->hostdata->hostname);
//	hostCount = MCFA_get_total_hosts(hostList);
    
	SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );
    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );
    
    /* Add the startprocs process to the SL_array */
    SL_proc_init ( MCFA_MASTER_ID, hostname, port );
    SL_this_procid = MCFA_MASTER_ID;
    SL_init_internal();
    
	    
    struct MCFA_proc_node *newproclist = NULL;
	numprocs = numprocs * redundancy;
	SL_numprocs = 0;

    newproclist = MCFA_set_lists(id,hostName,path,port,jobID,numprocs,hostCount,redundancy);
//	MCFA_printProclist(procList);
//                MCFA_printHostlist(hostList);

    MCFA_spawn_processes(hostName,path,port,jobID,numprocs,hostCount,redundancy,condor_flag,newproclist);
	



	
    
    SL_event_msg_header 		*header = NULL ;
    SL_qitem 				*event = NULL;
    int 				size = 0, count = 0;    
    int num;
    while(1)
    {
//        sleep(1); 
	
	size = size + MCFA_check_proc(procList);
	numprocs = numprocs - size;
	size = 0;
	if(numprocs <= 0){
		break;
	}
	
	SL_msg_progress();
	event = SL_get_next_event();
	if(NULL != event){
	 
	    header  = (SL_event_msg_header* )event->iov[1].iov_base;
	//	SL_proc_dumpall();

	    printf("Recieving a new msg by process :%d\n\n", header->id);
	    if(header->cmd == SL_CMD_DELETE_PROC){
//		SL_proc_dumpall();
		header->cmd = MCFA_CMD_DELETE_PROC;
	    }
	    else if(header->cmd == SL_CMD_ADD_PROC){
                header->cmd = MCFA_CMD_ADD_PROCS;
            }

	    	   
	    if(header->cmd == MCFA_CMD_PRINT_PROCS){
		MCFA_printProclist(procList);
	    }
	    else if(header->cmd == MCFA_CMD_ADD_PROCS || header->cmd == MCFA_CMD_ADD_JOB){
		MCFA_event_addprocs(header, numprocs);
		numprocs = numprocs + header->numprocs;
		MCFA_printProclist(procList);
		MCFA_printHostlist(hostList);
		
            }
	    else if (header->cmd == MCFA_CMD_ADD_PROCID){
		printf("MCFA_startprocs: Request to add process with id %d \n", header->procid);
		MCFA_event_addprocs(header,numprocs);
		numprocs++;
		MCFA_printProclist(procList);
//                MCFA_printHostlist(hostList);
	    }
	    

	    else if (header->cmd == MCFA_CMD_DELETE_JOB){
		PRINTF(("MCFA_startprocs: Request to delete processes \n"));
		if (MCFA_procjob_exists(procList,header->jobid)){		    
			MCFA_event_deletejob(header, numprocs, &num);
			size =  num;
//                        numprocs = numprocs - num;
                        MCFA_printProclist(procList);
//                        MCFA_printHostlist(hostList);
		}
		else
		    PRINTF(("Process doesnot exist\n"));
		
	    }
	    else if (header->cmd == MCFA_CMD_DELETE_PROC){
		printf("MCFA_startprocs: Request to delete process with procid %d \n",header->procid);
		if (MCFA_proc_exists(procList,header->procid)){	
			MCFA_event_deleteproc(header, numprocs);
			size ++;
            //            numprocs--;
                        MCFA_printProclist(procList);
//                        MCFA_printHostlist(hostList);	
		    
		}
		else
		    PRINTF(("Process doesnot exist\n"));
	    }

	    else if(header->cmd == MCFA_CMD_PRINT_JOBSTATUS){
		MCFA_event_printjobstatus(header);
	    }
	    
	    else if(header->cmd == MCFA_CMD_PRINT_PROCSTATUS){
		MCFA_event_printprocstatus(header);
            }

	    else if(header->cmd == MCFA_CMD_PRINT_HOSTSTATUS){
		MCFA_event_printhoststatus(header);
            }

	    else if(header->cmd == MCFA_CMD_PRINT_ALLJOBSTATUS){
		MCFA_event_printalljobstatus(header);
            }

	    else if(header->cmd == MCFA_CMD_PRINT_ALLPROCSTATUS){
		MCFA_event_printalljobstatus(header);
            }

	    else if(header->cmd == MCFA_CMD_PRINT_ALLHOSTSTATUS){
		MCFA_event_printallhoststatus(header);
            }

	   else if(header->cmd == MCFA_CMD_CLEAR_PROCS){
		count = MCFA_clear_procs(procList);
		numprocs = numprocs - count;
	   }

	    
        }
    }  
    
    
   
    
    /* After everything is done, close the socket */
    PRINTF(("SERVER: connection closed \n"));
//printf("Total SL_numprocs:%d\n",SL_numprocs);
    for(i=0;i<hostCount;i++)
	if(NULL == hostName[i])
		free(hostName[i]);
//    free(hostName); 
    
    MCFA_free_hostlist(hostList);
    MCFA_free_proclist(procList);
    MCFA_free_proclist(newproclist);
    free(path);
	for(i=0; i< hostCount; i++)
		if(NULL != hostName)

			free(hostName[i]);
//	free(hostName);
//    free(hostFile);

    SL_finalize_eventq();
    SL_Finalize();
//    MCFA_printf_finalize1();
    return 0;
    
}





/* Function to print details of all options for startproc command similar to man file */
void print_Options()
{
    printf("NAME\n");
    printf("\t ./mcfarun - to start specified number of programs\n\n");
    printf("SYNOPSIS\n");
    printf("\t./mcfarun [options]\n\n");
    printf("DESCRIPTION\n");
    printf("\t The mcfarun command is used used to start number of client processes specified by the user. \n\n");
    printf("\t -np,  --np\n");
    printf("\t\t specify total number of client processes to be started. Total number of processes = # of procs * redundancy\n");
    printf("\t\t For example: if 4 processes are to be started with redundancy 2 then command is\n");
    printf("\t\t ./mcfarun -np 8 -redundancy 2 -hostfile hostfile ./[name of executable]\n\n");
    printf("\t -redundancy, --redundancy\n");
    printf("\t\t Specify the redundancy level for processes\n\n");
    printf("\t -hostfile, --hostfile [name]\n");
    printf("\t\t Specify the name of hostfile\n\n");
    printf("\t -help, --help\n");
    printf("\t\t displays a list of options supported by startprocs\n\n");
    printf("\t -condor, --condor\n");
    printf("\t\t spawn process on condor pool.( if this flag is not available all \n");
    printf("\t\t\t processes are spawned on the frontend node using ssh\n\n");
    printf("\t -allocation, --allocation [scheme for allocation of processes]\n");
    printf("\t\t Specify how the processes can be allocated using a given hostfile\n");
    printf("\t\t\t\t [roundrobin] - processes are distributed in roundrobin manner for \n");
    printf("\t\t\t\t\t	given list of hosts. This allocation strategy is also default allocation scheme\n");
    printf("\t\t\t\t [concentrate]- processes are distributed such that maximum number of processes are on one host\n");
    printf("\t\t\t\t [straight] -   processes are distributed as hosts are given in hostfile without changing the order of hosts\n\n");

    
}


struct MCFA_proc_node* MCFA_spawn_processes(char **hostName, char *path, int port, int jobID,int numprocs,
							int hostCount, int redundancy,int spawn_flag, struct MCFA_proc_node *newproclist)



{

    char 	**arg = NULL;
//    struct 	MCFA_proc_node *newproclist = NULL;
    int 	i,k;
    int 	pid = 10;
    int 	msglen = 0;
    char 	*buf = NULL;
    FILE	*fp;
    char	fname[20];
    struct MCFA_proc_node *currlist = newproclist;
    char exec[MAXNAMELEN];
    SL_event_msg_header *header;
    SL_qitem                            *event = NULL;

    struct MCFA_host_node *currhost = hostList;



    SL_numprocs = SL_numprocs+numprocs;
    SL_init_numprocs = numprocs;
    
    arg = MCFA_set_args(id,hostName[0],path,port,jobID,numprocs,hostCount,redundancy, spawn_flag);
    
    
    if(spawn_flag == SSH){
    	for(i=0;i<numprocs;i++)
    	{
        	id = currlist->procdata->id;
	
	   //  SL_proc_init should be done for all processes on procList
	//	but fork should be done at each host in hostList not based on proc list
	    
		SL_proc_init(id, currlist->procdata->hostname, currlist->procdata->portnumber);
		currlist = currlist->next;

		
	
	    }

	printf("spawn flag is SSH temporarily added code\n");
    }
    else if (spawn_flag == CONDOR){
 	   currhost = hostList;
	   arg = MCFA_set_args1(currhost->hostdata, path, port, redundancy, spawn_flag);	
	   sprintf(fname, "volpex",id);
           PRINTF(("Creating a file:%s \n", fname));
           fp = fopen(fname, "w");
           for(k=3; k<MAXARGUMENTS-1; k++){
           	fprintf(fp,"%s\n",arg[k]);
           }
           fclose(fp);
    } 	    
	    
   else if (spawn_flag == BOINC){
        currhost = hostList;
        arg = MCFA_set_args1(currhost->hostdata, path, port, redundancy, spawn_flag);
        sprintf(fname, "volpex",id);
        PRINTF(("Creating a file:%s \n", fname));
        fp = fopen(fname, "w");
        for(k=3; k<MAXARGUMENTS-1; k++){
            fprintf(fp,"%s\n",arg[k]);
        }
        fclose(fp);
   }
        


	i = 0;

//forking proxy_server
	char **arg_proxy;
	char thostname[MAXHOSTNAMELEN];
        gethostname(thostname, MAXHOSTNAMELEN );
        SL_proc_init(MCFA_PROXY_ID, thostname, 828282);
	arg_proxy = (char **)malloc(5 * sizeof(char*));
        if(arg_proxy == NULL){
                printf("ERROR: in allocating memory\n");
        exit(-1);
        }
	if(pid != 0){
		pid = fork();
	arg_proxy[0] = strdup("./mcfa_proxy");
	arg_proxy[1] = strdup(thostname);
	arg_proxy[2] = (char *) malloc (sizeof(int) + 1);
        sprintf(arg_proxy[2], "%d", port);
        arg_proxy[3] = (char *) malloc (sizeof(int) + 1);
        sprintf(arg_proxy[2], "%d", spawn_flag);
        arg_proxy[4] = NULL;

        if (pid==0){
                execvp(arg_proxy[0], arg_proxy);
        	}
	}
    if (spawn_flag == SSH ){
        currhost = hostList;


	while(currhost != NULL && pid !=0 ){

		arg = MCFA_set_args1(currhost->hostdata, path, port, redundancy, spawn_flag);

//		printf("process is forking pid:%d\n",pid);
		pid=fork();

//		printf("CHILD PROC ID:%d\n",pid);
	        if(pid<0) {
        	        MCFA_printf("fork failed errno is %d (%s)\n", errno, strerror(errno));
                }
		currhost = currhost->next;
		i++;
		if (i== numprocs)
			break;

	}
    }
	




    if(pid==0 && spawn_flag == SSH ){
    	execvp(arg[0],arg);
    }

    if (spawn_flag == BOINC){
        MCFA_set_boinc_dir();

        char command[50];
        sprintf(command ,"cp volpex.* %s/download", BOINCDIR);
        printf("%s\n",command);
        system(command);
//        MCFA_create_boinc_wu_template(arg[2],arg[3]);
//        MCFA_create_boinc_re_template(arg[3],numprocs);
//        MCFA_create_boinc_script(arg[2], arg[3], numprocs);
          MCFA_create_volpex_job(arg[2], arg[3], numprocs);
          MCFA_create_boinc_script(arg[2], arg[3], numprocs);
          printf("HURRAY!!!we currently do provide support for BOINC\n");
    }

    if (spawn_flag == CONDOR){
//  	MCFA_create_condordesc(exec, numprocs);
//	MCFA_start_condorjob();
    }

    if (spawn_flag == CONDOR || spawn_flag == BOINC ){
       strcpy(exec, "");

	k=0;
	while(k<numprocs){
		SL_msg_progress();

		event = SL_get_next_event_noremove();
        	if(NULL != event){

            		header  = (SL_event_msg_header* )event->iov[1].iov_base;
			if (header->cmd == MCFA_CMD_GETID){
				SL_remove_event(event);
		            	printf("Recieving a new msg by process :%d\n\n", header->id);
				MCFA_update_proclist(procList, header->id, header->hostname,header->port);
				MCFA_update_proclist(newproclist,header->id,header->hostname,header->port);
//				MCFA_update_hostlist(hostList, ......)
				printf("\n\n");
				k++;
			}
			else{
				SL_move_eventtolast(event);
			}
		}

	}		

   }
	MCFA_printProclist(procList);	
//	MCFA_printHostlist(hostList);
//sending data to client
    if(pid!=0){
        buf = MCFA_pack_proclist(procList, &msglen);
        PRINTF(("MCFA_startprocs:Total number of processes spawned %d\n",numprocs));
    
    
    struct MCFA_proc_node *curr = newproclist;
	
        SL_Send(&msglen, sizeof(int), MCFA_PROXY_ID, 0, 0);
        SL_Send(buf, msglen, MCFA_PROXY_ID, 0, 0 );
    
    for(k=0;k<numprocs;k++)
    {
	PRINTF(("MCFA_startprocs:sending procList to process with id  %d\n",curr->procdata->id));
      	SL_Send(&msglen, sizeof(int), curr->procdata->id, 0, 0);
        SL_Send(buf, msglen, curr->procdata->id, 0, 0 );
	curr = curr->next;
    }	
  
if(redundancy>1)
            MCFA_node_selection(redundancy);

	buf = MCFA_pack_proclist(procList, &msglen);
        printf("MCFA_startprocs:Total number of processes spawned %d\n",numprocs);


    curr = newproclist;

    for(k=0;k<numprocs;k++)
    {
        SL_Send(buf, msglen, curr->procdata->id, 0, 0 );
        curr = curr->next;
    }


	}





 /*   for (i=0; i<12;i++){
    	if(NULL != arg[i])
        	free(arg[i]);
        }
        if(NULL != arg)
                free(arg);

   */ 
    free(buf);

    return newproclist;
}


int MCFA_update_proclist(struct MCFA_proc_node *proclist, int id, char *hostname, int port)
{
    struct MCFA_process* proc;
    proc = MCFA_search_proc(proclist,id);

    proc->hostname = strdup(hostname);
    proc->portnumber     = port;
  //  SL_proc_init(id, hostname, proc->portnumber);
    return 1;
}

int MCFA_check_proc(struct MCFA_proc_node *procList)
{
    SL_proc *dproc=NULL;
    int i, size;
    SL_event_msg_header header;
    int count = 0;
    struct MCFA_proc_node               *list =NULL;
    
    size = SL_array_get_last ( SL_proc_array );
    for ( i=0; i<= size; i++ ) {
        dproc = (SL_proc *) SL_array_get_ptr_by_pos ( SL_proc_array, i );
        if ( -1 == dproc->sock && dproc->id != SL_this_procid ) {
	    header.procid = dproc->id;
	    if (MCFA_proc_exists(procList,header.procid)){
		list = MCFA_delete_proc(&header);
		MCFA_proc_close(procList, header.procid);
		count++;
	    }
        }
    }
    MCFA_free_proclist(list);
    return count;
}
void MCFA_start_condorjob()
{
/*        system("ssh tpleblan@cusco.hpcc.uh.edu rm volpex*");
        system("ssh tpleblan@cusco.hpcc.uh.edu rm temp1");
        system("scp volpex* tpleblan@cusco.hpcc.uh.edu:");
        system("scp mcfastart_d tpleblan@cusco.hpcc.uh.edu:");
        system("scp temp1 tpleblan@cusco.hpcc.uh.edu:");
        system ("ssh tpleblan@cusco.hpcc.uh.edu condor_submit volpexjob.condor");
      system("scp volpexjob.condor tpleblan@cusco.hpcc.uh.edu");
*/
	system ("condor_submit volpexjob.condor");
}
   
