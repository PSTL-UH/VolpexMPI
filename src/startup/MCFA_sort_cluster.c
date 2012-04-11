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


int MCFA_sort_cluster(int **clusters, int numcluster, int *numelements, int **distmatrix)
{
        int i,j,k,temp1=0, temp2=0,p;
	int **newcluster, min, val, p1=0,p2=0, count=0;
	int node1,node2;


        newcluster = (int **) malloc (numcluster* sizeof(int*));
        for(i=0; i<numcluster; i++)
                newcluster[i] = (int *) malloc (numelements[i] * sizeof(int));


        for(i=0; i<numcluster; i++){
		if(numelements[i]<3)
			continue;
		while(count<numelements[i]){
                        min=1000;
			
                /*      temp1=clusters[i][j];
                        temp2=clusters[i][j+1];

                        if(clusters[i][j] > clusters[i][j+1])
                                min = distmatrix[clusters[i][j]][clusters[i][j+1]];
                        else
                                min = distmatrix[clusters[i][j+1]][clusters[i][j]];
                */
                  for(p=0;p<numelements[i];p++){
			if(clusters[i][p]==-1)
				continue;
			node1=clusters[i][p];
                        for(k=p+1; k<numelements[i]; k++){
                                if (clusters[i][k]==-1 )
                                        continue;
				node2=clusters[i][k];

                                if(node2>node1)
                                        val = distmatrix[node2][node1];
                                else
                                        val = distmatrix[node1][node2];
                                if(val < min){

                                        if(node2 > node1)
                                        min = distmatrix[node2][node1];
                                        else
                                        min = distmatrix[node1][node2];
//                                      min = distmatrix[clusters[i][k]][clusters[i][j]];
                                        temp1 = node1;
                                        temp2 = node2;
                                        p1=k;p2=p;
                                }
                        }
                }
                        printf("%d %d = %d\n", temp1, temp2,min);
                        newcluster[i][j] = temp1;
                        newcluster[i][j+1] = temp2;
                        clusters[i][p1]=-1;
                        clusters[i][p2]=-1;
			count=count+2;

                }
        }

        for(i=0; i<numcluster; i++){
        printf("Cluster:%d ->",i);
        for(j=0;j<numelements[i];j++)
            printf("%d ",newcluster[i][j]);
        printf("\n");
        }
    return 1;
}

int* MCFA_sortedlist (int ** clusters, int nclusters, int *numelements, int redundancy, int **distanceclusters
)
{

    int *numelementsleft,i, num=0, *newnodes, *closeclusters;
    int max=-1, pos=0, count=0,k,min=2000000,assignteam=0;
    int p,q, temp, minflag=0, maxflag=0, leftprocs=0;;
    int nextelem=0;
    int oldpos=0;
    int clusterpos=0;
    numelementsleft = malloc(nclusters*sizeof(int)); //calculates number of processes assined and number of
    closeclusters = malloc(nclusters*sizeof(int)); //procs unassined
    for(i=0;i<nclusters;i++){
        numelementsleft[i]=numelements[i];
    }



    newnodes = malloc(SL_numprocs*sizeof(int));
    while(assignteam < redundancy){
	max = -1;
        for(k=0;k<nclusters;k++){
            if(numelementsleft[k]>max){
                max = numelementsleft[k];
                pos = k;
            }
        }
        num=0;	
	while(num<SL_numprocs/redundancy){

		count = 0;
		for(i=0;i<numelements[pos];i++){
			if(clusters[pos][i]!=-1){
				newnodes[nextelem++] = clusters[pos][i];
				clusters[pos][i] = -1;
				num++; count++;
				if(num==SL_numprocs/redundancy)
					break;
			}
		}

		numelementsleft[pos] = numelementsleft[pos] - count;
                leftprocs = SL_numprocs/redundancy - num;

		if(leftprocs > 0){
			//Find the minimum distance
			oldpos = pos;
			clusterpos=0;
	                for(k=0;k<nclusters;k++){
        	            if (distanceclusters[k][oldpos] <= min && numelementsleft[k] > 0){
                	        min = distanceclusters[k][oldpos];
                    	    }
                	}

			for(k=0;k<nclusters;k++){
                    		if (distanceclusters[k][oldpos] == min && numelementsleft[k] > 0){
                        		closeclusters[clusterpos++] = k;
                        		if(numelementsleft[k]<leftprocs)
                                		minflag = 1;
                        		if(numelementsleft[k]>leftprocs)
                                		maxflag = 1;
                         	}
                	}

			//sort clusters based on the size of cluster
	
        	        for(p=0; p<clusterpos; p++){
                	    for(q=p;q<clusterpos; q++){
                        	if(numelementsleft[closeclusters[q]]<numelementsleft[closeclusters[p]]){
                            		temp = closeclusters[p];
                            		closeclusters[p]=closeclusters[q];
                            		closeclusters[q]=temp;
                        	}
                    	     }
                	}

			if(minflag==1 && maxflag==1){
		                for(k=0;k<clusterpos;k++){
                		        if(numelementsleft[closeclusters[k]] >= leftprocs){
                                		pos = k;
                                		break;
                        		}
                		}		
            		}
            		if(minflag==1 && maxflag==0){
                		pos = closeclusters[clusterpos - 1];
            		}
            		if(minflag==0 ){
                		pos = closeclusters[0];
            		}


		}
	}
	assignteam++;
    }

	return newnodes;
}

int MCFA_map(int **values, int *newnodes, int redundancy)
{
	int i,j,k,temp;
	int N= SL_numprocs/redundancy;
	for(i=0;i<redundancy;i++){

                for(j=0;j<N;j++){
                	for(k=j;k<N;k++){
                         	if(values[i][k] < values[i][j]){
                                	temp = newnodes[i*N+k];
                                        newnodes[i*N+k] = newnodes[i*N+j];
                                        newnodes[i*N+j] = temp;

					temp = values[i][j];
                                        values[i][j] = values[i][k];
                                        values[i][k] = temp;
                                        }

                         }
                }
        }
	return 1;
}
