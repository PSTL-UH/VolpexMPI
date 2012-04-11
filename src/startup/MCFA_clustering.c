#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"


//#define THRESHOLDCUT 600 
#define THRESHOLDCUT 3

int*** MCFA_dividedistmatrix(int **distmatrix, int redundancy, int *newnodes);
void MCFA_print_submatrix(int ***subdistmat, int redundancy, int *newnodes);

int MCFA_nodecompare(const void* a, const void* b)
/* Helper function for qsort. */
{ 
    const MCFA_node* node1 = (const MCFA_node*)a;
    const MCFA_node* node2 = (const MCFA_node*)b;
    const double term1 = node1->distance;
    const double term2 = node2->distance;
    if (term1 < term2) return -1;
    if (term1 > term2) return +1;
    return 0;
}

MCFA_node*  MCFA_tree(int **distmatrix, int numprocs)
{
    
    int i, j, k;
    const int nelements = numprocs;
    const int nnodes = nelements - 1;
    int* vector;
    double* temp;
    int* index;
    MCFA_node* result;
    temp = malloc(nnodes*sizeof(double));
    index = malloc(nelements*sizeof(int));
    vector = malloc(nnodes*sizeof(int));
    result = malloc(nelements*sizeof(MCFA_node));

    for (i = 0; i < nnodes; i++) vector[i] = i;
    
    for (i = 0; i < nelements; i++)
    { result[i].distance = 20000000;
	for (j = 0; j < i; j++) temp[j] = distmatrix[i][j];
	for (j = 0; j < i; j++)
	{ k = vector[j];
	    if (result[j].distance >= temp[j])
	    { if (result[j].distance < temp[k]) temp[k] = result[j].distance;
		result[j].distance = temp[j];
		vector[j] = i;
	    }
	    else if (temp[j] < temp[k]) temp[k] = temp[j];
	}
	for (j = 0; j < i; j++)
	{
	    if (result[j].distance >= result[vector[j]].distance) vector[j] = i;
	}
    }
    
    for (i = 0; i < nnodes; i++) result[i].left = i;
    qsort(result, nnodes, sizeof(MCFA_node), MCFA_nodecompare);
    
    for (i = 0; i < nelements; i++) index[i] = i;
    for (i = 0; i < nnodes; i++)
    { j = result[i].left;
	k = vector[j];
	result[i].left = index[j];
	result[i].right = index[k];
	index[k] = -i-1;
    }
    free(vector);
    free(index);
    
    
    result = realloc(result, nnodes*sizeof(MCFA_node));
    
//    MCFA_printtree(result, nnodes+1);

    free(temp);
    
    return result;
    
    
    
}


int** MCFA_cluster(MCFA_node *result, int totprocs, int **distmatrix, int *numclusters, int **numelems)
/* This process returns the newnode list where processes are arranged 
	such as first n processes form team 0
	next n processes form next team and so on

	the main function of this process is to divide processes in
	different teams
*/
{
    int i=0,j=0;
//    int nnodes = SL_numprocs-1;
    int nnodes = totprocs-1;
    int nclusters = 0;
    int *clusterid;
    int *numelements;
    int **clusters;
    int *pos;

//    int count=0;
//    int *newnodes;
//    int num=0;
//    int *numelementsleft;
  
/* this loop calculates how many clusters are to be formed
   if the distance between two nodes of a tree in more than
   the given THRESHOLDCUT value the that node should be assined 
   to a new cluster
*/  
    for(i=1; i<nnodes; i++){
	if (result[i].distance - result[i-1].distance > THRESHOLDCUT){
//	    nclusters = SL_numprocs + (-i);
	    nclusters = totprocs + (-i);
	    break;
	}
	
    }
	if( nclusters == 0)
		nclusters = 1;
    printf("Numberofclusters=%d\n",nclusters);
    
    
    numelements = malloc(nclusters*sizeof(int));
    for(i=0;i<nclusters;i++)
	numelements[i]=0;

    clusterid = malloc(totprocs*sizeof(int));

/* This function divides the tree into diffrent clusters */
//    MCFA_cuttree (SL_numprocs, result, nclusters, clusterid);
    MCFA_cuttree (totprocs, result, nclusters, clusterid);
//    for(i=0; i<SL_numprocs; i++){
    for(i=0; i<totprocs; i++){
	numelements[clusterid[i]]++;   //calculates number of elements in each cluster
    }
    
    pos = malloc(nclusters*sizeof(int));//calculates the position in each cluster where
    for(i=0;i<nclusters;i++)            //where the new element is to be inserted
        pos[i]=0;
    
    clusters = (int**)malloc(nclusters * sizeof(int*));
    for(i=0; i<nclusters; i++)
	clusters[i] = malloc (numelements[i]* sizeof(int));
    
    
//    for(i=0;i<SL_numprocs;i++){
    for(i=0;i<totprocs;i++){
	clusters[clusterid[i]][pos[clusterid[i]]] = i;
	pos[clusterid[i]]++;
    }
    
    for(i=0; i<nclusters; i++){
        PRINTF(("Cluster:%d ->",i));
        for(j=0;j<numelements[i];j++)
	    PRINTF(("%d ",clusters[i][j]));
        printf("\n");
    }
	*numclusters = nclusters;
	*numelems    = numelements;
	return clusters;
}
    
