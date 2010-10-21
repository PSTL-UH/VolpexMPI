#include "mpi.h"
#include "SL_msg.h"

extern int SL_this_procid;
extern SL_array_t *Volpex_proc_array;

extern NODEPTR head, insertpt, curr;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
extern int next_avail_comm;
extern int request_counter;

extern int SL_this_procid;

extern char *hostip;
extern char *hostname;

extern Max_tag_reuse *maxtagreuse;
//static int not_found=0;
 /*******************************/
	extern double timer;
    /*******************************/
int MAX_REUSE;
extern double init_msg_time;
extern double repeat_msg_time;

Volpex_returnheaderlist *returnheaderList;

extern Volpex_dest_source_fnct *Volpex_dest_source_select;
extern Volpex_target_list *Volpex_targets;

int  Volpex_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    MPI_Status mystatus;
    MPI_Request myrequest;
    
    Volpex_Isend(buf, count, datatype, dest, tag, comm, &myrequest);
    myrequest = MPI_REQUEST_NULL;
    Volpex_Wait(&myrequest, &mystatus);
    
    return MPI_SUCCESS;
}

int  Volpex_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
		 MPI_Comm comm, MPI_Status *status)
{
    MPI_Request myrequest;
    
    Volpex_Irecv(buf, count, datatype, source, tag, comm, &myrequest);
    Volpex_Wait(&myrequest, status);
    
    return MPI_SUCCESS;
}


int Volpex_numoftargets(int rank, int comm, int target)
{
	int i;
    Volpex_comm *communicator=NULL;
    Volpex_proc *proc=NULL,*tproc = NULL;
    Volpex_proclist *plist=NULL;
    int numoftargets = 0;
    int reuseval=0;

    communicator = Volpex_get_comm_byid ( comm );
    plist        = &communicator->plist[rank];

    for (i=0; i<plist->num; i++)
    {
	    tproc = Volpex_get_proc_byid(plist->ids[i]);
            if (tproc->id == target){
                reuseval = tproc->reuseval;
		PRINTF(("[%d]: REUSEVAl=%d TARGET=%d actualtarget=%d\n",SL_this_procid,reuseval,tproc->id,target));
	    }

    }
	
    for (i=0; i<plist->num; i++)
    {

        proc = Volpex_get_proc_byid(plist->ids[i]);
	 PRINTF(("[%d]: id=%d target=%d recvpost=%d reuseval=%d\n", SL_this_procid, proc->id, proc->plist->ids[i],
                                                                        proc->recvpost,proc->reuseval));
        if (proc->reuseval< reuseval)
		
                numoftargets++ ;
    }

		PRINTF(("[%d]: numoftargets=%d\n",SL_this_procid, numoftargets));

    return numoftargets;

}

int  Volpex_Waitall(int count, MPI_Request request[], MPI_Status status[])
{
    int i;
    
    PRINTF(("[%d] Into Volpex_Waitall\n", SL_this_procid));
    for(i = 0; i < count; i++){
	Volpex_Wait(&request[i], &status[i]);
    }
    return MPI_SUCCESS;
}

