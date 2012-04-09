
#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"




extern MCFA_set_lists_func *MCFA_set_lists;
extern struct      MCFA_host_node *hostList;
extern struct      MCFA_proc_node *procList;


char** MCFA_get_startproc( SL_event_msg_header *header, int *hostcount)
{
   struct      MCFA_host_node *curr = NULL, *next = NULL;
   int hostCount=0;
   char **hostName;

   PRINTF(("MCFA_event_addprocs: If a new host is given where the process is to be added\n"));
   if (strcmp(header->hostfile,"")!=0){
   	     curr = MCFA_set_hostlist(header->hostfile, NULL, header->numprocs, SL_this_procport,&hostCount, &hostName);
//	     hostName = MCFA_get_hostarray( curr, curr->hostdata->hostname);
//	     hostCount = MCFA_get_total_hosts(curr);
   }

   else{
        curr = hostList;
	while(curr->next != NULL){
        	next = curr->next;
	        if (next->hostdata->numofProcs <  curr->hostdata->numofProcs){
                	curr = next;
                	break;
	    	}
        	curr = curr->next;
        }

	hostName = MCFA_get_hostarray( hostList, curr->hostdata->hostname);
	hostCount = MCFA_get_total_hosts(hostList);
    }	
	*hostcount = hostCount;
	return (hostName);
}

int MCFA_inform_existing_procs(SL_event_msg_header *SL_header, char *buf, int numprocs)
{
    SL_proc                             *dproc;
    struct MCFA_proc_node               *curr = NULL;
    int                                 k;
    SL_Request                          reqs[3];

    curr = procList;
    PRINTF((" MCFA_startprocs: Sending new process list to already existing processes \n"));
    for(k=0;k<numprocs;k++)
    {
        dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata->id);
        if(curr->procdata->status != 0 && dproc->sock != -1){

                printf("MCFA_startprocs:sending new process list to process with rank  %d\n",curr->procdata->id);
                SL_event_post(SL_header, sizeof(SL_event_msg_header),curr->procdata->id, 0, 0,&reqs[0]);
                SL_msg_progress();

                if(dproc->sock != -1){
                        SL_event_post(buf, SL_header->msglen, curr->procdata->id, 0, 0,&reqs[2] );
                        SL_msg_progress();
                }
        }
        curr = curr->next;

    }
    return 1;
}



int MCFA_event_addprocs(SL_event_msg_header *header, int numprocs)
{
    struct MCFA_proc_node               *list =NULL;
    char                                *buf = NULL;
    int                                 msglen = 0;
    SL_event_msg_header                 SL_header ;
    char 				**start_host, path[MAXNAMELEN];
    struct      MCFA_process *proc = NULL;
    int id,port,jobID,newnumprocs,hostCount,redundancy,spawn_flag,cluster_flag;
	
	
   start_host = MCFA_get_startproc(header, &hostCount);
   id = header->procid;
   strcpy(path,header->executable);
   port = SL_this_procport;
   newnumprocs = header->numprocs;
   hostCount = MCFA_get_total_hosts(hostList);
   redundancy = 1;      //should be modified(hardcoded as of now)

  
   if (header->jobid != MCFA_EXISTING_JOBID){
        jobID = MCFA_get_nextjobID();
    }
    else{
        proc = MCFA_getlast_proc(procList);
        jobID = proc->jobid;
    }
   SL_header.cmd = SL_CMD_ADD_PROC;

   if (header->cmd == MCFA_CMD_ADD_PROCID){
	proc = MCFA_search_proc(procList,header->procid);
	strcpy(path,proc->executable); 
	jobID = proc->jobid;
	SL_header.cmd = SL_CMD_ADD_EXISTING_PROC;
   }

   
  PRINTF(("MCFA_event_addprocs:Updating hostlist and procList with the information of newproc\n"));

   list = MCFA_set_lists(id,start_host,path,port,jobID,newnumprocs,hostCount,redundancy);

    


   PRINTF(("MCFA_startprocs: Request to add new processes \n"));
    buf = MCFA_pack_proclist(list, &msglen);
    SL_header.msglen = msglen;

    MCFA_inform_existing_procs(&SL_header, buf, numprocs);

    spawn_flag = 0;	//should be modified(hardcoded as of now)
    cluster_flag = 0;
    char *argg = NULL;
    MCFA_spawn_processes(start_host,path,argg,port,jobID,newnumprocs,hostCount,redundancy,spawn_flag,cluster_flag,list);  
    return MCFA_SUCCESS;
}

