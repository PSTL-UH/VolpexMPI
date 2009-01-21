#include "mpi.h"
#include "SL_msg.h"

extern int SL_this_procid;
extern Global_Map GM[TOTAL_NODES][TOTAL_COMMS];
extern Tag_Reuse sendtagreuse[TAGLISTSIZE];
extern Tag_Reuse recvtagreuse[TAGLISTSIZE];
extern NODEPTR head, insertpt, curr;
extern Request_List reqlist[REQLISTSIZE];
extern int GM_numprocs;
extern int redundancy;
extern char fullrank[16];
extern char hostip[32];
extern char hostname[512];
extern Hidden_Data hdata[TOTAL_COMMS];
extern int GM_numprocs;
extern int next_avail_comm;
extern int request_counter;


/*
** sort-function for MPI_Comm_split 
*/
static int rankkeycompare(const void *, const void *);


int VolPEx_Comm_size(MPI_Comm comm, int *size)
{
	int i, mynumeric, tempsize = 0;
	char mylevel;
	for ( i=0; i< GM_numprocs; i++ ){
		sscanf(GM[i][comm].rank, "%d,%c", &mynumeric, &mylevel);
		if(mynumeric >= tempsize && GM[i][comm].port != 0)
			tempsize = mynumeric + 1;
	}
	hdata[comm].mysize = *size = tempsize;
	PRINTF(("MPI_Comm_size = %d\n", *size));
	return MPI_SUCCESS;
}

int VolPEx_Comm_rank(MPI_Comm comm, int *rank)
{
	int i, mynumeric;
	char mylevel;

	/*if(SL_this_procid == 4)
	  exit(0);*/

	for ( i=0; i< GM_numprocs; i++ ){
		if(GM[i][comm].id == SL_this_procid){
			sscanf(GM[i][comm].rank, "%d,%c", &mynumeric, &mylevel);
			hdata[comm].myrank = *rank = mynumeric;
			PRINTF(("MPI_Comm_rank = %d\n", *rank));
			return MPI_SUCCESS;
		}
	}
	return MPI_SUCCESS;
}

int  VolPEx_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    int Vnextcomm = next_avail_comm;
    int i;
	
    PRINTF(("Into VolPEx_Comm_dup\n"));

    VolPEx_Allreduce(&next_avail_comm, &Vnextcomm, 1, MPI_INT, MPI_MAX, comm);
    PRINTF(("Vnextcomm = %d\n", Vnextcomm));
    //printf("next_avail_comm = %d\n", next_avail_comm);
    *newcomm = Vnextcomm;
    for (i=0; i<GM_numprocs; i++) 
	GM[i][*newcomm] = GM[i][comm];
    hdata[*newcomm] = hdata[comm];
    // GM_print(*newcomm);
    next_avail_comm = Vnextcomm + 1;
    return MPI_SUCCESS;       
}

int VolPEx_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
	int p[3], *procs;
	int i, k, nextcomm = 0;
	int numeric, newsize, newrank;
	char level, myredrank[16];
	int localrank = 0;

	PRINTF(("Into VolPEx_Comm_split with color %d and key %d\n", color, key));

	VolPEx_Allreduce(&next_avail_comm, &nextcomm, 1, MPI_INT, MPI_MAX, comm);
	// printf("Past Allreduce and using nextcomm %d\n", nextcomm);
	next_avail_comm = nextcomm + 1;
	*newcomm = nextcomm;
	
	/* create an array of process information for doing the split */
	procs = (int *) malloc(3 * hdata[comm].mysize * sizeof(int));
	
	/* gather all process information at all processes */
	p[0] = color;
	p[1] = key;
	p[2] = hdata[comm].myrank; /* this is rank from original comm */
	
	VolPEx_Allgather(p, 3, MPI_INT, procs, 3, MPI_INT, comm);
	// printf("Past Allgather and using newcomm %d\n", *newcomm);
	/* locate and count the # of processes having my color */
	
	for ( i=0; i<hdata[comm].mysize; i++ ) {
	  PRINTF(("%d: i=%d color=%d key=%d rank=%d\n", hdata[comm].myrank, 
		  i, procs[3*i], procs[3*i+1], procs[3*i+2] ));
	}

	qsort ( (int *)procs, hdata[comm].mysize, sizeof(int)*3, 
		rankkeycompare );


	for (i = 0; i < hdata[comm].mysize; i++){
	    if(procs[i*3] == color){
		for (k = 0; k < GM_numprocs; k++){
		    sscanf(GM[k][comm].rank, "%d,%c", &numeric, &level);
		    if(numeric == procs[i*3+2]){
			GM[k][*newcomm].id = GM[k][comm].id;
			strcpy(GM[k][*newcomm].host, GM[k][comm].host);
			GM[k][*newcomm].port = GM[k][comm].port;
			
			sprintf(myredrank, "%d,%c", localrank, level);
			PRINTF(("For new comm %d my redrank is %s, id=%d\n", *newcomm, myredrank, GM[k][*newcomm].id ));
			strcpy(GM[k][*newcomm].rank, myredrank);
			localrank++;
		    }
		}
	    }
	}
	
	VolPEx_Comm_size(*newcomm, &newsize);
	hdata[*newcomm].mysize = newsize;
	VolPEx_Comm_rank(*newcomm, &newrank);
	hdata[*newcomm].myrank = newrank; 
	hdata[*newcomm].mybarrier = 0;

	PRINTF(("  VComm_split: rank=%d size=%d for comm_id = %d\n", 
		newrank, newsize, *newcomm));

	// GM_print(*newcomm);


	/* create a new context ID */
	if (color == MPI_UNDEFINED) {
	    /* free all the resources that have been allocated by this process. */
	    *newcomm = MPI_COMM_NULL;
	}

	free ( procs);
	return(MPI_SUCCESS);    
}

/* static functions */
/* 
** rankkeygidcompare() compares a tuple of (rank,key,gid) producing 
** sorted lists that match the rules needed for a MPI_Comm_split 
*/
static int rankkeycompare (const void *p, const void *q)
{
    int *a, *b;
  
    /* ranks at [0] key at [1] */
    /* i.e. we cast and just compare the keys and then the original ranks.. */
    a = (int*)p;
    b = (int*)q;
    
    /* simple tests are those where the keys are different */
    if (a[1] < b[1]) {
        return (-1);
    }
    if (a[1] > b[1]) {
        return (1);
    }
    
    /* ok, if the keys are the same then we check the original ranks */
    if (a[1] == b[1]) {
        if (a[0] < b[0]) {
            return (-1);
        }
        if (a[0] == b[0]) {
            return (0);
        }
        if (a[0] > b[0]) {
            return (1);
        }
    }
    return ( 0 );
}
