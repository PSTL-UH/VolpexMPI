#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"

extern int SL_this_procport;
extern int SL_init_numprocs;
extern struct      MCFA_host_node *hostList;
extern struct      MCFA_proc_node *procList;
extern char hostname[MAXHOSTNAMELEN];



char ** MCFA_set_args1(struct MCFA_host *host, char *path, int port, int redundancy, int condor_flag)
{


        /**
                arg[0]  = type of connection
                arg[1]  = name of host where process is to be spawned (since we need to do ssh shark01)
                arg[2]  = path of deamon
                arg[3]  = path of executable
                arg[4]  = hostname of mcfarun
                arg[5]  = port of mcfarun
                arg[6]  = rank of mcfarun
                arg[7]  = redundancy
                arg[8]  = spawn flag

                arg[9]  = number of proccesses on each host
                arg[10] = SL_id of procs

                arg[11] = NULL
        **/


        char        **arg = NULL;
//      arg = (char **)malloc(MAXARGUMENTS * sizeof(char*));
        arg = (char **)malloc(11 * sizeof(char*));
    if(arg == NULL){
        printf("ERROR: in allocating memory\n");
        exit(-1);
    }

char *d_path, *d_path1 ;

    d_path1 = (char*) malloc (MAXNAMELEN * sizeof(char));
    strcpy(d_path1, "./mcfastart_d");
   // d_path1   = strdup("./mcfastart_d");

    MCFA_get_path(d_path1, &d_path);
    arg[0] = strdup("ssh");
    arg[1] = strdup(host->hostname);
    arg[2] = strdup(d_path);
    arg[3] = strdup(path);
    arg[4] = strdup(hostname);
    arg[5] = (char *) malloc (MAXPORTSIZE + 1);
    if (NULL == arg[5]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[5], "%d" ,port);

    arg[6] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[6]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[6], "%d",MCFA_MASTER_ID);

    arg[7] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[7]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[7], "%d", redundancy);

    arg[8] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[8]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[8], "%d", condor_flag);

    arg[9] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[9]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[9], "%d", host->numofProcs);


char *tprocid;
char *procids;
int i;
procids = (char*)malloc(4*host->numofProcs*sizeof(char));
tprocid = (char *) malloc (sizeof(int) + 1);

    sprintf(tprocid, "%d", host->id[0].procID);
    strcpy(procids, tprocid);
    for(i=1;i<host->numofProcs;i++){
	     sprintf(tprocid, "%d", host->id[i].procID);
             strcat(procids, " ");
             strcat(procids,tprocid);
     }
     arg[10] = (char *) malloc (4*host->numofProcs);
     if (NULL == arg[10]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }

     strcpy(arg[10], procids);





    
	


    arg[11]= NULL;

//        free(d_path1);
//      free(d_path);
        return arg;
}












char ** MCFA_set_args(int id,char *hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy, int condor_flag)
{


	/**
		arg[0]	= type of connection
		arg[1]  = name of host where process is to be spawned (since we need to do ssh shark01)
		arg[2]	= path of deamon
		arg[3]	= path of executable
		arg[4]	= hostname of mcfarun
		arg[5]	= port of mcfarun
		arg[6]	= rank of mcfarun
		arg[7]	= redundancy
		arg[8]  = spawn flag
		
		arg[9]  = number of proccesses on each host
		arg[10]	= SL_id of procs
		arg[10]	= jobID
		arg[12] = Volpex_id of procs
		arg[13]	= fullrank (e.g 0,A)
		
		arg[14] = NULL 
	**/


	char        **arg = NULL;
//	arg = (char **)malloc(MAXARGUMENTS * sizeof(char*));
	arg = (char **)malloc(11 * sizeof(char*));
    if(arg == NULL){
        printf("ERROR: in allocating memory\n");
        exit(-1);
    }


    char *d_path, *d_path1 ;
   
    d_path1 = (char*) malloc (MAXNAMELEN * sizeof(char));
    strcpy(d_path1, "./mcfastart_d");
   // d_path1	= strdup("./mcfastart_d");

    MCFA_get_path(d_path1, &d_path);
    arg[0] = strdup("ssh");					
    arg[1] = strdup(hostName);
    arg[2] = strdup(d_path);					
    arg[3] = strdup(path);					
    arg[4] = strdup(hostname);					
    arg[5] = (char *) malloc (MAXPORTSIZE + 1);			
    if (NULL == arg[5]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[5], "%d" ,port);

    arg[6] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[6]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[6], "%d",MCFA_MASTER_ID);

    arg[7] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[7]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[7], "%d", redundancy);

    arg[8] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[8]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[8], "%d", condor_flag);
/*
    arg[6] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[6]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }

   sprintf(arg[6], "%d" ,numprocs);

    arg[7] = (char *) malloc (sizeof(int) + 1);			
    if (NULL == arg[6]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }

   sprintf(arg[6], "%d" ,jobID);

    arg[7] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[7]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[7], "%d" ,id);

    arg[8] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[8]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[8], "%d",MCFA_MASTER_ID);

    arg[9] = (char *) malloc (16 * sizeof(char)); //fullrank
    if (NULL == arg[9]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    arg[10] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[10]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[10], "%d", redundancy);

    arg[11] = (char *) malloc (sizeof(int) + 1);
    if (NULL == arg[11]){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    sprintf(arg[11], "%d", condor_flag);
*/	
    arg[11]= NULL;

//        free(d_path1);
//	free(d_path);
	return arg;
}

//MCFA_spawn_procs()

