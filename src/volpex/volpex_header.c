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
#include "mpi.h"
#include "SL_msg.h"


extern int SL_this_procid;
 
Volpex_msg_header* Volpex_get_msg_header(int len, int src, int dest, int tag, int comm, int reuse)
{
	Volpex_msg_header *header;
	
	header = (Volpex_msg_header *) malloc (sizeof(Volpex_msg_header));
	if(NULL == header){
	    return (Volpex_msg_header*)SL_ERR_NO_MEMORY;
	}

	header->len 	= len;
	header->src 	= src;
	header->dest 	= dest;
	header->tag	= tag;
	header->comm	= comm;
	header->reuse	= reuse;
	header->timestamp = SL_papi_time();
	return header;
}

Volpex_msg_header* Volpex_init_msg_header()
{
        Volpex_msg_header *header;

        header = (Volpex_msg_header *) malloc (sizeof(Volpex_msg_header));
        if(NULL == header){
                return (Volpex_msg_header*)SL_ERR_NO_MEMORY;
	}
        header->len     = -1;
        header->src     = -1;
        header->dest    = -1;
        header->tag     = -1;
        header->comm    = -1;
        header->reuse   = -1;

        return header;
}

/*int Volpex_compare_msg_header(Volpex_msg_header* header1, Volpex_msg_header* header2 )
{

	int flag;
	int maxreuseval = 0;
	if ( header1->len   >= header2->len     && 
	     header1->src   == header2->src     &&
	     header1->dest  == header2->dest    &&
	     header1->tag   == header2->tag     && 
	     header1->comm  == header2->comm    &&
	     header1->reuse == header2->reuse ) {
		flag = 1;
	}
	else
	    flag =  0;
	
	return flag;		
}*/

int Volpex_compare_msg_header(Volpex_msg_header* header1, Volpex_msg_header* header2 , int *reuse )
{

        int flag = 0;
        if ( header1->len   <= header2->len     &&
             header1->src   == header2->src     &&
             header1->dest  == header2->dest    &&
             header1->tag   == header2->tag     &&
             header1->comm  == header2->comm  ){
	     *reuse = header1->reuse;	

  
          if (header1->reuse == header2->reuse ) 
                flag = 1;
        }

        return flag;
}




int Volpex_compare_msg_progress(Volpex_msg_header* header1, Volpex_msg_header* header2, int *msgprogress)
{
	/** header2 = header recieved 
	    header1 = current header
	**/

        int flag;
        if ( header1->len   <= header2->len     &&
             header1->src   == header2->src     &&
             header1->dest  == header2->dest    &&
             header1->tag   == header2->tag     &&
             header1->comm  == header2->comm )   {
             
                flag = 1;
        }
        else
            flag =  0;

	if (flag == 1){
		printf("My reuse       %d\n",header1->reuse);
		printf("Recived reuse  %d\n",header2->reuse);
/*		Volpex_print_msg_header(header1);
		Volpex_print_msg_header(header2);*/
        	*msgprogress = header1->reuse - header2->reuse;
	}
	return flag;
}


void Volpex_print_msg_header ( Volpex_msg_header *header )
{
    printf("%d: header: len=%d src=%d dest=%d tag=%d comm=%d reuse=%d\n",
            SL_this_procid, header->len, header->src, header->dest,
            header->tag, header->comm, header->reuse );

    return;
}
