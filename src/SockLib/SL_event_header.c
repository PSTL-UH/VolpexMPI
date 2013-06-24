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

extern int SL_this_procid;

int print_options();


SL_event_msg_header* SL_get_msg_header (int cmd, int id, int jobid, int procid, int numprocs, int msglen, int port ,   
                                        char *executable, char *hostfile, char *hostname)
{
  SL_event_msg_header *header;
  
  header = (SL_event_msg_header *) malloc (sizeof(SL_event_msg_header));
  if(NULL == header){
    printf ("Memory error\n");
    exit(-1);
  }
  
  header->cmd       = cmd;
  header->id        = id;
  header->jobid     = jobid;
  header->procid    = procid;
  header->numprocs  = numprocs;
  header->msglen    = msglen;
  header->port	  = port;
  strcpy(header->executable, executable);
  strcpy(header->hostfile, hostfile);
  strcpy(header->hostname, hostname);
  
  return header;
}

SL_event_msg_header* SL_init_msg_header ( )
{
  SL_event_msg_header *header;
  
  header = (SL_event_msg_header *) malloc (sizeof(SL_event_msg_header));
  if(NULL == header){
    printf ("Memory error\n");
    exit(-1);
  }
  
  header->cmd       = -1;
  header->id        = -1;
  header->jobid     = -1;
  header->procid    = -1;
  header->numprocs  = -1;
  header->msglen    = -1;
  header->port      = -1;
  strcpy(header->executable, "");
  strcpy(header->hostfile, "");
  strcpy(header->hostname, "");
  
  return header;
}