int  Volpex_Wait(MPI_Request *request, MPI_Status *status)
{
    int i, done=0;
    int k, newrequest;
    int *trequest=NULL;
    int numoftargets = 0;
    int procid=-1;
    Volpex_proc *proc;
    int finaltarget;
    double min;
    
    PRINTF(("[%d] Into Volpex_Wait for request %d\n", SL_this_procid, *request));
    if(*request == MPI_REQUEST_NULL) {
	/*pre-set by Isend*/
	return MPI_SUCCESS;
    }
    
    i = *request;
    PRINTF(("[%d]:Volpex_Wait is working on reqnumber %d target:%d\n", SL_this_procid,i, reqlist[i].target));
	   
    
    PRINTF(("[%d]:Volpex_Wait: reqnumber=%d len=%d source=%d dest=%d tag=%d comm=%d reuse=%d "
            "in_use=%d req_type=%d target=%d flag=%d\n",
	    SL_this_procid, i, reqlist[i].header->len, reqlist[i].header->src,reqlist[i].header->dest, reqlist[i].header->tag,
            reqlist[i].header->comm, reqlist[i].header->reuse,
            reqlist[i].in_use, reqlist[i].req_type, reqlist[i].target, reqlist[i].flag));



    if (reqlist[i].assoc_recv !=NULL){
    	numoftargets = reqlist[i].numtargets;
    	trequest = (int*) malloc (reqlist[i].numtargets * sizeof(int));
    	for (k=0;k<reqlist[i].numtargets;k++)
		trequest[k]=reqlist[i].assoc_recv[k];
    }
    newrequest = i;
    while(1){
	Volpex_progress();	
	if (reqlist[i].assoc_recv ==NULL){
		if(reqlist[newrequest].flag == 1)
                        done = 1;
	}

	for (k=0;k<numoftargets;k++){
		newrequest = trequest[k];
		if(reqlist[newrequest].flag == 1){
	    		done = 1;
			break;
		}
	}
	if ( done == 1 ) {
	    PRINTF(("[%d] Volpex_Wait has request %d number %d set to done target:%d numoftargets:%d nummsg:%d\n", 
				SL_this_procid, *request,newrequest,reqlist[newrequest].target, numoftargets,
						Volpex_targets[reqlist[newrequest].target%(Volpex_numprocs/redundancy)].numofmsg ));
	    *request = MPI_REQUEST_NULL;


		procid = Volpex_get_volpexid(reqlist[newrequest].target);

//		Volpex_targets[procid].target = reqlist[newrequest].target;
		PRINTF(("[%d]: proc:%d numofmsg:%d MAX_MSG:%d\n", SL_this_procid, procid,
				Volpex_targets[procid].numofmsg, MAX_MSG));

                if (Volpex_targets[procid].numofmsg==MAX_MSG){
//                        Volpex_targets[procid].target = reqlist[newrequest].target;
			min = reqlist[newrequest].time;
			finaltarget = reqlist[newrequest].target;
			for (k=0;k<numoftargets;k++){
                                PRINTF(("[%d]:requestnum:%d recive_status:%d target:%d time:%f\n", SL_this_procid,
                                                trequest[k],reqlist[trequest[k]].flag, reqlist[trequest[k]].target,
                                                reqlist[trequest[k]].time));
				if (reqlist[trequest[k]].flag == 1 && min > reqlist[trequest[k]].time){
					min = reqlist[trequest[k]].time;
					finaltarget = reqlist[trequest[k]].target;
				}
                        }

                        Volpex_targets[procid].target = finaltarget;
                        printf("####[%d]: Setting target:%d for source:%d numofmsg:%d\n", SL_this_procid,
                                Volpex_targets[procid].target,procid, Volpex_targets[procid].numofmsg);


//                                Volpex_set_newtarget(Volpex_targets[procid].target,procid, MPI_COMM_WORLD);
                                Volpex_set_newtarget(finaltarget,procid, MPI_COMM_WORLD);





                        Volpex_targets[procid].numofmsg++;
                        proc = Volpex_get_proc_byid(Volpex_targets[procid].target);
                        proc->recvpost = 1;


            }



/*
//		if (Volpex_targets[procid].numofmsg==MAX_MSG){
		if (MAX_MSG_TIME < SL_papi_time() - init_msg_time && Volpex_targets[procid].numofmsg<MAX_MSG){
//                        Volpex_targets[procid].target = reqlist[newrequest].target;
			PRINTF(("####[%d]: Setting target:%d for source:%d\n", SL_this_procid,
                                Volpex_targets[procid].target,procid));
			for(i=0;i<Volpex_numprocs;i++){			
			//	if (Volpex_targets[i % (Volpex_numprocs/redundancy)].target == -1)
			//	printf("[%d]: BIIIG PROBLEM!!!!!!!!!!!!\n");
                        	Volpex_set_newtarget(Volpex_targets[i % (Volpex_numprocs/redundancy)].target, i % (Volpex_numprocs/redundancy),MPI_COMM_WORLD);
				printf("####[%d]: Setting target:%d for source:%d numofmsg:%d\n", SL_this_procid,
                                Volpex_targets[i % (Volpex_numprocs/redundancy)].target,i% (Volpex_numprocs/redundancy),
					Volpex_targets[i % (Volpex_numprocs/redundancy)].numofmsg);
				
                        	Volpex_targets[i % (Volpex_numprocs/redundancy)].numofmsg = MAX_MSG+1;
				proc = Volpex_get_proc_byid(Volpex_targets[i % (Volpex_numprocs/redundancy)].target);
				proc->recvpost = 1;
			}

            }

*/

	    for (k=0;k<numoftargets;k++){
		if(reqlist[trequest[k]].reqnumber !=newrequest){
			PRINTF(("[%d]:Setting %d recv_dup_status to 1\n",SL_this_procid,trequest[k]));
			reqlist[trequest[k]].recv_dup_status = 1;
		}
	    }


	    memcpy(reqlist[newrequest].actualbuf, reqlist[newrequest].buffer, reqlist[newrequest].header->len);
	    PRINTF(("[%d]: Copied buffer of req:%d len:%d target:%d\n", SL_this_procid, newrequest, 
			reqlist[newrequest].header->len, reqlist[newrequest].target ));

	    Volpex_free_request(newrequest);



	   if ( NULL != status && SL_STATUS_IGNORE != status ) {
                status->SL_SOURCE  = reqlist[i].returnheader.dest;
                status->SL_TAG     = reqlist[i].returnheader.tag;
                status->SL_ERROR   = SL_SUCCESS;
                status->SL_CONTEXT = reqlist[i].returnheader.comm;
                status->SL_LEN     = reqlist[i].returnheader.len;
            }
	    break;
	}
    }
    free(trequest);
    return MPI_SUCCESS;
}

