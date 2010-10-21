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





void Volpex_reqlist_search_bytag ( int tag )
{
    int i;

    for ( i=0; i<REQLISTSIZE; i++ ) {
        if ( reqlist[i].in_use == 1 && reqlist[i].header != NULL ) {
            if ( reqlist[i].header->tag == tag ) {
                printf("[%d] reqlist[%d] target=%d len=%d src=%d dest=%d tag=%d comm=%d reuse=%d send_status=%d recv_status=%d\n",
                       SL_this_procid, i, reqlist[i].target, reqlist[i].header->len, reqlist[i].header->src,
                       reqlist[i].header->dest, reqlist[i].header->tag, reqlist[i].header->comm, reqlist[i].header->reuse,
                       reqlist[i].send_status, reqlist[i].recv_status );
            }
        }
    }


    return;
}

void Volpex_reqlist_search_byreqid ( int id )
{
    int i;

    for ( i=0; i<REQLISTSIZE; i++ ) {
        if ( reqlist[i].request->id == id ) {
            printf("[%d] reqlist[%d] target=%d len=%d src=%d dest=%d tag=%d comm=%d reuse=%d send_status=%d recv_status=%d\n",
                   SL_this_procid, i, reqlist[i].target, reqlist[i].header->len, reqlist[i].header->src,
                   reqlist[i].header->dest, reqlist[i].header->tag, reqlist[i].header->comm, reqlist[i].header->reuse,
                   reqlist[i].send_status, reqlist[i].recv_status );
        }
    }

    return;
}


void Volpex_reqlist_search_byreturntag ( int tag )
{
    int i;

    for ( i=0; i<REQLISTSIZE; i++ ) {
        if ( reqlist[i].header != NULL ) {
            if ( reqlist[i].returnheader.tag == tag ) {
                printf("[%d] reqlist[%d] target=%d len=%d src=%d dest=%d tag=%d comm=%d reuse=%d send_status=%d recv_status=%d\n",
                       SL_this_procid, i, reqlist[i].target, reqlist[i].header->len, reqlist[i].header->src,
                       reqlist[i].header->dest, reqlist[i].header->tag, reqlist[i].header->comm, reqlist[i].header->reuse,
                       reqlist[i].send_status, reqlist[i].recv_status );
            }
        }
    }


    return;
}

void Volpex_insertbuffersearch_tag(int tag)
{
        NODEPTR curr, currpt;
        currpt = head;
        curr = head;
        do {
            if ( NULL != curr->header ) {
                    if(curr->header->tag == tag)
                    printf("[%d]:count:%d len:%d  src:%d  dest:%d tag:%d  comm:%d reuse:%d\n",
                                SL_this_procid,curr->counter,curr->header->len, curr->header->src, curr->header->dest,
                                curr->header->tag, curr->header->comm, curr->header->reuse);

                }

            curr = curr->fwd; /*search backwards since loaded forward*/
        } while ( curr != currpt );
        return;

}

void Volpex_reqlist_search_byprocid(int id)
{

    int i;

    for ( i=0; i<REQLISTSIZE; i++ ) {
        if(reqlist[i].request != NULL)
        if ( reqlist[i].request->proc->id == id ) {
            printf("[%d] reqlist[%d] target=%d len=%d src=%d dest=%d tag=%d comm=%d reuse=%d send_status=%d recv_status=%d\n",
                   SL_this_procid, i, reqlist[i].target, reqlist[i].header->len, reqlist[i].header->src,
                   reqlist[i].header->dest, reqlist[i].header->tag, reqlist[i].header->comm, reqlist[i].header->reuse,
                   reqlist[i].send_status, reqlist[i].recv_status );
        }
    }

    return;

}

