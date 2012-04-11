/*
 * Copyright (c) 2006-2012      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef __SL_ARRAY_H__
#define __SL_ARRAY_H__

struct SL_array_elem_s {
    int     id;  /* id to search for */
    int in_use;  /* is this element currently occupied? */
    void  *ptr;  /* data ptr */
};
typedef struct SL_array_elem_s SL_array_elem_t;

struct SL_array_s {
    char            name[64];  /* name of the type, for debugging purposes */
    int                 size;  /* length of the data array */
    int                 last;  /* last used element */
    SL_array_elem_t *array;
};
typedef struct SL_array_s SL_array_t;

int SL_array_init (SL_array_t **arr, const char name[64], int size );
int SL_array_free (SL_array_t **arr );

int SL_array_get_next_free_pos  ( SL_array_t *arr, int *pos);
void * SL_array_get_ptr_by_pos ( SL_array_t *arr, int pos );
void * SL_array_get_ptr_by_id  ( SL_array_t *arr, int id  );
int SL_array_set_element (SL_array_t *arr, int pos, int id, void *ptr);
int SL_array_remove_element ( SL_array_t *arr, int pos );
int SL_array_get_last ( SL_array_t *arr );
int SL_array_get_size ( SL_array_t *arr );

#endif /* __SL_ARRAY_H__ */

