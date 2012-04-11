#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

#include "mpi.h"

extern int SL_this_procid;
extern SL_array_t *Volpex_proc_array;
SL_array_t *Volpex_maxreuse_array;



Global_Map **GM=NULL;
Tag_Reuse **sendtagreuse=NULL;
Tag_Reuse **recvtagreuse=NULL;
Hidden_Data *hdata=NULL;
Request_List *reqlist=NULL;

NODEPTR head, insertpt, curr;
Max_tag_reuse *maxtagreuse=NULL;

int Volpex_numprocs;
int redundancy;
char fullrank[16];
char *hostip=NULL;
char *hostname;
int next_avail_comm;
int request_counter = 0;

/* Value maintaining the size of the main memory utilized by the buffer */
long memusage=0;

int Volpex_init_maxreuse(Max_tag_reuse **maxtagreuse)
{
    *maxtagreuse = NULL;
    return MPI_SUCCESS;
}

int Volpex_add_maxreuse(Max_tag_reuse **maxtagreuse, Volpex_msg_header *header)
{
    Volpex_msg_header *theader;
    Max_tag_reuse *next;
    Max_tag_reuse *curr;
    Max_tag_reuse *prev;
    
    theader = (Volpex_msg_header*)malloc (sizeof(Volpex_msg_header));
    if ( NULL == theader ) {
        return SL_ERR_NO_MEMORY;
    }
    
    theader->len = header->len;
    theader->dest = header->dest;
    theader->src = header->src;
    theader->tag = header->tag;
    theader->comm = header->comm;
    theader->reuse = header->reuse;
    
    curr = (Max_tag_reuse *) malloc (sizeof (Max_tag_reuse));
    if (NULL == curr){
	return SL_ERR_NO_MEMORY;
    }
    
    
    curr->next=NULL;
    curr->header = theader;
    
    if (*maxtagreuse == NULL)
    {
	*maxtagreuse=curr;
    }
    else
    {
	next=*maxtagreuse;
	prev=NULL;
	
      	while(next != NULL )
      	{
	    prev=next;
	    next=next->next;
      	}
	/* Now have a spot to insert */
	if (prev == NULL)
	{
	    *maxtagreuse=curr;
	    curr->next=next;
	}
	else
	{
	    curr->next = prev->next;
	    prev->next=curr;
	}
    }
    return MPI_SUCCESS;
}


int Volpex_insert_maxreuse(Max_tag_reuse *maxtagreuse,Volpex_msg_header *header)
{
    Max_tag_reuse *curr = maxtagreuse;
    int flag = 0;
    while(curr!=NULL)
    {
	if ((curr->header->dest == header->dest)&&
	    (curr->header->src == header->src) &&
	    (curr->header->tag == header->tag)&&
	    (curr->header->comm == header->comm)){
	    curr->header->reuse = header->reuse;
	    flag = 1;
	}
        
	curr = curr ->next;
    }
    
    return flag;
    
}

int Volpex_search_maxreuse(Max_tag_reuse *maxtagreuse,Volpex_msg_header header)
{
    Max_tag_reuse *curr = maxtagreuse;
    int maxreuse=-1;
    
    while(curr!=NULL)
    {
	if ((curr->header->dest == header.dest)&&
	    (curr->header->src == header.src) &&
	    (curr->header->tag == header.tag)&&
	    (curr->header->comm == header.comm)){
	    maxreuse = curr->header->reuse ;
	}
	
	curr = curr ->next;
    }
    
    return maxreuse;
    
    
}

void GM_tagreuse_init (void)
{
    int i, j;

    sendtagreuse = (Tag_Reuse **) malloc ( sizeof(Tag_Reuse *) * Volpex_numprocs );
    recvtagreuse = (Tag_Reuse **) malloc ( sizeof(Tag_Reuse *) * Volpex_numprocs );
    if ( NULL == sendtagreuse || NULL == recvtagreuse ) {
	printf("GM_tagreuse_init: could not allocate memory\n");
	exit (-1);
    }
    
    for ( i=0; i<Volpex_numprocs; i++ ) {
	sendtagreuse[i] = (Tag_Reuse *) malloc ( sizeof (Tag_Reuse) * TAGLISTSIZE );
	recvtagreuse[i] = (Tag_Reuse *) malloc ( sizeof (Tag_Reuse) * TAGLISTSIZE );
	if ( NULL == sendtagreuse[i] || NULL == recvtagreuse[i] ) {
	    printf("GM_tagreuse_init: could not allocate memory\n");
	    exit (-1);
	}
	for ( j=0; j<TAGLISTSIZE; j++ ) {
	    sendtagreuse[i][j].tag = -1;
	    recvtagreuse[i][j].tag = -1;
	}
    }
	SL_array_init(&(Volpex_maxreuse_array), "Volpex_maxreuse_array",32);
    return;
}

