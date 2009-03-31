#include "mpi.h"
#include "SL_msg.h"

extern NODEPTR head, insertpt, curr;
extern int GM_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char *hostip;
extern char *hostname;
extern int GM_numprocs;
extern int next_avail_comm;
extern int request_counter;

extern int SL_this_procid;

static int not_found=0;

int  VolPEx_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, 
		 MPI_Comm comm)
{
    MPI_Status mystatus;
    MPI_Request myrequest;
    
    VolPEx_Isend(buf, count, datatype, dest, tag, comm, &myrequest);
    myrequest = MPI_REQUEST_NULL;
    VolPEx_Wait(&myrequest, &mystatus);
    
    return MPI_SUCCESS;
}

int  VolPEx_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
		 MPI_Comm comm, MPI_Status *status)
{
    MPI_Request myrequest;
    
    VolPEx_Irecv(buf, count, datatype, source, tag, comm, &myrequest);
    VolPEx_Wait(&myrequest, status);
    
    return MPI_SUCCESS;
}

int  VolPEx_progress()
{
    SL_Status mystatus;
    int i, ret;
    int flag = 0;
    int answer = 0;
    

    SL_msg_progress ();
    for(i = 0; i < REQLISTSIZE; i++){

	/* handles progress on sends and isends from buffer */
	if(reqlist[i].in_use == 1 && reqlist[i].req_type == 0){  
	    flag = 0;
	    ret = SL_ERR_PROC_UNREACHABLE;
	    ret = SL_test_nopg(&reqlist[i].request, &flag, &mystatus);
	    
	    if(flag == 1 && reqlist[i].send_status == 0){
		if ( ret == SL_SUCCESS ) {
		    curr = VolPex_send_buffer_search(head, reqlist[i].header, &answer);
		    if(answer && reqlist[i].cktag == CK_TAG){
			answer = 0;
			PRINTF(("  VProgress: send req %d: Into SL_Send with %d,%d,%d,%d\n", i,	   
				reqlist[i].header->len, reqlist[i].target, reqlist[i].header->tag, 
				reqlist[i].header->comm));

			ret = SL_Isend(curr->buffer, reqlist[i].header->len, 
				       reqlist[i].target, reqlist[i].header->tag, 
				       reqlist[i].header->comm, &reqlist[i].request);
			reqlist[i].send_status = 1; 
			if ( ret != SL_SUCCESS ) {
			    PRINTF(("  VProgress: send req. %d: isending data to %d failed, ret = %d\n", 
				    i, reqlist[i].target, ret ));
			    /* TBD: free the request */
			    continue;
			}
			flag=0;
		    }
		    else {

			printf("  VProgress: send req. %d: Could not find entry in the send-buffer "
			       "to match %d %d %d %d\n", i, reqlist[i].header->len, reqlist[i].target, 
			       reqlist[i].header->tag, reqlist[i].header->comm);
			not_found++;
		    }
		}
		else{
		    PRINTF(("  VProgress: send req. %d: target died, ret = %d. Freeing request.\n", 
			    i, ret));
		    /* TBD: free the request */
		}
	    }
	    
	    if ( flag == 1 && reqlist[i].send_status == 1 ) {
		/* we can free the request here, since Wait operations 
		   do not check on the real request anyway. Actually,
		   we don't even care whether ret was SL_SUCCESS or not. 
		*/
		reqlist[i].in_use    = 0;
		reqlist[i].req_type  = -1;
		reqlist[i].target    = -1;
		reqlist[i].flag      = 0;
		free ( reqlist[i].header );
		reqlist[i].header = NULL;
		reqlist[i].recv_status = -1;
		reqlist[i].send_status = -1;

		Volpex_buffer_remove_ref ( reqlist[i].insrtbuf, reqlist[i].reqnumber );
		reqlist[i].reqnumber = -1;
		reqlist[i].insrtbuf  = NULL;
	    }
	}
	
	/* handles progress on irecvs from buffer */
	if(reqlist[i].in_use == 1 && reqlist[i].req_type == 1){  
	    flag = 0;
	    ret = SL_ERR_PROC_UNREACHABLE;
	    ret = SL_test_nopg(&reqlist[i].request, &flag, &mystatus);
	    
	    if(flag == 1 && reqlist[i].recv_status == 0 ) {
		if ( ret == SL_SUCCESS){
		    ret = SL_Irecv(reqlist[i].buffer, reqlist[i].header->len,
                                   reqlist[i].target, reqlist[i].header->tag,
                                   reqlist[i].header->comm, &reqlist[i].request);
		    reqlist[i].recv_status = 1; /* Ready to receive the real data */
		    PRINTF(("  VProgress: recv request:%d posted Irecv to %d ret=%d\n",
			    i, reqlist[i].target, ret ));
		    flag = 0;
		}
		
		if ( ret != SL_SUCCESS ) {
		    MPI_Request tmprequest = i;
		    GM_set_state_not_connected(reqlist[i].header->dest);
		    PRINTF(("  VProgress: recv request:%d reposting Irecv to %d, since prev. op. failed \n",
                            i, reqlist[i].header->dest ));
		    
		    ret = VolPEx_Irecv_ll ( reqlist[i].buffer, reqlist[i].header->len,
                                            reqlist[i].header->dest, reqlist[i].header->tag,
                                            reqlist[i].header->comm, &tmprequest, i );
		    if ( ret == MPI_ERR_OTHER ) {
			/* mark the request as done but set the error code. This 
			   operation can not finish, because there are not targets
			   left which are alive. */
			PRINTF(("  VProgress: recv request:%d  all targets for proc. %d dead. ret=%d\n",
                                i, reqlist[i].header->dest, ret ));
		    }
		}
	    }
	    
	    if(flag == 1 && reqlist[i].recv_status == 1 ) {
		if ( ret == SL_SUCCESS){
		    reqlist[i].flag = 1;				
		}
		else {
		    MPI_Request tmprequest = i;
		    GM_set_state_not_connected(reqlist[i].header->dest);

                    ret = VolPEx_Irecv_ll ( reqlist[i].buffer, reqlist[i].header->len,
                                            reqlist[i].header->dest, reqlist[i].header->tag,
                                            reqlist[i].header->comm, &tmprequest, i );
		    if ( ret == MPI_ERR_OTHER ) {
			/* mark the request as done but the error code. This 
			   operation can not finish, because there are not targets
			   left which are alive. */

			PRINTF(("  VProgress: recv request:%d  all targets for proc. %d dead. ret=%d\n",
                                i, reqlist[i].header->dest, ret ));
		    }
		}
	    }
	}
    }
    
    return 0;
}