//   newclusters = MCFA_sort_clusters(int *numelements, int **clusters, int nclusters); 
    
/* Sorting ranks */

int*** MCFA_dividedistmatrix(int **distmatrix, int redundancy, int *newnodes)
{
    int ***subdistmat;
    int k,row,col,i,j;
    
    subdistmat = (int***)malloc(redundancy*sizeof(int**));
    for(i=0;i<redundancy;i++){
        subdistmat[i] = (int**)malloc((SL_numprocs/redundancy)*sizeof(int*));
        for(j=0;j<SL_numprocs/redundancy;j++)
	    subdistmat[i][j] = (int*)malloc((SL_numprocs/redundancy)*sizeof(int));
    }
    
    for(i=0;i<redundancy;i++){
        for(j=0;j<SL_numprocs/redundancy;j++){
	    for(k=0;k<=j;k++){
		row = newnodes[i*SL_numprocs/redundancy+j];
		col = newnodes[i*SL_numprocs/redundancy+k];
		if(row>col)
		    subdistmat[i][j][k] = distmatrix[row][col];
		else
		    subdistmat[i][j][k] = distmatrix[col][row];
		
	    }
	    for(k=j;k<SL_numprocs/redundancy;k++)
		subdistmat[i][j][k] = 0;
        }
	
    }



    return subdistmat;
    
}

MCFA_node** MCFA_create_subtree(int ***subdistmat, int redundancy, int *newnodes)
{
	MCFA_node **subtree;
	int i,j;
    subtree = (MCFA_node**) malloc (redundancy * sizeof(MCFA_node*));

    for(i=0;i<redundancy;i++){
            subtree[i] = MCFA_tree(subdistmat[i], SL_numprocs/redundancy);

        for(j=0; j<SL_numprocs/redundancy-1; j++){
        if(subtree[i][j].left >=0 )
                subtree[i][j].left  = newnodes[i*SL_numprocs/redundancy+subtree[i][j].left];
        if(subtree[i][j].right >=0 )
        subtree[i][j].right = newnodes[i*SL_numprocs/redundancy+subtree[i][j].right];

        }
        MCFA_printtree(subtree[i], SL_numprocs/redundancy);
    }
	
	return subtree;
}

