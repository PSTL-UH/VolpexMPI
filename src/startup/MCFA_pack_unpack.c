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


int MCFA_pack_int(void *packbuf, int *from, int count, int *pos)
{
	char *cursor = packbuf;
	cursor += *pos;

	memcpy(cursor, from, count * sizeof(int));
	*pos += count * sizeof(int);
	return MCFA_SUCCESS;
}


int MCFA_pack_string(void *packbuf, char *from, int count, int *pos)
{
        char *cursor = packbuf;
        cursor += *pos;

         memcpy(cursor, from, count *  sizeof(char));
         *pos += count * sizeof(char);
	return MCFA_SUCCESS;
}

int MCFA_unpack_int(void *unpackbuf, int *to, int count, int *pos)
{
        char *cursor = unpackbuf;

        cursor += *pos;
	
        memcpy(to, cursor, count * sizeof(int));
        *pos += count * sizeof(int);
	return MCFA_SUCCESS;
}

int MCFA_unpack_string(void *unpackbuf, char *to, int count, int *pos)
{
        char *cursor = unpackbuf;

        cursor += *pos;

        memcpy(to,cursor, count * sizeof(char));
        *pos += count * sizeof(char);
	return MCFA_SUCCESS;
}

