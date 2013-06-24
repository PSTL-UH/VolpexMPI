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
extern MCFA_create_comm_matrix_func *MCFA_create_comm_matrix;


int MCFA_create_comm_matrix_sp(int redundancy, int **appcomm)
{
  int i,j;
  
  for(i=0;i<SL_numprocs/redundancy;i++){
    for(j=0;j<i;j++){
      if(i!=j)
        appcomm[i][j] = appcomm[j][i] = 1204;
      else
        appcomm[i][j] = 0;
    }
  } 
  
  appcomm[0][4]=appcomm[1][5]=appcomm[1][6]=appcomm[2][7]=appcomm[3][7]=appcomm[4][8]=appcomm[5][6]=0;
  appcomm[4][0]=appcomm[5][1]=appcomm[6][1]=appcomm[7][2]=appcomm[7][3]=appcomm[8][4]=appcomm[6][5]=0;
  appcomm[0][8]=appcomm[8][0]=0;
  
  return 1;
}


int MCFA_create_comm_matrix_bt(int redundancy, int **appcomm)
{
  int i,j;
  for(i=0;i<SL_numprocs/redundancy;i++){
    for(j=0;j<i;j++){
      if(i!=j)
        appcomm[i][j] = appcomm[j][i] = 604;
      else
        appcomm[i][j] = 0;
    }
  }
  
  appcomm[0][4]=appcomm[1][5]=appcomm[1][6]=appcomm[2][7]=appcomm[3][7]=appcomm[4][8]=appcomm[5][6]=0;
  appcomm[4][0]=appcomm[5][1]=appcomm[6][1]=appcomm[7][2]=appcomm[7][3]=appcomm[8][4]=appcomm[6][5]=0;
  appcomm[0][8]=appcomm[8][0]=0;
  return 1;
}


int MCFA_create_comm_matrix_cg(int redundancy, int **appcomm)
{
  int i,j;
  
  for(i=0;i<SL_numprocs/redundancy;i++){
    for(j=0;j<i;j++){
      appcomm[i][j] = 0;
    }
  }
  
  appcomm[0][1]=appcomm[0][2]=appcomm[1][0]=appcomm[2][0]=appcomm[1][3]=appcomm[3][1]=6004;
  appcomm[3][2]=appcomm[2][3]=appcomm[4][5]=appcomm[5][4]=appcomm[4][6]=appcomm[6][4]=6004;
  appcomm[5][7]=appcomm[7][5]=appcomm[6][7]=appcomm[7][6]=6004;
  appcomm[2][4]=appcomm[4][2]=appcomm[3][5]=appcomm[5][3]=1976;
  
  
  return 1;
}


int MCFA_create_comm_matrix_ep(int redundancy, int **appcomm)
{
  return 1;
}


int MCFA_create_comm_matrix_ft(int redundancy, int **appcomm)
{
  return 1;
}


int MCFA_create_comm_matrix_is(int redundancy, int **appcomm)
{
  return 1;
}


int MCFA_create_comm_matrix_bt16(int redundancy, int **appcomm)
{
  int i,j;
  
  for(i=0;i<SL_numprocs/redundancy;i++){
    for(j=0;j<i;j++){
      appcomm[i][j] = 0;
    }
  }
  for(i=0;i<12;i++){
    appcomm[i][3+i]=appcomm[3+i][i]=appcomm[i][4+i]=appcomm[4+i][i]=805;
  }
  
  
  for(i=0;i<SL_numprocs/redundancy-1;i++){
    appcomm[i][i+1]=appcomm[i+1][i]=805;
  }
  
  for(i=0;i<3;i++)
    appcomm[i][i+12]=appcomm[i+12][i]=appcomm[i][i+13]=appcomm[i+13][i]=805;
  
  appcomm[0][7]=appcomm[3][12]=appcomm[4][11]=appcomm[8][15]=appcomm[12][15]=appcomm[3][15]=805;
  appcomm[7][0]=appcomm[12][3]=appcomm[11][4]=appcomm[15][8]=appcomm[15][12]=appcomm[15][3]=805;
  appcomm[3][4]=appcomm[7][8]=appcomm[8][9]=appcomm[11][12]=0;	
  appcomm[4][3]=appcomm[8][7]=appcomm[9][8]=appcomm[12][11]=0;	
  
  return 1;
}


