#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"

extern int SL_this_procport;
extern struct      MCFA_host_node *hostList;
extern struct      MCFA_proc_node *procList;
extern char hostname[MAXHOSTNAMELEN];

char ** MCFA_set_args(int id,char **hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy, int condor_flag)
{


	/**
		arg[0]	= type of connection
		arg[1]	= hostnames for procs
		arg[2]	= path of deamon
		arg[3]	= path of executable
		arg[4]	= hostname of mcfarun
		arg[5]	= port of mcfarun
		arg[6]	= jobID
		arg[7]	= rank of procs
		arg[8]	= rank of mcfarun
		arg[9]	= fullrank (e.g 0,A)
		arg[10]	= redundancy
		arg[11] = NULL 
	**/


	char        **arg = NULL;
	arg = (char **)malloc(MAXARGUMENTS * sizeof(char*));
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
    arg[1] = strdup(hostName[0]);				
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
	
    arg[12]= NULL;

        free(d_path1);
	free(d_path);
	return arg;
}

//MCFA_spawn_procs()

void MCFA_set_lists(int id,char **hostName, char *path, int port, int jobID, int numprocs,int hostCount, int redundancy)
{
	struct      MCFA_host *node=NULL;
    	int         i,j=0;
    	char        fullrank[16];
    	char        level = 'A';
    	char        rank  = -1;


	for(i=0;i<numprocs;i++)
    	{
        	id = MCFA_get_nextID();
//		port++;
            node = MCFA_search_hostname(hostList,hostName[j]);
            if(node != NULL){
                node->id[node->numofProcs].procID = id;
                node->id[node->numofProcs].jobID = jobID;
                MCFA_get_exec_name(path,node->id[node->numofProcs].exec);
                node->numofProcs++;
               node->lastPortUsed++;
	//	node->lastPortUsed = port;
            }
            if (rank == numprocs/redundancy-1){
                        rank = 0;
                        level++;
                }
                else
                        rank++;

            sprintf(fullrank,"%d,%c",rank,level);


            port = node->lastPortUsed ;
            PRINTF(("MCFA_startprocs: Adding proc %d with jobid %d hostname %s to processList\n",id,jobID,hostname));
            MCFA_add_proc(&procList, id, hostName[j],port, jobID,-1, 1,path,fullrank);    //for each process adding it process list
            j++;
            if (j==hostCount){
                j=0;
//                port++;
            }

    }/* end for */
//	MCFA_printProclist(procList);
//      MCFA_printHostlist(hostList);

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
	fprintf(fp, "should_transfer_files = YES\n");
	fprintf(fp, "when_to_transfer_output = ON_EXIT\n\n");
	fprintf(fp, "Queue ");
	fprintf(fp, "%d\n", numprocs);
	fclose(fp);	

}

