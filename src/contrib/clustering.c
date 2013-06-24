#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"


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
  free(temp);
  return result;
}


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
