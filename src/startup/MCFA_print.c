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
#include <stdarg.h>
#include "boinc_api.h"
#include "filesys.h"

extern "C"{
#include "MCFA.h"
//#include "MCFA_internal.h"
}


#define MAXLINE 2048
static FILE *fd=NULL;
static char buffer[MAXLINE][128];
static int bufcnt=0;
int printf_silence=0;

int MCFA_printf_init1 ( int spawnflag,  int id)
{
    char filename[32000];
    char resolved_name[512];


    sprintf(filename, "output.%d",id);

    if (spawnflag == 2){
        boinc_resolve_filename("output", resolved_name, sizeof(resolved_name));
        fd = boinc_fopen(resolved_name, "w");
    }
    else{
        fd = fopen (filename, "w");
    }
    if ( NULL == fd ) {
    printf("Error in opening file");
    }
    return 1;
}







int MCFA_printf_finalize1 ( void )
{
    int i;
    if (fd !=NULL){
        if ( bufcnt > 0 ) {
        for (i=0; i< bufcnt; i++ ) {
            fprintf(fd, "%s", buffer[i] );
        }
    }

    fclose ( fd );
    }

    int spawnflag=0;
        if(SL_this_procid == -1)
            return 1;

            spawnflag      = atoi(getenv("MCFA_SPAWN_FLAG"));
            if (spawnflag == 2)
                boinc_finish(0);
    return 1;
}



int printf ( const char* format, ... )

{
    va_list ap;
if(fd !=NULL){
    if ( !printf_silence ) {
    va_start ( ap, format );

    vsprintf(buffer[bufcnt], format, ap);
    bufcnt++;
    if ( bufcnt == MAXLINE ) {
        int i;

        for (i=0; i< MAXLINE; i++ ) {
        fprintf(fd, "%s", buffer[i] );
        }
        bufcnt = 0;
    }
    va_end (ap);
    }
    }
    return 1;
}











/*
int printf ( const char* format, ... )
{
    char resolved_name[512];

    boinc_resolve_filename("output", resolved_name, sizeof(resolved_name));
    fd = boinc_fopen(resolved_name, "a");



//    fd = fopen ("output", "a");
    if ( NULL == fd ) {
        printf("Error in opening file");
    }

va_list ap;
    if(fd !=NULL){
        va_start ( ap, format );
//        vsprintf(buffer[bufcnt], format, ap);
        vsprintf(buffer[0], format, ap);
//        bufcnt++;
//        if ( bufcnt == MAXLINE ) {
//            int i;

//            for (i=0; i< MAXLINE; i++ ) {
                fprintf(fd, "%s", buffer[0] );
//            }
//            bufcnt = 0;
//        }
        va_end (ap);
    }
int i;
    if (fd !=NULL){
        if ( bufcnt > 0 ) {
            for (i=0; i< bufcnt; i++ ) {
                fprintf(fd, "%s", buffer[i] );
            }
        }
        

        fclose ( fd );
//    }
    boinc_finish(0);
    return 0;
}
*/