int  Volpex_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
    int i, done=0;
    int k, newrequest;
    int *trequest=NULL;
    int numoftargets = 0;


    PRINTF(("[%d] Into Volpex_Test for request %d\n", SL_this_procid, *request));
    if(*request == MPI_REQUEST_NULL) {
        *flag = 1;
        if ( NULL != status && SL_STATUS_IGNORE != status ) {
            status->SL_SOURCE = MPI_ANY_SOURCE;
            status->SL_TAG    = MPI_ANY_TAG;
            status->SL_ERROR  = MPI_SUCCESS;
            status->SL_LEN    = MPI_UNDEFINED;
            status->SL_CONTEXT = MPI_UNDEFINED;
        }
        return MPI_SUCCESS;
    }

    i = *request;
    PRINTF(("[%d]:Volpex_Test is working on reqnumber %d\n", SL_this_procid,i));


    PRINTF(("[%d] Test reqnumber=%d len=%d source=%d tag=%d comm=%d reuse=%d "
            "in_use=%d req_type=%d target=%d flag=%d\n",
            SL_this_procid, i, reqlist[i].header->len, reqlist[i].header->dest, reqlist[i].header->tag,
            reqlist[i].header->comm, reqlist[i].header->reuse,
            reqlist[i].in_use, reqlist[i].req_type, reqlist[i].target, reqlist[i].flag));



    if (reqlist[i].assoc_recv !=NULL){
        numoftargets = reqlist[i].numtargets;
        trequest = (int*) malloc (reqlist[i].numtargets * sizeof(int));
        for (k=0;k<reqlist[i].numtargets;k++)
                trequest[k]=reqlist[i].assoc_recv[k];
    }




    *flag = 0; /* FALSE */
    newrequest = i;
    Volpex_progress();
    
    if (reqlist[i].assoc_recv ==NULL){
                if(reqlist[newrequest].flag == 1)
                        done = 1;
        }

        for (k=0;k<numoftargets;k++){
                newrequest = trequest[k];
                if(reqlist[newrequest].flag == 1){
                        done = 1;
                        break;
                }
        }
