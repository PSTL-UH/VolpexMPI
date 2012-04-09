#include "mpi.h"
#include "SL_proc.h"

extern SL_array_t *SL_proc_array;
extern SL_array_t *Volpex_proc_array;
extern SL_array_t *Volpex_comm_array;
extern int SL_this_procid;

extern NODEPTR head, insertpt, curr;

extern int Volpex_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char *hostip;
extern char *hostname;
extern int next_avail_comm;
extern int request_counter;
extern int Volpex_numcomms;
extern Volpex_returnheaderlist *returnheaderList;

extern Max_tag_reuse *maxtagreuse;
Volpex_dest_source_fnct *Volpex_dest_source_select;
Volpex_target_list *Volpex_targets;
double init_msg_time;
double repeat_msg_time;
extern int Volpex_this_procid;

static void volpex_preconnect(void)
{

    int step;
    int size = Volpex_numprocs;
    int rank = SL_this_procid;
    int sbuf=1, rbuf=1;
//    SL_Request *reqs;
    SL_Request reqs[2];
    int sendrecv;



	for ( step=0; step< size; step++ ) {
                sendrecv = (size-rank+step)%size;
		PRINTF(("[%d]: communicating with proc:%d iteration:%d\n", SL_this_procid, sendrecv, step));
		if (SL_this_procid == sendrecv)
                        continue;
                if (SL_this_procid<sendrecv){
                        SL_Isend (&sbuf, sizeof(int), sendrecv, 0, 0, &reqs[0] );
                        SL_Irecv (&rbuf, sizeof(int), sendrecv, 0, 0, &reqs[1]);
                }
                else{
                        SL_Irecv (&rbuf, sizeof(int), sendrecv, 0, 0, &reqs[1]);
                        SL_Isend (&sbuf, sizeof(int), sendrecv, 0, 0, &reqs[0] );
                }

		SL_Waitall ( 2, reqs, SL_STATUS_IGNORE);
	}

    return;
}

int numofcores()
{
        system ("cat /proc/cpuinfo | grep processor > tempfile.txt");
        FILE *fp;
        char procnum[30];
        int num=0;

        fp = fopen("tempfile.txt","r");
        if(fp == NULL)
        {
                printf("Source file not found !\n");
                exit(-1);
        }

        while (!feof(fp))
        {
                 fgets(procnum,30,fp);
                num++;
        }
        printf("Number of cores = %d\n",num-1);
        return (num-1);

}


