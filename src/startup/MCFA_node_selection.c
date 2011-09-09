#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"
extern struct MCFA_proc_node *procList;

int MCFA_create_distmatrix(int **tarray)
{
	int msglen;
	int i,j;
	void *msgbuf;
	int pos =0;

	msglen = sizeof(int)*SL_numprocs;
    	MCFA_pack_size(SL_numprocs, 0, &msglen);
    	msgbuf = malloc(msglen);



    for(i=0; i<SL_numprocs; i++){
        for(j=0;j<SL_numprocs;j++){
            SL_Send(&i, sizeof(int), j, 0, 0);
        }
        SL_Recv ( msgbuf, msglen, i, 0, 0, SL_STATUS_NULL);
        MCFA_unpack_int(msgbuf, tarray[i], SL_numprocs, &pos);
        pos = 0;

    }


	MCFA_transpose_distmatrix(tarray);
	MCFA_print_distmatrix(tarray, SL_numprocs);
return 1;	
}

int MCFA_create_distmatrix2(int **tarray, int **clusters, int nclusters)
{
        int msglen;
        int i,j;
        void *msgbuf;
        int pos =0;


        msglen = sizeof(int)*SL_numprocs;
        MCFA_pack_size(SL_numprocs, 0, &msglen);
        msgbuf = malloc(msglen);



    for(i=0; i<SL_numprocs; i++){
        for(j=0;j<SL_numprocs;j++){
            SL_Send(&i, sizeof(int), j, 0, 0);
        }
        SL_Recv ( msgbuf, msglen, i, 0, 0, SL_STATUS_NULL);
        MCFA_unpack_int(msgbuf, tarray[i], SL_numprocs, &pos);
        pos = 0;

    }


        MCFA_transpose_distmatrix(tarray);
        MCFA_print_distmatrix(tarray, SL_numprocs);
return 1;
}

int MCFA_create_distmatrix1(int **tarray)
{
	struct MCFA_proc_node *currlist1 = procList;
	struct MCFA_proc_node *currlist2 = procList;
	char *ptr1, *ptr2;
        char subnet1[16], subnet2[16];
        int len1,len2;
	int i=0,j=0,k;

	while(currlist1 != NULL){
                ptr1 = strrchr(currlist1->procdata->hostname, '.');
		if(ptr1==NULL)
			len1 = strlen(currlist1->procdata->hostname);
		else
                	len1 = strlen(currlist1->procdata->hostname)-strlen(ptr1);
                strncpy(subnet1,currlist1->procdata->hostname,len1);
                subnet1[len1] = '\0';

		currlist2 = procList;
		j=0;
                while(currlist2 != NULL){
                        ptr2 = strrchr(currlist2->procdata->hostname, '.');
			if(ptr2==NULL)
				len2 = strlen(currlist2->procdata->hostname);
			else
                        	len2 = strlen(currlist2->procdata->hostname)-strlen(ptr2);
                        strncpy(subnet2,currlist2->procdata->hostname,len2);
                        subnet2[len2] = '\0';

                        if(0==strcmp(currlist1->procdata->hostname,currlist2->procdata->hostname)){
                                tarray[i][j] = 1;
				if(j==i)
					break;				
				j++;
				currlist2 = currlist2->next;
                                continue;
                        }


                        if(0==strcmp(subnet1,subnet2))
                                tarray[i][j] = 4;
                        else
                                tarray[i][j] = 8;
			if(j==i)
				break;
			j++;
			currlist2 = currlist2->next;

                }

                for(k=i;k<SL_numprocs;k++)
                        tarray[i][k] = 0;
		i++;
		currlist1 = currlist1->next;
        }
        MCFA_print_distmatrix(tarray, SL_numprocs);	
//MCFA_transpose_distmatrix(tarray);
//        MCFA_print_distmatrix(tarray, SL_numprocs);	
	return 1;
}

