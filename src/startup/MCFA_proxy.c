/*
#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
*/
#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"


extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;
//extern int SL_init_numprocs;
//extern char        hostname[MAXHOSTNAMELEN];

int main(int argc, char *argv[])
{
    int         i=0;                    //intializing loop iterators
    int         port,len=MAXHOSTNAMELEN;
    char  	*msgbuf = NULL;
    int  msglen;
    char *hostname;
    int spawnflag;

    hostname = strdup(argv[1]);
    port     = atoi(argv[2]);
    spawnflag= atoi(argv[3]);
//    MCFA_printf_init1(0,-2);

    SL_array_init ( &(SL_proc_array), "SL_proc_array", 32 );
    FD_ZERO( &SL_send_fdset );
    FD_ZERO( &SL_recv_fdset );

    SL_this_procport = 6262;
    if ( gethostname(hostname, len ) != 0 ) {
        printf("SERVER: could not determine my own hostname \n" );
    }


    SL_proc_init ( MCFA_PROXY_ID, hostname, SL_this_procport );
    SL_this_procid = MCFA_PROXY_ID;
    SL_init_internal();

    SL_proc_init ( MCFA_MASTER_ID, hostname, port );

	/* Determine the length of the message to be received */

    SL_Recv ( &msglen, sizeof(int), MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL );

    /* Receive the buffer containing all the information */
    msgbuf = (char *) malloc ( msglen );
    if ( NULL == msgbuf ) {
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }
    SL_Recv ( msgbuf, msglen, MCFA_MASTER_ID, 0, 0, SL_STATUS_NULL);

    SL_numprocs = MCFA_proc_read_init(msgbuf,msglen);
    int sbuf=1;
    for(i=0; i<SL_numprocs; i++)
	SL_Send(&sbuf, sizeof(int), i, 0, 0);

    SL_numprocs ++;
    printf(" MY ID IS :::::::::::::::::::::::::::::%d\n",SL_this_procid);
    SL_proc_dumpall();
	i=0;
	printf("[%d]:SL_proxy_numprocs:%d\n", SL_this_procid, SL_proxy_numprocs);
	while(1){

		if(SL_proxy_numprocs<=0)
			break;
		SL_msg_progress();


	}

	SL_Finalize();

	return 1;
}
