

#include"MCFA.h"
#include "MCFA_internal.h"



char* MCFA_pack_proclist(struct MCFA_proc_node *procList, int *msglen)
{
        struct MCFA_proc_node *curr = procList;

        char *buffer = NULL;
	
	int numprocs = 0;
        int len = 0;
        int pos=0;
	int noofints = 0;
	int noofchars = 0;

	while(curr !=NULL)
	{
		numprocs++;
		curr = curr->next;
	}
	noofints = numprocs * 6;
	noofchars = numprocs * (MAXHOSTNAMELEN + MAXNAMELEN + MAXRANK);
         
	MCFA_pack_size(noofints, noofchars, &len);
        buffer = (char*)malloc(len*sizeof(char));
        if(buffer == NULL){
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }
	
	curr = procList;

        while(curr !=NULL)
        {
                MCFA_pack_int(buffer, &curr ->procdata->id ,1,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->volpex_id ,1,&pos);
                MCFA_pack_string(buffer, curr ->procdata->hostname ,MAXHOSTNAMELEN,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->portnumber ,1,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->jobid ,1,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->sock ,1,&pos);
		MCFA_pack_int(buffer, &curr ->procdata->status ,1,&pos);
		MCFA_pack_string(buffer, curr->procdata->executable, MAXNAMELEN, &pos);
		MCFA_pack_string(buffer,curr->procdata->fullrank, MAXRANK, &pos);
                curr = curr->next;
        }

	PRINTF(("Lenth of processBuffer   %d\n",len));	
        *msglen = len;
	return buffer;
}




struct MCFA_proc_node* MCFA_unpack_proclist(char *buffer, int len)
{

        int id,portnumber,jobid,sock,status,volpex_id;
        char hostname[MAXHOSTNAMELEN], exec[MAXNAMELEN], fullrank[MAXRANK];
        int pos=0,proc_count = 0;
        int i;

        struct MCFA_proc_node *tempList;

        MCFA_initProcList(&tempList);
//      printf("Lenth of processBuffer   %d\n",len);
        for(i=0;i<len;)
        {

                proc_count ++ ;

                MCFA_unpack_int(buffer, &id ,1,&pos);
                MCFA_unpack_int(buffer, &volpex_id ,1,&pos);
                MCFA_unpack_string(buffer, hostname ,MAXHOSTNAMELEN,&pos);
                MCFA_unpack_int(buffer, &portnumber ,1,&pos);
                MCFA_unpack_int(buffer, &jobid ,1,&pos);
                MCFA_unpack_int(buffer, &sock ,1,&pos);
		MCFA_unpack_int(buffer, &status ,1,&pos);
		MCFA_unpack_string(buffer, exec, MAXNAMELEN,&pos);
		MCFA_unpack_string(buffer, fullrank, MAXRANK,&pos);

                MCFA_add_proc(&tempList, id, hostname, portnumber, jobid,sock,status,exec,fullrank);

             PRINTF(("%d\t  %s\t  %d\t  %d\n \n",id,hostname,portnumber,jobid));
                i= pos;
        }

	return tempList;
      PRINTF(("Num of procs = %d\n",proc_count));

}



