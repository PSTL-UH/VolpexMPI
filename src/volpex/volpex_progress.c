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
static int not_found=0;
 /*******************************/
        extern double timer;
    /*******************************/
int MAX_REUSE;

Volpex_returnheaderlist *returnheaderList;
extern Volpex_dest_source_fnct *Volpex_dest_source_select;
extern Volpex_target_list *Volpex_targets;

int  Volpex_progress()
{
    SL_Status mystatus;
    int i, ret, ret1 ;
    int flag = 0;
    int answer = 0;
    NODEPTR temp;
    int maxreuseval = 0;
    int loglength;
    SL_Request request;
    int minreuseval = 999999;
    Volpex_returnheaderlist *tcurr;
    int istart;
    SL_msg_progress ();
        not_found = 0;
    if (returnheaderList != NULL)
    {
        tcurr = returnheaderList;
        while(tcurr != NULL){
            MAX_REUSE = Volpex_search_maxreuse(maxtagreuse,tcurr->rheader.header);
            if ((MAX_REUSE >= tcurr->rheader.header.reuse)&&(tcurr->rheader.header.reuse>-1)){
                temp = Volpex_send_buffer_search(insertpt, &tcurr->rheader.header, &answer, &maxreuseval, &minreuseval);
                if ( answer ){
                    answer = 0;
                    PRINTF(("[%d]VProgress: send returnheader id:%d: Into SL_Send with len=%d, target=%d, tag=%d, reuse=%d, maxreuse:%d\n",
                           SL_this_procid,  tcurr->rheader.id,
                           temp->header->len, tcurr->rheader.target, temp->header->tag, temp->header->reuse,maxreuseval));
                        if (tcurr->rheader.header.timestamp-temp->header->timestamp !=0.0)
                    PRINTF(("[%d]second:timestamp difference message =%f target:%d\n",SL_this_procid,
                                tcurr->rheader.header.timestamp-temp->header->timestamp, tcurr->rheader.target));
                    ret = SL_send_post(temp->buffer, temp->header->len,
                                       tcurr->rheader.target, temp->header->tag,
                                       temp->header->comm,SL_ACCEPT_MAX_TIME, maxreuseval, temp->header->reuse,&request);





                         istart  = Volpex_request_get_counter ( 1 );

                        Volpex_request_clean ( istart, 1 );

                        PRINTF(("[%d] Vprogress:Setting Irecv to target->%d cktag: %d comm: %d for reqnumber %d  \n",
                SL_this_procid, tcurr->rheader.target,  CK_TAG, temp->header->comm, istart));

                reqlist[istart].target = tcurr->rheader.target;
                reqlist[istart].cktag = CK_TAG; /*for regular buffer check*/
                reqlist[istart].req_type = 0;  /*0 = send*/
                reqlist[istart].in_use = 1;
                reqlist[istart].flag = 0;
                reqlist[istart].send_status = 0;
                reqlist[istart].reqnumber = istart;
                reqlist[istart].header = Volpex_get_msg_header(0, hdata[temp->header->comm].myrank,
                                                tcurr->rheader.target, CK_TAG, temp->header->comm, 0);

                        ret = SL_recv_post(&reqlist[istart].returnheader, sizeof(Volpex_msg_header), tcurr->rheader.target,
                           CK_TAG, temp->header->comm,SL_ACCEPT_INFINITE_TIME, &reqlist[istart].request);



                    PRINTF(("[%d]:Value removed src=%d dest=%d comm=%d reuse=%d tag=%d len=%d target=%d id=%d\n",
                        SL_this_procid, tcurr->rheader.header.src,tcurr->rheader.header.dest,
                        tcurr->rheader.header.comm,tcurr->rheader.header.reuse,tcurr->rheader.header.tag,
                        tcurr->rheader.header.len,tcurr->rheader.target,tcurr->rheader.id));

                    tcurr = Volpex_remove_returnheader(&returnheaderList, tcurr->rheader.id);

                    if ( ret != SL_SUCCESS ) {
                        PRINTF(("[%d]  VProgress: send req. %d: isending data to %d failed, ret = %d ret1 = %d\n",
                                SL_this_procid, i, tcurr->rheader.target, ret, ret1 ));
                        continue;
                    }
                }
            }
            else {
                not_found++;
                tcurr = tcurr->next;
            }


        }
    }



    for(i = 0; i < REQLISTSIZE; i++){

        /* handles progress on sends and isends from buffer */
        if(reqlist[i].in_use == 1 && reqlist[i].req_type == 0 ){
            flag = 0;
            ret = SL_ERR_PROC_UNREACHABLE;
            ret1 = SL_test_nopg(&reqlist[i].request, &flag, &mystatus, &loglength);
	    maxreuseval = 0;
            if(flag == 1 && reqlist[i].send_status == 0 ){
                reqlist[i].returnheader.timestamp = SL_papi_time();
                MAX_REUSE = Volpex_search_maxreuse(maxtagreuse,reqlist[i].returnheader);

                if ( (ret1 == SL_SUCCESS) && (MAX_REUSE >= reqlist[i].returnheader.reuse)&&(reqlist[i].returnheader.reuse>-1) ) {

                    /*Check return header list */
                    temp = Volpex_send_buffer_search(insertpt, &reqlist[i].returnheader, &answer, &maxreuseval, &minreuseval);


                if ( answer ){
                        answer = 0;
                        PRINTF(("[%d]  VProgress: send req %d: Into SL_Send with len=%d, target=%d, tag=%d, reuse=%d, maxreuse=%d\n",
                                SL_this_procid,  i,
                                temp->header->len, reqlist[i].target, temp->header->tag, temp->header->reuse,maxreuseval));
                        if (reqlist[i].returnheader.timestamp-temp->header->timestamp !=0.0)
                        PRINTF(("[%d]timestamp difference message =%f target:%d request:%d\n",SL_this_procid,
                                reqlist[i].returnheader.timestamp-temp->header->timestamp, reqlist[i].target,i));
                        ret = SL_send_post(temp->buffer, temp->header->len,
                                           reqlist[i].target, temp->header->tag,
                                           temp->header->comm,SL_ACCEPT_MAX_TIME, maxreuseval, temp->header->reuse,&reqlist[i].request);

                        reqlist[i].send_status = 1;




/*mark as request is posted */

                        istart  = Volpex_request_get_counter ( 1 );
                        Volpex_request_clean ( istart, 1 );

                        PRINTF(("[%d] Vprogress1:Setting Irecv to target->%d cktag: %d comm: %d for reqnumber %d  \n",
                SL_this_procid, reqlist[i].target,  CK_TAG, temp->header->comm, istart));

                reqlist[istart].target = reqlist[i].target;
                reqlist[istart].cktag = CK_TAG; /*for regular buffer check*/
                reqlist[istart].req_type = 0;  /*0 = send*/
                reqlist[istart].in_use = 1;
                reqlist[istart].flag = 0;
                reqlist[istart].send_status = 0;
                reqlist[istart].reqnumber = istart;
                reqlist[istart].header = Volpex_get_msg_header(0, hdata[temp->header->comm].myrank,
                                                reqlist[i].target, CK_TAG, temp->header->comm, 0);

                        ret = SL_recv_post(&reqlist[istart].returnheader, sizeof(Volpex_msg_header), reqlist[i].target,
                           CK_TAG, temp->header->comm,SL_ACCEPT_INFINITE_TIME, &reqlist[istart].request);


                        if ( ret != SL_SUCCESS ) {
                            printf("[%d]  VProgress: send req. %d: isending data to %d failed, ret = %d ret1 = %d\n",
                                    SL_this_procid, i, reqlist[i].target, ret, ret1 );
                            /* TBD: free the request */
                            continue;
                        }
                        flag=0;
                    }
                }
                else  {

                    PRINTF(("[%d]  VProgress: send req. %d: Inserting  "
                            "len=%d dest=%d tag=%d reuse=%d target=%d MAX_REUSE=%d\n", SL_this_procid, i,
                            reqlist[i].returnheader.len, reqlist[i].returnheader.dest,
                            reqlist[i].returnheader.tag, reqlist[i].returnheader.reuse,reqlist[i].target, MAX_REUSE));
                    not_found++;

                        Volpex_insert_returnheader(&returnheaderList, reqlist[i].returnheader, reqlist[i].target);
                    reqlist[i].send_status = 1;
                }
                if  (ret1 != SL_SUCCESS){
                    PRINTF(("[%d]  VProgress: send req. %d: target %d died, ret = %d. Freeing request.\n",
                            SL_this_procid, i, reqlist[i].target, ret));
                    /* NOTE for RAKHI: potentially dangereous if we want to add processes. since this
                       might remove an entry from the send buffer */
                    Volpex_request_clean ( i, 1);
                }
            }


            if ( flag == 1 && reqlist[i].send_status == 1 ) {
                /* we can free the request here, since Wait operations
                   do not check on the real request anyway. Actually,
                   we don't even care whether ret was SL_SUCCESS or not.
                */
                PRINTF(("[%d]:!!!Req %d target:%d is DONE\n", SL_this_procid, i,reqlist[i].target));
                reqlist[i].in_use    = -1;
                reqlist[i].req_type  = -1;
                reqlist[i].target    = -1;
                reqlist[i].flag      = 0;
              free ( reqlist[i].header );
                reqlist[i].header    = NULL;
                reqlist[i].recv_status = -1;
                reqlist[i].send_status = -1;
                reqlist[i].returnheader.len   = -1;
                reqlist[i].returnheader.src   = -1;
                reqlist[i].returnheader.dest  = -1;
                reqlist[i].returnheader.tag   = -1;
                reqlist[i].returnheader.comm  = -1;
                reqlist[i].returnheader.reuse = -1;
                if (reqlist[i].insrtbuf !=NULL)
                Volpex_buffer_remove_ref ( reqlist[i].insrtbuf, reqlist[i].reqnumber );
                reqlist[i].reqnumber = -1;
                reqlist[i].insrtbuf  = NULL;
            }


        }

        /* handles progress on irecvs from buffer */
        if(reqlist[i].in_use == 1 && reqlist[i].req_type == 1){
            flag = 0;
            ret = SL_ERR_PROC_UNREACHABLE;
            ret = SL_test_nopg(&reqlist[i].request, &flag, &mystatus, &loglength);

	if ((reqlist[i].flag ==1  || reqlist[i].flag==2)&& reqlist[i].recv_dup_status == 1)
            {
                PRINTF(("[%d]: Freeing req:%d target:%d\n", SL_this_procid,i, reqlist[i].target));
                Volpex_free_request(i);
            }


/* if long enough switch target */
            if(flag == 1 && reqlist[i].recv_status == 0 ) {
                if ( ret == SL_SUCCESS){

                    ret = SL_Irecv(reqlist[i].buffer, reqlist[i].header->len,
                                   reqlist[i].target, reqlist[i].header->tag,
                                   reqlist[i].header->comm, &reqlist[i].request);
                    reqlist[i].recv_status = 1; /* Ready to receive the real data */

                    PRINTF(("[%d]  VProgress: recv request:%d posted Irecv to target=%d len=%d tag=%d reuse=%d\n",
                            SL_this_procid, i, reqlist[i].target, reqlist[i].header->len, reqlist[i].header->tag,
                            reqlist[i].header->reuse ));
                    flag = 0;
                }

                if ( ret != SL_SUCCESS ) {
                    MPI_Request tmprequest = i;
                    PRINTF(("[%d] 1. Error: , setting:   %d  VOLPEX_PROC_STATE_NOT_CONNECTED\n",
                                SL_this_procid,reqlist[i].target));

                    Volpex_set_state_not_connected(reqlist[i].target);

                    PRINTF(("[%d]  VProgress: recv request:%d reposting Irecv to %d, since prev. op. failed \n",
                              SL_this_procid, i, reqlist[i].header->src ));


                    ret = Volpex_Irecv_ll ( reqlist[i].buffer, reqlist[i].header->len,
                                            reqlist[i].header->src, reqlist[i].header->tag,
                                            reqlist[i].header->comm, &tmprequest, i );
                    if ( ret == MPI_ERR_OTHER ) {
                        /* mark the request as done but set the error code. This
                           operation can not finish, because there are not targets
                           left which are alive. */

                        PRINTF(("[%d]  VProgress: recv request:%d  all targets for proc. %d dead. ret=%d\n",
                                SL_this_procid, i, reqlist[i].header->src, ret ));
                    }
                }
            }
/*	    if ((reqlist[i].flag ==1  || reqlist[i].flag==2)&& reqlist[i].recv_dup_status == 1)
            {
		PRINTF(("[%d]: Freeing req:%d target:%d\n", SL_this_procid,i, reqlist[i].target));
		Volpex_free_request(i);
	    }
*/
            if(flag == 1 && reqlist[i].recv_status == 1 && reqlist[i].flag ==0) {
                if ( ret == SL_SUCCESS){
               	reqlist[i].flag = 1;
               	reqlist[i].time = SL_papi_time() - reqlist[i].time;
		reqlist[i].returnheader.src     = reqlist[i].header->src;
                reqlist[i].returnheader.tag     = mystatus.SL_TAG;
                reqlist[i].returnheader.comm    = mystatus.SL_CONTEXT;
                reqlist[i].returnheader.len     = mystatus.SL_LEN;

                PRINTF(("[%d]: Recieved message from target:%d reqlist:%d time:%f\n", 
					SL_this_procid, reqlist[i].target,i, reqlist[i].time)); 	




			if(reqlist[i].numtargets>0)
			Volpex_target_info_insert(reqlist[i].time, loglength, reqlist[i].header->reuse, reqlist[i].target);



			if(reqlist[i].header!= NULL){
                        if (reqlist[i].header->len> MTU){
                            Volpex_msg_performance_insert(reqlist[i].time, reqlist[i].header->len,reqlist[i].target);
                        }
			

                        int reuseval = 0;
                        if (loglength != -1)
                                reuseval = loglength - reqlist[i].header->reuse;
                    Volpex_insert_reuseval(reqlist[i].target, reuseval);


		int procid;
		int num;
		int newtarget;
		Volpex_proc *proc;
			procid = Volpex_get_volpexid(reqlist[i].target);
                      if (reuseval>5 && Volpex_targets[procid].numofmsg>MAX_MSG && reqlist[i].recv_dup_status != 1 ){
                                printf("[%d]:target : %d  tag:%d  loglength: %d  myloglength: %d, numofmsg:%d\n",
					SL_this_procid,reqlist[i].target, reqlist[i].header->tag,loglength, 
					reqlist[i].header->reuse, Volpex_targets[procid].numofmsg);
				PRINTF(("[%d]: src:%d Numofmsg:%d, Target:%d\n", SL_this_procid,procid, 
					Volpex_targets[procid].numofmsg,Volpex_targets[procid].target));
				num = Volpex_numoftargets(reqlist[i].header->src, reqlist[i].header->comm, reqlist[i].target);
				if (num>0){	
                                	newtarget = Volpex_change_target(reqlist[i].header->src, reqlist[i].header->comm);
					printf("[%d]: Newtarget=%d\n", SL_this_procid,newtarget);
					proc = Volpex_get_proc_byid(newtarget);
					proc->recvpost = 1;
					Volpex_targets[procid].target = newtarget;
					
				}
                        }



                }

}
                else {
                    MPI_Request tmprequest = i;
                    PRINTF(("[%d] 2. Error: , setting:    %d VOLPEX_PROC_STATE_NOT_CONNECTED\n",
                        SL_this_procid,reqlist[i].target));

                    Volpex_set_state_not_connected(reqlist[i].target);

                        PRINTF(("[%d]  VProgress: recv request:%d reposting Irecv to %d, since prev. op. failed \n",
                          SL_this_procid, i, reqlist[i].header->dest ));

                    ret = Volpex_Irecv_ll ( reqlist[i].buffer, reqlist[i].header->len,
                                            reqlist[i].header->src, reqlist[i].header->tag,
                                            reqlist[i].header->comm, &tmprequest, i );
                    if ( ret == MPI_ERR_OTHER ) {
                        /* mark the request as done but the error code. This
                           operation can not finish, because there are not targets
                           left which are alive. */

                        PRINTF(("[%d]  VProgress: recv request:%d  all targets for proc. %d dead. ret=%d\n",
                                SL_this_procid, i, reqlist[i].header->src, ret ));
                    }
                }
            }
        }
    }

    return 0;
}




