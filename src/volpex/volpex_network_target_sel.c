#include "mpi.h"
#include "SL_proc.h"


SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_comm_array;
extern int SL_this_procid;
extern char *hostip;
extern char *hostname;
extern int Volpex_numprocs;
extern int redundancy;
extern int Volpex_numcomms;

Volpex_msg_perf* Volpex_msg_performance_init()
{
        Volpex_msg_perf *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= MSGPERF; i++){
                node= (Volpex_msg_perf *)malloc(sizeof(Volpex_msg_perf));
                node->pos    = i;
                node->msglen = -1;
                node->time   = -1;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
                }
                if(i > 0 && i < MSGPERF){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == MSGPERF){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}

Volpex_net_perf* Volpex_net_performance_init()
{
        Volpex_net_perf *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= MSGPERF; i++){
                node= (Volpex_net_perf *)malloc(sizeof(Volpex_net_perf));
                node->pos    = i;
                node->latency = -1.0;
                node->bandwidth   = -1.0;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
                }
                if(i > 0 && i < MSGPERF){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == MSGPERF){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}

void Volpex_msg_performance_insert(double time, int msglen, int src)
{

	Volpex_proc *proc;
	Volpex_msg_perf* tmppt ;
        static double latency = -1.0; 
	double bandwidth = -1.0, band_time = 0.0;
        double total_time = 0.0, lat_time = 0.0;
        int total_len = 0;
        int i;
        int lcount = 0, bcount = 0;
	
	proc = Volpex_get_proc_byid(src);
	tmppt = proc->msgperf;
	
	if (proc->msgperf->pos == MSGPERF){
		proc->msgperf = proc->msgperf->fwd;
                for(i=0; i<MSGPERF ; i++){
                        tmppt = tmppt->fwd;
                        total_time = total_time + tmppt->time;
                        if (tmppt->msglen < MTU){
				lat_time = lat_time + tmppt->time;
                                lcount++;
           	          }
	    
                        else{
				band_time = band_time + tmppt->time;
                                total_len  = total_len  + tmppt->msglen;
                                bcount++;
                        }
	   
                }
	

		if (lcount != 0)
                	latency = (lat_time/1000000.0/(double)lcount);
		else
			latency = -1;

        	if (bcount != 0){
					
			bandwidth = ((double)total_len/(double)(1024L*1024L))/((band_time/1000000.0));
//			printf("[%d]:bcount=%d total_len=%d band_time=%f procid=%d\n", 
//			SL_this_procid, bcount, total_len, band_time, proc->id);
		}
		else
			bandwidth = -1;
	
        	Volpex_net_performance_insert( latency, bandwidth,proc);
	}
	else{
//		if(msglen > MTU){
	        proc->msgperf->msglen = msglen ;
        	proc->msgperf->time = time;
	        proc->msgperf = proc->msgperf->fwd;
		
//		printf("[%d]:Time :%f msglen: %d Target:%d procID: %d proc->msgperf->pos %d\n",SL_this_procid,
//					time,msglen, src, proc->id, proc->msgperf->pos);
//		}
	}
}



void Volpex_net_performance_insert(double latency, double bandwidth, Volpex_proc *proc)
{

	proc->netperf->latency = latency;
	proc->netperf->bandwidth = bandwidth;	
//	printf("#########[%d]:Bandwidth :%f Latency: %f procID: %d \n",SL_this_procid,bandwidth,latency,proc->id);
}