int MCFA_create_comm_matrix_cg16(int redundancy, int **appcomm)
{
  int i,j;
  
  for(i=0;i<SL_numprocs/redundancy;i++){
    for(j=0;j<i;j++){
      appcomm[i][j] = 0;
    }
  }
  
  appcomm[0][1]=appcomm[0][2]=appcomm[1][3]=appcomm[2][3]=6;
  appcomm[1][0]=appcomm[2][0]=appcomm[3][1]=appcomm[3][2]=6;
  
  appcomm[4][5]=appcomm[4][6]=appcomm[5][7]=appcomm[6][7]=6;
  appcomm[5][4]=appcomm[6][4]=appcomm[7][5]=appcomm[7][6]=6;
  
  appcomm[8][9]=appcomm[8][10]=appcomm[9][11]=appcomm[10][11]=6;
  appcomm[9][8]=appcomm[10][8]=appcomm[11][9]=appcomm[11][10]=6;	
  
  appcomm[12][13]=appcomm[12][14]=appcomm[13][15]=appcomm[14][15]=6;
  appcomm[13][12]=appcomm[14][12]=appcomm[15][13]=appcomm[15][14]=6;
  
  
  appcomm[1][4]=appcomm[2][9]=appcomm[3][13]=appcomm[6][9]=appcomm[7][13]=appcomm[11][14]=2;
  appcomm[4][1]=appcomm[9][2]=appcomm[13][3]=appcomm[9][6]=appcomm[13][7]=appcomm[14][11]=2;
  return 1;
}


int MCFA_create_comm_matrix_ep16(int redundancy, int **appcomm)
{
  return 1;
}


int MCFA_create_comm_matrix_ft16(int redundancy, int **appcomm)
{
  return 1;
}


int MCFA_create_comm_matrix_is16(int redundancy, int **appcomm)
{
  return 1;
}


int MCFA_create_comm_matrix_sp16(int redundancy, int **appcomm)
{
  int i,j;
  
  for(i=0;i<SL_numprocs/redundancy;i++) {
    for(j=0;j<i;j++){
      appcomm[i][j] = 0;
    }
  }
  for(i=0;i<12;i++) {
    appcomm[i][3+i]=appcomm[3+i][i]=appcomm[i][4+i]=appcomm[4+i][i]=1700;
  }
  for(i=0;i<SL_numprocs/redundancy-1;i++) {
    appcomm[i][i+1]=appcomm[i+1][i]=1700;
  }
  for(i=0;i<3;i++)
    appcomm[i][i+12]=appcomm[i+12][i]=appcomm[i][i+13]=appcomm[i+13][i]=1700;
  
  appcomm[0][7]=appcomm[3][12]=appcomm[4][11]=appcomm[8][15]=appcomm[12][15]=appcomm[3][15]=1700;
  appcomm[7][0]=appcomm[12][3]=appcomm[11][4]=appcomm[15][8]=appcomm[15][12]=appcomm[15][3]=1700;
  appcomm[3][4]=appcomm[7][8]=appcomm[8][9]=appcomm[11][12]=0;
  appcomm[4][3]=appcomm[8][7]=appcomm[9][8]=appcomm[12][11]=0;
  return 1;
}


int MCFA_create_comm_matrix_default(int redundancy, int **appcomm)
{
  int i,j;
  
  for(i=0;i<SL_numprocs/redundancy;i++){
    for(j=0;j<i;j++){
      if(i!=j)
        appcomm[i][j] = appcomm[j][i] = 8;
      else
        appcomm[i][j] = 0;
    }
  }
  return 1;
}

