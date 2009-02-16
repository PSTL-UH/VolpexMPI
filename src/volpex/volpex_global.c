#include "mpi.h"

extern int SL_this_procid;
Global_Map **GM=NULL;
Tag_Reuse *sendtagreuse=NULL;
Tag_Reuse *recvtagreuse=NULL;
Hidden_Data *hdata=NULL;
Request_List *reqlist=NULL;

NODEPTR head, insertpt, curr;

int GM_numprocs;
int redundancy;
char fullrank[16];
char *hostip=NULL;
char *hostname;
int GM_numprocs;
int next_avail_comm;
int request_counter = 0;

void GM_allocate_global_data ( void ) 
{
    int i;

    sendtagreuse = (Tag_Reuse *) malloc ( sizeof(Tag_Reuse) * TAGLISTSIZE );
    recvtagreuse = (Tag_Reuse *) malloc ( sizeof(Tag_Reuse) * TAGLISTSIZE );
    reqlist      = (Request_List *) malloc ( sizeof(Request_List) * REQLISTSIZE );
    hdata        = (Hidden_Data *) malloc ( sizeof(Hidden_Data ) * TOTAL_COMMS);
    GM           = (Global_Map **) malloc ( sizeof (Global_Map *) * TOTAL_NODES);
    if ( NULL == sendtagreuse || NULL == recvtagreuse ||
	 NULL == reqlist      || NULL == hdata        ||
	 NULL == GM ) {
	printf("could not allocate global arrays\n");
	return;
    }
 
    for ( i=0; i<TOTAL_NODES; i++ ) {
	GM[i] = (Global_Map *) malloc ( sizeof (Global_Map) * TOTAL_COMMS );
	if ( NULL == GM[i] ) {
	    printf("Could not allocate global array GM[%d}\n", i );
	    return;
	}
    }

    return;
}

void GM_free_global_data ( void )
{
    int i;
    
    for ( i=0; i<TOTAL_NODES; i++ ) {
	if ( NULL != GM[i] ) {
	    free ( GM[i] );
	}
    }

    if ( NULL != sendtagreuse ) {
	free ( sendtagreuse );
    }
    if ( NULL != recvtagreuse ) {
	free ( recvtagreuse );
    }
    if ( NULL != reqlist ) {
	free ( reqlist );
    }
    if ( NULL != hdata ) {
	free ( hdata );
    }

    return;
}

void GM_host_ip(void)
{
	char thostname[512];
	const char *tmp=NULL;
	char **pptr;

	char str[INET_ADDRSTRLEN];
	struct hostent *hptr;

#ifdef MINGW
  	WORD wVersionRequested;
  	WSADATA wsaData;
  	wVersionRequested = MAKEWORD(1, 1);
  	if (WSAStartup(wVersionRequested,&wsaData)){
     		printf("\nWSAStartup() failed");
     		exit(1);
  	}
#endif
	
	gethostname(thostname, 512);
	hptr = gethostbyname(thostname);
	if ( NULL == hptr ) {
	    printf("failed to get host structure\n");
	    return;
	}

	pptr = hptr->h_addr_list;
	tmp = inet_ntop ( hptr->h_addrtype, *pptr, str,  sizeof(str));

	hostname = strdup (thostname );
	hostip   = strdup ( str );

	return;
}

int GM_print(int comm)
{
	int i;

	for ( i=0; i< GM_numprocs; i++ ){
		if(GM[i][comm].id != -1)
			printf("GM[%d][%d]: id %d host %s port %d rank %s state %d\n", 
			       i, comm, GM[i][comm].id, GM[i][comm].host, GM[i][comm].port, 
			       GM[i][comm].rank, GM[i][comm].state);
	}
	return 0;
}

int GM_proc_read_and_set (void)
{
  	FILE *fp;
  	int ret, i, j;
  	char host[80];
  	int port;
	int comm = 1;
  	int id;
  	char redrank[16];

  	fp = fopen ("SL.config", "r");
  	if ( NULL == fp ) {
	    printf ("Could not open configuration file SL.config\n");
	    exit ( -1 );
  	}
  	fscanf (fp, "%d", &GM_numprocs);
  	PRINTF(("Total number of processes: %d\n", GM_numprocs ));
	fscanf (fp, "%d", &redundancy);
	if(GM_numprocs % redundancy != 0){
	    printf("Total node vs. redundancy is not correct!\n");
	    exit(0);
  	}
  	for(i=0; i< GM_numprocs; i++) {
	    for(j = 0; j < TOTAL_COMMS; j++){
		GM[i][j].id = -1;
	    }
	    ret = fscanf ( fp, "%d %s %d", &id, host, &port );
	    if ( EOF == ret ) {
		printf("Configuration file does not have the requested number of entries\n");
		exit (0);
	    }
	    GM[i][comm].id = id;
	    strncpy(GM[i][comm].host, host, 32);
	    GM[i][comm].port = port;
	    if(id < GM_numprocs/redundancy)
		sprintf(redrank, "%d,A", i);
	    else if(GM_numprocs/redundancy <= id && id < GM_numprocs/redundancy*2)
		sprintf(redrank, "%d,B", i-GM_numprocs/redundancy);
	    else
		sprintf(redrank, "%d,C", i-GM_numprocs/redundancy*2);
	    strncpy(GM[i][comm].rank,redrank,16);
	    GM[i][comm].state = VOLPEX_PROC_CONNECTED;
	    for(j = 0; j < TOTAL_COMMS; j++){
		GM[i][j].state = VOLPEX_PROC_CONNECTED;
	    }
	    PRINTF(("GM[%d][%d]: id %d host %s port %d rank %s state %d\n", i, comm, GM[i][comm].id, 
		    GM[i][comm].host, GM[i][comm].port, GM[i][comm].rank, GM[i][comm].state));
  	}
  	fclose(fp);
	
  	return 0;
}