int  MPI_Init( int *argc, char ***argv )
{
    int i;
    next_avail_comm = 3;
    int maxrank;
    Volpex_numcomms = 0;
    PRINTF(("[%d]:Moving into MCFA_Init\n",SL_this_procid));
//    MCFA_Init(*argc, *argv);
    MCFA_Init();
    GM_allocate_global_data();
    GM_host_ip();
    PRINTF(("[%d]:Hostname: %s\n", SL_this_procid, hostname));
    PRINTF(("[%d]:HostIP: %s\n", SL_this_procid, hostip));
    Volpex_get_fullrank(fullrank);

/** max rank is needed in case we add a new process
	total number of process = Volpex_numprocs
	but total mpiprocesses != Volpex_numprocs/redundancy
	    total mpiprocesses = maximum mpi rank
**/

    maxrank = Volpex_get_max_rank();
    // Set up MPI_COMM_WORLD and MPI_COMM_SELF
//    Volpex_init_comm_world ( Volpex_numprocs, redundancy );


    Volpex_init_comm_world ( maxrank+1 );
    Volpex_init_comm_self ();

//    Volpex_init_procplist(redundancy);
    Volpex_init_procplist();
    Volpex_init_returnheader(&returnheaderList);
    Volpex_init_maxreuse(&maxtagreuse);
    Volpex_init_targetlist();
//	Volpex_print_procplist();
    for(i = 0; i < REQLISTSIZE; i++){
        reqlist[i].insrtbuf = NULL;
	reqlist[i].returnheader.len = -1;
	reqlist[i].returnheader.reuse = -1;
	reqlist[i].returnheader.tag = -1;
	reqlist[i].returnheader.dest = -1;
	reqlist[i].returnheader.src = -1;
	reqlist[i].numtargets = 0;
	reqlist[i].recv_dup_status = 0;
	reqlist[i].header = NULL;
	reqlist[i].buffer = NULL;
	reqlist[i].assoc_recv = NULL;
    }

    GM_tagreuse_init();

    PRINTF(("[%d]:My full rank is %s\n", SL_this_procid,fullrank));
    head = insertpt = curr = Volpex_send_buffer_init();
    Volpex_dest_source_select = Volpex_dest_src_locator;

   volpex_preconnect();
//   Volpex_Redundancy_Barrier ( MPI_COMM_WORLD, hdata[MPI_COMM_WORLD].myrank );
//if(redundancy>1)
 //  Volpex_set_target();
//    Volpex_Barrier ( MPI_COMM_WORLD);
//	Volpex_Complete_Barrier(MPI_COMM_WORLD);	
    init_msg_time = SL_papi_time();
    repeat_msg_time = SL_papi_time();
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

#pragma weak MPI_Finalize = PMPI_Finalize

int  PMPI_Finalize()
{
    return Volpex_Finalize();
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Comm_size = PMPI_Comm_size

int PMPI_Comm_size(MPI_Comm comm, int *size)
{
    return Volpex_Comm_size(comm, size);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Comm_rank = PMPI_Comm_rank

int PMPI_Comm_rank(MPI_Comm comm, int *rank)
{
    return Volpex_Comm_rank(comm, rank);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Send = PMPI_Send

int  PMPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm)
{
    return Volpex_Send(buf, count, datatype, dest, tag, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

#pragma weak MPI_Recv = PMPI_Recv

int  PMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
              MPI_Comm comm, MPI_Status *status)
{
    return Volpex_Recv(buf, count, datatype, source, tag, comm, status);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Bcast = PMPI_Bcast

int  PMPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
               MPI_Comm comm)
{
    return Volpex_Bcast(buf, count, datatype, root, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Reduce = PMPI_Reduce

int PMPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm)
{
    return Volpex_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Allreduce = PMPI_Allreduce

int PMPI_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                  MPI_Op op, MPI_Comm comm)
{
    return Volpex_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Isend = PMPI_Isend

int  PMPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
               MPI_Comm comm, MPI_Request *request)
{
    return Volpex_Isend(buf, count, datatype, dest, tag, comm, request);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Irecv = PMPI_Irecv

int  PMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
               MPI_Comm comm, MPI_Request *request)
{
    return Volpex_Irecv(buf, count, datatype, source, tag, comm, request);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Waitall = PMPI_Waitall

int  PMPI_Waitall(int count, MPI_Request request[], MPI_Status status[])
{
    return Volpex_Waitall(count, request, status);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Waitany = PMPI_Waitany

int  PMPI_Waitany(int count, MPI_Request request[], int *index, MPI_Status status[])
{
    return Volpex_Waitany (count, request, index, status);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Wait = PMPI_Wait

int  PMPI_Wait(MPI_Request *request, MPI_Status *status)
{
    return Volpex_Wait(request, status);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Test = PMPI_Test

int  PMPI_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
    return Volpex_Test(request, flag, status);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Barrier = PMPI_Barrier

int PMPI_Barrier(MPI_Comm comm)
{
    return Volpex_Barrier(comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int MPI_Abort(MPI_Comm comm, int errorcode)
{
    printf("Error %d occured on context_id %d. Aborting.\n", errorcode, comm);
    exit(errorcode);
    return MPI_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
double MPI_Wtime()
{
    struct timeval tp;
    double sec=0.0;
    double psec=0.0;

    gettimeofday( &tp, NULL );
    sec = (double)tp.tv_sec;
    psec = ((double)tp.tv_usec)/((double)1000000.0);
    return (sec+psec);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Alltoall = PMPI_Alltoall

int  PMPI_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
                  int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    return Volpex_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Alltoallv = PMPI_Alltoallv

int  PMPI_Alltoallv(void *sendbuf, int *sendcount, int *sdispls, MPI_Datatype sendtype,
                   void *recvbuf, int *recvcount, int *rdispls, MPI_Datatype recvtype,
                   MPI_Comm comm)
{
    return Volpex_Alltoallv(sendbuf, sendcount, sdispls, sendtype, recvbuf, recvcount, rdispls,
                            recvtype, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Comm_dup = PMPI_Comm_dup

int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    return Volpex_Comm_dup(comm, newcomm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Comm_split = PMPI_Comm_split

int  PMPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
    return Volpex_Comm_split(comm, color, key, newcomm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Gather = PMPI_Gather

int  PMPI_Gather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf,
                int recvcnt, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    return Volpex_Gather(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Allgather = PMPI_Allgather

int PMPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
                  int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    return Volpex_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Pack = PMPI_Pack

int PMPI_Pack(void *inbuf, int count, MPI_Datatype datatype, void *outbuf,
                  int outsize, int* position, MPI_Comm comm)
{
    return Volpex_Pack(inbuf, count, datatype, outbuf, outsize, position, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Unpack = PMPI_Unpack

int PMPI_Unpack(void *inbuf, int insize, int* position, void *outbuf,
                  int outcount, MPI_Datatype datatype, MPI_Comm comm)
{
    return Volpex_Unpack(inbuf, insize, position, outbuf, outcount, datatype, comm);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#pragma weak MPI_Type_size = PMPI_Type_size

int PMPI_Type_size ( MPI_Datatype datatype, int *size )
{
    return Volpex_Type_size ( datatype, size );
}

int MPI_rank (int *actualrank)
{
        *actualrank = SL_this_procid;
        return 1;
}