int  VolPEx_Waitall(int count, MPI_Request request[], MPI_Status status[])
{
    int i;
    
    PRINTF(("Into VolPEx_Waitall\n"));
    for(i = 0; i < count; i++){
	VolPEx_Wait(&request[i], &status[i]);
    }
    return MPI_SUCCESS;
}

int  VolPEx_Wait(MPI_Request *request, MPI_Status *status)
{
    int i, done=0;
    
    PRINTF(("Into VolPEx_Wait\n"));
    if(*request == MPI_REQUEST_NULL) {
	/*pre-set by Isend*/
	return MPI_SUCCESS;
    }
    
    i = *request;
    PRINTF(("VolPEx_Wait is working on reqnumber %d\n", i));
    PRINTF(("reqnumber %d\n len%d\n source%d\n tag%d\n comm%d\n reuse%d\n "
            "in_use%d\n req_type%d\n target%d\n flag%d\n",
            i, reqlist[i].header->len, reqlist[i].header->dest, reqlist[i].header->tag,
            reqlist[i].header->comm, reqlist[i].header->reuse,
            reqlist[i].in_use, reqlist[i].req_type, reqlist[i].target, reqlist[i].flag));

    while(1){
	VolPEx_progress();	
	if(reqlist[i].flag == 1){
	    done = 1;
	    PRINTF(("VolPEx_Wait has set done = %d\n", done));
	}
	
	if ( done == 1 ) {
	    *request = MPI_REQUEST_NULL;
	    reqlist[i].in_use    = 0;
	    reqlist[i].req_type  = -1;
	    reqlist[i].target    = -1;
	    reqlist[i].flag      = 0;
	    free ( reqlist[i].header );
	    reqlist[i].header = NULL;
	    reqlist[i].recv_status = -1;
	    reqlist[i].send_status = -1;
	    reqlist[i].reqnumber = -1;
	    

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
    return MPI_SUCCESS;
}

int  VolPEx_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, 
		  MPI_Comm comm, MPI_Request *request)
{
    int i, istart, j, len;
    int reuse, ret;
    int targets[3] = {-1,-1,-1};
    VolPex_msg_header *header;
    void *buffer;
    
    /* used to kill associated requests as msg buffer is re-written or traget is unreachable*/
    int assoc_reqs[3] = {-1,-1,-1}; 
    
    PRINTF(("Into VolPEx_Isend\n"));
    len = VolPex_get_len(count, datatype);
    reuse = VolPex_tag_reuse_check(dest,tag, 0);

    if(dest == hdata[comm].myrank){
	i = Volpex_request_get_counter ( 1 );
	Volpex_request_clean ( i, 1 );
	Volpex_request_update_counter ( 1 );
	reqlist[i].target = hdata[comm].myrank ;
	reqlist[i].req_type = 0;  /*1 = irecv*/
	reqlist[i].in_use = 1;
	reqlist[i].flag = 0;
	reqlist[i].buffer = buf;
	reqlist[i].header = VolPex_get_msg_header(len, dest, tag, comm, reuse);
	reqlist[i].reqnumber = i;
	reqlist[i].send_status = 1; /* no need to post a follow up operation */

	SL_Isend(buf, len, SL_this_procid, tag, comm, &reqlist[i].request);

	PRINTF(("VIsend: send-to-self from volpex tag=%d reuse=%d SL-request=%d\n", tag, reuse, 
		reqlist[i].request->id));
	*request = MPI_REQUEST_NULL;
	return MPI_SUCCESS;
    }
    

    header = VolPex_get_msg_header(len, dest, tag, comm, reuse);
    PRINTF(("VIsend: To Send Buffer: dest %d, len %d, tag %d, comm %d, reuse%d\n", dest, len, tag, comm, reuse));
    GM_dest_src_locator(dest, comm, fullrank, targets);
    PRINTF(("VIsend: Targets are %d %d %d\n", targets[0], targets[1], targets[2]));

    istart  = Volpex_request_get_counter ( redundancy );
    Volpex_request_clean ( istart, redundancy );
    PRINTF(("VIsend: request_counter = %d\n", request_counter));
    
    for(j = 0, i=istart ; i < istart+redundancy; j++, i++){
	if(targets[j] != -1){
	    reqlist[i].target = targets[j];
	    reqlist[i].cktag = CK_TAG; /*for regular buffer check*/
	    reqlist[i].req_type = 0;  /*0 = send*/
	    reqlist[i].in_use = 1;
	    reqlist[i].flag = 0;
	    reqlist[i].header = VolPex_get_msg_header(len, dest, tag, comm, reuse);
	    reqlist[i].send_status = 0;
	    reqlist[i].reqnumber = i;
	    assoc_reqs[j] = i;
	    buffer = (VolPex_msg_header*) malloc(sizeof(VolPex_msg_header));
	    PRINTF(("VIsend: Setting Irecv to %d %d %d %d for reqnumber %d\n", 
		    CK_LEN, targets[j], reqlist[i].cktag, comm, i));

	    ret = SL_recv_post(&reqlist[i].returnheader, sizeof(VolPex_msg_header), targets[j],
                               reqlist[i].cktag, comm,
                               SL_ACCEPT_INFINITE_TIME, &reqlist[i].request);


	    if(ret != SL_SUCCESS){
		PRINTF(("VIsend Error: After SL_recv_post in VolPEx_Send, setting "
			"VOLPEX_PROC_STATE_NOT_CONNECTED\n"));
		GM_set_state_not_connected(targets[j]);
		PRINTF(("VIsend: GM[%d][%d]: id %d host %s port %d rank %s state %d\n", 
			targets[j], comm, GM[targets[j]][comm].id,
			GM[targets[j]][comm].host, 
			GM[targets[j]][comm].port, GM[targets[j]][comm].rank, 
			GM[targets[j]][comm].state));  	
	    }
	}
    }
    Volpex_request_update_counter ( redundancy );
    insertpt = VolPex_send_buffer_insert(insertpt, header, assoc_reqs, buf);
    for ( i=0; i<redundancy; i++ ) {
	if ( assoc_reqs[i] != -1 ) {
	    reqlist[assoc_reqs[i]].insrtbuf = insertpt; 
	}
    }

    *request = MPI_REQUEST_NULL;
    PRINTF(("Moving into VolPEx_progress from VIsend\n"));
    VolPEx_progress();
    
    return MPI_SUCCESS;
}

int  VolPEx_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
		  MPI_Comm comm, MPI_Request *request)
{
    int len;
    len = VolPex_get_len(count, datatype);
    
    PRINTF(("VIrecv: count %d, from %d, tag %d, comm %d\n", count, source, tag, comm));
    
    if(source == MPI_ANY_SOURCE){
	printf("VIrecv from any_source\n");
    }
    
    return VolPEx_Irecv_ll ( buf, len, source, tag, comm, request, -1 );
}


