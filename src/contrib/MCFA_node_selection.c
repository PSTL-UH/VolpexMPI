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

extern struct MCFA_proc_node *procList;
extern MCFA_create_distmatrix_func *MCFA_create_distmatrix;
extern MCFA_create_comm_matrix_func *MCFA_create_comm_matrix; 


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
    int **clusters, numclusters;
    int *numelems;
    MCFA_node *tree;
    tree = MCFA_tree(procarray, SL_numprocs);
    
    
    clusters = MCFA_cluster(tree, SL_numprocs,procarray,&numclusters, &numelems);
    
    for(i=0; i<numclusters; i++){
        PRINTF(("Numelements:%d Cluster:%d ->",numelems[i],i));
        for(j=0;j<numelems[i];j++)
            PRINTF(("%d ",clusters[i][j]));
        PRINTF(("\n"));
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
/***************
first node from each cluster should send messages to each cluster and 
again create a distance matrix
Finally perform clustering based on it

******************/
    
    
    newnodes = MCFA_sortedlist(clusters, numclusters, numelems, redundancy, distanceclusters);
    
    int ***submat = NULL;
    submat = MCFA_dividedistmatrix(procarray, redundancy,newnodes);
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
    
    
    
    MCFA_print_distmatrix(submat[0], SL_numprocs/redundancy);
    MCFA_print_distmatrix(comm_mat, SL_numprocs/redundancy);
    for(i=0; i<redundancy; i++)
   	map_MPIPP(5,SL_numprocs/redundancy,Value[i],comm_mat, submat[i]);
    
    
    
    PRINTF(("\n"));
    for(i=0; i<redundancy; i++)
        for(j=0;j<SL_numprocs/redundancy;j++)
	    PRINTF(("%2d ", Value[i][j]));
    
    MCFA_map(Value, newnodes, redundancy);
    
    PRINTF(("\n"));
    for(i=0;i<SL_numprocs;i++)
    {
        printf("%2d ", newnodes[i]);
    }
    
    
    
    MCFA_update_fullrank(newnodes,redundancy);
    
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



void MCFA_update_fullrank(int *newnodes, int redundancy)
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





