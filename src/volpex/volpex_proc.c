#include "mpi.h"
#include "SL_proc.h"
#include "SL_msg.h"

SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_comm_array;
SL_array_t *Volpex_rank_array;
extern int SL_this_procid;
extern char *hostip;
extern char *hostname;
extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
int Volpex_this_procid;

int Volpex_init_proc(int id, int SL_id, char *hostname, int port, char *fullrank)
{
	Volpex_proc *tproc = NULL;
	int pos;
	int numrank;
	char level;
	
	tproc = (Volpex_proc *) malloc (sizeof(Volpex_proc));
	if ( NULL == tproc ) {
        return SL_ERR_NO_MEMORY;
	}

	tproc->id		= id;
	tproc->SL_id		= SL_id;
	tproc->hostname         = strdup ( hostname );
	tproc->port		= port;
        
	sscanf(fullrank,"%d,%c",&numrank, &level);
	tproc->state		= VOLPEX_PROC_CONNECTED;
	tproc->rank_MCW         = numrank;
	strcpy(tproc->rank , fullrank);
	tproc->level	        = level;         
     
	tproc->numofmsg		= -1;
	tproc->msgnum		= 0;
	tproc->recvpost		= 0;
	tproc->reuseval		= 0;
	tproc->msgperf		= Volpex_msg_performance_init();
	tproc->netperf		= Volpex_net_performance_init();
        
        tproc->target_info      = Volpex_target_info_init();

	tproc->plist    = (Volpex_proclist*) malloc(sizeof(Volpex_proclist));
    	if(NULL == tproc->plist){
        	return SL_ERR_NO_MEMORY;
    	}
	tproc->plist->num	= 0;

	Volpex_init_purgelist(&tproc->purgelist);
	SL_array_get_next_free_pos ( Volpex_proc_array, &pos );
        SL_array_set_element ( Volpex_proc_array, pos, id, tproc );

	return SL_SUCCESS;

}


int Volpex_free_proclist()
{
	int size,i;
	Volpex_proc *tproc = NULL;

	size = SL_array_get_last(Volpex_proc_array ) + 1;
	for(i=0; i<size; i++){
		tproc = (Volpex_proc*) SL_array_get_ptr_by_pos ( Volpex_proc_array, i );
		Volpex_msg_performance_free(tproc);
		Volpex_net_performance_free(tproc);
		Volpex_target_info_free(tproc);
		free(tproc->hostname);
		free(tproc->plist->ids);
		free(tproc->plist->SL_ids);
		free(tproc->plist->recvpost);
		free(tproc->plist);
		free(tproc);
		
        }
	return SL_SUCCESS;

}
int Volpex_get_max_rank()
{
        int size,i;
        Volpex_proc *tproc = NULL;
        int maxrank = 0 ;

        size = SL_array_get_last(Volpex_proc_array ) + 1;
        for(i=0; i<size; i++){
                tproc = (Volpex_proc*) SL_array_get_ptr_by_pos ( Volpex_proc_array, i );
                if (tproc->rank_MCW > maxrank)
                        maxrank = tproc->rank_MCW;
        }
        return maxrank;
}

int Volpex_get_volpexid(int SL_id)
{
	Volpex_proc *tproc = NULL;

	tproc = Volpex_get_proc_bySLid(SL_id);

	return (tproc->rank_MCW);
}


int Volpex_msg_performance_free(Volpex_proc *tproc)
{
	Volpex_msg_perf *tnode = NULL;
	
	while (tproc->msgperf!=NULL){
		tnode = tproc->msgperf;
		if(tnode->pos != MSGPERF){
			tproc->msgperf = tproc->msgperf->fwd;
			free(tnode);
		}
		else
		break;
	}
	return SL_SUCCESS;

}