int  VolPEx_Irecv_ll(void *buf, int len, int source, int tag, 
		     MPI_Comm comm, MPI_Request *request, int new_req )
{
    int i, j, ret;
    int reuse;
    int targets[3] = {-1,-1,-1};
    int num_errors = 0;
    
    PRINTF(("Into VolPEx_Irecv\n"));
    if ( new_req == -1 ) {
	reuse = VolPex_tag_reuse_check(source,tag, 1);
	i = Volpex_request_get_counter ( 1 );
	Volpex_request_clean ( i, 1 );
	Volpex_request_update_counter ( 1 );
    }
    else {
	reuse = reqlist[new_req].header->reuse;
	i = new_req;
//	free ( reqlist[new_req].header );
    }

    if(source == hdata[comm].myrank){
	reqlist[i].target = hdata[comm].myrank ;
	reqlist[i].req_type = 1;  /*1 = irecv*/
	reqlist[i].in_use = 1;
	reqlist[i].flag = 0;
	reqlist[i].buffer = buf;
	reqlist[i].header = VolPex_get_msg_header(len, source, tag, comm, reuse);
	reqlist[i].reqnumber = i;
	reqlist[i].recv_status = 1; /* no need to post a follow up operation */
	SL_Irecv(buf, len, SL_this_procid, tag, comm, &reqlist[i].request);

	PRINTF(("VIrecv: Recv from self tag=%d reuse=%d SL-request %d\n", tag, reuse,
		reqlist[i].request->id));
	*request = i;
	return MPI_SUCCESS;
    }
     
    GM_dest_src_locator(source, comm, fullrank, targets);
    PRINTF(("VIrecv: Targets are %d %d %d\n", targets[0], targets[1], targets[2]));
    
    for(j = 0; j < redundancy; j++){
	if(targets[j] != -1){
	    PRINTF(("VIrecv: Isend to primary target %d %d %d %d for reqnumber %d\n", 
		    CK_LEN, targets[j], CK_TAG, comm, i));
	    reqlist[i].target = targets[j];
	    reqlist[i].req_type = 1;  /*1 = irecv*/
	    reqlist[i].in_use = 1;
	    reqlist[i].flag = 0;
	    reqlist[i].buffer = buf;
	    reqlist[i].header = VolPex_get_msg_header(len, source, tag, comm, reuse);
	    reqlist[i].reqnumber = i;
	    reqlist[i].recv_status = 0;
	    ret = SL_Isend(reqlist[i].header, sizeof(VolPex_msg_header), targets[j], 
			   CK_TAG, comm, &reqlist[i].request);
	    if(ret != SL_SUCCESS){
		PRINTF(("VIrecv Error: After SL_Test in VolPEx_Irecv, setting "
			"VOLPEX_PROC_STATE_NOT_CONNECTED\n"));
		GM_set_state_not_connected(targets[j]);
		PRINTF(("VIrecv: GM[%d][%d]: id %d host %s port %d rank %s state %d\n", 
			targets[j], comm, GM[targets[j]][comm].id, GM[targets[j]][comm].host, 
			GM[targets[j]][comm].port, GM[targets[j]][comm].rank, 
			GM[targets[j]][comm].state));
	    }
	    if(ret == SL_SUCCESS){
		*request = i;
		VolPEx_progress();
		return MPI_SUCCESS;
	    }
	}
	num_errors++;
	if ( num_errors == redundancy ) {
	    return MPI_ERR_OTHER;
	}
    }
    return 0;
}

