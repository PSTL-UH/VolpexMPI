#include "mpi.h"
#include "SL_msg.h"

extern int SL_this_procid;

VolPex_msg_header* VolPex_get_msg_header(int len, int dest, int tag, int comm, int reuse)
{
	VolPex_msg_header *header;
	
	header = (VolPex_msg_header *) malloc (sizeof(VolPex_msg_header));
	if(NULL == header){
		return NULL;
	}

	header->len 	= len;
	header->dest 	= dest;
	header->tag	= tag;
	header->comm	= comm;
	header->reuse	= reuse;
	
	return header;
}

VolPex_msg_header* VolPex_init_msg_header()
{
        VolPex_msg_header *header;

        header = (VolPex_msg_header *) malloc (sizeof(VolPex_msg_header));
        if(NULL == header){
                return NULL;
	}
        header->len     = -1;
        header->dest    = -1;
        header->tag     = -1;
        header->comm    = -1;
        header->reuse   = -1;

        return header;
}

int VolPex_compare_msg_header(VolPex_msg_header* header1, VolPex_msg_header* header2)
{

	int flag;
	if(header1->len <= header2->len && header1->dest == header2->dest &&
		header1->tag == header2->tag && header1->comm == header2->comm &&
		header1->reuse == header2->reuse){
		
		flag = 1;
	}
	else
		flag =  0;

	return flag;		
}

void VolPex_print_msg_header ( VolPex_msg_header *header )
{
    PRINTF(("%d: header: len=%d dest=%d tag=%d comm=%d reuse=%d\n", 
	    SL_this_procid, header->len, header->dest, 
	    header->tag, header->comm, header->reuse ));

    return;
}
