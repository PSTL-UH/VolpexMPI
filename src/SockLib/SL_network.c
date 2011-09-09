#include "SL.h"
#include "SL_proc.h"
#include "SL_array.h"
#include "SL_msgqueue.h"
#include "SL_msg.h"
#include "SL_event_handling.h"

SL_msg_perf* SL_msg_performance_init()
{
        SL_msg_perf *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= PERFBUFSIZE; i++){
                node= (SL_msg_perf *)malloc(sizeof(SL_msg_perf));
                node->pos    = i;
                node->msglen = -1;
                node->time   = -1;
                node->msgtype = -1;
                node->elemid = -1;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
                }
                if(i > 0 && i < PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}



SL_network_perf* SL_network_performance_init()
{
        SL_network_perf *node = NULL, *head = NULL, *curr = NULL;
        int i;

        for (i = 0; i <= PERFBUFSIZE; i++){
                node= (SL_network_perf *)malloc(sizeof(SL_network_perf));
                node->pos        = i;
                node->latency    = -1;
                node->bandwidth  = -1;
                if(i == 0){
                        head = curr = node;
                        node->back = NULL;
                        node->fwd = NULL;
                }
                if(i > 0 && i < PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = NULL;
                        curr = curr->fwd;
                }
                if(i == PERFBUFSIZE){
                        curr->fwd = node;
                        node->back = curr;
                        node->fwd = head;
                        curr = curr->fwd;
                        head->back = curr;
                }
        }

        return head;

}


void SL_msg_performance_insert(int msglen,double time, int msgtype, int elemid, SL_proc *proc)
{
        /** if pointer reaches to the end of buffer
        **      Calculate latency and bandwidth using least square method
                using Hockney's formula

                t(s) =  {l              if s<MTU
                        {l + s/b        else

                where t = execution time
                      l = latency
                      s = msglength
                      b = bandwidth
        **      insert it into th circular buffer for that proc
        **/

        SL_msg_perf* tmppt = proc->msgperf;
        double latency = -1.0, bandwidth = -1.0;
        double total_time = 0.0;
        int total_len = 0;
        int i;
        int lcount = 0, bcount = 0;

        if (proc->msgperf->pos == PERFBUFSIZE){
                proc->msgperf = proc->msgperf->fwd;
                for(i=0; i<PERFBUFSIZE ; i++){
                        tmppt = tmppt->fwd;
                        if (tmppt->msglen < MTU){
                                total_time = total_time + tmppt->time;
                                lcount++;
                        }
                        else{
                                total_len  = total_len  + tmppt->msglen;
                                bcount++;
                        }
                }
        if (lcount != 0)
                latency = (total_time/(double)lcount)/1000000.0;

        if (bcount != 0)
                bandwidth = ((double)total_len/(double)(1024L*1024L))/((total_time/1000000.0)- latency);


        SL_network_performance_insert( latency, bandwidth,proc);
//      SL_print_msg_performance(proc->msgperf);

        }
        else if (msgtype == RECV){
                proc->msgperf->msglen = msglen;
                proc->msgperf->time   = time;
                proc->msgperf->msgtype= msgtype;
                proc->msgperf->elemid = elemid;
                proc->msgperf = proc->msgperf->fwd;


        }

/*              double c0, c1, cov00, cov01, cov11, chisq;
                xmsglen[i] = (double)tmppt->msglen/1024.0;
                ytime[i] = tmppt->time/1000000.0        ;

                gsl_fit_linear (ytime, 1, xmsglen, 1, PERFBUFSIZE,
                        &c0, &c1, &cov00, &cov01, &cov11,
                        &chisq);
                latency = c0;
                bandwidth = 1/c1;
*/
}




void SL_print_msg_performance(SL_msg_perf *insertpt)
{
        SL_msg_perf *temp;
        FILE *fp;

        int i;

        fp = fopen("tmp","wa");
        temp = insertpt;
        printf(" Pos   MSGLEN   TIME            MSGTYPE         \n");
              for(i=0; i<=PERFBUFSIZE ; i++){
                      printf("%d        %d      %g                      %d      \n",temp->pos,temp->msglen,
                                temp->time, temp->msgtype );
                        if (i!=0){
                                fprintf(fp,"%g %g\n",(double)(temp->msglen)/(double)(1024L),(double)(temp->time)/1000000);
                        }
                        temp = temp->fwd;
              }

        system("graph -T X -m 2 -C -W 0.001 -X x -Y y< tmp");
        fclose(fp);
}


void SL_network_performance_insert(double latency, double bandwidth, SL_proc *proc)
{
        if (proc->netperf->pos == PERFBUFSIZE){
//              SL_print_net_performance(proc->netperf);
//              proc->netperf = proc->netperf->fwd;

        }
        else{
        proc->netperf->latency   = latency;
        proc->netperf->bandwidth = bandwidth;
        proc->netperf = proc->netperf->fwd;
        }
}


void SL_print_net_performance(SL_network_perf *insertpt)
{
        SL_network_perf *temp;
        int i;
        temp = insertpt;
        printf("**************************************************************************\n");
        printf(" Pos    Latency         Bandwidth\n");
        for(i=0; i<=PERFBUFSIZE ; i++){
                printf("%d  %g          %g \n", temp->pos, temp->latency,temp->bandwidth);
                 temp = temp->fwd;
        }
        printf("**************************************************************************\n");



}

int SL_net_performance_free(SL_proc *tproc)
{
        SL_network_perf *tnode = NULL;

        while (tproc->netperf!=NULL){
                tnode = tproc->netperf;
                if(tnode->pos != PERFBUFSIZE){
                        tproc->netperf = tproc->netperf->fwd;
                        free(tnode);
                }
                else
                break;
        }
        return SL_SUCCESS;

}


int SL_msg_performance_free(SL_proc *tproc)
{
        SL_msg_perf *tnode = NULL;

        while (tproc->netperf!=NULL){
                tnode = tproc->msgperf;
                if(tnode->pos != PERFBUFSIZE){
                        tproc->msgperf = tproc->msgperf->fwd;
                        free(tnode);
                }
                else
                break;
        }
        return SL_SUCCESS;

}


