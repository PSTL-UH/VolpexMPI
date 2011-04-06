
#include"MCFA.h"
#include "MCFA_internal.h"


int MCFA_initHostList(struct MCFA_host_node **hostList)
{
        *hostList = NULL;
	return MCFA_SUCCESS;
}

struct MCFA_host* MCFA_search_hostname(struct MCFA_host_node *hostList, char *host)
{
        struct MCFA_host_node *curr = hostList;
        while(curr!=NULL)
        {
                if (strcmp(curr -> hostdata->hostname, host)==0)
                        return (curr -> hostdata);
                curr = curr ->next;
        }
        return NULL;
}

struct MCFA_host* MCFA_init_hostnode(char *hostname,int maxprocs, int port)
{
	struct MCFA_host *node;
	int i;
	node = (struct MCFA_host*) malloc (sizeof(struct MCFA_host));
        if (node == NULL)
        {
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }
	strcpy(node->hostname,hostname);
        node->numofProcs = 0;
        node->lastPortUsed = port;
//        strcpy(node->executable,"");
        node->status = 0;
	node->id = (struct MCFA_ID*) malloc (maxprocs * sizeof (struct MCFA_ID));
	if (NULL == node->id){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }

	
	for(i=0;i<maxprocs;i++)
	{
		node->id[i].procID = 0;
		node->id[i].jobID = 0;
		strcpy(node->id[i].exec,"");
	}
	return node;


	
}


void MCFA_free_hostlist(struct MCFA_host_node *hostList)
{
    struct MCFA_host_node *currhost = NULL, *currnext = NULL;
    currhost = hostList;
    if (hostList !=NULL){
        do{
                currnext = currhost->next;
		if (NULL != currhost->hostdata->id)
			free(currhost->hostdata->id);
                free(currhost);
                currhost = currnext;

        }while(currhost !=NULL);
    }

//    free(hostList);
}




int MCFA_add_host(struct MCFA_host_node **hostList,struct MCFA_host *node)
{
	struct  MCFA_host_node *next;
        struct MCFA_host_node *prev;
        struct MCFA_host_node *curr ;
	
	curr=(struct MCFA_host_node *)malloc(sizeof(struct MCFA_host_node));
   if ( curr == NULL)
      return(-1);

   curr->next=NULL;
//   curr->hostdata = *node;

   curr->hostdata = node;
   if (*hostList == NULL)
   {
      *hostList=curr;
   }
   else
   {
      next=*hostList;
      prev=NULL;

      while(next != NULL )
      {
         prev=next;
         next=next->next;
      }
      /* Now have a spot to insert */
      if (prev == NULL)
      {
         *hostList=curr;
         curr->next=next;
      }
      else
      {
         curr->next = prev->next;
         prev->next=curr;
      }
   }
        return MCFA_SUCCESS;
}



int MCFA_add_hostname(struct MCFA_host_node **hostList, char *hostname,int numofProcs,int lastPortUsed, int status )
{
        struct  MCFA_host_node *next;
        struct MCFA_host_node *prev;
        struct MCFA_host_node *curr;
        struct MCFA_host *newhost;

        newhost = (struct MCFA_host*)malloc(sizeof(struct MCFA_host));
	strcpy(newhost->hostname,hostname);
        newhost->numofProcs = numofProcs;
        newhost->lastPortUsed = lastPortUsed;
        newhost->status = status;

/*        strcpy(newhost.hostname,hostname);
        newhost.numofProcs = numofProcs;
        newhost.lastPortUsed = lastPortUsed;
        newhost.status = status;
*/

   curr=(struct MCFA_host_node *)malloc(sizeof(struct MCFA_host_node));
   if ( curr == NULL)
      return(-1);

   curr->next=NULL;
   curr->hostdata=newhost;

   if (*hostList == NULL)
   {
      *hostList=curr;
   }
   else
   {
      next=*hostList;
      prev=NULL;

      while(next != NULL )
      {
         prev=next;
         next=next->next;
      }
      /* Now have a spot to insert */
      if (prev == NULL)
      {
         *hostList=curr;
         curr->next=next;
      }
      else
      {
         curr->next = prev->next;
         prev->next=curr;
      }
   }
	return MCFA_SUCCESS;
}

int MCFA_get_total_hosts(struct MCFA_host_node *hostList)
{
        struct MCFA_host_node *curr = NULL;
        int count = 0;

        curr = hostList;
        while(curr != NULL)
        {
                count++;
                curr = curr->next;
        }
        return count;
}