/* 
    if(reqlist[i].flag == 1){
        done  = 1;
        *flag = 1;
    }
*/
    if ( done == 1 ) {
        PRINTF(("[%d] Volpex_Test has request %d set to done\n", SL_this_procid, *request));


        PRINTF(("[%d] Volpex_Test has request %d number %d set to done target:%d\n",
                                SL_this_procid, *request,newrequest,reqlist[newrequest].target ));
            *request = MPI_REQUEST_NULL;
            if (Volpex_targets[reqlist[newrequest].header->src].numofmsg==MAX_MSG){
                    Volpex_targets[reqlist[newrequest].header->src].target = reqlist[newrequest].target;
                   printf("###[%d]:SETTING TARGET AS %d  \n",SL_this_procid,
                        reqlist[newrequest].target );

            }

            for (k=0;k<numoftargets;k++){
                if(reqlist[trequest[k]].reqnumber !=newrequest){
                        PRINTF(("[%d]:Setting %d recv_dup_status to 1\n",SL_this_procid,trequest[k]));
                        reqlist[trequest[k]].recv_dup_status = 1;
                }
            }


            memcpy(reqlist[newrequest].actualbuf, reqlist[newrequest].buffer, reqlist[newrequest].header->len);
            PRINTF(("[%d]: Copied buffer of req:%d len:%d target:%d\n", SL_this_procid, newrequest,
                        reqlist[newrequest].header->len, reqlist[newrequest].target ));

            Volpex_free_request(newrequest);


        




/*






        *request = MPI_REQUEST_NULL;
        reqlist[i].in_use    = 0;
        reqlist[i].req_type  = -1;
        reqlist[i].target    = -1;
        reqlist[i].flag      = 0;
        free ( reqlist[i].header );
        reqlist[i].recv_status = -1;
        reqlist[i].send_status = -1;
        reqlist[i].reqnumber = -1;

*/
        if ( NULL != status && SL_STATUS_IGNORE != status ) {
            status->SL_SOURCE  = reqlist[i].returnheader.dest;
            status->SL_TAG     = reqlist[i].returnheader.tag;
            status->SL_ERROR   = SL_SUCCESS;
            status->SL_CONTEXT = reqlist[i].returnheader.comm;
            status->SL_LEN     = reqlist[i].returnheader.len;
        }
    }
    free(trequest);
    return MPI_SUCCESS;
}

int Volpex_Waitany ( int count, MPI_Request *requests, int *index, MPI_Status *status )
{
    int ret=MPI_SUCCESS;
    int invalid_requests=0;
    int i=0, flag=0;
    MPI_Status stat;

    if (count == 0 ) {
        *index = MPI_UNDEFINED;
        if ( NULL != status && SL_STATUS_IGNORE != status ) {
            status->SL_SOURCE = MPI_ANY_SOURCE;
            status->SL_TAG    = MPI_ANY_TAG;
            status->SL_ERROR  = MPI_SUCCESS;
            status->SL_LEN    = MPI_UNDEFINED;
            status->SL_CONTEXT = MPI_UNDEFINED;
        }
        return ret;
    }


    do {
        if ( i == count ) {
            /* Quit if we only have invalid requests */
            if (invalid_requests == count ) {
                break;
            }
            invalid_requests=0;
            i=0;
        }

        if (requests[i] != MPI_REQUEST_NULL ) {
            Volpex_Test ( &(requests[i]), &flag, &stat );
        }
        else {
            invalid_requests ++;
        }
        i++;

    } while ( flag == 0);
    if (invalid_requests == count ) {
        *index = MPI_UNDEFINED;
        if ( NULL != status && SL_STATUS_IGNORE != status ) {
            status->SL_SOURCE = MPI_ANY_SOURCE;
            status->SL_TAG    = MPI_ANY_TAG;
            status->SL_ERROR  = MPI_SUCCESS;
            status->SL_LEN    = MPI_UNDEFINED;
            status->SL_CONTEXT = MPI_UNDEFINED;
        }
    }
    else {
        *index = i-1;
        if ( NULL != status && SL_STATUS_IGNORE != status ) {
            status->SL_SOURCE = stat.SL_SOURCE;
            status->SL_TAG    = stat.SL_TAG;
            status->SL_ERROR  = stat.SL_ERROR;
            status->SL_LEN    = stat.SL_LEN;
            status->SL_CONTEXT = stat.SL_CONTEXT;
            ret = stat.SL_ERROR;
        }
    }

    return ret;
}