int GM_get_fullrank(char *myredrank)
{
	int i;
	int comm = 1;

	for ( i=0; i< GM_numprocs; i++ )
		if(GM[i][comm].id  == SL_this_procid){
			if(strcmp(hostip,GM[i][comm].host) == 0)
				strcpy(myredrank, GM[i][comm].rank);
			else {
				printf("HostIP does not match for this proc ID - SL.config is incorrect!\n");
				exit(-1);
			}
		}
	return 0;
}

int GM_get_procid_fullrank(char *myredrank)
{
	int i;
	int comm = 1;

	for ( i=0; i< GM_numprocs; i++ ){
		if(strcmp(hostip,GM[i][comm].host) == 0){
			strcpy(myredrank, GM[i][comm].rank);
			return GM[i][comm].id;
		}
	}
	return -1;
}

int GM_dest_src_locator(int rank, int comm, char *myfullrank, int tar[3])
{
	int i, numeric, mynumeric;
	char level, mylevel;

	sscanf(myfullrank, "%d,%c", &mynumeric, &mylevel);
	for ( i=0; i< GM_numprocs; i++ ){
		if(GM[i][comm].state == VOLPEX_PROC_CONNECTED){
			sscanf(GM[i][comm].rank, "%d,%c", &numeric, &level);
			if(numeric == rank && level == mylevel){
				tar[0] = GM[i][comm].id;
			}
			else if(numeric == rank && level != mylevel){
				if(redundancy == 2){
					if((mylevel == 'A' && level == 'B') || (mylevel == 'B' && level == 'A'))
						tar[1] = GM[i][comm].id;
				}
				if(redundancy == 3){
					if((mylevel == 'A' && level == 'B') || (mylevel == 'B' && level == 'C') || (mylevel == 'C' && level == 'A'))
						tar[1] = GM[i][comm].id;
					if((mylevel == 'A' && level == 'C') || (mylevel == 'B' && level == 'A') || (mylevel == 'C' && level == 'B'))
						tar[2] = GM[i][comm].id;
				}
			}
		}
	}
	return 0;
}

int GM_set_state_not_connected(int target)
{
	int i, j;
	int comm = 1;

	for ( i=0; i < GM_numprocs; i++ ){
		if(GM[i][comm].id  == target){
			for(j = 0; j < TOTAL_COMMS; j++){
				GM[i][j].state = VOLPEX_PROC_NOT_CONNECTED;
			}
		}
	}
	return 0;
}

NODEPTR VolPex_send_buffer_init()
{
	NODEPTR newnode;
	int i;
	
	for (i = 1; i <= SENDBUFSIZE; i++){
		newnode= (NODE *)malloc(sizeof(NODE));
		newnode->counter = i;

		newnode->header    = VolPex_init_msg_header();
/*		newnode->header[0] = -1;
		newnode->header[1] = -1;
		newnode->header[2] = -1;
		newnode->header[3] = -1;
		newnode->header[4] = -1;*/
		newnode->reqnumbers[0] = -1;
		newnode->reqnumbers[1] = -1;
		newnode->reqnumbers[2] = -1;
		newnode->buffer = NULL;
		if(i == 1)
			head = curr = newnode;
			newnode->back = NULL;
			newnode->fwd = NULL;
		if(i > 1 && i < SENDBUFSIZE){
			curr->fwd = newnode;
			newnode->back = curr;
			newnode->fwd = NULL;
			curr = curr->fwd;
		}
		if(i == SENDBUFSIZE){
			curr->fwd = newnode;
			newnode->back = curr;
			newnode->fwd = head;
			curr = curr->fwd;
			head->back = curr;
		}
	}
     	
	return head;
}

void VolPex_send_buffer_delete()
{
   	NODEPTR tempPtr;
	
	while(head != NULL){	
        tempPtr = head;
		VolPEx_Cancel_byReqnumber(tempPtr->reqnumbers[0]);
		VolPEx_Cancel_byReqnumber(tempPtr->reqnumbers[1]);
		VolPEx_Cancel_byReqnumber(tempPtr->reqnumbers[2]);
		if(head->counter != SENDBUFSIZE){	
         		head = head->fwd;	
         		free(tempPtr); 
		}
		else
			break;     
   	}
      PRINTF(("Buffer deleted\n"));
}