int Volpex_free_request(int i)
{
	PRINTF(("[%d]:Freeing request %d\n", SL_this_procid, i));
            reqlist[i].in_use           = 0;
            reqlist[i].req_type         = -1;
            reqlist[i].target           = -1;
            reqlist[i].flag             = 0;
            reqlist[i].recv_status      = -1;
            reqlist[i].send_status      = -1;
            reqlist[i].reqnumber        = -1;
            reqlist[i].request          = NULL;
            reqlist[i].numtargets       = 0;
            reqlist[i].recv_dup_status  = 0;
	    reqlist[i].time             = 0.0;
            reqlist[i].len              = 0;


            if (reqlist[i].header      != NULL){
                free(reqlist[i].header);
                reqlist[i].header       = NULL;
            }
            if (reqlist[i].buffer      != NULL){
                free(reqlist[i].buffer);
                reqlist[i].buffer       = NULL;
            }

            if (reqlist[i].assoc_recv  != NULL){
                free(reqlist[i].assoc_recv);
                reqlist[i].assoc_recv   = NULL;
            }

	return 0;
}










int Volpex_free_recvrequest(int i)
{
	reqlist[i].in_use    = 0;
            reqlist[i].req_type  = -1;
            reqlist[i].target    = -1;
            reqlist[i].flag      = 0;
            reqlist[i].recv_status = -1;
            reqlist[i].send_status = -1;
            reqlist[i].reqnumber = -1;
            reqlist[i].request = NULL;
            reqlist[i].numtargets = 0;
	    reqlist[i].time = 0.0;
	    reqlist[i].len = 0;
	    reqlist[i].recv_dup_status = 0;
//	    free(reqlist[i].buffer);
//		reqlist[i].buffer = NULL;
           free(reqlist[i].assoc_recv);
		reqlist[i].assoc_recv = NULL;
	    free(reqlist[i].header);
		reqlist[i].header = NULL;


	return 1;
}