int  Volpex_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, 
		  MPI_Comm comm, MPI_Request *request)
{
    int i, len, k=0;
    int j, istart;
    int reuse, ret;
    int *targets=NULL, numoftargets;
    int *assoc_reqs = NULL;
    Volpex_proc *proc=NULL;
    NODEPTR thisptr;
    Volpex_msg_header *theader, *header;
    int flag;

	Volpex_dest_source_select(dest, comm, fullrank, &targets, &numoftargets, count,SEND);
    if ( numoftargets == 0 ) {
	printf("[%d] VIsend: no target left for destination %d\n", SL_this_procid, dest );
	return MPI_ERR_OTHER;
    }
    PRINTF(("[%d] VIsend: total number of targets are %d\n", SL_this_procid, numoftargets));
    for(k=0; k<numoftargets; k++)
    	PRINTF(("[%d] VIsend: Target(%d) = %d\n", SL_this_procid, k, targets[k]));

    proc  = Volpex_get_proc_byid ( targets[0] );
    reuse = Volpex_tag_reuse_check ( proc->rank_MCW, tag, comm,0);
    len   = Volpex_get_len ( count, datatype);
    theader = Volpex_get_msg_header(len, hdata[comm].myrank, dest, tag, comm, reuse);
    flag = Volpex_insert_maxreuse(maxtagreuse, theader);
    if(flag==0)
	Volpex_add_maxreuse(&maxtagreuse, theader);
	
    PRINTF(("[%d] Into Volpex_Isend len=%d dest=%d tag=%d reuse=%d\n", 
	    SL_this_procid, len, dest, tag, reuse));

    if(dest == hdata[comm].myrank){
	i = Volpex_request_get_counter ( 1 );
	Volpex_request_clean ( i, 1 );
	reqlist[i].target = hdata[comm].myrank ;
	reqlist[i].req_type = 0;  /*1 = irecv*/
	reqlist[i].in_use = 1;
	reqlist[i].flag = 0;
	reqlist[i].buffer = buf;
	reqlist[i].header = Volpex_get_msg_header(len, dest, dest, tag, comm, reuse);
	reqlist[i].reqnumber = i;
	reqlist[i].send_status = 1; /* no need to post a follow up operation */

	ret = SL_Isend(buf, len, SL_this_procid, tag, comm, &reqlist[i].request);
	if ( ret != SL_SUCCESS ) {
	    PRINTF((" hae?\n"));
	}

	PRINTF(("[%d] VIsend: send-to-self from volpex tag=%d reuse=%d SL-request=%d\n", 
		SL_this_procid, tag, reuse, reqlist[i].request->id));
	*request = MPI_REQUEST_NULL;
	if ( NULL != targets ) {
	    free ( targets );
	}
	return MPI_SUCCESS;
    }
    

    PRINTF(("[%d]:VIsend: To Send Buffer: dest %d, len %d, tag %d, comm %d, reuse%d\n", SL_this_procid, dest, len, tag, comm, reuse));



    for (j=0;j<numoftargets; j++){
        if(Volpex_check_recvpost(comm,targets[j]))
                continue;


	istart  = Volpex_request_get_counter(1);
        Volpex_request_clean ( istart, 1 );
        i = istart;

        reqlist[i].target = targets[j];
        reqlist[i].cktag = CK_TAG; //for regular buffer check
        reqlist[i].req_type = 0;  //0 = send
        reqlist[i].in_use = 1;
        reqlist[i].flag = 0;
        reqlist[i].header = Volpex_get_msg_header(len, hdata[comm].myrank, dest, tag, comm, reuse);
        reqlist[i].send_status = 0;
        reqlist[i].reqnumber = i;
//      assoc_reqs[j] = i;



        PRINTF(("[%d] VIsend: Setting Irecv to %d len: %d cktag: %d comm: %d for reqnumber %d reuse %d tag %d\n",
                SL_this_procid, targets[j], reqlist[i].header->len, reqlist[i].cktag, comm, i,
                reqlist[i].header->reuse, reqlist[i].header->tag));



        ret = SL_recv_post(&reqlist[i].returnheader, sizeof(Volpex_msg_header), targets[j],
                           reqlist[i].cktag, comm,
                           SL_ACCEPT_INFINITE_TIME, &reqlist[i].request);


        Volpex_set_recvpost(comm, targets[j]);
        if(ret != SL_SUCCESS){
            printf("[%d] VIsend Error: After SL_recv_post in Volpex_Send, setting "
                    "VOLPEX_PROC_STATE_NOT_CONNECTED for process : %d\n", SL_this_procid,targets[j]);
            Volpex_set_state_not_connected(targets[j]);
        }





     }
    PRINTF(("[%d]: Checking reuse len=%d, rank:%d, dest:%d, target:%d tag:%d, comm:%d, reuse:%d\n", SL_this_procid, 
		len, hdata[comm].myrank, dest, targets[0], tag, comm, reuse));
            header = Volpex_get_msg_header(len, hdata[comm].myrank, dest, tag, comm, reuse);
    thisptr = Volpex_send_buffer_insert(insertpt, header, assoc_reqs, buf);

    insertpt = thisptr->fwd;

    if ( NULL != targets ) {
	free ( targets );
    }
    *request = MPI_REQUEST_NULL;
    PRINTF(("[%d]:Moving into Volpex_progress from VIsend\n",SL_this_procid));
    Volpex_progress();
    
    return MPI_SUCCESS;
}

