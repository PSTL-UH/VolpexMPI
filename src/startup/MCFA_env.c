#include "MCFA.h"
#include "MCFA_internal.h"


int MCFA_init_env()
{
        setenv("MCFA_PATH",               "",     0);
        setenv("MCFA_HOSTNAME",           "",     0);
        setenv("MCFA_PORT",               "",     0);
        setenv("MCFA_JOBID",              "",     0);
        setenv("MCFA_ID",                 "",     0);
        setenv("MCFA_EVENT_HANDLER",      "",     0);
        setenv("MCFA_FULLRANK",           "",  	  0);
        setenv("MCFA_REDUNDANCY",         "",     0);
	setenv("MCFA_SPAWN_FLAG",	  "",	  0);
	setenv("MCFA_CLUSTER_FLAG",	  "",	  0);
        return 1;

}


int MCFA_set_env(char *path, char *hostname, int port, int id, int ehandler, int red , 
		int flag, int cluster_flag)
{
        char *port1,  *id1, *ehandler1, *red1, *flag1, *flag2;

        setenv("MCFA_PATH",             path,     1);
        setenv("MCFA_HOSTNAME",                 hostname, 1);

        port1 = (char *) malloc (sizeof(int) + 1);
        sprintf(port1, "%d", port);
        setenv("MCFA_PORT",             port1   , 1);
        free(port1);

        id1 = (char *) malloc (sizeof(int) + 1);
        sprintf(id1, "%d", id);
        setenv("MCFA_ID",               id1     , 1);
        free(id1);

        ehandler1 = (char *) malloc (sizeof(int) + 1);
        sprintf(ehandler1, "%d", ehandler);
        setenv("MCFA_EVENT_HANDLER",    ehandler1, 1);
        free(ehandler1);


        red1 = (char *) malloc (sizeof(int) + 1);
        sprintf(red1, "%d", red);
        setenv("MCFA_REDUNDANCY",       red1, 1);
        free(red1);

        flag1 = (char *) malloc (sizeof(int) + 1);
        sprintf(flag1, "%d", flag);
        setenv("MCFA_SPAWN_FLAG",             flag1   , 1);
        free(flag1);

	flag2 = (char *) malloc (sizeof(int) + 1);
        sprintf(flag2, "%d", cluster_flag);
        setenv("MCFA_CLUSTER_FLAG",           flag2   , 1);
        free(flag2);

        return 1;
}

