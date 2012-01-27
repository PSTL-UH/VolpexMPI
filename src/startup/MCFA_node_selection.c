#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"
extern struct MCFA_proc_node *procList;
extern MCFA_create_distmatrix_func *MCFA_create_distmatrix;
extern MCFA_create_comm_matrix_func *MCFA_create_comm_matrix;
int MCFA_distmatrix_communication(int **tarray)
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
//	MCFA_print_distmatrix(tarray, SL_numprocs);
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
  //      MCFA_print_distmatrix(tarray, SL_numprocs);
return 1;
}

int MCFA_distmatrix_ipaddress(int **tarray)
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
//        MCFA_print_distmatrix(tarray, SL_numprocs);	
//MCFA_transpose_distmatrix(tarray);
//        MCFA_print_distmatrix(tarray, SL_numprocs);	
	return 1;
}

int MCFA_node_selection(int redundancy)
{
/* The main purpose of this function
	1. to recieve distance matrix based on communication/IP Address
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
    int i, j;
    
    procarray = (int**) malloc (SL_numprocs * sizeof(int *));
    for(i=0; i<SL_numprocs; i++){
	procarray[i] = (int *) malloc (SL_numprocs * sizeof(int));
    }
    
    MCFA_create_distmatrix(procarray);
    
    
    
    int *newnodes;
    MCFA_node *tree, *mpitree;
    int **clusters, numclusters;
    int *numelems;
//    MCFA_print_distmatrix(procarray, SL_numprocs);
    tree = MCFA_tree(procarray, SL_numprocs);
//    MCFA_printtree(tree, SL_numprocs);
    
    
    clusters = MCFA_cluster(tree, SL_numprocs,procarray,&numclusters, &numelems);
    
    for(i=0; i<numclusters; i++){
        printf("Numelements:%d Cluster:%d ->",numelems[i],i);
        for(j=0;j<numelems[i];j++)
            printf("%d ",clusters[i][j]);
        printf("\n");
    }

//creating the distance matrix for clusters    
    int **distanceclusters;
    distanceclusters = (int**)malloc(numclusters * sizeof(int*));
    for(i=0; i<numclusters; i++){
        distanceclusters[i] = (int*)malloc(numclusters * sizeof(int));
    }
    for(i=0; i<numclusters; i++){
	for(j=i; j<numclusters; j++){
	    if(clusters[i][0] > clusters[j][0])
		distanceclusters[i][j] = 
		    distanceclusters[j][i] = procarray[clusters[i][0]][clusters[j][0]]; 	
	    else
		distanceclusters[i][j] = 
		    distanceclusters[j][i] = procarray[clusters[j][0]][clusters[i][0]];
	}
    }
//    MCFA_print_distmatrix(distanceclusters, numclusters);
/***************
first node from each cluster should send messages to each cluster and 
again create a distance matrix
Finally perform clustering based on it

******************/
    
    
//newnodes = MCFA_sortedlist(clusters, numclusters, numelems, redundancy);
    newnodes = MCFA_sortedlist(clusters, numclusters, numelems, redundancy, distanceclusters);
    
    int ***submat = NULL;
    submat = MCFA_dividedistmatrix(procarray, redundancy,newnodes);
//   MCFA_print_submatrix(submat,redundancy,newnodes);
//   MCFA_print_distmatrix(submat[0], SL_numprocs/redundancy); 
//   MCFA_print_distmatrix(submat[1], SL_numprocs/redundancy); 
    int **comm_mat;
    comm_mat = (int**) malloc (SL_numprocs/redundancy * sizeof(int *));
    for(i=0; i<SL_numprocs/redundancy; i++){
        comm_mat[i] = (int *) malloc (SL_numprocs/redundancy * sizeof(int));
    }
   MCFA_create_comm_matrix(redundancy, comm_mat);
   int **Value,k;
   
   Value=(int**)malloc(sizeof(int*)*redundancy);
   for(i=0; i<redundancy; i++)
   	Value[i]=(int*)malloc(sizeof(int)*SL_numprocs/redundancy);
  
   for(i=0; i<redundancy; i++)
	for(j=0;j<SL_numprocs/redundancy;j++) 
		for(k=j;k<SL_numprocs/redundancy;k++)
			submat[i][j][k] = submat[i][k][j];



   for(i=0; i<redundancy; i++)
   	map_MPIPP(1,SL_numprocs/redundancy,Value[i],comm_mat, submat[i]);

for(i=0;i<SL_numprocs;i++)
    {
        printf("%2d ", newnodes[i]);
    }


printf("\n");
   for(i=0; i<redundancy; i++)
        for(j=0;j<SL_numprocs/redundancy;j++)
			printf("%2d ", Value[i][j]);

MCFA_map(Value, newnodes, redundancy);

printf("\n");
for(i=0;i<SL_numprocs;i++)
    {
        printf("%2d ", newnodes[i]);
    }

    
    
/*    MCFA_node **subtree;
    subtree = MCFA_create_subtree(submat, redundancy, newnodes);
    
    mpitree = MCFA_create_comm_matrix_sp(redundancy, comm_mat);
    MCFA_print_distmatrix(comm_mat,SL_numprocs/redundancy);
//    mpitree = MCFA_app_comm_matrix(redundancy);
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
*/    
    MCFA_update_fullrank1(newnodes,redundancy);
    
//freeing submat
    for(i=0;i<redundancy;i++){
        for(j=0;j<SL_numprocs/redundancy;j++)
            free(submat[i][j]);
    }
    for(i=0;i<redundancy;i++)
        free(submat[i]);
    free(submat);
    for(i=0; i<numclusters; i++){
        free(distanceclusters[i]) ;
    } 
    free(distanceclusters); 
 
    for(i=0; i<SL_numprocs/redundancy; i++){
        free(comm_mat[i]) ;
    }
    free(comm_mat);
 
   for(i=0; i<redundancy; i++)
        free(Value[i]);
    free(Value);    
    
    for(i=0;i<SL_numprocs;i++)
	free(procarray[i]); 
    free(procarray);
    

    return SL_SUCCESS;
    
    
}
/*
int MCFA_create_comm_matrix(int redundancy, int **appcomm)
{
//	int **appcomm;
        int i,j;

        appcomm = (int**)malloc(SL_numprocs/redundancy * sizeof(int*));

        for(i=0;i<SL_numprocs/redundancy;i++)
                appcomm[i] = (int*)malloc(SL_numprocs/redundancy * sizeof(int));

        for(i=0;i<SL_numprocs/redundancy;i++){
                for(j=0;j<SL_numprocs/redundancy;j++){
                        if(i==j)
                                appcomm[i][j]=0;
                        else
                                appcomm[i][j]=1;
                }

        }

        appcomm[0][1] = appcomm[1][0] = appcomm[1][2] = appcomm[2][1] = appcomm[2][3] = appcomm[3][2] = 1000;
        appcomm[4][5] = appcomm[5][4] = appcomm[5][6] = appcomm[6][5] = appcomm[6][7] = appcomm[7][6] = 1000;
        appcomm[0][4] = appcomm[4][0] = appcomm[1][5] = appcomm[5][1] = appcomm[2][6] = appcomm[6][2] = 100;
        appcomm[3][7] = appcomm[7][3]  = 100;
//        appcomm[4][5] = appcomm[5][4] = 10;
        appcomm[0][2] = appcomm[2][0] = appcomm[4][6] = appcomm[6][4] = 10;


	appcomm[0][1] = appcomm[1][0] = appcomm[1][2] = appcomm[2][1] = appcomm[2][3] = appcomm[3][2] = 1000;

//	printf("\n\n Application Communication matrix\n");
//        MCFA_print_distmatrix(appcomm, SL_numprocs/redundancy);

	return 1;


}
MCFA_node* MCFA_app_comm_matrix(int redundancy)
{
	int **appcomm;
	int i,j;

	appcomm = (int**)malloc(SL_numprocs/redundancy * sizeof(int*));

	for(i=0;i<SL_numprocs/redundancy;i++)
		appcomm[i] = (int*)malloc(SL_numprocs/redundancy * sizeof(int));

	for(i=0;i<SL_numprocs/redundancy;i++){
                for(j=0;j<SL_numprocs/redundancy;j++){
                        if(i==j)
                                appcomm[i][j]=0;
                        else
                                appcomm[i][j]=1000;
                }

        }

	appcomm[0][1] = appcomm[1][0] = appcomm[1][2] = appcomm[2][1] = appcomm[2][3] = appcomm[3][2] = 1;
        appcomm[4][5] = appcomm[5][4] = appcomm[5][6] = appcomm[6][5] = appcomm[6][7] = appcomm[7][6] = 1;
        appcomm[0][4] = appcomm[4][0] = appcomm[1][5] = appcomm[5][1] = appcomm[2][6] = appcomm[6][2] = 10;
        appcomm[3][7] = appcomm[7][3]  = 10;
//	appcomm[4][5] = appcomm[5][4] = 100;
        appcomm[0][2] = appcomm[2][0] = appcomm[4][6] = appcomm[6][4] = 100;



	appcomm[0][1] = appcomm[1][0] = appcomm[1][2] = appcomm[2][1] = appcomm[2][3] = appcomm[3][2] = 1;




	for(i=0;i<SL_numprocs/redundancy;i++){
                for(j=i;j<SL_numprocs/redundancy;j++){
                                appcomm[i][j]=0;
                }

        }

	printf("\n\n Application Communication matrix\n");
//	MCFA_print_distmatrix(appcomm, SL_numprocs/redundancy);


	MCFA_node *tree;
	tree = MCFA_tree(appcomm, SL_numprocs/redundancy);
	MCFA_printtree(tree, SL_numprocs/redundancy);
	int **clusters, numclusters;
	int *numelems;
	clusters = MCFA_cluster(tree, SL_numprocs/redundancy,appcomm,&numclusters, &numelems);
	MCFA_sort_cluster(clusters, numclusters, numelems, appcomm);

	for(i=0; i<numclusters; i++){
        printf("Cluster:%d ->",i);
        for(j=0;j<numelems[i];j++)
            printf("%d ",clusters[i][j]);
        printf("\n");
    	}	

	return tree;	
}
*/