int  Volpex_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
		  MPI_Comm comm, MPI_Request *request)
{
    int len;
    len = Volpex_get_len(count, datatype);
    
    PRINTF(("[%d] VIrecv: count %d, from %d, tag %d, comm %d\n", 
	    SL_this_procid, count, source, tag, comm));
    
    if(source == MPI_ANY_SOURCE){
	printf("VIrecv from any_source not supported!\n");
    }
    
    return Volpex_Irecv_ll ( buf, len, source, tag, comm, request, -1 );
}


int  Volpex_Irecv_ll(void *buf, int len, int source, int tag, 
		     MPI_Comm comm, MPI_Request *request, int new_req )
{
    int k=0,i, j, ret;
    int reuse;
    int *targets=NULL, numoftargets;
    int num_errors = 0;
    Volpex_proc *proc=NULL;
    int *assoc_recv;
    int procid=-1;



	 Volpex_dest_source_select(source, comm, fullrank, &targets, &numoftargets, len,RECV); 
    if ( 0 == numoftargets ) {
	printf("[%d] VIrecv_ll: no target left for source %d\n", SL_this_procid, source );
	return MPI_ERR_OTHER;
    }
    PRINTF(("[%d] VIrecv: total number of targets are %d\n", SL_this_procid, numoftargets));
    for(k=0; k<numoftargets; k++)
        PRINTF(("[%d] VIrecv: Target(%d)=%d\n", SL_this_procid, k, targets[k]));

	assoc_recv = (int *)malloc(numoftargets * sizeof(int));

	procid = Volpex_get_volpexid(targets[0]);


    if ( new_req == -1 ) {
	proc = Volpex_get_proc_byid ( targets[0] );
	reuse = Volpex_tag_reuse_check ( proc->rank_MCW,tag,comm, 1);
	i = Volpex_request_get_counter ( 1 );
	Volpex_request_clean ( i, 1 );
    }
    else {
	reuse = reqlist[new_req].header->reuse;
	i = new_req;
    }
    
    PRINTF(("[%d] Into Volpex_Irecv_ll len=%d tag=%d source=%d reuse=%d new_req=%d\n", 
	    SL_this_procid, len, tag, source, reuse, new_req));
    
    
    if(source == hdata[comm].myrank){
	reqlist[i].target = SL_this_procid;
	reqlist[i].req_type = 1;  /*1 = irecv*/
	reqlist[i].in_use = 1;
	reqlist[i].flag = 0;
	reqlist[i].buffer = malloc (len);
	reqlist[i].actualbuf = buf;
	reqlist[i].header = Volpex_get_msg_header(len, source, source, tag, comm, reuse);
	reqlist[i].reqnumber = i;
	reqlist[i].recv_status = 1; /* no need to post a follow up operation */
        reqlist[i].time = SL_papi_time();
	reqlist[i].numtargets = 1;
	reqlist[i].assoc_recv = (int*) malloc (sizeof (int));
	reqlist[i].assoc_recv[0]=i;
	
	SL_Irecv(reqlist[i].buffer, len, SL_this_procid, tag, comm, &reqlist[i].request);
	
	PRINTF(("[%d] VIrecv: Recv from self tag=%d reuse=%d SL-request %d reqid:%d\n", SL_this_procid, 
		tag, reuse, reqlist[i].request->id, i));
	*request = i;
	if ( NULL != targets ) {
	    free ( targets );
	}
	return MPI_SUCCESS;
    }
    
    
    

    for(j = 0; j < numoftargets; j++){
	    reqlist[i].target = targets[j];
	    reqlist[i].req_type = 1;  /*1 = irecv*/
	    reqlist[i].in_use = 1;
	    reqlist[i].flag = 0;
	    reqlist[i].actualbuf = buf;
	    reqlist[i].buffer = malloc(len);
	    reqlist[i].header = Volpex_get_msg_header(len, source, hdata[comm].myrank, tag, comm, reuse);
	    reqlist[i].reqnumber = i;
	    reqlist[i].recv_status = 0;
	    reqlist[i].time = SL_papi_time();
	    reqlist[i].numtargets = numoftargets;
            PRINTF(("[%d]:VIrecv: Isend for len %d to primary target %d %d %d  for reqnumber %d reuse %d numofmsg:%d\n", SL_this_procid,
                    reqlist[i].header->len, targets[j], CK_TAG, comm, i, reqlist[i].header->reuse,Volpex_targets[source].numofmsg ));

	
	    ret = SL_Isend(reqlist[i].header, sizeof(Volpex_msg_header), targets[j], 
			   CK_TAG, comm, &reqlist[i].request);


	    if(ret != SL_SUCCESS){
		printf("[%d]VIrecv Error: setting VOLPEX_PROC_STATE_NOT_CONNECTED\n",SL_this_procid);
		Volpex_set_state_not_connected(targets[j]);
		reqlist[i].flag = 2;
		num_errors++;		
	    }
	    else if(Volpex_targets[procid].numofmsg>MAX_MSG){
                *request = i;
                Volpex_progress();
                if ( NULL != targets ) {
                    free ( targets );
		    targets = NULL;
                }
//		Volpex_targets[procid].numofmsg++;
//		if(Volpex_targets[procid].numofmsg == 400 ){
		if(SL_papi_time()-repeat_msg_time > MAX_MSG_REPEAT ){
			PRINTF(("[%d]: Repeating learning phase for procid:%d\n", SL_this_procid,procid));
			repeat_msg_time = SL_papi_time();
//			for(i=0;i<Volpex_numprocs;i++){
//                                Volpex_targets[i % (Volpex_numprocs/redundancy)].numofmsg = 0;

//			}
			Volpex_targets[procid].numofmsg = 0;
		}

                return MPI_SUCCESS;
            }

	
		assoc_recv[j]=i;
	//	reqlist[i].numtargets = numoftargets;
	        reqlist[i].assoc_recv = (int*) malloc (numoftargets * sizeof (int));
		i  = Volpex_request_get_counter ( 1 );
                Volpex_request_clean ( i, 1 );


}
int p,q;
for(p = 0; p < numoftargets; p++){
	for(q = 0; q < numoftargets; q++)
		reqlist[assoc_recv[p]].assoc_recv[q] = assoc_recv[q];
}
	PRINTF(("[%d]: checking proc:%d numofmsg:%d time:%f\n", SL_this_procid, procid, Volpex_targets[procid].numofmsg, 
				SL_papi_time()-init_msg_time));
	if ((MAX_MSG_TIME<SL_papi_time()-init_msg_time) && (Volpex_targets[procid].numofmsg == 0  )){
/*		for(k=0;k<Volpex_numprocs/redundancy;k++){	
			Volpex_targets[k].numofmsg++;
			printf("[%d]: Incrementing proc:%d numofmsg:%d\n", SL_this_procid, k, Volpex_targets[k].numofmsg);
		}
*/		PRINTF(("[%d]: Incrementing proc:%d numofmsg:%d\n", SL_this_procid, procid, Volpex_targets[procid].numofmsg));
		Volpex_targets[procid].numofmsg++;

	}




	*request = i-1;

	if(targets !=NULL){
		free ( targets );
		targets = NULL;
	}
        Volpex_progress();

/*
	    if(ret == SL_SUCCESS){
		*request = i;
		Volpex_progress();
		if ( NULL != targets ) {
		    free ( targets );
		}
		return MPI_SUCCESS;
	    }

	num_errors++;

	if ( num_errors == numoftargets ) {
	    if ( NULL != targets ) {
		free ( targets );
	    }
	    return MPI_ERR_OTHER;
	}
*/    

    return MPI_SUCCESS;
}

