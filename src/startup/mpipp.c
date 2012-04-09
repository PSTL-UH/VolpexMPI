/*#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
*/
#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"
//#define DBL_MAX 100000.00

typedef struct {
  int *arity; /* arity of the nodes of each level*/
  int nb_levels; /*number of levels of the tree*/ 
  int *nb_nodes; /*nb of nodes of each level*/
  int **node_id; /*ID of the nodes of the tree for each level*/
}tm_topology_t;

typedef struct{
  int  val;
  long key;
 }hash_t;

 int hash_asc(const void* x1,const void* x2); 
int eval_sol(int *sol,int N,int **comm, int **arch);
void exchange(int *sol,int i,int j);
 int gain_exchange(int *sol,int l,int m,double eval1,int N,int **comm, int **arch);
 void select_max(int *l,int *m,double **gain,int N,int *state);
int *generate_random_sol(int N,int level,int seed);
void compute_gain(int *sol,int N,double **gain,int **comm, int **arch);
 void map_MPIPP(int nb_seed,int N,int *Value,int **comm, int **arch);
double print_sol(int N,int *Value,int **comm, int **arch);
void free_topology(tm_topology_t *topology);

 int hash_asc(const void* x1,const void* x2){ 
 
 hash_t *e1,*e2;
 
 e1=((hash_t*)x1);
 e2=((hash_t*)x2);

   
 return e1->key<e2->key?-1:1;
} 

int eval_sol(int *sol,int N,int **comm, int **arch){
  double res;
  int i,j;
   int a,c;

   res=0;
   for (i=0;i<N;i++){
     for (j=i+1;j<N;j++){
       c=comm[i][j];
       a=arch[sol[i]][sol[j]];
       res+=(double)c*(double)a;
    }
   }
   return res;
 }

void exchange(int *sol,int i,int j){
   int tmp;
   tmp=sol[i];
   sol[i]=sol[j];
   sol[j]=tmp;
 }

 int gain_exchange(int *sol,int l,int m,double eval1,int N,int **comm, int **arch){
   double eval2;
   if(l==m)
     return 0;
   exchange(sol,l,m);
   eval2=eval_sol(sol,N,comm,arch);
   exchange(sol,l,m);
   return eval1-eval2;
 }

 void select_max(int *l,int *m,double **gain,int N,int *state){
   int i,j;
   double max;
   max=-DBL_MAX;

   for(i=0;i<N;i++){
     if(!state[i]){
       for(j=0;j<N;j++){
           if((i!=j)&&(!state[j])){
             if(gain[i][j]>max){
               *l=i;*m=j;
               max=gain[i][j];
             }
           }
       }
     }
   }

 }

int *generate_random_sol(int N,int level,int seed){
   hash_t *hash_tab;
   int *sol,i;
   int *nodes_id;

   nodes_id=(int*)malloc(sizeof(int)*N);
   for(i=0;i<N;i++)
   	nodes_id[i]=i;

   hash_tab=(hash_t*)malloc(sizeof(hash_t)*N);
   sol=(int*)malloc(sizeof(int)*N);

   srandom(seed);

   for(i=0;i<N;i++){
     hash_tab[i].val=nodes_id[i];
     hash_tab[i].key=random();
   }

   qsort(hash_tab,N,sizeof(hash_t),hash_asc);
   for(i=0;i<N;i++){
     sol[i]=hash_tab[i].val;
   }
  free(hash_tab);
  return sol;
 }