char* MCFA_pack_hostlist(struct MCFA_host_node *hostList, int *msglen)
{
        struct MCFA_host_node *curr = hostList;

        char *buffer;

        int len = 0, hostcount = 0;
        int pos=0;
	int numprocseachhost = 0;
	int noofints = 0;
	int noofchars = 0;

        int i;
	
	while(curr !=NULL)
        {
		hostcount++;
		numprocseachhost = numprocseachhost + curr->hostdata->numofProcs;
		curr = curr->next;
        }

	noofints = 3 * hostcount + 2 * numprocseachhost;
	noofchars = hostcount * MAXHOSTNAMELEN + numprocseachhost * MAXNAMELEN;
	
        MCFA_pack_size(noofints, noofchars, &len);
        buffer = (char*)malloc(len*sizeof(char));
        if(buffer == NULL){
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }
	
	curr = hostList;
        while(curr !=NULL)
        {

                MCFA_pack_string(buffer, curr ->hostdata->hostname, MAXHOSTNAMELEN, &pos);
                MCFA_pack_int(buffer, &curr ->hostdata->numofProcs, 1, &pos);
                MCFA_pack_int(buffer, &curr ->hostdata->lastPortUsed, 1, &pos);
                for(i=0;i<curr ->hostdata->numofProcs;i++)
                {
                        MCFA_pack_int(buffer, &curr ->hostdata->id[i].procID, 1, &pos);
                        MCFA_pack_int(buffer, &curr ->hostdata->id[i].jobID, 1, &pos);
			MCFA_pack_string(buffer, curr->hostdata->id[i].exec, MAXNAMELEN, &pos);
                }
//                MCFA_pack_string(buffer, curr ->hostdata.executable, MAXHOSTNAMELEN, &pos);
                MCFA_pack_int(buffer, &curr ->hostdata->status, 1, &pos);

                curr = curr->next;
        }

	PRINTF(("lenth of hostbuffer  %d",len));
        *msglen = len;
	return buffer;

}


struct MCFA_host_node* MCFA_unpack_hostlist(char *buffer, int len)
{
	int num,port,status;
	char hostname[MAXHOSTNAMELEN], exec[MAXNAMELEN];
	int pos=0 ,host_count = 0,procid,jobid;
	int i,j;

	struct MCFA_host_node *tempList;
	struct MCFA_host node;

	
        MCFA_initHostList(&tempList);


	for(i=0;i<len;)
        {

                host_count ++ ;

                MCFA_unpack_string(buffer, hostname ,MAXHOSTNAMELEN,&pos);
		strcpy(node.hostname,hostname);

                MCFA_unpack_int(buffer, &num ,1,&pos);
		node.numofProcs = num;

                MCFA_unpack_int(buffer, &port ,1,&pos);
		node.lastPortUsed = port;

		node.id = (struct MCFA_ID*) malloc (num * sizeof(struct MCFA_ID));
		if(NULL == node.id){
                	printf("ERROR: in allocating memory\n");
                	exit(-1);
    		}

                for(j=0;j<num;j++)
                {
                        MCFA_unpack_int(buffer, &procid, 1, &pos);
			node.id[j].procID= procid;
                        MCFA_unpack_int(buffer, &jobid, 1, &pos);
			node.id[j].jobID= jobid;
			MCFA_unpack_string(buffer,exec,MAXNAMELEN,&pos);
			strcpy(node.id[j].exec,exec); 
                }

//                MCFA_unpack_string(buffer, exec ,MAXHOSTNAMELEN,&pos);
//		strcpy(node.executable,exec);

                MCFA_unpack_int(buffer, &status,1,&pos);
		node.status = status;
		
		MCFA_add_host(&tempList, &node);


        //        PRINTF(("%s\t  %d\t  %d\t  %d\t %d\t  %s\t  %d\n \n",hostname, num, port,procid[0],jobid[0],exec,status ));
                i= pos+1;
        }


        PRINTF(("Num of Hosts = %d\n\n",host_count));
	return tempList;

}