int  Volpex_Abort(MPI_Comm comm, int errorcode)
{
    PRINTF(("[%d]:Error %d occured on context_id %d. Aborting.\n", SL_this_procid, errorcode, comm));
    exit(errorcode);
    return MPI_SUCCESS;
}

int Volpex_Cancel_byReqnumber(int reqnumber)
{
    int flag;
    int j = reqnumber;
    
    if ( j < 0 || j >= REQLISTSIZE ) {
	return 0;
    }
PRINTF(("[%d]: Volpex_Cancel_byReqnumber:%d\n", SL_this_procid, j));

     if ( reqlist[j].in_use == 1){
	reqlist[j].returnheader.len   = -1;
        reqlist[j].returnheader.src   = -1;
        reqlist[j].returnheader.dest  = -1;
        reqlist[j].returnheader.tag   = -1;
        reqlist[j].returnheader.comm  = -1;
        reqlist[j].returnheader.reuse = -1;
	SL_Cancel(&reqlist[j].request, &flag);
        PRINTF(("[%d]:SL_Cancel executed for request number %d\n", SL_this_procid, reqlist[j].reqnumber));
        if ( !flag ) {
            printf("[%d]: Could not cancel SL request %d volpex request %d. What now?\n", SL_this_procid,
                   reqlist[j].request->id, j );
        }
	Volpex_free_request(j);
	reqlist[j].insrtbuf = NULL;

     }


    return 0;
}
int Volpex_request_get_counter ( int red )
{
    int ret;

    if ( (request_counter + red) >= REQLISTSIZE ) {
        /* need to reset the request counter to 0 and
           start over */
        PRINTF(("[%d]:REQUEST_COUNTER reset to 0\n",SL_this_procid));
	ret = 0;
    }

    else {
        ret = request_counter;
    }


    while (reqlist[ret].in_use == 1){
	ret++;
	
    }

    request_counter = ret+ red;

    PRINTF(("[%d]:REQUEST_COUNTER=%d \n",SL_this_procid,request_counter));
    return ret;
}
int Volpex_request_clean ( int start, int red )
{
    int i;
//    int flag;

    for ( i=start; i<start+red; i++ ) {
        if ( reqlist[i].in_use == 1 ) {
            /* this request is currently utilized and we need to
               free it */
		Volpex_Cancel_byReqnumber(i);

        }

    }

    return 0;
}
int Volpex_buffer_remove_ref  ( NODEPTR elem,  int reqid )
{
    int i;
    
    if ( NULL != elem )  {
	for ( i=0; i<redundancy; i++ ) {
	    if ( elem->reqnumbers[i] == reqid ) {
		elem->reqnumbers[i] = -1;
	    }
	}
	}

    return 0;
}

