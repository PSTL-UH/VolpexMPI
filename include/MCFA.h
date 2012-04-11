#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
#ifndef __MCFA_H__
#define __MCFA_H__

//#include "../src/startup/MCFA_internal.h"
//#include "../src/startup/MCFA_file.h"
#include "SL.h"

#define MCFA_SUCCESS 0
#define MCFA_ERROR   1

#define MAXHOSTNAMELEN 256
#define MAXNAMELEN	256


//int MCFA_Init( int argc, char **arg);

int MCFA_Init();
int MCFA_Finalize ( void );

int MCFA_printf ( const char* format, ... );




#endif
