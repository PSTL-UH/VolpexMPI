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


//#define THRESHOLDCUT 600 
#define THRESHOLDCUT 3

int*** MCFA_dividedistmatrix(int **distmatrix, int redundancy, int *newnodes);
void MCFA_print_submatrix(int ***subdistmat, int redundancy, int *newnodes);

int** MCFA_cluster(MCFA_node *result, int totprocs, int **distmatrix, int *numclusters, int **numelems)
/* This process returns the newnode list where processes are arranged 
	such as first n processes form team 0
	next n processes form next team and so on

	the main function of this process is to divide processes in
	different teams
*/
{
    int i=0,j=0;
    int nnodes = totprocs-1;
    int nclusters = 0;
    int *clusterid;
    int *numelements;
    int **clusters;
    int *pos;

  
/* this loop calculates how many clusters are to be formed
   if the distance between two nodes of a tree in more than
   the given THRESHOLDCUT value the that node should be assined 
   to a new cluster
*/  
    for(i=1; i<nnodes; i++){
	if (result[i].distance - result[i-1].distance > THRESHOLDCUT){
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
    MCFA_cuttree (totprocs, result, nclusters, clusterid);
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

void MCFA_printclusterdist(int *clusterid)
{
    int i;
    printf("=============== Cutting a hierarchical clustering tree ==========\n");
    for(i=0; i<SL_numprocs; i++){
	printf("Proc %2d: cluster %2d\n", i, clusterid[i]);
    }
}

