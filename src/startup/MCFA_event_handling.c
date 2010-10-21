
#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"


extern struct      MCFA_host_node *hostList;
extern struct      MCFA_proc_node *procList;

int MCFA_event_addprocs(SL_event_msg_header *header, int numprocs)
{
    SL_Request                          reqs[3];
    struct MCFA_proc_node               *list =NULL;
    char                                *buf = NULL;
    int                                 msglen = 0;
    struct MCFA_proc_node               *curr = NULL;
    SL_event_msg_header                 SL_header ;
    int                                 k;
    SL_proc                             *dproc;
    
    
    PRINTF(("MCFA_startprocs: Request to add new processes \n"));
    list = MCFA_add_procs(header);
    buf = MCFA_pack_proclist(list, &msglen);
    PRINTF((" MCFA_startprocs: Sending new process list to already existing processes \n"));
    curr = procList;
    SL_header.cmd = SL_CMD_ADD_PROC;
    SL_header.msglen = msglen;
    for(k=0;k<numprocs;k++)
    {
	dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata.id);
        if(curr->procdata.status != 0 && dproc->sock != -1){
	
		PRINTF(("MCFA_startprocs:sending new process list to process with rank  %d\n",curr->procdata.id));
		SL_event_post(&SL_header, sizeof(SL_event_msg_header),curr->procdata.id, 0, 0,&reqs[0]);
		SL_msg_progress();

/*		SL_event_post(&msglen, sizeof(int), curr->procdata.id, 0, 0,&reqs[1]);
		SL_msg_progress();*/

		if(dproc->sock != -1){
			SL_event_post(buf, msglen, curr->procdata.id, 0, 0,&reqs[2] );
			SL_msg_progress();
		}
	}
	curr = curr->next;
	
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
	    dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata.id);
            if(curr->procdata.status != 0 && dproc->sock != -1){
	    	PRINTF(("MCFA_startprocs:sending deleted process list to process with rank  %d\n",k));
	    	SL_event_post(&SL_header, sizeof(SL_event_msg_header),curr->procdata.id, 0, 0,&reqs[0]);
	    	SL_msg_progress();
	 /*   SL_event_post(&msglen, sizeof(int), curr->procdata.id, 0, 0,&reqs[1]);
	    SL_msg_progress();*/

	    	if (dproc->sock != -1){
	    		SL_event_post(buf, msglen, curr->procdata.id, 0, 0,&reqs[2] );
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
	    dproc = SL_array_get_ptr_by_id ( SL_proc_array,curr->procdata.id);   
	    if(curr->procdata.status != 0 && dproc->sock != -1){
		    PRINTF(("MCFA_startprocs:sending deleted process list to process with rank  %d\n",curr->procdata.id));
		    SL_event_post(&SL_header, sizeof(SL_event_msg_header),curr->procdata.id, 0, 0,&reqs[0]);
		    SL_msg_progress();

/*		    SL_event_post(&msglen, sizeof(int), curr->procdata.id, 0, 0,&reqs[1]);
		    SL_msg_progress();*/
		    if (dproc->sock != -1){	
		    	SL_event_post(buf, msglen, curr->procdata.id, 0, 0,&reqs[2] );
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




