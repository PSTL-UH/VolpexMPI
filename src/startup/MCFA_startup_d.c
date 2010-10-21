#include "MCFA.h"
#include "MCFA_internal.h"





int main(int argc, char **argv )

{
    char *hostname;
    int port;
    int id,jobID,event_handler_id;
    char *path;
    char *rank;
    int red;
    int flag;
    int pid = 10;
    int status;
    char **arg = NULL;

//	arg = MCFA_read_argfile();

	/** For condor since same executable is used by each deamon rename the file and then use it using 
		system( 'rename volpex.* volpex' );*/ 


    if (argc > 1){

    path 		= strdup (argv[1]);
    hostname            = strdup ( argv[2] );
    port                = atoi ( argv[3] );
    jobID               = atoi (argv[4]);
    id                  = atoi (argv[5]);
    event_handler_id    = atoi (argv[6]);
    rank	        = strdup(argv[7]);
    red         	= atoi (argv[8]);   
    flag		= atoi (argv[9]);
}
else
{
	arg = MCFA_read_argfile();
	path                = strdup (arg[1]);
    hostname            = strdup ( arg[2] );
    port                = atoi ( arg[3] );
    jobID               = atoi (arg[4]);
    id                  = atoi (arg[5]);
    event_handler_id    = atoi (arg[6]);
    rank                = strdup(arg[7]);
    red                 = atoi (arg[8]);
    flag                = atoi (arg[9]);

}




   MCFA_set_env(path, hostname, port, jobID, id, event_handler_id, rank, red, flag);
   PRINTF(("path           : %s\n \
	hostname         : %s\n \
	port             : %d\n \
	jobID            : %d\n \
	id               : %d\n \
	event_handler_id : %d\n \
        rank		 : %s\n \
	red		 : %d\n \
	flag		 : %d\n",
        path, hostname, port, jobID, id, event_handler_id,rank,red,flag));


char **arg1;
        arg1 = (char **) malloc (2*sizeof(char*));
 if(arg1 == NULL){
        printf("ERROR: in allocating memory");
        exit(-1);
    }

        arg1[0] = strdup(path);
        arg1[1] = NULL;


//	execv(path,arg1);
   pid = fork();

   if (pid == 0){
        if(-1 ==  execv(path, arg1)){
        	printf("Error!! in spawning the program\n");
		return 0;
	}
}
else
	wait(&status);
        if (WIFEXITED(status)) {
                       PRINTF(("exited, status=%d proc=%d\n", WEXITSTATUS(status),id));
                   } else if (WIFSIGNALED(status)) {
                       printf("killed by signal %d proc=%d\n", WTERMSIG(status),id);
                   } else if (WIFSTOPPED(status)) {
                       printf("stopped by signal %d proc=%d\n", WSTOPSIG(status),id);
                   }


/*	int i;
	for (i=0; i<MAXARGUMENTS;i++){
		if(NULL != arg[i])
			free(arg[i]);
	}
	if(NULL != arg)
		free(arg);
*/

        return 1;
}

char ** MCFA_read_argfile()
{
    FILE *fp;
    char        **arg = NULL;
    char exec[MAXNAMELEN];
    
   system("mv volpex.* volpex" );

 //   sprintf
    
    fp = fopen ("volpex", "r");
    if (fp == NULL)
	printf(" Error in opening condor input file \n");

    
    arg = (char **)malloc(MAXARGUMENTS * sizeof(char*));
    if(arg == NULL){
        printf("ERROR: in allocating memory\n");
        exit(-1);
    }
    
    arg[0] = (char *) malloc (10);
    if (NULL == arg[0]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    sprintf(arg[0], "%s", "condor");
    
    arg[1] = (char *) malloc (MAXNAMELEN);
    if (NULL == arg[0]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", exec);
	strcpy(arg[1], exec);
    
    arg[2] = (char *) malloc (MAXHOSTNAMELEN);
    if (NULL == arg[2]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[2]);
    
    arg[3] = (char *) malloc (sizeof (int)+1);
    if (NULL == arg[3]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[3]);
    
    
    arg[4] = (char *) malloc (sizeof (int)+1);
    if (NULL == arg[4]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[4]);
    
    arg[5] = (char *) malloc (sizeof (int)+1);
    if (NULL == arg[5]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[5]);
    
    arg[6] = (char *) malloc (sizeof (int)+1);
    if (NULL == arg[6]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[6]);
    
    
    arg[7] = (char *) malloc (16 * sizeof(char));
    if (NULL == arg[7]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[7]);
    
    arg[8] = (char *) malloc (sizeof (int)+1);
    if (NULL == arg[8]){
	printf("ERROR: in allocating memory\n");
	exit(-1);
    }
    fscanf(fp, "%s", arg[8]);
    

    arg[9] = (char *) malloc (sizeof (int)+1);
    if (NULL == arg[9]){
        printf("ERROR: in allocating memory\n");
        exit(-1);
    }
    fscanf(fp, "%s", arg[9]);

    arg[10]= NULL;
    
    return arg;
    
}
