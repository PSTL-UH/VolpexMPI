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
  return 1;
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
  PRINTF(("\n      "));
  for(i=0; i<size; i++)
    PRINTF(("[%6d] ", i));
  PRINTF(("\n"));
  for(i=0; i<size; i++){
    PRINTF(("[%2d]: ", i));
    for(j=0; j<size; j++){
      PRINTF(("%6d ", procarray[i][j]));
    }
    PRINTF(("\n"));
  }
  return 0;
}

