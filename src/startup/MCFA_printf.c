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
#include <stdarg.h>

#define MAXLINE 2048

#ifdef MCFA_FILE_PER_PROC
static FILE *fd=NULL;
static char buffer[MAXLINE][128];
static int bufcnt=0;
#endif

int MCFA_printf_silence=0;

int MCFA_printf_init ( int jobid, int procid )
{
#ifdef MCFA_FILE_PER_PROC
  char filename[32000];
  
  char path[BUFFERSIZE]=".";
  getcwd(path,BUFFERSIZE);
  
  sprintf(filename, "%s/%d.%d.out", path,jobid,procid);
  
  fd = fopen (filename, "w");
  if ( NULL == fd ) {
	printf("Error in opening file");
  }
#endif
  return MCFA_SUCCESS;
}


int MCFA_printf_finalize ( void )
{
#ifdef MCFA_FILE_PER_PROC
  int i;
  if (fd !=NULL){
    if ( bufcnt > 0 ) {
      for (i=0; i< bufcnt; i++ ) {
	    fprintf(fd, "%s", buffer[i] );
      }
    }
    
    fclose ( fd );
  }
#endif
  return MCFA_SUCCESS;
}


int MCFA_printf ( const char* format, ... )
{
  va_list ap;
  if(fd !=NULL){
    if ( !MCFA_printf_silence ) {
      va_start ( ap, format );
#ifdef MCFA_FILE_PER_PROC
      vsprintf(buffer[bufcnt], format, ap);
      bufcnt++;
      if ( bufcnt == MAXLINE ) {
	    /*
	     * dump everything to the file and reset
	     * the counter
	     */
	    int i;
        
	    for (i=0; i< MAXLINE; i++ ) {
          fprintf(fd, "%s", buffer[i] );
	    }
	    bufcnt = 0;
      }
#else
      vprintf( format, ap );
#endif
      va_end (ap);
    }
  }
  return MCFA_SUCCESS;
}