char* MCFA_pack_jobstatus(struct MCFA_proc_node *procList,int jobid, int *msglen)
{
    struct MCFA_proc_node *curr = procList;

        char *buffer = NULL;
	
	int numprocs = 0;
        int len = 0;
        int pos=0;
	int noofints = 0;
	int noofchars = 0;

	while(curr !=NULL)
	{
	    if (curr->procdata->jobid == jobid)
		numprocs++;
	    curr = curr->next;
	}

	noofints = numprocs * 6;
        noofchars = numprocs * (MAXHOSTNAMELEN + MAXNAMELEN + MAXRANK);


	MCFA_pack_size(noofints, noofchars, &len);
        buffer = (char*)malloc(len*sizeof(char));
        if(buffer == NULL){
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }
	
	curr = procList;

	 while(curr !=NULL)
	 { 
	     if (curr->procdata->jobid == jobid)
	     {
		MCFA_pack_int(buffer, &curr ->procdata->id ,1,&pos);
		MCFA_pack_int(buffer, &curr ->procdata->volpex_id ,1,&pos);
                MCFA_pack_string(buffer, curr ->procdata->hostname ,MAXHOSTNAMELEN,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->portnumber ,1,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->jobid ,1,&pos);
                MCFA_pack_int(buffer, &curr ->procdata->sock ,1,&pos);
		MCFA_pack_int(buffer, &curr ->procdata->status ,1,&pos);
		MCFA_pack_string(buffer, curr->procdata->executable, MAXNAMELEN,&pos);
		MCFA_pack_string(buffer, curr->procdata->fullrank, MAXRANK, &pos);
	     }
	     
	     curr = curr->next;

	 }
	 *msglen = len;
	 return buffer;
	     
}

struct MCFA_proc_node* MCFA_unpack_jobstatus(char *buffer, int len)
{  
	int id,portnumber,jobid,sock,status,volpex_id;
        char hostname[MAXHOSTNAMELEN], exec[MAXNAMELEN], fullrank[MAXRANK];
        int pos=0,proc_count = 0;
        int i;

        struct MCFA_proc_node *tempList;

        MCFA_initProcList(&tempList);
//      printf("Lenth of processBuffer   %d\n",len);
        for(i=0;i<len;)
        {

                proc_count ++ ;

                MCFA_unpack_int(buffer, &id ,1,&pos);
                MCFA_unpack_int(buffer, &volpex_id ,1,&pos);
                MCFA_unpack_string(buffer, hostname ,MAXHOSTNAMELEN,&pos);
                MCFA_unpack_int(buffer, &portnumber ,1,&pos);
                MCFA_unpack_int(buffer, &jobid ,1,&pos);
                MCFA_unpack_int(buffer, &sock ,1,&pos);
		MCFA_unpack_int(buffer, &status ,1,&pos);
		MCFA_unpack_string(buffer,exec,MAXNAMELEN, &pos);
		MCFA_unpack_string(buffer,fullrank,MAXRANK, &pos);

                MCFA_add_proc(&tempList, id, hostname, portnumber, jobid,sock,status,exec,fullrank);
		i= pos;
        }
	
	return tempList;
	//    printf("Num of procs = %d\n",proc_count);
	
}
		
    



char* MCFA_pack_procstatus(struct MCFA_proc_node *procList,int procid, int *msglen)
{
	
	struct MCFA_process *node;
	 char *buffer = NULL;

        int len;
        int pos=0;

        node = MCFA_search_proc(procList,procid);
	if(node !=NULL)
        {
//                if(node->jobid == jobid)
                MCFA_pack_size( 6, MAXHOSTNAMELEN + MAXNAMELEN + MAXRANK, &len);
        	buffer = (char*)malloc(len*sizeof(char));
		if(NULL == buffer){
                	printf("ERROR: in allocating memory\n");
                exit(-1);
    		}

		MCFA_pack_int(buffer, &node->id ,1,&pos);
		MCFA_pack_int(buffer, &node->volpex_id ,1,&pos);
	        MCFA_pack_string(buffer,node->hostname ,MAXHOSTNAMELEN,&pos);
        	MCFA_pack_int(buffer, &node->portnumber ,1,&pos);
                MCFA_pack_int(buffer, &node->jobid ,1,&pos);
		MCFA_pack_int(buffer, &node ->sock ,1,&pos);
		MCFA_pack_int(buffer, &node ->status ,1,&pos);
		MCFA_pack_string(buffer,node->executable,MAXNAMELEN,&pos);
		MCFA_pack_string(buffer,node->fullrank, MAXRANK, &pos);

                
        }

	*msglen = len;

	return buffer;
}

struct MCFA_process* MCFA_unpack_procstatus(char *buffer, int len)
{

        int pos=0;
        int i;
	
