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
#include "SL_proc.h"
#include "SL_msg.h"



int Volpex_insert_returnheader(Volpex_returnheaderlist **returnheaderList, Volpex_msg_header header, int target)
{
        Volpex_returnheaderlist *next;
        Volpex_returnheaderlist *prev;
        Volpex_returnheaderlist *curr;
        Volpex_returnheader theader;
	static int id;

        theader.header.src     = header.src;
        theader.header.dest    = header.dest;
        theader.header.comm    = header.comm;
        theader.header.reuse   = header.reuse;
        theader.header.tag     = header.tag;
        theader.header.len     = header.len;
        theader.target          = target;
	theader.header.timestamp	= header.timestamp;
	theader.id		= id++;

   curr=(Volpex_returnheaderlist *)malloc(sizeof(Volpex_returnheaderlist));
   if ( curr == NULL)
      return(-1);

   curr->next=NULL;
   curr->rheader=theader;

   if (*returnheaderList == NULL)
   {
      *returnheaderList=curr;
   }
   else
 {
      next=*returnheaderList;
      prev=NULL;

      while(next != NULL )
      {
         prev=next;
         next=next->next;
      }
      /* Now have a spot to insert */
      if (prev == NULL)
      {
         *returnheaderList=curr;
         curr->next=next;
      }
      else
      {
         curr->next = prev->next;
         prev->next=curr;
      }
   }

	PRINTF(("[%d]Value inserted src=%d dest=%d comm=%d reuse=%d tag=%d len=%d target=%d id=%d\n",
			SL_this_procid, header.src,header.dest,header.comm,
			header.reuse,header.tag,header.len,target,theader.id));
        return SL_SUCCESS;

}



int Volpex_init_returnheader( Volpex_returnheaderlist **returnheaderList)
{
	*returnheaderList = NULL;
	return SL_SUCCESS;
}

Volpex_returnheaderlist* Volpex_remove_returnheader(Volpex_returnheaderlist **returnheaderList, int id)
{
	Volpex_returnheaderlist *curr = *returnheaderList;
        Volpex_returnheaderlist *previous = NULL, *temp = NULL;

        while(curr->rheader.id != id)
        {
		previous = curr;
                curr = curr->next;
        }
        if(curr!=NULL)
        {
		if (curr == *returnheaderList){
			temp = *returnheaderList;
			*returnheaderList = temp->next;
			free(temp);
			return (*returnheaderList);
		}
		else{
                        previous->next = curr ->next;
			free(curr);
                }

		
        }

        return previous->next;

}

int Volpex_free_returnheader()
{
	return SL_SUCCESS;

}
