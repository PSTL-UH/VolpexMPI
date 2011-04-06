#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"

extern int SL_this_listensock;
extern fd_set SL_send_fdset;
extern fd_set SL_recv_fdset;
extern int SL_this_procport;
extern SL_array_t *SL_proc_array;
extern int SL_numprocs;
extern int SL_this_procid;

int MCFA_connect(int id)
{

    char *hostname = NULL;
    int len=256;
    SL_proc *dproc = NULL;
    int tmp,ret;

    hostname = (char*)malloc(256 * (sizeof(char)));
    if(hostname == NULL){
        printf("ERROR: in allocating memory\n");
        exit(-1);
    }

    if ( gethostname(hostname, len ) != 0 ){
        MCFA_printf("SERVER: could not determine my own hostname \n" );
    }


    SL_this_procid = id ;//MCFA_CONSTANT_ID;


    dproc = SL_array_get_ptr_by_id ( SL_proc_array, MCFA_MASTER_ID );
    SL_open_socket_conn ( &dproc->sock, dproc->hostname, dproc->port );
    dproc->state = SL_PROC_CONNECTED;
    /* set the read and write fd sets */
    FD_SET ( dproc->sock, &SL_send_fdset );
    FD_SET ( dproc->sock, &SL_recv_fdset );
    if ( dproc->sock > SL_fdset_lastused ) {
        SL_fdset_lastused = dproc->sock;
    }

    if ( dproc->sock > SL_fdset_lastused ) {
        SL_fdset_lastused = dproc->sock;
    }

    ret = SL_socket_read ( dproc->sock, ( char *) &tmp, sizeof(int),
                           dproc->timeout);
    if ( SL_SUCCESS != ret ) {
        return ret;
    }

    ret = SL_socket_write ( dproc->sock, (char *) &SL_this_procid, sizeof(int),
                            dproc->timeout );


    PRINTF(("Reciving newID\n"));
/*    ret = SL_socket_write ( dproc->sock, (char *) &SL_this_procport, sizeof(int),
                            dproc->timeout );
    if ( SL_SUCCESS != ret ) {
        return ret;
    }*/
    ret = SL_socket_write ( dproc->sock,  hostname, MAXHOSTNAMELEN,dproc->timeout );
    if ( SL_SUCCESS != ret ) {
        return ret;
    }

    free(hostname);
	return MCFA_SUCCESS;
}

int MCFA_connect_stage2()
{
	int ret, newid, port;
	SL_proc *dproc = NULL;

	dproc = SL_array_get_ptr_by_id ( SL_proc_array, MCFA_MASTER_ID );
    ret = SL_socket_read ( dproc->sock, ( char *) &newid, sizeof(int),
                           -1);
    if ( SL_SUCCESS != ret ) {
        return ret;
    }

    ret = SL_socket_read ( dproc->sock, ( char *) &port, sizeof(int),
                           -1);
    if ( SL_SUCCESS != ret ) {
        return ret;
    }

    SL_this_procport = port;
    return newid;

}
