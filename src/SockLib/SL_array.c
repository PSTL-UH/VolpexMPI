/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "SL.h"
//#include "SL_array.h"


int SL_array_init (SL_array_t **arr, const char name[64], int size )
{
    SL_array_t *tarr;

    tarr = (SL_array_t *) malloc ( sizeof(SL_array_t) );
    if ( NULL == tarr ) {
	return SL_ERR_NO_MEMORY;
    }

    strncpy ( tarr->name, name, 64 );
    tarr->last = -1;
    tarr->size = size;

    tarr->array=(SL_array_elem_t*)calloc(1,size*sizeof(SL_array_elem_t));
    if ( NULL == tarr->array ) {
	free ( tarr );
	return SL_ERR_NO_MEMORY;
    }

    *arr = tarr;
    return SL_SUCCESS;
}

/*
** Right now, we are not checking whether the element array is
** really free
*/
int SL_array_free (SL_array_t **arr )
{
    SL_array_t *tarr=*arr;

    if ( NULL != tarr) {
	if ( NULL != tarr->array ) {
	    free ( tarr->array );
	}
	free ( tarr );
    }
    *arr = NULL;
    return SL_SUCCESS;
}

/* Please note, we do not reserve this field at this step! */
int SL_array_get_next_free_pos  ( SL_array_t *arr, int *pos)
{
    SL_array_elem_t *oldarray=NULL;
    int oldsize;
    int i;
    int thispos=-1;

    if ( arr->last < (arr->size -1) ) {
	for (i=0; i< arr->size; i++ ) {
	    if ( arr->array[i].in_use == 0 ) {
		thispos = i;
		break;
	    }
	}
    }
    else {
	oldarray = arr->array;
	oldsize  = arr->size;
	
	arr->size *= 2;
	arr->array = (SL_array_elem_t*) calloc ( 1, arr->size *
						 sizeof(SL_array_elem_t));
	if ( NULL == arr->array ) {
	    *pos = -1;
	    return SL_ERR_NO_MEMORY;
	}
	
	memcpy ( arr->array, oldarray, oldsize * sizeof(SL_array_elem_t));
	free (oldarray);
	thispos = oldsize;
    }
    
    
    *pos = thispos;
    return SL_SUCCESS;
}

void * SL_array_get_ptr_by_pos ( SL_array_t *arr, int pos )
{
    if ( pos < arr->size ) {
	if ( arr->array[pos].in_use == 1 ) {
	    return arr->array[pos].ptr;
	}
    }
    
    return NULL;
}

void * SL_array_get_ptr_by_id  ( SL_array_t *arr, int id  )
{
    int i;
    
    for (i=0; i<arr->size; i++ ) {
	if ( arr->array[i].in_use == 1 && arr->array[i].id == id ) {
	    return arr->array[i].ptr;
	}
    }
    
    return NULL;
}

int SL_array_get_size ( SL_array_t *arr )
{
    return arr->size;
}

int SL_array_get_last ( SL_array_t *arr )
{
    return arr->last;
}

int SL_array_set_element (SL_array_t *arr, int pos, int id, void *ptr)
{
    int ret = SL_ERR_INTERNAL;
    
    if ( pos < arr->size ) {
	if ( 0 == arr->array[pos].in_use ) {
	    arr->array[pos].id      = id;
	    arr->array[pos].in_use  = 1;
	    arr->array[pos].ptr     = ptr;
	    ret = SL_SUCCESS;
	    if ( pos > arr->last ) {
		arr->last = pos;
	    }
	}
    }
    
    return ret;
}

int SL_array_remove_element ( SL_array_t *arr, int pos )
{
    if ( 1 == arr->array[pos].in_use ) {
	arr->array[pos].ptr    = NULL;
	arr->array[pos].id     = SL_UNDEFINED;
	arr->array[pos].in_use = FALSE;
	
	if ( pos == arr->last ) {
	    arr->last--;
	}
    }
    
    
    return SL_SUCCESS;
}