void Volpex_request_update_counter ( int num )
{
    request_counter += num;
    return;
}

int Volpex_Pack(void* inbuf, int count, MPI_Datatype datatype, void* outbuf,
                int outsize, int *position, MPI_Comm comm)
{

    int ret = MPI_SUCCESS;
    int len;
    char *inbuf_p, *outbuf_p;

    /* Note: this implementation of Pack only works for basic, contiguous
    ** data types and arrays
    */

    inbuf_p  = (char *) (inbuf);
    outbuf_p = (char *) outbuf + *position;
    len = Volpex_get_len(count, datatype);

    memcpy (outbuf_p, inbuf_p, len );
    *position += len;

    return ret;
}

int Volpex_Unpack( void* inbuf, int insize, int *position, void* outbuf, int outcount,
                   MPI_Datatype datatype, MPI_Comm comm)

{
    int ret = MPI_SUCCESS, len;
    char *inbuf_p, *outbuf_p;

    /* Note: this implementation of Unpack only works for basic, contiguous
    ** data types and arrays
    */
    inbuf_p  = (char *) inbuf + *position;
    outbuf_p = (char *) outbuf;
    len = Volpex_get_len ( outcount, datatype);

    memcpy ( outbuf_p, inbuf_p, len);
    *position += len;

    return ret;
}
 
int Volpex_Type_size ( MPI_Datatype datatype, int *size )
{
    *size = Volpex_get_len ( 1, datatype );
    return MPI_SUCCESS;
}