int Volpex_dest_src_locator1(int rank, int comm, char *myfullrank, int **target,
                            int *numoftargets, int msglen, int msgtype)
{

    int i, j=0 ;
    int pos = 0 ;
    int *tar=NULL;
    int numtargets=0;
    Volpex_comm *communicator=NULL;
    Volpex_proc *proc=NULL, *dproc = NULL, *tproc=NULL;

    int newtarget;
    Volpex_proclist *plist=NULL, *tplist=NULL;
    int position;

    communicator = Volpex_get_comm_byid ( comm );
    tplist        = &communicator->plist[rank];

    tproc = Volpex_get_proc_byid (tplist->ids[0]);
    plist = tproc->plist;


//    printf("[%d]:Entering Volpex_dest_src_locator1 for msgtype: %d\n", SL_this_procid, msgtype);

        for(i=0; i< plist->num; i++){
                proc = Volpex_get_proc_byid (plist->ids[i]);
                if (proc->state == VOLPEX_PROC_CONNECTED){
                    numtargets ++;
                }

        }



    tar = (int *) malloc ( numtargets * sizeof(int));
    if ( NULL == tar ) {
        return SL_ERR_NO_MEMORY;
    }


    if (msgtype == RECV){

        for(i=0; i< plist->num; i++){
                proc = Volpex_get_proc_byid (plist->ids[i]);
                if(proc->numofmsg > MSGPERF && proc->msgnum < 0 - MSGPERF){
                        pos++;
			PRINTF(("[%d] proc->id %d POS %d\n",SL_this_procid,proc->id, pos));
                }
        }

        if (pos == numtargets){
//        position = proc->netperf->back->pos;
            position = proc->netperf->pos;

            newtarget = Volpex_compare_perf(plist,position);
            printf("##########[%d]:New Target : %d\n", SL_this_procid, newtarget);

            for (i=0; i<plist->num; i++){
                proc = Volpex_get_proc_byid (plist->ids[i]);
                proc->numofmsg = 0;
                if (proc->id == newtarget)
                        proc->msgnum = TOTALMSGCOMM; //setting threshold value for number of messages after which algorithm is repeated
                                                        // value = ___ + MSGPERF
                else
                        proc->msgnum = 0;
                }
		Volpex_set_newtarget(newtarget,rank,comm);
        }

        dproc = Volpex_get_proc_byid (plist->ids[0]);
//        printf("[%d]:dproc->id :%d, dproc->numofmsg: %d, dproc->msgnum: %d\n",
//                        SL_this_procid,dproc->id,dproc->numofmsg, dproc->msgnum);
// Swapping to next target
        if (dproc->numofmsg > MSGPERF && dproc->msgnum < 0 - MSGPERF  && hdata[comm].myrank != rank ){
		PRINTF(("Changing targets\n"));
		Volpex_change_target(rank,comm);
    }
}
 int numofmsg;
    for(i=0; i< plist->num; i++){
        proc = Volpex_get_proc_byid (plist->ids[i]);
        if (i==0 && msgtype == RECV && msglen>MTU ){
            numofmsg = ++proc->numofmsg;
            proc->msgnum--;
//          printf("[%d]:For proc: %d msgtype: %d msglen %d  proc->numofmsg:%d proc->msgnum:%d\n",SL_this_procid,
//                              proc->id, msgtype, msglen, proc->numofmsg,proc->msgnum);
        }

        if(VOLPEX_PROC_CONNECTED == proc->state){
            tar[j] = proc->id;
            j++;
        }
    }
/*
if (msgtype != 0 && msglen>MTU){
   printf("[%d]:Numofmsg = %d  msgtype %d ",SL_this_procid,numofmsg,msgtype);
    printf("NUM: %d Targets: ", numtargets);
    for (i=0;i<numtargets;i++)
        printf("%d, " ,tar[i]);
    printf("\n");
     }
*/
//      printf("lastlevel  : %c\n", dproc->lastlevel);
    *target = tar;
    *numoftargets = numtargets;

    return MPI_SUCCESS;
}







int Volpex_compare_perf(Volpex_proclist *plist,int pos)
{
        Volpex_proc *proc = NULL, *retlatproc = NULL, *retbandproc = NULL, *retproc = NULL;
        Volpex_net_perf *curr ;
        int i;
        double min_lat = 9999, max_band = -1;
//	MCFA_printf("\nComparing for procs: ");
        for(i=0; i< plist->num; i++){
	    proc = Volpex_get_proc_byid (plist->ids[i]);
	//    printf("[%d]: proc->id %d, proc->msgnum %d, proc->numofmsg %d	\n", SL_this_procid,proc->id,proc->msgnum, proc->numofmsg);
	    retproc = proc;
	    if (proc->state == 1 ){
		curr = proc->netperf;
		while (curr->pos != pos){
		    curr = curr->fwd;
		}
	//	printf("[%d]:proc->id: %d, latency: %f, pos: %d\n", SL_this_procid,proc->id,curr->latency,curr->pos);
		if (min_lat > curr->latency){
		    min_lat = curr->latency;
		    retlatproc = proc;
		}
		   PRINTF(("[%d]:Proc->id: %d bandwidth:%f pos:%d\n ",SL_this_procid,proc->id, curr->bandwidth, curr->pos));
		if (max_band < curr->bandwidth){
		    max_band = curr->bandwidth;
		    retbandproc = proc;
		}
	    }
        }
		if (retbandproc != NULL){
			retproc = retbandproc;
		//	if (SL_this_procid == 0)
//				printf(" Bandwidth %f    \n ", max_band);
		}
		else if (retlatproc != NULL){
			retproc = retlatproc;
		//		if (SL_this_procid == 0)
//                                printf(" Latency   %f \n ", min_lat);
		}

//	if (SL_this_procid == 0)
	//	printf("[%d]: New Target: %d\n", SL_this_procid,retproc->id);
		        


	return retproc->id;
}