MCFA_exchange(int **clusters, int *numelements, int clusternum, int pos, int temp1, int temp2)
{
	int i,j,temp;
	for(i=0; i<numelements[clusternum];i++)
	{
		temp = clusters[clusternum][pos];
		clusters[clusternum][pos] =  temp1;
		for(j=0;j<numelements[clusternum]; j++)
		{
			if(clusters[clusternum][j] == temp1)
				clusters[clusternum][j] = temp;
		}

		temp = clusters[clusternum][pos+1];
                clusters[clusternum][pos] =  temp2;
                for(j=0;j<numelements[clusternum]; j++)
                {
                        if(clusters[clusternum][j] == temp2)
                                clusters[clusternum][j] = temp;
                }

	}
	return 1;
}

int MCFA_sort_cluster1(int **clusters, int numcluster, int *numelements, int **distmatrix)
{
	int i,j,k,temp1, temp2,p;
	int **newcluster, min, val, p1,p2;
	

	newcluster = (int **) malloc (numcluster* sizeof(int*));
        for(i=0; i<numcluster; i++)
                newcluster[i] = (int *) malloc (numelements[i] * sizeof(int));
	

	for(i=0; i<numcluster; i++){
        	for(j=0;j<numelements[i];j++){
			min=1000;
		/*	temp1=clusters[i][j];
			temp2=clusters[i][j+1];

			if(clusters[i][j] > clusters[i][j+1])
				min = distmatrix[clusters[i][j]][clusters[i][j+1]];
			else
				min = distmatrix[clusters[i][j+1]][clusters[i][j]];
		*/
		  for(p=0;p<numelements[i];p++){		
			for(k=p+1; k<numelements[i]; k++){
				if (clusters[i][k]==-1 || clusters[i][p]==-1)
					continue;	
				if(clusters[i][k]>clusters[i][p])
					val = distmatrix[clusters[i][k]][clusters[i][p]];
				else
					val = distmatrix[clusters[i][p]][clusters[i][k]];
				if(distmatrix[clusters[i][k]][clusters[i][p]] < min){

					if(clusters[i][k]>clusters[i][p])
                                        min = distmatrix[clusters[i][k]][clusters[i][p]];
                                	else
                                        min = distmatrix[clusters[i][p]][clusters[i][k]];
//					min = distmatrix[clusters[i][k]][clusters[i][j]];
					temp1 = clusters[i][k];
					temp2 = clusters[i][p];
					p1=k;p2=p;
				}
			}
		}
			printf("%d %d = %d\n", temp1, temp2,min);
			newcluster[i][j] = temp1;
			newcluster[i][j+1] = temp2;
			clusters[i][p1]=-1;
			clusters[i][p2]=-1;
			
		}
	}

	for(i=0; i<numcluster; i++){
        printf("Cluster:%d ->",i);
        for(j=0;j<numelements[i];j++)
            printf("%d ",newcluster[i][j]);
        printf("\n");
        }

/*
	int **edgesw;

	edgesw = (int **) malloc (numcluster* sizeof(int*));
	for(i=0; i<numcluster; i++){
		edgesw[i] = (int *) malloc (numelements[i]-1 * sizeof(int));
		for(j=1;j<numelements[i];j++){		
			edgesw[i][j-1] = distmatrix[j][j-1];
		}
		for(k=0; k<numelements[i]-1; k++){
                	printf("%d ", edgesw[i][k]);
        	}
	}
*/        
/*
	for(i=0; i<numcluster; i++){
        for(j=0;j<numelements[i];j=j+2){
		for(k=1; k<numelements[i]; k=k+2){
			if (distmatrix[j][k]<distmatrix[j][k+1]){
				printf("Exchanging %d <=> %d\n", clusters[i][k-1],clusters[i][j-1]);
				temp1 = clusters[i][j+1];
				clusters[i][j+1]= clusters[i][k];
				clusters[i][k]=temp1;

				temp2 = clusters[i][k];
                                clusters[i][k]= clusters[i][j];
                                clusters[i][j]=temp2;

			}
		}

	for(k=0; k<numelements[i]; k++){	
		printf("%d ", clusters[i][k]);
	}
	printf("\n");
	}
    }
*/

    return 1;
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
        	printf("[%6d] ", i);
	printf("\n");
    	for(i=0; i<size; i++){
        	printf("[%2d]: ", i);
        	for(j=0; j<size; j++){
	            printf("%6d ", procarray[i][j]);
        	}
        	printf("\n");
    }
	return 0;
}






