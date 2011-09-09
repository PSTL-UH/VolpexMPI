#include "MCFA.h"
#include "MCFA_internal.h"


/**


	
                  arg[0]  = type of connection
                  arg[1]  = name of host where process is to be spawned (since we need to do ssh shark01)
             0.   arg[2]  = path of deamon
             1.   arg[3]  = path of executable
             2.   arg[4]  = hostname of mcfarun
             3.   arg[5]  = port of mcfarun
             4.   arg[6]  = rank of mcfarun
             5.   arg[7]  = redundancy
             6.   arg[8]  = spawn flag

             7.   arg[9]  = number of proccesses on each host
             8.   arg[10] = SL_id of procs  
             9.  arg[11] = jobID(no need)
             10.  arg[12] = Volpex_id of procs(no need)
             11.  arg[13] = fullrank (e.g 0,A)(no need)

             12.  arg[14] = NULL
        
**/




int main(int argc, char **argv )

{
    char *hostname;
    int port;
    int id,event_handler_id;
    char *path;
//    char *rank;
    int red,numprocs;
    int spawn_flag;
    int pid = 10;
    int status;
    char **arg = NULL;

//	arg = MCFA_read_argfile();
    
    /** For condor since same executable is used by each deamon rename the file and then use it using 
	system( 'rename volpex.* volpex' );*/ 
//    sleep(10);
    
    if (argc > 1){
	
	path 		= strdup (argv[1]);
	hostname            = strdup ( argv[2] );
	port                = atoi ( argv[3] );
	event_handler_id    = atoi (argv[4]);                
	red         	= atoi (argv[5]);   
	spawn_flag		= atoi (argv[6]);
	numprocs		= atoi (argv[7]);
//    ids                 = strdup(argv[9]);
	
//    jobID               = atoi (argv[4]);
//    id                  = atoi (argv[5]);   //startid or listofids
//    event_handler_id    = atoi (argv[6]);   //list of ranks 
//    rank	        = strdup(argv[7]);
//    red         	= atoi (argv[8]);   
//    flag		= atoi (argv[9]);
    }
    else
    {
	arg = MCFA_read_argfile();
	path                = strdup (arg[1]);
	hostname            = strdup ( arg[2] );
	port                = atoi ( arg[3] );
	event_handler_id    = atoi (arg[4]);
	red                 = atoi (arg[5]);
        spawn_flag                = atoi (arg[6]);
	numprocs                = atoi (argv[7]);


//	jobID               = atoi (arg[4]);
//	id                  = atoi (arg[5]);
//	event_handler_id    = atoi (arg[6]);
//	rank                = strdup(arg[7]);
//	red                 = atoi (arg[8]);
//	spawn_flag                = atoi (arg[9]);
	
    }
    int i;
    printf ("Total number of arguments:%d\n",argc);
    for(i=0;i<argc;i++)
    {
	PRINTF(("%d.  %s\n",i,argv[i]));
    }
    
    
    
    
    
    
    
    int nextSL;
    
    
    nextSL = 8;
    if (spawn_flag != 0)
	numprocs = 1;
 
    for(i=0;i<numprocs;i++){
	
	id = atoi(argv[nextSL]);
	MCFA_set_env(path, hostname, port, id, event_handler_id, red, spawn_flag);
	nextSL++;
	
	PRINTF(("path           : %s\n \
	hostname         : %s\n	      \
	port             : %d\n	      \
	id               : %d\n	      \
	event_handler_id : %d\n	      \
	red		 : %d\n	      \
	flag		 : %d\n",
	       path, hostname, port, id, event_handler_id,red,spawn_flag));
	
	
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
    }
    
    for (i=0;i<numprocs;i++){
	
	wait(&status);
	
	
	
        if (WIFEXITED(status)) {
	    printf("exited, status=%d proc:%d\n", WEXITSTATUS(status),i);
	} else if (WIFSIGNALED(status)) {
	    printf("killed by signal %d proc:%d\n", WTERMSIG(status),i);
	} else if (WIFSTOPPED(status)) {
	    printf("stopped by signal %d\n", WSTOPSIG(status));
	}
    }
/*    
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
    
//   system("mv volpex.* volpex" );

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
    if (NULL == arg[1]){
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
    
/*
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
*/
//    arg[10]= NULL;
    
    return arg;
    
}