int Volpex_net_performance_free(Volpex_proc *tproc)
{
	Volpex_net_perf *tnode = NULL;

        while (tproc->netperf!=NULL){
                tnode = tproc->netperf;
                if(tnode->pos != MSGPERF){
                        tproc->netperf = tproc->netperf->fwd;
                        free(tnode);
                }
                else
                break;
        }
	return SL_SUCCESS;

}
/*
int Volpex_init_procplist(int redcy)
{
   Volpex_proc *proc, *tproc;
    int i, j;
    int *parray = NULL;
    int myrank;
    char mylevel;
    int k,pos=0;


        sscanf(fullrank, "%d,%c",&myrank,&mylevel);

        for(i=0;i<Volpex_numprocs/redcy;i++){
                proc = Volpex_get_proc_byid(i);
                proc->plist->num = redcy;
                proc->plist->ids = (int *) malloc (redcy * sizeof(int));
                if ( NULL == proc->plist->ids ) {
                   return SL_ERR_NO_MEMORY;
                }
		


		parray = (int *) malloc (redcy * sizeof(int));
                for (j=0;j<redcy; j++){

                                parray[j] = j * Volpex_numprocs/redcy + i;
                }
		pos = 0;
		for(k =mylevel - 'A'; k<redcy; k++){
                	proc->plist->ids[pos] = parray[k];
                	pos++;
	        }
        	for(k=0; k<mylevel - 'A'; k++){
                	proc->plist->ids[pos]= parray[k];
               	 	pos++;
        	}
	}	

	pos = 0;
	while(i<Volpex_numprocs){
	
		tproc = Volpex_get_proc_byid(i);
                tproc->plist->num = redcy;
                tproc->plist->ids = (int *) malloc (redcy * sizeof(int));
		i++;
		for(j=0;j<tproc->plist->num;j++)
		{
			proc = Volpex_get_proc_byid(pos);
			tproc->plist->ids[j] = proc->plist->ids[j];
		}
		pos++;
		if (pos == Volpex_numprocs/redcy)
			pos = 0;
	}

	free(parray);
        return MPI_SUCCESS;

}
*/


int Volpex_init_procplist1(int redcy)
{
   Volpex_proc *proc, *tproc;
    int i, j;
    int myrank;
    char mylevel;
    int pos=0;


    for ( i=0; i< Volpex_numprocs; i++ ) {
        proc = Volpex_get_proc_byid(i);
        for(j=0;j<Volpex_numprocs;j++){
                tproc = Volpex_get_proc_byid(j);
                if(proc->rank_MCW == tproc->rank_MCW)
                        proc->plist->num++ ;
        }
    }
sscanf(fullrank, "%d,%c",&myrank,&mylevel);
    for ( i=0; i< Volpex_numprocs; i++ ) {
        proc = Volpex_get_proc_byid(i);
        proc->plist->ids = (int *) malloc (proc->plist->num * sizeof(int));
        proc->plist->SL_ids = (int *) malloc (proc->plist->num * sizeof(int));
        for(j=0;j<Volpex_numprocs;j++){
                tproc = Volpex_get_proc_byid(j);

                if(proc->rank_MCW == tproc->rank_MCW){
                        if((tproc->level-mylevel)<0)
                                pos = tproc->level-mylevel+proc->plist->num;
                        else
                                pos = tproc->level-mylevel;
                        proc->plist->ids[pos]= tproc->id;
                        proc->plist->SL_ids[pos]= tproc->SL_id;
                }
    }
}
 return MPI_SUCCESS;

}


int Volpex_init_procplist()
{
    Volpex_proc *proc, *tproc;
    int i, j;
    int myrank;
    char mylevel;
    int pos=0;
    
    
    for ( i=0; i< Volpex_numprocs; i++ ) {
        proc = Volpex_get_proc_byid(i);
        for(j=0;j<Volpex_numprocs;j++){
	    tproc = Volpex_get_proc_byid(j);
	    if(proc->rank_MCW == tproc->rank_MCW)
		proc->plist->num++ ;
        }
    }
    sscanf(fullrank, "%d,%c",&myrank,&mylevel);
    for ( i=0; i< Volpex_numprocs; i++ ) {
        proc = Volpex_get_proc_byid(i);
        proc->plist->ids = (int *) malloc (proc->plist->num * sizeof(int));
        proc->plist->SL_ids = (int *) malloc (proc->plist->num * sizeof(int));
        proc->plist->recvpost = (int *) malloc (proc->plist->num * sizeof(int));
        for(j=0;j<Volpex_numprocs;j++){
	    tproc = Volpex_get_proc_byid(j);
	    
	    if(proc->rank_MCW == tproc->rank_MCW){
		if((tproc->level-mylevel)<0)
		    pos = tproc->level-mylevel+proc->plist->num;
		else
		    pos = tproc->level-mylevel;
		proc->plist->ids[pos]= tproc->id;
		proc->plist->SL_ids[pos]= tproc->SL_id;
	    }
		proc->plist->recvpost[pos] = 0;
	}
    }
    return MPI_SUCCESS;
    
}