int MCFA_node_selection(int redundancy)
{
/* The main purpose of this function
	1. to recieve distance matrix based on communication
	2. Convert it to proper distance matrix
	3. Create the hierachial tree
	4. Divide the tree into clusters such that nodes with
		distance more than threshold value will reside 
		in a diffrent cluster
	5. Divide the nodes based on redundancy such nodes
		nodes closest to each other are in one team
	6. Divide the distance matrix such that each team have 
		their own distance matrix
	7. Create subtrees for each team based on that distance
		matrix
	8. Get the communication charactristics of the application
		in the form of a distance matrix
	9. Create a hierarchial tree for the application
	10. Using the application charactristics and nodes architecture
		create nodemapping such that each MPI rank corresponds
		to one actual rank
	11. Update procList with this information
*/

    int **procarray;
    int pos=0;
    int msglen, i, j;
    void *msgbuf;
    
    procarray = (int**) malloc (SL_numprocs * sizeof(int *));
    for(i=0; i<SL_numprocs; i++){
	procarray[i] = (int *) malloc (SL_numprocs * sizeof(int));
    }

    MCFA_create_distmatrix1(procarray);


    
    int *newnodes;
    MCFA_node *tree, *mpitree;
    int **clusters, numclusters;
    int *numelems;

tree = MCFA_tree(procarray, SL_numprocs);
MCFA_printtree(tree, SL_numprocs);

//newnodes = MCFA_cluster(tree, redundancy,procarray);

clusters = MCFA_cluster(tree, redundancy,procarray,&numclusters, &numelems);

for(i=0; i<numclusters; i++){
        printf("Cluster:%d ->",i);
        for(j=0;j<numelems[i];j++)
            printf("%d ",clusters[i][j]);
        printf("\n");
    }


/***************
first node from each cluster should send messages to each cluster and 
again create a distance matrix
Finally perform clustering based on it

******************/


newnodes = MCFA_sortedlist(clusters, numclusters, numelems, redundancy);

int ***submat = NULL;
    submat = MCFA_dividedistmatrix(procarray, redundancy,newnodes);
    MCFA_print_submatrix(submat,redundancy,newnodes);


MCFA_node **subtree;
	subtree = MCFA_create_subtree(submat, redundancy, newnodes);

    
 mpitree = MCFA_app_comm_matrix(redundancy);

int **mapnodes;

mapnodes = (int**)malloc(redundancy * sizeof(int*));    
   
for(i=0; i<redundancy; i++){
	mapnodes[i] = MCFA_create_mapping(mpitree, subtree[i],SL_numprocs/redundancy);
}
int k=0;
for(i=0; i<redundancy; i++){
	for(j=0; j<SL_numprocs/redundancy; j++){
		newnodes[k] = mapnodes[i][j];
		k++;
	}
}
 
for(i=0;i<SL_numprocs;i++)
    {
        printf("%d  ", newnodes[i]);
    }
    
    MCFA_update_fullrank1(newnodes,redundancy);
   
//freeing submat
for(i=0;i<redundancy;i++){
        for(j=0;j<SL_numprocs/redundancy;j++)
            free(submat[i][j]);
    }
    for(i=0;i<redundancy;i++)
        free(submat[i]);
     free(submat);



for(i=0;i<SL_numprocs;i++)
	free(procarray[i]); 
free(procarray);
    return SL_SUCCESS;
    
    
}


MCFA_node* MCFA_app_comm_matrix(int redundancy)
{
	int **appcommmat;
	int i,j;

	appcommmat = (int**)malloc(SL_numprocs/redundancy * sizeof(int*));

	for(i=0;i<SL_numprocs/redundancy;i++)
		appcommmat[i] = (int*)malloc(SL_numprocs/redundancy * sizeof(int));



	for(i=0;i<SL_numprocs/redundancy;i++){
		for(j=0;j<SL_numprocs/redundancy;j++){
			if(j==(i-1))
				appcommmat[i][j] = 0;	
			else if(j == (i-3))
				appcommmat[i][j] = 10;
			else
				appcommmat[i][j] = 100;

		}
	}

	for(i=sqrt(SL_numprocs/redundancy);i<SL_numprocs/redundancy-1;i=i+sqrt(SL_numprocs/redundancy)){
		appcommmat[i][i-1] = 100;
	}

/*	for(i=0;i<SL_numprocs/redundancy;i++){
                for(j=0;j<SL_numprocs/redundancy;j++){
                        if(j==(i-1))
                                appcommmat[i][j] = 0;
                        else if(j == (i-4))
                                appcommmat[i][j] = 1;
			else
				appcommmat[i][j] = 100;
			
			if(i<=j)
                        	appcommmat[i][j] = 1000;
                }
        }

	appcommmat[4][3] = 100;
	appcommmat[2][0] = 10;
	appcommmat[6][4] = 10;
*/
	printf("\n\n Application Communication matrix\n");
	MCFA_print_distmatrix(appcommmat, SL_numprocs/redundancy);


	MCFA_node *tree;
	tree = MCFA_tree(appcommmat, SL_numprocs/redundancy);
	MCFA_printtree(tree, SL_numprocs/redundancy);

	return tree;	
}

void MCFA_update_fullrank1(int *newnodes, int redundancy)
{
        int i,j,k;
        char level='A';
        char fullrank[16];
        struct MCFA_process *proc;

        k=0;
        level = 'A';
        for (i=0;i<redundancy; i++){
                for(j=0;j<SL_numprocs/redundancy;j++){
                        proc = MCFA_search_proc(procList,newnodes[k]);
                        proc->volpex_id = k;
                        sprintf(fullrank,"%d,%c",j,level);
                        strcpy(proc->fullrank, fullrank);
                        k++;
                }
                level++;
        }
        MCFA_printProclist(procList);
}


int MCFA_transpose_distmatrix(int **procarray)
{
	int i,j,save;
        for (i = 0; i < SL_numprocs; i++) {
              for (j = i + 1; j < SL_numprocs; j++) {
                      save = procarray[i][j];
                      procarray[i][j] = procarray[j][i];
                      procarray[j][i] = save;
              }
      }
	return 0;
}

int MCFA_print_distmatrix(int **procarray, int size)
{
	int i,j;
	printf("\n      ");
	for(i=0; i<size; i++)
        	printf("[%d] ", i);
	printf("\n");
    	for(i=0; i<size; i++){
        	printf("[%2d]: ", i);
        	for(j=0; j<size; j++){
	            printf("%3d ", procarray[i][j]);
        	}
        	printf("\n");
    }
	return 0;
}