int MCFA_printHostlist(struct MCFA_host_node *hostList)
{

        struct MCFA_host_node *curr = hostList;
        int i;

	printf("\nHostname\t");
        printf("Numof Procs\t");
        printf("lastportused\t");
        printf("ProcIDs\t\t");
	printf("JobIDs\t\t");
        printf("executable\t\t\t");
        printf("Status\n");

        while(curr !=NULL)
        {
                printf("%s\t\t ",curr ->hostdata->hostname);
                printf("%d\t\t",curr ->hostdata->numofProcs);
                printf("%d\t\t",curr ->hostdata->lastPortUsed);
                for(i=0;i<curr ->hostdata->numofProcs;i++)
                {
                        printf("%d ",curr ->hostdata->id[i].procID);
                }
		
		printf("%c\t\t",' ');
                for(i=0;i<curr ->hostdata->numofProcs;i++)
                {
                      printf("%d ",curr ->hostdata->id[i].jobID);
                }
		printf("%c\t\t",' ');
		for(i=0;i<curr ->hostdata->numofProcs;i++)
                {
                      printf("%s, ",curr ->hostdata->id[i].exec);
                }

		printf("\t\t");
//                printf("%s\t  ",curr ->hostdata.executable);
                printf("%d  ",curr ->hostdata->status);
                curr = curr->next;
                printf("\n");
        }
        return MCFA_SUCCESS;


}






int MCFA_remove_host(struct MCFA_host_node **hostList, char *hostname)
{
	struct MCFA_host_node *curr = NULL;
	struct MCFA_host_node *previous= NULL;
	struct MCFA_host_node *temp = NULL;

	while(strcmp(curr->hostdata->hostname,hostname)==0)
	{
		previous = curr;
		curr = curr->next;
	}
	if(curr!=NULL)
	{
		if (curr == *hostList){
			temp = *hostList;
			*hostList = temp->next;
                        free(temp);
                }
                else{
                        previous->next = curr ->next;
                        free(curr);
                }
        }

	
	return MCFA_SUCCESS;
	
}

int MCFA_initProcList(struct MCFA_proc_node **procList)
{
        *procList = NULL;
        return MCFA_SUCCESS;
}
int MCFA_get_total_procs(struct MCFA_proc_node *procList)
{
        struct MCFA_proc_node *curr = NULL;
        int count = 0;

        curr = procList;
        while(curr != NULL)
        {
                count++;
                curr = curr->next;
        }
        return count;
}

struct MCFA_process* MCFA_search_proc(struct MCFA_proc_node *procList, int procID)
{
        struct MCFA_proc_node *curr = procList;
        while(curr !=NULL)
        {
                if (curr -> procdata->id == procID )
                        return (curr -> procdata);
                curr = curr ->next;
        }
        return NULL;
}



int MCFA_add_proc(struct MCFA_proc_node **procList, int procID ,char *host, int portNumber,
			int jobID, int sock,int status, char *exec, char *fullrank)
{
        struct  MCFA_proc_node *next;
        struct MCFA_proc_node *prev;
        struct MCFA_proc_node *curr;
        struct MCFA_process *newproc;

	newproc = (struct MCFA_process*)malloc(sizeof(struct MCFA_process));
	newproc->id = procID;
	newproc->volpex_id = procID;
	newproc->hostname = (char*)malloc(MAXHOSTNAMELEN * sizeof(char));
	strcpy(newproc->hostname,host);	
	newproc->portnumber = portNumber;
	newproc->jobid = jobID;
	newproc->sock = sock;	
	newproc->status = status;
	newproc->executable = (char*) malloc (MAXNAMELEN * sizeof(char));
	strcpy(newproc->executable,exec);
	strcpy(newproc->fullrank,fullrank);

   curr=(struct MCFA_proc_node *)malloc(sizeof(struct MCFA_proc_node));
   if ( curr == NULL)
      return(-1);

   curr->next=NULL;
   curr->procdata=newproc;

   if (*procList == NULL)
   {
      *procList=curr;
   }
   else
 {
      next=*procList;
      prev=NULL;

      while(next != NULL )
      {
         prev=next;
         next=next->next;
      }
      /* Now have a spot to insert */
      if (prev == NULL)
      {
         *procList=curr;
         curr->next=next;
      }
      else
      {
         curr->next = prev->next;
         prev->next=curr;
      }
   }
        return MCFA_SUCCESS;
}

int MCFA_printProclist(struct MCFA_proc_node *procList)
{


	printf("\nProcess ID\t");
	printf("Volpex_ID\t");
	printf("Host Name\t");
	printf("Port Num\t");
	printf("Job ID\t");
	printf("Socket \t");
	printf("Status\t");
	printf("Fullrank\t");
	printf("Executable\n");

        struct MCFA_proc_node *curr = procList;
        while(curr !=NULL)
        {
                printf("%4d\t\t  ",curr ->procdata->id);
                printf("%4d\t\t  ",curr ->procdata->volpex_id);
                printf("%s\t  ",curr ->procdata->hostname);
		printf("%d\t\t  ",curr ->procdata->portnumber);
		printf("%d\t  ",curr ->procdata->jobid);
		printf("%d\t  ",curr ->procdata->sock);
		printf("%d\t  ",curr ->procdata->status);
		printf("%s\t\n  ",curr->procdata->fullrank);
//		printf("%s\n  ",curr->procdata->executable);
                curr = curr->next;
                printf("\n");
        }
        return MCFA_SUCCESS;


}