void GM_free_global_data ( void )
{
    int i;
    

    Max_tag_reuse *curr = NULL, *currnext = NULL;
    curr = maxtagreuse;
    if (maxtagreuse !=NULL){
        do{
	    currnext = curr->next;
	    if(NULL != curr->header)
		free(curr->header);
	    free(curr);
	    curr = currnext;
	    
        }while(curr !=NULL);
    }
    
    
    for ( i=0; i<Volpex_numprocs; i++ ) {
	if ( NULL != sendtagreuse[i] ) {
	    free ( sendtagreuse[i] );
	}
	if ( NULL != recvtagreuse[i] ) {
	    free ( recvtagreuse[i] );
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




void GM_allocate_global_data ( void )
{

    reqlist = (Request_List *) malloc ( sizeof(Request_List) * REQLISTSIZE );
    hdata   = (Hidden_Data *) malloc ( sizeof(Hidden_Data ) * TOTAL_COMMS);
    if ( NULL == reqlist      || NULL == hdata  ) {
        printf("could not allocate global arrays\n");
        return;
    }


    return;
}


int Volpex_print(int comm)
{
	int i;
	int size;
	Volpex_proc *proc;
	size = SL_array_get_last(Volpex_proc_array ) + 1;
        for(i=0; i<size; i++){
                proc = (Volpex_proc*) SL_array_get_ptr_by_pos (Volpex_proc_array, i);
                if ( NULL == proc ) {
                   continue;
                }
		printf("proc : id %d host %s port %d rank %s state %d\n",
			proc->id, proc->hostname, proc->port, proc->rank, proc->state);
	}
	return 0;
}

NODEPTR Volpex_send_buffer_init()
{
	NODEPTR newnode;
	int i,j;
	
	for (i = 1; i <= SENDBUFSIZE; i++){
		newnode= (NODE *)malloc(sizeof(NODE));
		newnode->counter = i;

		newnode->header    = NULL;

		newnode->reqnumbers = (int*)malloc(redundancy * sizeof(int));
		for(j=0;j<redundancy;j++)
			newnode->reqnumbers[j] = -1;
		
	/*	newnode->reqnumbers[0] = -1;
		newnode->reqnumbers[1] = -1;
		newnode->reqnumbers[2] = -1;*/
		newnode->buffer = NULL;
		if(i == 1){
			head = curr = newnode;
			newnode->back = NULL;
			newnode->fwd = NULL;
		}
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

void Volpex_send_buffer_delete()
{
    NODEPTR tempPtr;
    int j;
    
    while(head != NULL){	
	tempPtr = head;
	
	for(j=0;j<redundancy;j++) {
	    if ( tempPtr->reqnumbers[j] > 0 ) {
		Volpex_Cancel_byReqnumber(tempPtr->reqnumbers[j]);
	    }
	}
	if(head->counter != SENDBUFSIZE){	
	    head = head->fwd;	
	    free(tempPtr); 
	}
	else
	    break;     
    }

    PRINTF(("Buffer deleted\n"));
    return;
}
int Volpex_check_sendstatus(int reqnumber)
{
//	int flag;
    int j = reqnumber;

    if ( j < 0 || j >= REQLISTSIZE ) {
        return 0;
    }
	if (reqlist[j].send_status == 2)
		return -1;
	else
		return 1;
}

NODEPTR Volpex_send_buffer_insert(NODEPTR currinsertpt, Volpex_msg_header *header, int *new_reqs, void *buf)
{
	char *tmpbuf=NULL;
	int j;
//	int retstatus;

	PRINTF(("[%d]:currinsertpt->counter = %d\n", SL_this_procid,currinsertpt->counter));

	
	if ( NULL != currinsertpt->header ) {
            /* This element is already in use, free it. */
            PRINTF(("[%d] send_buffer_insert: Overwriting data in buffer %d\n",
		   SL_this_procid, currinsertpt->counter ));




//	    for(j=0;j<redundancy;j++)
			j=0;
                        Volpex_Cancel_byReqnumber(currinsertpt->reqnumbers[j]);

            memusage -= (header->len + sizeof(header));

            free ( currinsertpt->header );
            if ( NULL != currinsertpt->buffer ) {
                free ( currinsertpt->buffer );
            }
        }
	currinsertpt->header = Volpex_get_msg_header(header->len,  header->src, 
						     header->dest, header->tag, 
						     header->comm, header->reuse);


/*	for(j=0;j<redundancy;j++)
		currinsertpt->reqnumbers[j] = new_reqs[j];
*/
        if ( header->len > 0 ) {
                tmpbuf  = (char *) malloc ( header->len);
                if ( NULL == tmpbuf ) {
                    printf("send_buffer_insert: Could not allocate memory of size %d\n",
                           header->len );
                }
                memcpy ( tmpbuf, buf, header->len);
        }
        currinsertpt->buffer = tmpbuf;
	memusage += (header->len + sizeof(header));
	PRINTF(("send_buffer_insert: Currently used main memory: %ld\n", memusage ));

	return currinsertpt;
}

NODEPTR Volpex_send_buffer_search(NODEPTR currpt, Volpex_msg_header *header, int *answer, int *reuseval, int *minreuseval)
//NODEPTR Volpex_send_buffer_search(NODEPTR currpt, Volpex_msg_header *header, int *answer)

{
	int search_count = 1;
	NODEPTR curr=currpt;
	int maxreuseval = 0;	
	int msgprogress;
	*answer = 0; /*no*/
	msgprogress = -1; /*no progress*/




//	PRINTF(("currpt->counter: %d\n", currpt->counter));


/*	
	do {
            if ( NULL != curr->header ) {
                 if( Volpex_compare_msg_progress(curr->header, header, &msgprogress))
//                    printf("msg diff = %d\n", *msgprogress);
			break;
                }
//	    Volpex_insert_reuseval(header->dest, header->comm);
            
            curr = curr->back; 
            search_count++;
        } while ( curr != currpt );
*/
	int flag = 0;
	curr = currpt;
	do {
	    if ( NULL != curr->header ) {
		flag = Volpex_compare_msg_header(curr->header, header, &maxreuseval) ;
		if (maxreuseval > *reuseval)
                        *reuseval = maxreuseval;

		if (maxreuseval<*minreuseval)
			*minreuseval = maxreuseval;
//	        PRINTF(("header->reuse = %d , *reuseval = %d curr->counter = %d\n",header->reuse, *reuseval, curr->counter));

		if( flag){
		    *answer = 1; /*yes*/
		    PRINTF(("Found msg at curr->counter = %d\n", curr->counter));
		    return curr;
		}
	    }
	    curr = curr->back; /*search backwards since loaded forward*/
	    search_count++;
	} while ( curr != currpt );

	
	if(currpt == NULL)
	    return head;
	

	return currpt;
}

void Volpex_send_buffer_print(NODEPTR head)
{
    NODEPTR printPtr = head;
    int i;

	FILE *fp;
	fp = fopen("file1", "w");
   

	printf ("Printing output to file1\n");
    while(printPtr != NULL){
	fprintf(fp, "%d ", printPtr->counter);
        fprintf(fp, "currpt->header = %d,%d,%d,%d,%d\n", printPtr->header->len, printPtr->header->dest, 
	       printPtr->header->tag, printPtr->header->comm, printPtr->header->reuse);
	fprintf(fp, "Reqnum:");
	for(i=0;i<redundancy;i++)
		fprintf(fp, " %d", printPtr->reqnumbers[i]);
	fprintf(fp, "\n");
        printPtr = printPtr->fwd;
	if(printPtr == head){
	    break;
	}
    }
	fclose(fp);
    return;
}

void Volpex_sendbuffer_search_byreq(NODEPTR head, int req)
{
	NODEPTR printPtr = head;
	int j, flag=0;

    while(printPtr != NULL){
	for(j=0;j<redundancy;j++){
		if (printPtr->reqnumbers[j] == req){
	        	printf("%d ", printPtr->counter);
        		printf("currpt->header = %d,%d,%d,%d,%d\n", printPtr->header->len, printPtr->header->dest,
               			printPtr->header->tag, printPtr->header->comm, printPtr->header->reuse);
			flag = 1;
		}
	}
		if (flag==1)
			break;

	        printPtr = printPtr->fwd;
        if(printPtr == head){
            break;
        }
    }

    return;

}

int Volpex_tag_reuse_check(int rank_MCW, int tag, int comm,int type)
{
    int i;
    
    if(type == 0){    /* send tags */
	for(i = 0; i < TAGLISTSIZE; i++){
	    if((sendtagreuse[rank_MCW][i].tag == tag) && 
              (sendtagreuse[rank_MCW][i].comm == comm )){
		sendtagreuse[rank_MCW][i].reuse_count += 1;
		return sendtagreuse[rank_MCW][i].reuse_count;
	    }
	    if(sendtagreuse[rank_MCW][i].tag == -1){
		sendtagreuse[rank_MCW][i].tag = tag;
	        sendtagreuse[rank_MCW][i].comm = comm;
		sendtagreuse[rank_MCW][i].reuse_count = 1;
		return sendtagreuse[rank_MCW][i].reuse_count;
	    }
	}
    }
	
    if(type == 1){    /* recv tags */
	for(i = 0; i < TAGLISTSIZE; i++){
	    if((recvtagreuse[rank_MCW][i].tag == tag) &&
	      (recvtagreuse[rank_MCW][i].comm == comm ) ){
		recvtagreuse[rank_MCW][i].reuse_count += 1;
		return recvtagreuse[rank_MCW][i].reuse_count;
	    }
	    if(recvtagreuse[rank_MCW][i].tag == -1){
		recvtagreuse[rank_MCW][i].tag = tag;
		recvtagreuse[rank_MCW][i].comm = comm;
		recvtagreuse[rank_MCW][i].reuse_count = 1;
		return recvtagreuse[rank_MCW][i].reuse_count;
	    }
	}
    }

printf("Return -1\n\n");
    return -1;
}

int Volpex_get_len(int count, MPI_Datatype datatype)
{
    int len = 0;

    if( (datatype == MPI_BYTE) ||
        (datatype == MPI_CHAR) ||
        (datatype == MPI_CHARACTER) ||
        (datatype == MPI_PACKED)  )
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