NODEPTR VolPex_send_buffer_insert(NODEPTR currinsertpt, VolPex_msg_header *header, int new_reqs[3], void *buf)
//NODEPTR VolPex_send_buffer_insert(NODEPTR currinsertpt, int header[5], int new_reqs[3], void *buf)
{
	char *tmpbuf=NULL;
	
	PRINTF(("currinsertpt->counter = %d\n", currinsertpt->counter));
	
/*	currinsertpt->header[0] = header[0];
	currinsertpt->header[1] = header[1];
	currinsertpt->header[2] = header[2];
	currinsertpt->header[3] = header[3];
	currinsertpt->header[4] = header[4];*/

	currinsertpt->header = VolPex_get_msg_header(header->len,header->dest,header->tag,header->comm,header->reuse);
	
	VolPEx_Cancel_byReqnumber(currinsertpt->reqnumbers[0]);
	currinsertpt->reqnumbers[0] = new_reqs[0];
	VolPEx_Cancel_byReqnumber(currinsertpt->reqnumbers[1]);
	currinsertpt->reqnumbers[1] = new_reqs[1];
	VolPEx_Cancel_byReqnumber(currinsertpt->reqnumbers[2]);
	currinsertpt->reqnumbers[2] = new_reqs[2];
		
/*	if ( header[0] > 0 ) {
		tmpbuf  = (char *) malloc ( header[0]);
		memcpy ( tmpbuf, buf, header[0]);
	}*/

	if ( header->len > 0 ) {
                tmpbuf  = (char *) malloc ( header->len);
                memcpy ( tmpbuf, buf, header->len);
        }
	currinsertpt->buffer = tmpbuf;

	return currinsertpt->fwd;
}

NODEPTR VolPex_send_buffer_search(NODEPTR currpt, VolPex_msg_header *header, int *answer)
//NODEPTR VolPex_send_buffer_search(NODEPTR currpt, int header[5], int *answer)

{
	int search_count = 1;
	NODEPTR curr=currpt;
//	while(search_count <= SENDBUFSIZE && currpt != NULL){
	do {
/*		if(curr->header[0] <= header[0] && curr->header[1] == header[1] && 
		   curr->header[2] == header[2] && curr->header[3] == header[3] && 
		   curr->header[4] == header[4]){*/

		if( VolPex_compare_msg_header(curr->header, header)){
			*answer = 1; /*yes*/
			PRINTF(("Found msg at curr->counter = %d\n", curr->counter));
			return curr;
		}
		curr = curr->back; /*search backwards since loaded forward*/
		search_count++;
	} while ( curr != currpt );
	
	*answer = 0; /*no*/
	if(currpt == NULL)
		return head;
	return currpt;
}

void VolPex_send_buffer_print(NODEPTR head)
{
   	NODEPTR printPtr = head;

   	while(printPtr != NULL){
		printf("%d ", printPtr->counter);
        printf("currpt->header = %d,%d,%d,%d,%d\n", printPtr->header->len, printPtr->header->dest, 
	       printPtr->header->tag, printPtr->header->comm, printPtr->header->reuse);
        printPtr = printPtr->fwd;
		if(printPtr == head){
			break;
		}
    }
}

int VolPex_tag_reuse_check(int tag, int type)
{
	int i;

	if(type == 0){    /* send tags */
		for(i = 0; i < TAGLISTSIZE; i++){
			if(sendtagreuse[i].tag == tag){
				sendtagreuse[i].reuse_count += 1;
				return sendtagreuse[i].reuse_count;
			}
			if(sendtagreuse[i].tag == -1){
				sendtagreuse[i].tag = tag;
				sendtagreuse[i].reuse_count = 1;
				return sendtagreuse[i].reuse_count;
			}
		}
	}
	if(type == 1){    /* recv tags */
		for(i = 0; i < TAGLISTSIZE; i++){
			if(recvtagreuse[i].tag == tag){
				recvtagreuse[i].reuse_count += 1;
				return recvtagreuse[i].reuse_count;
			}
			if(recvtagreuse[i].tag == -1){
				recvtagreuse[i].tag = tag;
				recvtagreuse[i].reuse_count = 1;
				return recvtagreuse[i].reuse_count;
			}
		}
	}
	return -1;
}

int VolPex_get_len(int count, MPI_Datatype datatype)
{
	int len = 0;
	
	if(datatype == MPI_BYTE)
	    len = count;
	else if(datatype == MPI_INT || datatype == MPI_INTEGER)
	    len = count*sizeof(int);
    else if(datatype == MPI_FLOAT || datatype == MPI_REAL)
        len = count*sizeof(float);
    else if(datatype == MPI_DOUBLE || datatype == MPI_DOUBLE_PRECISION)
	    len = count*sizeof(double);
	else if(datatype == MPI_DOUBLE_COMPLEX)
		len = count*sizeof(double _Complex);
	else
		printf("MPI_Datatype possibly incorrect.\n");
	return len;
}