void Volpex_print_procplist()
{
        int i,j;
        Volpex_proc *proc;
        for(i=0;i<Volpex_numprocs;i++){
                proc = Volpex_get_proc_byid(i);
                printf("[%d]:procid:%d-> ",SL_this_procid,proc->id);
                for (j=0; j<proc->plist->num;j++)
                        printf("%d ", proc->plist->ids[j]);
		printf("  SL_id ->");
                for (j=0; j<proc->plist->num;j++)
                        printf("%d ", proc->plist->SL_ids[j]);

                printf("\n");
        }


}

Volpex_proc* Volpex_get_proc_bySLid(int id)
{
    Volpex_proc *dproc = NULL, *proc = NULL;
    int i, size;

    size = SL_array_get_last(Volpex_proc_array ) + 1;
    for(i=0; i<size; i++){
        proc = (Volpex_proc*) SL_array_get_ptr_by_pos (Volpex_proc_array, i);
        if ( NULL == proc ) {
            continue;
        }

        if (id == proc->SL_id){
            dproc = proc;
            break;
        }
    }

    return dproc;
}




int Volpex_get_procid_fullrank(char *fullrank)
{
    Volpex_proc *proc = NULL;
    int i, size;
    
    size = SL_array_get_last(Volpex_proc_array ) + 1;
    for(i=0; i<size; i++){
	proc = (Volpex_proc*) SL_array_get_ptr_by_pos (Volpex_proc_array, i);
	if ( NULL == proc ) {
	    continue;
	}
	
	if(0 == strcmp(hostip, proc->hostname)|| 0 == strcmp(hostname, proc->hostname)){	
	    strcpy(fullrank,proc->rank);
	    return (proc->id);
	}
    }
    
printf("Return -1 !!!!!!!\n");
    return -1;
}

Volpex_proc* Volpex_get_proc_byid(int id)
{
    Volpex_proc *dproc = NULL, *proc = NULL;
    int i, size;
    
    size = SL_array_get_last(Volpex_proc_array ) + 1;
    for(i=0; i<size; i++){
	proc = (Volpex_proc*) SL_array_get_ptr_by_pos (Volpex_proc_array, i);
	if ( NULL == proc ) {
	    continue;
	}
	
	if (id == proc->id){
	    dproc = proc;
	    break;
	}
    }
    
    return dproc;
}

int Volpex_get_rank()
{
	char myfullrank[16];
	int rank;
	int mynumeric;
	char mylevel;

	Volpex_get_fullrank(myfullrank);
	sscanf(myfullrank, "%d,%c", &mynumeric, &mylevel);
	
	rank = mynumeric;
	return rank;	
}



int Volpex_get_fullrank(char *myredrank)
{
    Volpex_proc *dproc=NULL;
    
    dproc = (Volpex_proc *)SL_array_get_ptr_by_id ( Volpex_proc_array, Volpex_this_procid );
    strcpy(myredrank, dproc->rank);
    
    return 0;
}

