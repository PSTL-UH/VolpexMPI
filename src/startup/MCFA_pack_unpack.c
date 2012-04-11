#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#include "MCFA.h"
#include "MCFA_internal.h"

int MCFA_pack_size(int num_of_ints, int num_of_chars, int *buffer_length)
{
        int len=0;
        len += sizeof(int) * num_of_ints;
	len += sizeof(char) * num_of_chars;
	*buffer_length = len;
	return MCFA_SUCCESS;

}

int MCFA_pack_size_double(int num_of_doubles, int *buffer_length)
{
        int len=0;
        len += sizeof(double) * num_of_doubles;
        *buffer_length = len;
        return MCFA_SUCCESS;

}

int MCFA_pack_double(void *packbuf, int *from, int count, int *pos)
{
        char *cursor = (char*)packbuf;
        cursor += *pos;

        memcpy(cursor, from, count * sizeof(double));
        *pos += count * sizeof(double);
        return MCFA_SUCCESS;
}

int MCFA_pack_int(void *packbuf, int *from, int count, int *pos)
{
	char *cursor = (char*)packbuf;
	cursor += *pos;

	memcpy(cursor, from, count * sizeof(int));
	*pos += count * sizeof(int);
	return MCFA_SUCCESS;
}


int MCFA_pack_string(void *packbuf, char *from, int count, int *pos)
{
        char *cursor = (char*)packbuf;
        cursor += *pos;

         memcpy(cursor, from, count *  sizeof(char));
         *pos += count * sizeof(char);
	return MCFA_SUCCESS;
}

int MCFA_unpack_double(void *unpackbuf, int *to, int count, int *pos)
{
        char *cursor = (char*)unpackbuf;

        cursor += *pos;

        memcpy(to, cursor, count * sizeof(double));
        *pos += count * sizeof(double);
        return MCFA_SUCCESS;
}

int MCFA_unpack_int(void *unpackbuf, int *to, int count, int *pos)
{
        char *cursor = (char*)unpackbuf;

        cursor += *pos;
	
        memcpy(to, cursor, count * sizeof(int));
        *pos += count * sizeof(int);
	return MCFA_SUCCESS;
}

int MCFA_unpack_string(void *unpackbuf, char *to, int count, int *pos)
{
        char *cursor = (char*)unpackbuf;

        cursor += *pos;

        memcpy(to,cursor, count * sizeof(char));
        *pos += count * sizeof(char);
	return MCFA_SUCCESS;
}

