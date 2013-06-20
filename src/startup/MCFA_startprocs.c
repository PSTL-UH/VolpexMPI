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

#include "MCFA.h"
#include "MCFA_internal.h"

extern int SL_this_listensock;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;

void print_Options();
void MCFA_gethostname ( char *hostname, int len );

int MCFA_add_host(struct MCFA_host_node **hostList,struct MCFA_host *node);
char ** MCFA_set_args(struct MCFA_host *host, char *path, char *argg, int port, int redundancy, 
                      int condor_flag, int cluster_flag);

struct      MCFA_host_node *hostList=NULL;
struct      MCFA_proc_node *procList=NULL;
char        hostname[MAXHOSTNAMELEN];
static int 	id=-1;
MCFA_set_lists_func *MCFA_set_lists;
MCFA_create_distmatrix_func *MCFA_create_distmatrix;
MCFA_create_comm_matrix_func *MCFA_create_comm_matrix;

int main(int argc, char *argv[])
{
  int   i=0;			//intializing loop iterators
  int 	flag=0;		    //intializing value to false for -hostfile option
  int 	jobID=1; 
  int 	numprocs=-1;	//total number of clients
  char 	**hostName=NULL;//contains all names of all hosts which are available
  char 	*hostFile=NULL;
  int 	hostCount = 0;	//contains total number of hosts
  int 	next=1;	
  char 	*path;
  int 	port,len=MAXHOSTNAMELEN;
  int 	redundancy = 1;
  int   condor_flag = 0;
  int 	cluster_flag;
  char	**arguments;
  int 	numarguments=0;
  
  arguments = (char**) malloc (argc * sizeof(char*));
  for (i=0;i<argc;i++)
	arguments[i] = (char*) malloc (MAXNAMELEN * sizeof(char));
  
  port = SL_this_procport = 45000;
  jobID = MCFA_get_nextjobID();
  
  /*Server code */
  /* Determine our own hostname. Need to print that on the screen later */
  if ( gethostname(hostname, len ) != 0 ) {
    printf("SERVER: could not determine my own hostname \n");
  }
  char *hname;
  hname = (char*) malloc (256 *sizeof(char));
  
  MCFA_get_ip(&hname);
  strcpy(hostname,hname);
  
  MCFA_set_lists =  MCFA_set_liststraight;
  MCFA_create_distmatrix = MCFA_distmatrix_ipaddress;
  cluster_flag = NOCLUSTER;
  
  /* Parsing startup options */
  while(next<argc){
    if(!strcmp(argv[next],"-np")||!strcmp(argv[next],"--np")) {
      numprocs = atoi(argv[next+1]);	
      next= next+2;
    }
    else if(!strcmp(argv[next],"-redundancy")||!strcmp(argv[next],"--redundancy")) {
      redundancy = atoi(argv[next+1]);
      next= next+2;
    }
    else if(!strcmp(argv[next],"-hostfile")||!strcmp(argv[next],"--hostfile")) {
      flag = 1;
      hostFile = (char *) malloc (MAXHOSTNAMELEN * sizeof(char));
      strcpy(hostFile, argv[next+1]);
      next=next+2;
    }
    else if(!strcmp(argv[next], "-condor")||!strcmp(argv[next], "--condor")) {
      condor_flag = 1;
      next = next +1;
    }
    else if(!strcmp(argv[next], "-boinc")||!strcmp(argv[next], "--boinc")) {
      condor_flag = 2;
      next = next +1;
    }
    else if(!strcmp(argv[next], "-random")||!strcmp(argv[next], "--random")) {
      condor_flag = 3;
      next = next +1;
    }
    else if(!strcmp(argv[next], "-hpx")||!strcmp(argv[next], "--hpx")) {
      condor_flag = 4;
      next = next +1;
    }
    else if(!strcmp(argv[next],"-help")||!strcmp(argv[next],"--help")) {
      print_Options();
      exit(-1);
    }
    else if(!strcmp(argv[next],"-allocation")||!strcmp(argv[next],"--allocation")) {
      if(!strcmp(argv[next+1],"straight"))
        MCFA_set_lists = MCFA_set_liststraight;
      else if (!strcmp(argv[next+1],"concentrate"))
        MCFA_set_lists = MCFA_set_listsconcentrate;
      else if(!strcmp(argv[next+1],"roundrobin"))
        MCFA_set_lists = MCFA_set_listsroundrobin;
      else if(!strcmp(argv[next+1],"random"))
        MCFA_set_lists = MCFA_set_listsrandom;
      
      next=next+2;
    }
    else if(!strcmp(argv[next],"-cluster") || !strcmp(argv[next],"--cluster")) {
      if(!strcmp(argv[next+1],"nocluster")){
        cluster_flag = NOCLUSTER;
      }
      if(!strcmp(argv[next+1],"communication")){
        MCFA_create_distmatrix = MCFA_distmatrix_communication;
        cluster_flag = COMMUNICATION;
      }
      else if (!strcmp(argv[next+1],"ipaddress")){
        MCFA_create_distmatrix = MCFA_distmatrix_ipaddress;
        cluster_flag = IPADDRESS;
      }
      
      next=next+2;
    }
    else {
      strcpy(arguments[numarguments++], argv[next]);
      next=next+1;
    }
  }/* end while */
  
  char *argg=NULL;
  for(i=1;i<numarguments;i++) {
	if(argg == NULL){
      argg = realloc ( NULL, strlen(arguments[i])+2);
      strcpy(argg, arguments[i]);
	}
	else {
      argg = realloc ( argg, (strlen(argg) + strlen(arguments[i])+2));
      strcat(argg, arguments[i]);
	}
	strcat(argg, " ");
  }
  
  MCFA_get_path(arguments[0], &path);
  
  char exec[MAXNAMELEN];
  MCFA_get_exec_name(path,exec);
  
  MCFA_create_comm_matrix = MCFA_create_comm_matrix_default;
  
  MCFA_initHostList(&hostList);				//initialing a hostlist
  MCFA_initProcList(&procList);
  
  MCFA_set_hostlist(hostFile, hostname, numprocs, port, &hostCount, &hostName);
  
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
  
  MCFA_spawn_processes(hostName,path,argg,port,jobID,numprocs,hostCount,redundancy,condor_flag,
                       cluster_flag, newproclist);
  
  SL_event_msg_header *header = NULL ;
  SL_qitem 			  *event = NULL;
  int 				  size = 0, count = 0;    
  int                 num;
  while(1) {
    size = size + MCFA_check_proc(procList);
    numprocs = numprocs - size;
    size = 0;
    if(numprocs <= 0) {
      break;
    }
    
    SL_msg_progress();
    event = SL_get_next_event();
    if(NULL != event) {
      header  = (SL_event_msg_header* )event->iov[1].iov_base;
      PRINTF(("Receiving a new msg by process: %d\n\n", header->id));
      if(header->cmd == SL_CMD_DELETE_PROC){
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
      }
      else if (header->cmd == MCFA_CMD_DELETE_JOB){
        PRINTF(("MCFA_startprocs: Request to delete processes \n"));
        if (MCFA_procjob_exists(procList,header->jobid)){		    
          MCFA_event_deletejob(header, numprocs, &num);
          size =  num;
        }
        else
          PRINTF(("Process doesnot exist\n"));
      }
      else if (header->cmd == MCFA_CMD_DELETE_PROC){
        PRINTF(("MCFA_startprocs: Request to delete process with procid %d \n",header->procid));
        if (MCFA_proc_exists(procList,header->procid)){	
          MCFA_event_deleteproc(header, numprocs);
          size ++;
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
  
  MCFA_free_hostlist(hostList);
  MCFA_free_proclist(procList);
  MCFA_free_proclist(newproclist);
  free(path);

  if(NULL != hostName) {
    for(i=0; i< hostCount; i++) {
      if ( NULL != hostName[i] ) {
        free(hostName[i]);
        hostName[i]=NULL;
      }
    }
    free ( hostName );
  }
  
  SL_finalize_eventq();
  SL_Finalize();

  for (i=0;i<argc;i++)
    free(arguments[i]);
  free(arguments);
  free(hname);
  free(hostFile);

  return 0;
}


struct MCFA_proc_node* MCFA_spawn_processes(char **hostName, char *path, char *argg, int port, int jobID,
                                            int numprocs,int hostCount, int redundancy,int spawn_flag, 
                                            int cluster_flag, struct MCFA_proc_node *newproclist)
{
  char 	**arg = NULL;
  int 	i,k;
  int 	pid = 10;
  int 	msglen = 0;
  char 	*buf = NULL;
  FILE	*fp;
  char	fname[20];
  struct MCFA_proc_node *currlist = newproclist;
  char exec[MAXNAMELEN], deamon[MAXNAMELEN];
  SL_event_msg_header *header;
  SL_qitem                            *event = NULL;
  
  struct MCFA_host_node *currhost = hostList;
  
  SL_numprocs = SL_numprocs+numprocs;
  SL_init_numprocs = numprocs;
  
  if(spawn_flag == SSH) {
    for(i=0;i<numprocs;i++) {
      id = currlist->procdata->id;
      //  SL_proc_init should be done for all processes on procList
      //	but fork should be done at each host in hostList not based on proc list
      SL_proc_init(id, currlist->procdata->hostname, currlist->procdata->portnumber);
      currlist = currlist->next;
    }
    i=0;
    currhost = hostList;
    while(currhost != NULL && pid !=0 ) {
      arg = MCFA_set_args(currhost->hostdata, path, argg, port, redundancy, spawn_flag, 
                          cluster_flag);
      pid=fork();
      if(pid<0) {
        printf("fork failed errno is %d (%s)\n", errno, strerror(errno));
      }
      currhost = currhost->next;
      i++;
      if (i== numprocs)
        break;
    }
  }
  if(spawn_flag == RANDOM) {
    i=0;
    currhost = hostList;
    while(currhost != NULL ) {
      for(k=0;k<currhost->hostdata->numofProcs;k++) {
		if(pid != 0){
          arg = MCFA_set_args(currhost->hostdata, path, argg, port, redundancy, 
                              spawn_flag, cluster_flag);
          i++;
          pid=fork();
          if(pid<0) {
            printf("fork failed errno is %d (%s)\n", errno, strerror(errno));
          }
		}
      }
      currhost = currhost->next;
      if (i== numprocs) {
		break;
      }
    }
  }
  else if (spawn_flag == CONDOR) {
	currhost = hostList;
	arg = MCFA_set_args(currhost->hostdata, path, argg, port, redundancy, spawn_flag,
                        cluster_flag);	
	sprintf(fname, "volpex");
	PRINTF(("Creating a file:%s \n", fname));
	fp = fopen(fname, "w");
	for(k=3; k<MAXARGUMENTS-1; k++) {
      fprintf(fp,"%s\n",arg[k]);
	}
	fclose(fp);
  } 	    
  else if (spawn_flag == BOINC) {
    currhost = hostList;
    arg = MCFA_set_args(currhost->hostdata, path, argg, port, redundancy, spawn_flag,
                        cluster_flag);
    sprintf(fname, "volpex");
    PRINTF(("Creating a file:%s \n", fname));
    fp = fopen(fname, "w");
    for(k=3; k<MAXARGUMENTS-1; k++) {
      fprintf(fp,"%s\n",arg[k]);
    }
    fclose(fp);
  }
  else if (spawn_flag == HPX) { //hpx
    //for hpx we spawn another process for agas server 
    currhost = hostList;
    arg = MCFA_set_args(currhost->hostdata, path, argg, port, redundancy, RANDOM, 
                        cluster_flag);
    sprintf(fname, "volpex");
    PRINTF(("Creating a file:%s \n", fname));
    fp = fopen(fname, "w");
    for(k=3; k<MAXARGUMENTS-1; k++) {
      fprintf(fp,"%s\n",arg[k]);
    }
    fclose(fp);

    pid = fork();
    if(pid==0) { //child
      //here we set the arguments for front node (console + agas_server)
      //hard-coded for testing purposes
      char agas_server[128];
      gethostname(agas_server, sizeof agas_server);
      
      char *argv[6];
      argv[0] = strdup(path);
      argv[1] = (char *) malloc ( 128 );
      sprintf (argv[1],"-x%s:7912",agas_server);
      //sprintf (argv[2],"-x%s:7912",currhost->hostdata->hostname);
      argv[2] = (char *) malloc ( 32);
      sprintf (argv[2],"-l%d",(numprocs));
      argv[3] = strdup("--hpx:console");
      argv[4] = strdup("--hpx:run-agas-server");
      argv[5] = NULL;
      
      execvp(argv[0], argv); 
    }    
    
    i=0;
    while(currhost != NULL && pid !=0 ){
      arg = MCFA_set_args(currhost->hostdata, path, argg, port, redundancy, spawn_flag, 
                          cluster_flag);
      
      pid=fork();
      if(pid<0) {
        printf("fork failed errno is %d (%s)\n", errno, strerror(errno));
      }
      
      currhost = currhost->next;
      i++;
      if (i== numprocs-1)
        break;
    } 
  }
  
  if ( pid==0 && (spawn_flag == SSH || spawn_flag == RANDOM || spawn_flag == HPX) ) {
    execvp(arg[0],arg);
  }
  
  
  if (spawn_flag == BOINC) {
    MCFA_set_boinc_dir();
    
    char command[50];
    sprintf(command ,"cp volpex.* %s/download", BOINCDIR);
    printf("%s\n",command);
    system(command);
    MCFA_create_volpex_job(arg[2], arg[3], numprocs);
    MCFA_create_boinc_script(arg[2], arg[3], numprocs);
    printf("HURRAY!!!we currently do provide support for BOINC\n");
  }
  if (spawn_flag == CONDOR) {
    strcpy(exec, "");
    strcpy(deamon, "");
    MCFA_get_exec_name(arg[2],deamon);
    MCFA_get_exec_name(arg[3],exec);
    MCFA_create_condordesc(deamon, exec, numprocs);
    MCFA_start_condorjob();
  }
  
  if (spawn_flag == CONDOR || spawn_flag == BOINC || spawn_flag == RANDOM || spawn_flag == HPX) {
    
    k=0;
    while(k<numprocs) {
      SL_msg_progress();
      
      event = SL_get_next_event_noremove();
      if(NULL != event) {
        
        header  = (SL_event_msg_header* )event->iov[1].iov_base;
        if (header->cmd == MCFA_CMD_GETID) {
          SL_remove_event(event);
          PRINTF(("Receiving a new msg by process :%d\n\n", header->id));
          MCFA_update_proclist(procList, header->id, header->hostname,header->port);
          MCFA_update_proclist(newproclist,header->id,header->hostname,header->port);
          //				MCFA_update_hostlist(hostList, ......)
          PRINTF("\n\n");
          k++;
        }
        else {
          SL_move_eventtolast(event);
        }
      }
    }		
  }
  MCFA_printProclist(procList);	

  //sending data to client
  if(pid!=0) {
    buf = MCFA_pack_proclist(procList, &msglen);
    PRINTF(("MCFA_startprocs:Total number of processes spawned %d\n",numprocs));

    struct MCFA_proc_node *curr = newproclist;
    for(k=0;k<numprocs;k++)
      {
        PRINTF(("MCFA_startprocs:sending procList to process with id  %d\n",curr->procdata->id));
        SL_Send(&msglen, sizeof(int), curr->procdata->id, 0, 0);
        SL_Send(buf, msglen, curr->procdata->id, 0, 0 );
        curr = curr->next;
      }	
    
#ifdef CLUSTER    
    if(cluster_flag != 0 && numprocs/redundancy > 1)
      MCFA_node_selection(redundancy);
#endif
    
    buf = MCFA_pack_proclist(procList, &msglen);
     
    curr = newproclist;
    for(k=0;k<numprocs;k++) {
        SL_Send(buf, msglen, curr->procdata->id, 0, 0 );
        curr = curr->next;
      }
  }    
  
  free(buf);
  return newproclist;
}


int MCFA_update_proclist(struct MCFA_proc_node *proclist, int id, char *hostname, int port)
{
  struct MCFA_process* proc;
  proc = MCFA_search_proc(proclist,id);
  proc->hostname = strdup(hostname);
  proc->portnumber     = port;
  return 1;
}


int MCFA_check_proc(struct MCFA_proc_node *procList)
{
  SL_proc *dproc = NULL;
  int i, size;
  SL_event_msg_header header;
  int count = 0;
  struct MCFA_proc_node *list = NULL;
  
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
  system ("condor_submit volpexjob.condor");
}
