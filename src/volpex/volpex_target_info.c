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
extern Volpex_target_list *Volpex_targets;



int Volpex_init_targetlist()
{
        int i;
	Volpex_proc *proc;
        Volpex_targets = (Volpex_target_list*)malloc(Volpex_numprocs/redundancy * sizeof(Volpex_target_list));
        for(i=0;i<Volpex_numprocs/redundancy;i++){
/*                if (i == (SL_this_procid % (Volpex_numprocs/redundancy)))
                {
                        Volpex_targets[i].numofmsg = MAX_MSG+1;
                        Volpex_targets[i].volpex_id = i;
                        Volpex_targets[i].target = SL_this_procid;

                }
                else{
*/
                Volpex_targets[i].numofmsg = 0;
                Volpex_targets[i].volpex_id = i;
		proc = Volpex_get_proc_byid(i);
		Volpex_targets[i].target = proc->plist->ids[0];
//                Volpex_targets[i].target = -1;
//                }

        }
        return MPI_SUCCESS;

}

int Volpex_print_targetlist()
{
        int i;
        for(i=0;i<Volpex_numprocs/redundancy;i++){
                printf("[%d]: Numofmsgs:%d  Source:%d  Target:%d\n",
                        SL_this_procid, Volpex_targets[i].numofmsg, Volpex_targets[i].volpex_id, Volpex_targets[i].target);
        }
        return MPI_SUCCESS;
}
void Volpex_free_targetlist()
{
        free(Volpex_targets);
}









Volpex_target_info* Volpex_target_info_init()
{
	
        Volpex_target_info *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= MAX_MSG; i++){
                node= (Volpex_target_info *)malloc(sizeof(Volpex_target_info));
                node->pos    = i;
                node->reuse  = -1;
		node->myreuse = -1;
                node->time   = -1.0;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
                }
                if(i > 0 && i < MAX_MSG){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == MAX_MSG){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}


int Volpex_target_info_insert(double time, int reuse, int myreuse, int src)
{
	Volpex_proc *proc;
	Volpex_target_info *tmppt;


	proc = Volpex_get_proc_byid(src);
        tmppt = proc->target_info;

	proc->target_info->time = time;
	proc->target_info->reuse = reuse;
	proc->target_info->myreuse = myreuse;


	PRINTF(("[%d]: %d. Target:%d reuse:%d myreuse:%d time:%f\n", SL_this_procid,
                        proc->target_info->pos, proc->id,proc->target_info->reuse,proc->target_info->myreuse, proc->target_info->time));

	proc->target_info = proc->target_info->fwd;
	
	return MPI_SUCCESS;

}

int Volpex_compare_target_info(int target)
{
	return MPI_SUCCESS;
}



int Volpex_target_info_free(Volpex_proc *tproc)
{
        Volpex_target_info *tnode = NULL;

        while (tproc->target_info!=NULL){
                tnode = tproc->target_info;
                if(tnode->pos != MAX_MSG){
                        tproc->target_info = tproc->target_info->fwd;
                        free(tnode);
                }
                else
                break;
        }
        return SL_SUCCESS;

}


void Volpex_print_target_info(int source)
{
	Volpex_proc *proc;
	int i;

	proc = Volpex_get_proc_byid(source);

	printf("\n[%d]: --------Process:%d-----------------------\n",SL_this_procid, proc->id);
	for(i=0;i<MAX_MSG; i++){
		printf("[%d]: %d. reuse:%d myresue:%d time:%f\n", SL_this_procid, 
			proc->target_info->pos, proc->target_info->reuse, proc->target_info->myreuse, proc->target_info->time);
		proc->target_info = proc->target_info->fwd;

	}
	
}

void Volpex_print_alltarget_info()
{
	int i;
	for(i=0;i<Volpex_numprocs; i++){
                if (i != (SL_this_procid % (Volpex_numprocs/redundancy)))
			Volpex_print_target_info(i);
	}
}