int MCFA_remove_proc(struct MCFA_proc_node **procList, int procid)
{
        struct MCFA_proc_node *curr = NULL;
	struct MCFA_proc_node *previous = NULL;
        struct MCFA_proc_node *temp = NULL;

	curr = *procList;
        while(curr->procdata->id != procid)
        {	
		previous = curr;
                curr = curr->next;
        }

	if(curr != NULL)
	{
		if (curr == *procList){ 	      //	deleting from head
			temp = *procList;
			*procList = temp->next;
			free(temp);
		}
        	else{
			previous->next = curr ->next;
			free(curr);
	        }
	}
	return MCFA_SUCCESS;

}

int MCFA_print_proc(struct MCFA_process *proc)
{
	printf("\nVolpex_id\t");
	printf("Hostname\t");
        printf("Portnumber\t");
        printf("Socket\t\t");
	printf("Status\t\t");
	printf("Executbale\t");
	printf("Fullrank \n");

	printf("%d\t\t %s\t\t  %d\t\t  %d\t\t %d\t\t %s\t %s\n",proc->volpex_id,proc->hostname,proc->portnumber,proc->sock,
							proc->status,proc->executable,proc->fullrank);

	return MCFA_SUCCESS;
}

int MCFA_print_host(struct MCFA_host *host)
{
	int j;

	printf("\nHostname\t");
        printf("Numof Procs\t");
        printf("lastportused\t");
        printf("ProcIDs\t\t");
        printf("JobIDs\t\t");
        printf("executable\t\t\t");
        printf("Status\n");
	
	printf("%s\t\t  %d\t\t  %d\t\t  ",host->hostname, host->numofProcs, host->lastPortUsed);
	
	for(j=0;j<host->numofProcs;j++)
	        printf("%d ",host->id[j].procID);
	
	printf("\t\t");

        for(j=0;j<host->numofProcs;j++)
        	printf("%d ",host->id[j].jobID);

        printf("\t");

	for(j=0;j<host->numofProcs;j++)
                printf("%s ",host->id[j].exec);
	printf("\t");

        printf(" %d\n \n",host->status );


	return MCFA_SUCCESS;
}

struct MCFA_process* MCFA_getlast_proc(struct MCFA_proc_node *procList)
{
	struct MCFA_proc_node *curr = procList;
	while(curr->next!=NULL)
		curr = curr->next;
	return (curr -> procdata);

	
}



void MCFA_free_proclist(struct MCFA_proc_node *proclist)
{
    struct MCFA_proc_node *currproc = NULL, *currnext = NULL;
    currproc = proclist;
    if (proclist !=NULL){
        do{
                currnext = currproc->next;
		if(NULL != currproc->procdata->hostname)
			free(currproc->procdata->hostname);
	//	if(NULL != currproc->procdata->executable)
	//		free(currproc->procdata->executable);	
                free(currproc);
                currproc = currnext;

        }while(currproc !=NULL);
    }

  //  free(procList);
}

int MCFA_proc_close(struct MCFA_proc_node *proclist, int procid)
{
	struct MCFA_process *proc;

        proc = MCFA_search_proc(proclist,procid);
	if (proc !=NULL && proc->status != 0)
		proc->status = 0;

	return MCFA_SUCCESS;	
}

int MCFA_clear_procs(struct MCFA_proc_node *procList)
{
        struct MCFA_proc_node *next = NULL;
        struct MCFA_proc_node *curr = NULL;
	int count = 0;

        curr = procList;
        while(curr != NULL)
        {
                next = curr->next;
                if (curr->procdata->status == 0){
                        MCFA_remove_proc(&procList,curr->procdata->id);
			count++;
		}
                curr = next;
        }
        return count;
}

int MCFA_proc_exists(struct MCFA_proc_node *procList,int id)
{
//    struct MCFA_proc_node *currproc = procList;
	struct MCFA_process *proc;

    proc = MCFA_search_proc(procList,id);
    if (proc !=NULL && proc->status != 0)
    	return 1;
    else
	return 0;            
	
/*    while(NULL != currproc)
    {
        if(id == currproc->procdata.id && currproc->procdata.status != 0)
            return 1;
        currproc = currproc->next;
    }*/
//    return 0;

}

int MCFA_procjob_exists(struct MCFA_proc_node *procList,int id)
{
    struct MCFA_proc_node *currproc = procList;
    while(NULL != currproc)
    {
        if(id == currproc->procdata->jobid && 0 != currproc->procdata->status)
            return 1;
        currproc = currproc->next;
    }
    return 0;

}