int  VolPEx_Abort(MPI_Comm comm, int errorcode)
{
    PRINTF(("Error %d occured on context_id %d. Aborting.\n", errorcode, comm));
    exit(errorcode);
    return MPI_SUCCESS;
}

int VolPEx_Cancel_byReqnumber(int reqnumber)
{
    int j, flag;
    
    for(j = 0; j < request_counter; j++){
	if(reqlist[j].reqnumber == reqnumber && reqlist[j].in_use == 1){
	    reqlist[j].in_use = 0;
	    reqlist[j].req_type = -1;
	    reqlist[j].target   = -1;
	    reqlist[j].flag     = 0;
	    free ( reqlist[j].header );
	    reqlist[j].recv_status = -1;
	    SL_Cancel(&reqlist[j].request, &flag);
	    PRINTF(("SL_Cancel executed for request number %d\n", 
		    reqlist[j].reqnumber));
	    reqlist[j].reqnumber = -1;
	    reqlist[j].insrtbuf = NULL;
	}
    }
    return 0;
}

int Volpex_request_get_counter ( int red )
{
    int ret;

    if ( (request_counter + red) >= REQLISTSIZE ) {
	/* need to reset the request counter to 0 and
	   start over */
	ret = 0;
    }
    else {
	ret = request_counter;
    }
    request_counter = ret+ red;

    return ret;
}

int Volpex_request_clean ( int start, int red )
{
    int i;

    for ( i=start; i<start+red; i++ ) {
	if ( reqlist[i].in_use == 1 ) {
	    /* this request is currently utilized and we need to 
	       free it */
	    PRINTF(("request_clean: removing entry %d from the reqlist\n", i ));
	    reqlist[i].in_use    = 0;
	    reqlist[i].req_type  = -1;
	    reqlist[i].target    = -1;
	    reqlist[i].flag      = 0;
	    free ( reqlist[i].header );
	    reqlist[i].header = NULL;
	    reqlist[i].recv_status = -1;
	    reqlist[i].send_status = -1;
	    if ( NULL != reqlist[i].insrtbuf ) {
		Volpex_buffer_remove_ref ( reqlist[i].insrtbuf, reqlist[i].reqnumber );
	    }
	    reqlist[i].reqnumber = -1;
	    reqlist[i].insrtbuf  = NULL;
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