//struct MCFA_proc_node* MCFA_set_lists(int id,char **hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy)
struct MCFA_proc_node* MCFA_set_lists1(int initid,char **hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy)
{
/*
	initid   - val = -1 if a process is spawned first time
		      != -1 if a processes is added to the existing job pool
	
	hostName - consists of the array of hostnames taken from hostfile. If a particular host is repeated n times then there are n entries
	of it.
	path     - path of the executable file
	port     - port at which mcfarun is running
	numprocs - total number of processes to be spawned
	hostCount - total number of entries in hostfile
	redundancy - 
*/

	struct      MCFA_host *node=NULL;
//      struct 	    MCFA_host_node *currhost=NULL;
//	struct	    MCFA_process *proc=NULL;
    	int         i,id, j=0;
    	char        fullrank[16];
    	char        level = 'A';
    	char        rank = -1  ;
	struct    MCFA_proc_node *newproclist = NULL;
	char	newlevel;
        
/*        node = MCFA_search_hostname(hostList,starthost);
	currhost = hostList;
	while(strcmp(currhost->hostdata->hostname,node->hostname)){
		currhost = currhost->next;
	}
*/

	for(i=0;i<numprocs;i++)
    	{
            id = MCFA_get_nextID();
	    node = MCFA_search_hostname(hostList,hostName[j]);		
//	    node = currhost->hostdata;
            if(node != NULL){
                node->id[node->numofProcs].procID = id;
                node->id[node->numofProcs].jobID = jobID;
                MCFA_get_exec_name(path,node->id[node->numofProcs].exec);
                node->numofProcs++;
               node->lastPortUsed++;
            }
	    if (initid != -1){
		newlevel = MCFA_search_rank_lastlevel(procList,initid);
		newlevel++;
		sprintf(fullrank,"%d,%c",initid,newlevel);
	    }
	    else if(initid == -1){
	            if (rank == (numprocs/redundancy)-1){
                        rank = 0;
        	        level++;
                    }
                    else
                        rank++;

           	sprintf(fullrank,"%d,%c",rank,level);
	   }


            port = node->lastPortUsed ;
            PRINTF(("MCFA_startprocs: Adding proc %d with jobid %d hostname %s to processList\n",id,jobID,hostname));
            MCFA_add_proc(&procList, id, node->hostname,port, jobID,-1, 1,path,fullrank);    //for each process adding it process list
	    MCFA_add_proc(&newproclist, id, node->hostname,port, jobID,-1, 1,path,fullrank);
	    j++;
            if (j==hostCount){
                j=0;
           } 
/*
	    if(currhost->next == NULL)
		currhost = hostList;
	    else
		currhost = currhost->next;
*/
	
    }/* end for */
	return(newproclist);

}

char MCFA_search_rank_lastlevel(struct MCFA_proc_node *procList, int initid)
{
	struct MCFA_proc_node *curr = procList;
	char level, lastlevel;
	int id;

        while(curr !=NULL)
        {
		sscanf(curr -> procdata->fullrank,"%d,%c",&id,&level);
		if (id == initid){
			lastlevel = level;
		}
                curr = curr ->next;
        }
        return lastlevel;

}

struct MCFA_host_node* MCFA_set_hostlist(char *hostFile, char *hostname, int numprocs, int port, int *hostCount, char ***hostName)
{
	char        **thostName=NULL;
//	int         hostCount = 0;
	int         maxprocspernode = MAXPROCSPERNODE;
	int 	    i;
	struct      MCFA_host *newnode=NULL;
	struct 	    MCFA_host_node *newhostlist;
	
	if (hostFile != NULL){
        /* allocating hostfile to array named hostName */
        thostName = MCFA_allocate_func(hostFile,hostCount);
        /* counting number of hosts */
        *hostCount = *hostCount-1;
        }

    else{                                                       //if hostfile option do not exist all procceses
        thostName = (char**)malloc(sizeof(char*));
        if (NULL == hostName){
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }
        thostName[0] = strdup(hostname);
        *hostCount = 1;
    }
/* creating host list */
    MCFA_initHostList(&newhostlist);
    int num = (int)ceil((double)numprocs/(double)*hostCount);
    if(maxprocspernode<num)
        maxprocspernode = num;

    for(i=0;i<*hostCount;i++)
    {
        PRINTF(("MCFA_startprocs: adding hosts to hostlist\n"));
      if((MCFA_search_hostname(hostList,thostName[i])) == NULL){       //for each host in hostfile creating an entry in hostList
            PRINTF(("MCFA_startprocs: Adding host : %s\n",hostName[i]));
            newnode = MCFA_init_hostnode(thostName[i],maxprocspernode,port);
            MCFA_add_host(&hostList, newnode);
	    MCFA_add_host(&newhostlist, newnode);
        }

    }
    *hostName = thostName;

    return newhostlist;


}



void MCFA_create_condordesc(char *exe, int numprocs)
{
	FILE *fp;
	
	fp = fopen("volpexjob.condor", "w");

	fprintf(fp,"\nExecutable	=");
	fprintf(fp," %s\n\n\n", exe);
	fprintf(fp,"Universe	= vanilla\n");
	fprintf(fp,"Input	= volpex.$(Process)\n");
	fprintf(fp,"Output	= output.$(Process)\n");
	fprintf(fp, "Notification = never\n");
	fprintf(fp,"Log		= volpex.log\n\n\n");
	fprintf(fp, "transfer_input_files = %s , volpex.$(Process)\n", exe);
	fprintf(fp, "should_transfer_files = YES\n");
	fprintf(fp, "when_to_transfer_output = ON_EXIT\n\n");
	fprintf(fp, "Queue ");
	fprintf(fp, "%d\n", numprocs);
	fclose(fp);	

}