int* MCFA_create_mapping(MCFA_node *mpitree, MCFA_node *coretree, int nnodes)
{
	int *mapnodes;
	int i,j;
	int leftflag=0, rightflag=0, leftassign = 0;


	mapnodes = (int*)malloc(nnodes * sizeof(int));
        nnodes = nnodes-1;
	for(i=0,j=0; i<nnodes;i++){
		rightflag = leftflag = 0;
		if (mpitree[i].left >=0){
			while(!leftflag){
				if(coretree[j].left >= 0 && leftassign == 0){
	                                mapnodes[mpitree[i].left] = coretree[j].left;
					leftassign = 1;
        	                        leftflag=1;
	                        }
				else if(coretree[j].right >= 0){
                	                mapnodes[mpitree[i].left] =  coretree[j].right;
					leftflag=1;
					j++;
					leftassign = 0;

				}
				if(leftflag == 0){
					j++;
					leftassign = 0;
				}
			
			}
		}
		if (mpitree[i].right >=0){
			while(!rightflag){
				if(leftassign == 0){
					if(coretree[j].left >= 0 ){
        	                        	mapnodes[mpitree[i].right] = coretree[j].left;
						rightflag = 1;
						leftassign = 1;
	                        	}
					else if(coretree[j].right >= 0){
                                		mapnodes[mpitree[i].right] =  coretree[j].right;
                                		rightflag = 1;
						j++;
	                                        leftassign = 0;
					}
/*					if (rightflag == 0){
                                		j++;
						leftassign = 0;
					}
*/
				}
				else if(coretree[j].right >= 0){
                                	mapnodes[mpitree[i].right] =  coretree[j].right;
	                                rightflag = 1;
					leftassign = 0;
        	                        j++;
                                }
				if(rightflag == 0){
                                        j++;
                                        leftassign = 0;
                                }

					

			}
		}
	}

	printf("\n\nMPI_rank          Actual_rank\n");
	for(i=0;i<nnodes+1;i++){
		printf("%d                   %d\n",i, mapnodes[i]);
	}
	
	return mapnodes;
}


void MCFA_print_submatrix(int ***subdistmat, int redundancy, int *newnodes)
{
	int i,j,k;
	for(i=0;i<redundancy;i++){
        printf("\nCluster:%d ",i);
        printf("\n      ");
        for(j=0;j<SL_numprocs/redundancy;j++){
            printf("[%3d]: ", newnodes[i*SL_numprocs/redundancy+j]);
        }
        printf("\n");
        for(j=0;j<SL_numprocs/redundancy;j++){
            printf("[%2d]: ", newnodes[i*SL_numprocs/redundancy+j]);
            for(k=0;k<SL_numprocs/redundancy;k++){
                printf("%6d  ",subdistmat[i][j][k]) ;
            }
            printf("\n");
        }
    }

	
}
void MCFA_cuttree (int nelements, MCFA_node* tree, int nclusters, int clusterid[])
{
    int i, j, k;
    int icluster = 0;
    const int n = nelements-nclusters; /* number of nodes to join */
    int* nodeid;
    for (i = nelements-2; i >= n; i--)
    { k = tree[i].left;
	if (k>=0)
	{ clusterid[k] = icluster;
	    icluster++;
	}
	k = tree[i].right;
	if (k>=0)
	{ clusterid[k] = icluster;
	    icluster++;
	}
    }
    nodeid = malloc(n*sizeof(int));
    if(!nodeid)
    { for (i = 0; i < nelements; i++) clusterid[i] = -1;
	return;
    }
    for (i = 0; i < n; i++) nodeid[i] = -1;
    for (i = n-1; i >= 0; i--)
    { if(nodeid[i]<0)
	{ j = icluster;
	    nodeid[i] = j;
	    icluster++;
	}
	else j = nodeid[i];
	k = tree[i].left;
	if (k<0) nodeid[-k-1] = j; else clusterid[k] = j;
	k = tree[i].right;
	if (k<0) nodeid[-k-1] = j; else clusterid[k] = j;
    }
    free(nodeid);
    return;
    
}


void MCFA_printtree(MCFA_node* result, int nnodes)
{
    
    int i;
    nnodes = nnodes-1;
    printf("\nNode     Item 1   Item 2    Distance\n");
    for(i=0; i<nnodes; i++){
	printf("%3d:%9d%9d      %g\n",
	       -i-1, result[i].left, result[i].right, result[i].distance);
	
    }
}
void MCFA_printclusterdist(int *clusterid)
{
    int i;
    printf("=============== Cutting a hierarchical clustering tree ==========\n");
    for(i=0; i<SL_numprocs; i++){
	printf("Proc %2d: cluster %2d\n", i, clusterid[i]);
    }
}