	struct MCFA_process *proc;

	proc = (struct MCFA_process*) malloc(sizeof(struct MCFA_process));
	if(NULL == proc){
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }

	proc->hostname = (char*)malloc(MAXHOSTNAMELEN * sizeof(char));
	proc->executable = (char*)malloc(MAXNAMELEN * sizeof(char));
	if(buffer !=NULL)
	
	        for(i=0;i<len;)
        	{
	                MCFA_unpack_int(buffer, &proc->id ,1,&pos);
	                MCFA_unpack_int(buffer, &proc->volpex_id ,1,&pos);
        	        MCFA_unpack_string(buffer, proc->hostname ,MAXHOSTNAMELEN,&pos);
                	MCFA_unpack_int(buffer, &proc->portnumber ,1,&pos);
	                MCFA_unpack_int(buffer, &proc->jobid ,1,&pos);
        	        MCFA_unpack_int(buffer, &proc->sock ,1,&pos);
			MCFA_unpack_int(buffer, &proc->status ,1,&pos);
			MCFA_unpack_string(buffer, proc->executable,MAXNAMELEN,&pos);
			MCFA_unpack_string(buffer, proc->fullrank,MAXRANK, &pos);

	
//      	        printf("%s\t\t  %d\t\t  %d\n",hostname,portnumber,sock);
                	i= pos;
	        }
	
	return proc;
}
char* MCFA_pack_procs(struct MCFA_proc_node *procList,int *ids, int numprocs,int *msglen)
{

        struct MCFA_process *node;
         char *buffer = NULL;

        int len;
        int pos=0,i;
	int noofints = 0;
        int noofchars = 0;
	
	noofints = numprocs * 6;
        noofchars = numprocs * (MAXHOSTNAMELEN + MAXNAMELEN + MAXRANK);

        MCFA_pack_size(noofints, noofchars, &len);
        buffer = (char*)malloc(len*sizeof(char));
        if(buffer == NULL){
                printf("ERROR: in allocating memory\n");
                exit(-1);
        }

	for(i=0; i<numprocs; i++){
	        node = MCFA_search_proc(procList,ids[i]);
        	if(node !=NULL)
        	{
                	MCFA_pack_int(buffer, &node->id ,1,&pos);
                	MCFA_pack_int(buffer, &node->volpex_id ,1,&pos);
	                MCFA_pack_string(buffer,node->hostname ,MAXHOSTNAMELEN,&pos);
        	        MCFA_pack_int(buffer, &node->portnumber ,1,&pos);
                	MCFA_pack_int(buffer, &node->jobid ,1,&pos);
	                MCFA_pack_int(buffer, &node ->sock ,1,&pos);
        	        MCFA_pack_int(buffer, &node ->status ,1,&pos);
                	MCFA_pack_string(buffer,node->executable,MAXNAMELEN,&pos);
	                MCFA_pack_string(buffer,node->fullrank, MAXRANK, &pos);
		}

        }

        *msglen = len;

        return buffer;
}
struct MCFA_proc_node* MCFA_unpack_procs(char *buffer, int len)
{

        int id,portnumber,jobid,sock,status,volpex_id;
        char hostname[MAXHOSTNAMELEN], exec[MAXNAMELEN], fullrank[MAXRANK];
        int pos=0,proc_count = 0;
        int i;

        struct MCFA_proc_node *tempList;

        MCFA_initProcList(&tempList);
//      printf("Lenth of processBuffer   %d\n",len);
        for(i=0;i<len;)
        {

                proc_count ++ ;

                MCFA_unpack_int(buffer, &id ,1,&pos);
                MCFA_unpack_int(buffer, &volpex_id ,1,&pos);
                MCFA_unpack_string(buffer, hostname ,MAXHOSTNAMELEN,&pos);
                MCFA_unpack_int(buffer, &portnumber ,1,&pos);
                MCFA_unpack_int(buffer, &jobid ,1,&pos);
                MCFA_unpack_int(buffer, &sock ,1,&pos);
                MCFA_unpack_int(buffer, &status ,1,&pos);
                MCFA_unpack_string(buffer, exec, MAXNAMELEN,&pos);
                MCFA_unpack_string(buffer, fullrank, MAXRANK,&pos);

                MCFA_add_proc(&tempList, id, hostname, portnumber, jobid,sock,status,exec,fullrank);

             printf("[%d]:%d\t  %s\t  %d\t  %d\n \n",SL_this_procid,id,hostname,portnumber,jobid);
                i= pos;
        }