int Volpex_set_state_not_connected(int target)
{
    int i;
    Volpex_proc *dproc = NULL, *tproc=NULL;
    int tmp,j,tnum;
   Volpex_proclist *plist=NULL;
    dproc = Volpex_get_proc_byid(target);


   if (dproc->state == VOLPEX_PROC_NOT_CONNECTED)
	return 1;


    plist = dproc->plist;
    tnum = plist->num;
    PRINTF(("[%d]: Dproc:%d Num:%d  ",SL_this_procid,dproc->id,plist->num));
    for(i=0;i<tnum;i++){
	PRINTF(("%d  ",plist->ids[i]));
	if (plist->ids[i] != target){
		tproc = (Volpex_get_proc_byid(plist->ids[i]));
		for(j=0;j<tproc->plist->num;j++){
			if (tproc->plist->ids[j] == target){
				tmp = tproc->plist->ids[plist->num-1];	
				tproc->plist->ids[plist->num-1] = target;
				tproc->plist->ids[j] = tmp;	
			}
		}	
		tproc->plist->num--;	
	}		
	
    }

    for(j=0;j<dproc->plist->num;j++){
               if (dproc->plist->ids[j] == target){
                    tmp = dproc->plist->ids[dproc->plist->num-1];
                    dproc->plist->ids[dproc->plist->num-1] = target;
                    dproc->plist->ids[j] = tmp;
		    break;
                    }
                }
	dproc->plist->num--;

    dproc->state = VOLPEX_PROC_NOT_CONNECTED;

	



    return 1;
    
}

int Volpex_proc_dumpall()
{
    int size, i;
	Volpex_proc *proc;
	
	size = SL_array_get_last(Volpex_proc_array ) + 1;
	for(i=0; i<size; i++){
	    proc = Volpex_get_proc_byid (i);
	    PRINTF(("proc %d hostname %s port %d  rank %s\n", proc->id,
		    proc->hostname, proc->port, proc->rank));
	}
	return 1;
}

int Volpex_init_purgelist(Volpex_cancel_request **list)
{
	*list = NULL;
	return 1;
	
}

int Volpex_insert_purgelist(int procid, Request_List req, int id)
{
	Volpex_cancel_request *next;
	Volpex_cancel_request *prev;
	Volpex_cancel_request *curr;

	Volpex_proc *proc;

	SL_msg_header header;

	proc = Volpex_get_proc_byid(procid);
	
	curr = (Volpex_cancel_request *) malloc (sizeof (Volpex_cancel_request));
	if (curr == NULL)
		return SL_ERR_NO_MEMORY;

	header.cmd = SL_MSG_CMD_P2P;
	header.from = req.target;
	header.to   = SL_this_procid;
	header.context = req.header->comm;
	header.len 	   = req.header->len;
	header.loglength  = -1;
        header.temp	   = req.header->reuse;
	header.tag	   = req.header->tag;
        header.id	   = id;	

	curr->id 	= id;  
	curr->cancel_request = header;
	curr->next = NULL;

	MCFA_printf(" Inserted %d %d %d %d %d %d %d\n",SL_MSG_CMD_P2P,req.target,SL_this_procid,req.header->comm,req.header->len,req.header->reuse,req.header->tag,id);
	if (proc->purgelist == NULL)
	{
//		curr->id = 0;
		proc->purgelist = curr;
		
		
	}
	else
	{
		next = proc->purgelist;
		prev = NULL;
		
		while(next != NULL)
		{
		         prev=next;
         		next=next->next;
//			id++;
      		}
      /* Now have a spot to insert */
        if (prev == NULL)
      	{
//		curr->id = id;
        	proc->purgelist=curr;
         	curr->next=next;
      	}
      	else
      	{
//		curr->id = id;
        	 curr->next = prev->next;
         	prev->next=curr;
      	}
      }
	return 0;
}

int Volpex_remove_purgelist(int procid,  int id)
{
	Volpex_cancel_request *temp;
        Volpex_cancel_request *prev;
        Volpex_cancel_request *curr;
	Volpex_proc *proc;

        proc = Volpex_get_proc_byid(procid);


	curr = proc->purgelist;
        while(curr->id != id)
        {
                prev = curr;
                curr = curr->next;
        }

        if(curr != NULL)
        {
                if (curr == proc->purgelist){               //        deleting from head
                        temp = proc->purgelist;
                        proc->purgelist = temp->next;
			free(temp);
                }
                else{
                        prev->next = curr ->next;
			free(curr);
                }
        }
        return 0;

}