int MCFA_event_addprocid(SL_event_msg_header *header)
{
	int id, jobid,port,hostCount;
	char **start_host,*path;
	struct MCFA_process *proc;
//	struct MCFA_host *host;
	struct MCFA_proc_node               *list =NULL;

	start_host = MCFA_get_startproc(header, &hostCount);
	id = header->procid;
	proc = MCFA_search_proc(procList,header->procid);
	MCFA_get_path(proc->executable,&path);
	jobid = proc->jobid;
	hostCount = MCFA_get_total_hosts(hostList);
	port = SL_this_procid;
	
     if (strcmp(proc->hostname, start_host[0])!=0){

	list = MCFA_set_lists(id,start_host,path,port,jobid,hostCount,hostCount,1);
	}
	else{
		printf("Do something....no use\n");
		exit(-1);
	}


	return MCFA_SUCCESS;
	

}
int MCFA_event_deletejob(SL_event_msg_header *header, int numprocs, int *num)
{
    
    SL_Request                          reqs[3];
    struct MCFA_proc_node               *list =NULL;
    char                                *buf = NULL;
    int                                 msglen = 0;
    struct MCFA_proc_node               *curr = NULL;
    SL_event_msg_header                 SL_header ;
    int 				k;
    SL_proc                             *dproc;

    
    PRINTF(("MCFA_startprocs: Request to delete processes \n"));
    list = MCFA_delete_job(header, num);
    if (list != NULL){
	buf = MCFA_pack_proclist(list, &msglen);
	curr = procList;
	SL_header.cmd = SL_CMD_DELETE_PROC;
	SL_header.msglen = msglen;
	for(k=0;k<numprocs;k++)
	{
	    dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata->id);
            if(curr->procdata->status != 0 && dproc->sock != -1){
	    	PRINTF(("MCFA_startprocs:sending deleted process list to process with rank  %d\n",k));
	    	SL_event_post(&SL_header, sizeof(SL_event_msg_header),curr->procdata->id, 0, 0,&reqs[0]);
	    	SL_msg_progress();
	 /*   SL_event_post(&msglen, sizeof(int), curr->procdata.id, 0, 0,&reqs[1]);
	    SL_msg_progress();*/

	    	if (dproc->sock != -1){
	    		SL_event_post(buf, msglen, curr->procdata->id, 0, 0,&reqs[2] );
	    		SL_msg_progress();
	    	}
	    curr = curr->next;
	    }
	}
    }
    
    return MCFA_SUCCESS;
}

int MCFA_event_deleteproc(SL_event_msg_header *header, int numprocs)
{
    SL_Request                          reqs[3];
    struct MCFA_proc_node               *list =NULL;
    char                                *buf = NULL;
    int                                 msglen = 0;
    struct MCFA_proc_node               *curr = NULL;
    SL_event_msg_header                 SL_header ;
    int                                 k;
    SL_proc				*dproc;
    
    
    list = MCFA_delete_proc(header);
    if (list != NULL){
	buf = MCFA_pack_procstatus(list, header->procid, &msglen);
	curr = procList;
	SL_header.cmd = SL_CMD_DELETE_PROC;
	SL_header.msglen = msglen;
	
	for(k=0;k<numprocs;k++){
	    dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata->id);   
	    if(curr->procdata->status != 0 && dproc->sock != -1){
		    PRINTF(("MCFA_startprocs:sending deleted process list to process with rank  %d\n",curr->procdata->id));
		    SL_event_post(&SL_header, sizeof(SL_event_msg_header),curr->procdata->id, 0, 0,&reqs[0]);
		    SL_msg_progress();

/*		    SL_event_post(&msglen, sizeof(int), curr->procdata.id, 0, 0,&reqs[1]);
		    SL_msg_progress();*/
		    if (dproc->sock != -1){	
		    	SL_event_post(buf, msglen, curr->procdata->id, 0, 0,&reqs[2] );
		    	SL_msg_progress();
		    }
	   }
	    curr = curr->next;
	}
    
	dproc = SL_array_get_ptr_by_id ( SL_proc_array,header->procid  );
        PRINTF(("MCFA_startprocs: Handling error for proc %d\n\n\n\n",dproc->id));
        SL_proc_handle_error ( dproc, SL_ERR_PROC_UNREACHABLE,FALSE);
}
	MCFA_proc_close(procList, header->procid);
    
    return MCFA_SUCCESS;
}


int MCFA_event_printjobstatus(SL_event_msg_header *header)
{
	char   *buf = NULL;
	int    msglen = 0;

	
	buf = MCFA_pack_jobstatus(procList, header->jobid, &msglen);
        SL_Send(&msglen, sizeof(int), header->id, 0, 0);
        SL_Send(buf, msglen, header->id, 0, 0 );
	return MCFA_SUCCESS;
}

int MCFA_event_printprocstatus(SL_event_msg_header *header)
{
	char   *buf = NULL;
        int    msglen = 0;

	buf = MCFA_pack_procstatus(procList, header->procid, &msglen);
        SL_Send(&msglen, sizeof(int), header->id, 0, 0);
        SL_Send(buf, msglen, header->id, 0, 0 );

        return MCFA_SUCCESS;
}

int MCFA_event_printalljobstatus(SL_event_msg_header *header)
{
	char   *buf = NULL;
        int    msglen = 0;

	buf = MCFA_pack_proclist(procList, &msglen);
        SL_Send(&msglen, sizeof(int), header->id, 0, 0);
        SL_Send(buf, msglen, header->id, 0, 0 );

	return MCFA_SUCCESS;
}

int MCFA_event_printhoststatus(SL_event_msg_header *header)
{
	char   *buf = NULL;
        int    msglen = 0;

	buf = MCFA_pack_hoststatus(hostList, header->hostname, &msglen);
        SL_Send(&msglen, sizeof(int), header->id, 0, 0);
        SL_Send(buf, msglen, header->id, 0, 0 );

        return MCFA_SUCCESS;
}

int MCFA_event_printallhoststatus(SL_event_msg_header *header)
{
	char   *buf = NULL;
        int    msglen = 0;

	buf = MCFA_pack_hostlist(hostList,&msglen);
        SL_Send(&msglen, sizeof(int), header->id, 0, 0);
        SL_Send(buf, msglen, header->id, 0, 0 );
        return MCFA_SUCCESS;
}