void compute_gain(int *sol,int N,double **gain,int **comm, int **arch){
   int i,j;
   int eval1;
   eval1=eval_sol(sol,N,comm,arch);
   for(i=0;i<N;i++){
     for(j=0;j<=i;j++){
       gain[i][j]=gain[j][i]=gain_exchange(sol,i,j,eval1,N,comm,arch);
     }
   }
 }


 void map_MPIPP(int nb_seed,int N,int *Value,int **comm, int **arch){
   int *sol;
   int *state;
   double **gain;
   int **history;
   double *temp;
   int i,j,t,l=0,m=0,loop=0,seed=0;
   double max,sum,best_eval,eval;


   gain=(double**)malloc(sizeof(double*)*N);
   for(i=0;i<N;i++){
     gain[i]=(double*)malloc(sizeof(double)*N);
     if(!gain[i]){
     }
   }
   history=(int**)malloc(sizeof(int*)*N);
   for(i=0;i<N;i++)
     history[i]=(int*)malloc(sizeof(int)*3);

   state=(int*)malloc(sizeof(int)*N);
   temp=(double*)malloc(sizeof(double)*N);

   sol=generate_random_sol(N,0,seed++);
   for(i=0;i<N;i++)
     Value[i]=sol[i];

   best_eval=DBL_MAX;
while(seed<=nb_seed){
     loop=0;
     do{

       for(i=0;i<N;i++){
         state[i]=0;
         PRINTF(("%d ",sol[i]));
       }
       PRINTF(("\n"));
       compute_gain(sol,N,gain,comm,arch);

/*	for(i=0;i<N;i++){
             for(j=0;j<N;j++){
                printf("%d ",comm[i][j]);
        }
        printf("\n");
        }
	for(i=0;i<N;i++){
             for(j=0;j<N;j++){
                printf("%6.1f ",gain[i][j]);
        }
    	printf("\n");
        }
*/
//      display_tab(gain,N);
      //exit(-1);
       for(i=0;i<N/2;i++){
         select_max(&l,&m,gain,N,state);
         PRINTF(("%d: %d <=> %d : %f\n",i,l,m,gain[l][m]));
         state[l]=1;state[m]=1;
         exchange(sol,l,m);
	 for(j=0;j<N;j++){
                PRINTF(("%d ",sol[j]));
       }
	PRINTF(("\n"));
         history[i][1]=l;history[i][2]=m;
         temp[i]=gain[l][m];
         compute_gain(sol,N,gain,comm,arch);
       }

       t=-1;
       max=0;
       sum=0;
       for(i=0;i<N/2;i++){
         sum+=temp[i];
         if(sum>max){
           max=sum;
           t=i;
         }
       }
       for(j=0;j<=t;j++)
         PRINTF(("exchanging: %d with %d for gain: %f\n",history[j][1],history[j][2],temp[j])); 
       for(j=t+1;j<N/2;j++){
         exchange(sol,history[j][1],history[j][2]);
         PRINTF(("Undoing: %d with %d for gain: %f\n",history[j][1],history[j][2],temp[j]));
       }
       PRINTF(("max=%f\n",max));
      for(i=0;i<N;i++){
         PRINTF(("%d ",sol[i]));
         }
         PRINTF(("\n"));
       eval=eval_sol(sol,N,comm,arch);
       if(eval<best_eval){
         best_eval=eval;
         for(i=0;i<N;i++)
           Value[i]=sol[i];
       }


     }while(max>0);

     sol=generate_random_sol(N,0,seed++);

   }

   if (NULL != gain){
        for (i =0; i< N; i++){
            if (gain[i] != NULL){
                free(gain[i]);
            }
        }
        free(gain);
    }
    if (NULL != history){
        for (i =0; i< N; i++){
            if (history[i] != NULL){
                free(history[i]);
            }
        }
        free(history);
    }


 }

double print_sol(int N,int *Value,int **comm, int **arch){
  double sol;
  int i,j;
  int a,c;

  sol=0;
  for (i=0;i<N;i++){
    for (j=i+1;j<N;j++){
       c=comm[i][j];
       a=arch[Value[i]][Value[j]];
      printf("T_%d_%d %d/%d=%f\n",i,j,c,a,(double)c/a);
       sol+=(double)c/(double)a;
     }
   }
   for (i = 0; i < N; i++) {
     printf("%d", Value[i]);
    if(i<N-1)
       printf(",");
       
   }
   printf(" : %g\n",sol);
 
   return sol;
 }

void free_topology(tm_topology_t *topology){
   int i;
   for(i=0;i<topology->nb_levels;i++)
     free(topology->node_id[i]);
   free(topology->node_id);
   free(topology->nb_nodes);
   free(topology->arity);
   free(topology);
 }

void display_tab(int **mat,int N){
int i,j;
	for(i=0;i<N;i++){
             for(j=0;j<N;j++){
                printf("%6d ",mat[i][j]);
        }
     printf("\n");
        }

}

