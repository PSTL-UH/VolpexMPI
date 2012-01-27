#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"

MCFA_node* MCFA_create_comm_matrix_sp(int redundancy, int **appcomm)
{
	int i,j;

/*        appcomm = (int**)malloc(SL_numprocs/redundancy * sizeof(int*));

        for(i=0;i<SL_numprocs/redundancy;i++)
                appcomm[i] = (int*)malloc(SL_numprocs/redundancy * sizeof(int));

	for(i=0;i<SL_numprocs/redundancy;i++){
                for(j=0;j<SL_numprocs/redundancy;j++){
                                appcomm[i][j]=0;
		}
	}
*/
	for(i=0;i<SL_numprocs/redundancy;i++){
                for(j=0;j<i;j++){
			if(i!=j)
				appcomm[i][j] = appcomm[j][i] = 8;
			else
				appcomm[i][j] = 0;
		}
	} 

	appcomm[0][4]=appcomm[1][5]=appcomm[1][6]=appcomm[2][7]=appcomm[3][7]=appcomm[4][8]=appcomm[5][6]=1;
	appcomm[4][0]=appcomm[5][1]=appcomm[6][1]=appcomm[7][2]=appcomm[7][3]=appcomm[8][4]=appcomm[6][5]=1;
	appcomm[0][8]=appcomm[8][0]=1;

	MCFA_node *tree;
        tree = MCFA_tree(appcomm, SL_numprocs/redundancy);
	printf("Application communication tree\n");
        MCFA_printtree(tree, SL_numprocs/redundancy);
	return tree;

}
