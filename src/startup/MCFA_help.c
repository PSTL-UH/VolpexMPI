#include "MCFA.h"
#include "MCFA_internal.h"

/* Function to print details of all options for startproc command similar to man file */
void print_Options()
{
    printf("NAME\n");
    printf("\t ./mcfarun - to start specified number of programs\n\n");
    printf("SYNOPSIS\n");
    printf("\t./mcfarun [options]\n\n");
    printf("DESCRIPTION\n");
    printf("\t The mcfarun command is used used to start number of client processes specified by the user. \n\
n");
    printf("\t -np,  --np\n");
    printf("\t\t specify total number of client processes to be started. "
		"Total number of processes = # of procs * redundancy\n");
    printf("\t\t For example: if 4 processes are to be started with redundancy 2 then command is\n");
    printf("\t\t ./mcfarun -np 8 -redundancy 2 -hostfile hostfile ./[name of executable]\n\n");

    printf("\t -redundancy, --redundancy\n");
    printf("\t\t Specify the redundancy level for processes\n\n");

    printf("\t -hostfile, --hostfile [name]\n");
    printf("\t\t Specify the name of hostfile\n\n");

    printf("\t -help, --help\n");
    printf("\t\t displays a list of options supported by startprocs\n\n");

    printf("\t -condor, --condor\n");
    printf("\t\t spawn process on condor pool\n");

    printf("\t -boinc, --boinc\n");
    printf("\t\t spawn process on boinc pool\n");
    
    printf("\t -allocation, --allocation [scheme for allocation of processes]\n");
    printf("\t\t Specify how the processes can be allocated using a given hostfile\n");
    printf("\t\t\t\t [roundrobin] - processes are distributed in roundrobin manner for \n");
    printf("\t\t\t\t\t  given list of hosts. This allocation strategy is also default allocation scheme\n");
    printf("\t\t\t\t [concentrate]- processes are distributed such that maximum number "
			"of processes are on one  host\n");
    printf("\t\t\t\t [straight] -   processes are distributed as hosts are given in "
			"hostfile without changing the order of hosts\n\n");

    printf("\t -cluster, --cluster [criterion to cluster given set of nodes]\n");
    printf("\t\t Specify how the nodes should be clustered \n");
    printf("\t\t\t\t [communication] - nodes are clustered based on their communication distance\n");
    printf("\t\t\t\t [ipaddress] - nodes are clustered based on their ip addresses\n");
}