        return tempList;
      printf("[%d]:Num of procs = %d\n",SL_this_procid,proc_count);

}

char* MCFA_pack_hoststatus(struct MCFA_host_node *hostList,char *host, int *msglen)
{
	
	char *buffer = NULL;

        int len;
        int pos=0;
	
	struct  MCFA_host *node;

        int i;
	int numofints = 0, numofchars = 0;
        node = MCFA_search_hostname(hostList,host);


	 if (node !=NULL)
        {
		numofints = 3 + 2 * node->numofProcs;
		numofchars = MAXHOSTNAMELEN + MAXNAMELEN * node->numofProcs;
                MCFA_pack_size( numofints, numofchars, &len);
        	buffer = (char*)malloc(len*sizeof(char));
		if(NULL == buffer){
                	printf("ERROR: in allocating memory\n");
                	exit(-1);
    		}

 
                MCFA_pack_string(buffer, node->hostname, MAXHOSTNAMELEN, &pos);
                MCFA_pack_int(buffer, &node->numofProcs, 1, &pos);
                MCFA_pack_int(buffer, &node->lastPortUsed, 1, &pos);
                for(i=0;i<node->numofProcs;i++)
                {
	                MCFA_pack_int(buffer, &node->id[i].procID, 1, &pos);
                        MCFA_pack_int(buffer, &node->id[i].jobID, 1, &pos);
			MCFA_pack_string(buffer, node->id[i].exec, MAXNAMELEN,&pos);
                 }
//                MCFA_pack_string(buffer, node->executable, MAXHOSTNAMELEN, &pos);
                MCFA_pack_int(buffer, &node->status, 1, &pos);

        }


	*msglen = len;
//	free(node);
	return buffer;
}


struct MCFA_host* MCFA_unpack_hoststatus(char *buffer, int len)
{
	int pos=0 ,host_count = 0;
        int i,j;
	struct MCFA_host *host;


	host = (struct MCFA_host*)malloc(sizeof(struct MCFA_host));
	if(NULL == host){
                printf("ERROR: in allocating memory\n");
                exit(-1);
    }


	if(buffer != NULL)
	{
	        for(i=0;i<len;)
        	{
	
        	        host_count ++ ;
	
        	        MCFA_unpack_string(buffer, host->hostname ,MAXHOSTNAMELEN,&pos);
                	MCFA_unpack_int(buffer, &host->numofProcs ,1,&pos);
	                MCFA_unpack_int(buffer, &host->lastPortUsed ,1,&pos);
			host->id =(struct MCFA_ID*) malloc(host->numofProcs*sizeof(struct MCFA_ID));
			if(NULL == host->id){
		                printf("ERROR: in allocating memory\n");
                		exit(-1);
			}

        	        for(j=0;j<host->numofProcs;j++)
                	{
                        	MCFA_unpack_int(buffer, &host->id[j].procID, 1, &pos);
		                MCFA_unpack_int(buffer, &host->id[j].jobID, 1, &pos);
				MCFA_unpack_string(buffer,host->id[j].exec, MAXNAMELEN, &pos);
				
        	        }
	
  //      	        MCFA_unpack_string(buffer, host->executable ,MAXHOSTNAMELEN,&pos);
                	MCFA_unpack_int(buffer, &host->status,1,&pos);

                	i= pos;
	        }
	}

  //      MCFA_printf("Num of Hosts = %d\n\n",host_count);
    //    *hostList = tempList;
        return host;

}

