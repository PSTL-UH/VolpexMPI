#include "SL.h"

extern int SL_this_procid;

int SL_socket ( void )
{
#ifdef MINGW 
    SOCKET handle;

    if (( handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        printf("WinSL: Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }
#else
    int handle;

    if ( ( handle = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
	PRINTF(("SL_socket: socket failed, %d %s\n", errno, strerror(errno)));
	return -1;
    }
#endif
    return ( handle );
}

int SL_bind_static ( int handle, int port )
{
#ifdef MINGW
    struct sockaddr_in lserver;
 
    lserver.sin_family = AF_INET;
    lserver.sin_addr.s_addr = INADDR_ANY;
    lserver.sin_port = htons(port);
 
    if (bind((SOCKET)handle, (SOCKADDR *)&lserver, sizeof(lserver)) == SOCKET_ERROR)
    {
        PRINTF(("Windows SL_bind_static: bind() failed: %ld.\n", WSAGetLastError()));
        return -1;
    }
#else
	struct sockaddr_in lserver;

	lserver.sin_family      = AF_INET;
	lserver.sin_addr.s_addr = INADDR_ANY;
	lserver.sin_port        = htons (port);

	if ( bind ( handle,(struct sockaddr *) &lserver, sizeof (lserver) ) )
    {
		PRINTF(("SL_bind_static: %d failed to bind to port %d, %d %s\n", SL_this_procid, port, errno, strerror(errno) ));
		return -1;
    }
#endif
  return ( ntohs(lserver.sin_port) );
}

int SL_bind_dynamic (  int handle, int port )
{
#ifdef MINGW
    struct sockaddr_in lserver;
    int retry=0;

    lserver.sin_family = AF_INET;
    lserver.sin_addr.s_addr = INADDR_ANY;
    lserver.sin_port = htons(port);
 
    while ( (bind((SOCKET)handle, (SOCKADDR *)&lserver, sizeof(lserver)) == SOCKET_ERROR) &&  ( retry < SL_RECONN_MAX ))
    {
        PRINTF(("Windows SL_open_bind_static: bind() failed: %ld.\n", WSAGetLastError()));
		lserver.sin_port = htons (port++);
        retry++;
    }
#else
  struct sockaddr_in lserver;
  int retry=0;

  lserver.sin_family      = AF_INET;
  lserver.sin_addr.s_addr = INADDR_ANY;
  lserver.sin_port        = htons (port);

  while ( ( bind ( handle,(struct sockaddr *) &lserver, sizeof (lserver) ) ) &&
          ( retry < SL_RECONN_MAX ))
    {
      PRINTF(("SL_open_bind_dynamic: %d failed to bind to port %d\n", SL_this_procid,port ));
      lserver.sin_port = htons (port++);
      retry++;
    }
#endif
  return ( ntohs(lserver.sin_port) );
}

int SL_socket_close ( int handle )
{
#ifdef MINGW
	Sleep(1);
    
    	PRINTF(("close_socket: close socket\n"));
    	if ( closesocket( handle) < 0) {
	PRINTF(("close_socket: failed , reason: %s\n", strerror(errno)));
	return -1;
    }
#else
	sleep(1);
    
      PRINTF(("close_socket: close socket\n"));

	if ( close( handle) < 0) {
	PRINTF(("close_socket: failed , reason: %s\n", strerror(errno)));
	return -1;
    }
#endif


    return SL_SUCCESS;
}

int SL_open_socket_conn ( int *handle, const char *as_host, int port )
{
    struct sockaddr_in server;
    struct hostent     *host;
    int conn_ret=-1;
    int reconn=0;
    unsigned sleep_ret=0;
    
    do {
	/*
	** Get a socket
	*/
	*handle = SL_socket ();
	
	/*
	** Bind it to any port starting with port+SL_BIND_PORTSHIFT
	*/
	SL_bind_dynamic ( *handle, port+SL_BIND_PORTSHIFT );
	
	/*
	** make the socket fast and nice
	*/
	SL_configure_socket( *handle);
	
	/*
	** Get the hostent structure of the host we want to connect
	** ATTENTION: gethostbyname does NOT set errno, but instead the h_errno!!!
	!
	*/
	if ( (host = gethostbyname ( as_host ) ) == NULL )   {
	    PRINTF (("SL_open_socket_conn: failed to get hostname:%s, %d %s\n", 
		    as_host, h_errno, strerror(h_errno)));
	    return -1;
	}

	/*
	** Fill in the important elements of the server-structure
	*/
	memcpy (&server.sin_addr, host->h_addr, host->h_length );
	server.sin_family = AF_INET;
	server.sin_port   = htons ( port );
	
	do  {
	    conn_ret = connect(*handle,(struct sockaddr *)&server, sizeof(server));
	    if ( conn_ret < 0 )  {
		PRINTF (("SL_open_socket_conn: Connect returned with ret = %d, "
			"reason %s", conn_ret, strerror(errno )));
#ifdef MINGW
		Sleep (SL_SLEEP_TIME);
#else
		sleep_ret = sleep (SL_SLEEP_TIME);
		if ( sleep_ret != 0 )
		    sleep (SL_SLEEP_TIME);
#endif
		
		}
		
        } while( (conn_ret < 0) && (errno==EINTR)  );

	/*
	** If the host was not ready for the connect call, we retry
	*/
	if ( conn_ret < 0 )
        {
	    fprintf(stderr, "SL_open_socket_conn: Connect %d failed to host %s on "
		    "port %d \n",
		    reconn, as_host, port );
	    SL_socket_close ( *handle);
	    reconn++;
#ifdef MINGW
		Sleep (SL_SLEEP_TIME);
#else
	   sleep_ret = sleep (SL_SLEEP_TIME);
	    if ( sleep_ret != 0 ) {
		sleep (SL_SLEEP_TIME);
		}
#endif
		
		
		}
	
    } while ( (conn_ret < 0) && (reconn < SL_RECONN_MAX ) ) ;

    /*
    ** After SL_RECONN_MAX repetitions we give up contacting the host
    */
    if ( conn_ret < 0) {
	PRINTF(("SL_open_socket_conn: failed to open connection with port %d",port));
	return -1;
    }

    return SL_SUCCESS;
}


int SL_open_socket_conn_nb ( int *handle, const char *as_host, int port )
{
    struct sockaddr_in server;
    struct hostent     *host;
    int conn_ret=-1;
    
    /*
    ** Get a socket
    */
    *handle = SL_socket ();
    
    /*
    ** Bind it to any port starting with port+SL_BIND_PORTSHIFT
    */
    SL_bind_dynamic ( *handle, port+SL_BIND_PORTSHIFT );
    
    /*
    ** make the socket fast and nice
    */
    SL_configure_socket_nb ( *handle);
    
    /*
    ** Get the hostent structure of the host we want to connect
    ** ATTENTION: gethostbyname does NOT set errno, but instead the h_errno!!!
    */
    if ( (host = gethostbyname ( as_host ) ) == NULL )   {
      PRINTF (("SL_open_socket_conn_nb: failed to get hostname:%s, %d %s\n", 
	      as_host, h_errno, strerror(h_errno)));
      return -1;
    }
    
    /*
    ** Fill in the important elements of the server-structure
    */
    memcpy (&server.sin_addr, host->h_addr, host->h_length );
    server.sin_family = AF_INET;
    server.sin_port   = htons ( port );
    
    conn_ret = connect(*handle,(struct sockaddr *)&server, sizeof(server));
#ifdef MINGW
	if ( conn_ret < 0 && errno != WSAEINPROGRESS) {
        PRINTF (("SL_open_socket_conn_nb: Connect returned with ret = %d, reason %s\n", conn_ret, strerror(errno)));
	return errno;
    }
#else
	if ( conn_ret < 0 && errno != EINPROGRESS)  {
        PRINTF (("SL_open_socket_conn_nb: Connect returned with ret = %d, "
		"reason %s\n", conn_ret, strerror(errno)));
	return errno;
    }
#endif
    return SL_SUCCESS;
}


int SL_open_socket_bind ( int *handle, int port )
{
  int local_port;

  /*
  ** Get a socket
  */
  *handle = SL_socket ();

  /*
  ** Bind a port to this socket
  */
  local_port = SL_bind_static ( *handle, port );

  /*
  ** make the socket fast and nice
  */
  SL_configure_socket( *handle);

  return (  local_port );
}


int SL_open_socket_listen ( int sock )
{
#ifdef MINGW
    SOCKET handle;
#else
    int handle;
#endif
 
    if(listen ( sock, 20) < 0 ) {
	PRINTF(("SL_open_socket_list: error in listen, %d %s\n", 
	       errno, strerror(errno)));
	return -1;
    }
    
    do {
	handle = accept ( sock, 0, 0 );       
    } while( (handle==-1) && (errno==EINTR) );
    
    if (  handle < 0 )  {
	PRINTF(("SL_open_socket_list: failed to listen at socket %d, %d %s",
	       sock, errno, strerror(errno)));
	return -1;
    }

    SL_configure_socket (handle);
    return ( handle );
}

int SL_open_socket_listen_nb ( int *handle, int port )
{
    /*
    ** Get a socket
    */
    *handle = SL_socket ();
    
     /*
    ** Bind a port to this socket
    */
    SL_bind_static ( *handle, port );
    
    /*
    ** make the socket fast and nice
    */
    SL_configure_socket_nb ( *handle);
    
    if(listen ( *handle, 5 ) < 0 ) {
	PRINTF(("SL_open_socket_listen_nb: error in listen, %d %s\n", 
	       errno, strerror(errno)));
	return errno;
    }
    
    return SL_SUCCESS;
}

/* 
** A note on the following communication routines:
** - SL_socket_write_nb and SL_socket_read_nb are supposed to be 
**   the routines used for large messages, high performance commmunication
** - SL_socket_write and SL_socket_read are supposed to be used 
**   whenever you have to emulate the behaviour of blocking sockets. This
**   is supposed to happen only for shutdown and startup handshakes with 
**   relatively small amount of data (e.g. a couple of integer values maximum).
**   Please note, that these routines take a timeout argument. If the communication does not
**   succeed within the time frame specified, we return an SL_ERR_PROC_UNREACHABLE. 
**   A timeout of zero means infinite ( = no timeout).
*/

int SL_socket_write ( int hdl, char *buf, int num, double timeout )
{
    int lcount=0;
    int a;
    char *wbuf = buf;
    double starttime = SL_Wtime();
    double endtime;

    do {
#ifdef MINGW
	a = send ( hdl, wbuf, num, 0 );
#else
	a = write ( hdl, wbuf, num);
#endif
#ifdef MINGW
	if ( a == -1 ) {
	    if ( errno == WSAEINTR || errno == WSAEINTR || errno == WSAEINPROGRESS ||
		 errno == WSAEWOULDBLOCK ) {
	    }
	    else {
		PRINTF(("SL_socket_write: error while writing to socket %s\n", 
			strerror(errno)));
		return errno;
	    }
#else
	if ( a == -1 ) {
	    if ( errno == EINTR || errno == EAGAIN || errno == EINPROGRESS ||
		 errno == EWOULDBLOCK ) {
	    }
	    else {
		PRINTF(("SL_socket_write: error while writing to socket %s\n", 
			strerror(errno)));
		return errno;
		}
#endif

	    lcount++;
	    continue;
	}
	wbuf += a;
	num -= a;
	
	if ( timeout > 0 ) {
	    endtime = SL_Wtime();
	    if ( (endtime - starttime) > timeout ) {
		PRINTF(("SL_socket_write: data transfer operation timed out after %lf secs\n",
			(endtime-starttime)));
		return SL_ERR_PROC_UNREACHABLE;
	    }
	}
	
    } while ( num > 0 );
    PRINTF (("SL_socket_write: wrote %d bytes to %d lcount=%d num=%d\n", 
	     a, hdl, lcount, num ));
    
    return SL_SUCCESS;
}

int SL_socket_write_nb ( int hdl, char *buf, int num, int *numwritten )
{
    int ret;

#ifdef MINGW
    ret = send ( hdl, buf, num, 0 );
#else
   ret = write ( hdl, buf, num);
#endif
#ifdef MINGW
    if ( ret == -1 ) {
	*numwritten = 0;
	if ( errno == WSAEINTR || errno == WSAEINTR || errno == WSAEINPROGRESS ||
	     errno == WSAEWOULDBLOCK ) {
	    ret = SL_SUCCESS;
	}
	else {
	    PRINTF(("SL_socket_write_nb: error while writing to socket %s\n", 
		    strerror(errno)));
	    ret =  errno;
	}
#else
	if ( ret == -1 ) {
	*numwritten = 0;
	if ( errno == EINTR || errno == EAGAIN || errno == EINPROGRESS ||
	     errno == EWOULDBLOCK ) {
	    ret = SL_SUCCESS;
	}
	else {
	    PRINTF(("SL_socket_write_nb: error while writing to socket %s\n", 
		    strerror(errno)));
	    ret =  errno;
	}
#endif
    }
    else {
	*numwritten = ret;
	ret = SL_SUCCESS;
    }

    return ret;
}

int SL_socket_read ( int hdl, char *buf, int num, double timeout )
{
    int lcount=0;
    int a;
    char *wbuf = buf;
    double starttime = SL_Wtime();
    double endtime;
    
    do {
#ifdef MINGW
	a = recv ( hdl, wbuf, num, 0 );
#else
	a = read ( hdl, wbuf, num);
#endif
#ifdef MINGW
	
	if ( a < 0 ) {
		errno = WSAGetLastError();
		if ( errno == WSAEINTR ||  errno == WSAEINPROGRESS ||
		 errno == WSAEWOULDBLOCK) {
	    }
	   else {
		PRINTF(("SL_socket_read: error while reading from socket %s\n", 
			strerror(errno)));
		return errno;
	    }
#else
	if ( a == -1 ) {
	    if ( errno == EINTR || errno == EAGAIN || errno == EINPROGRESS ||
		 errno == EWOULDBLOCK) {
	    }
	    else {
		PRINTF(("SL_socket_read: error while reading from socket %s\n", 
			strerror(errno)));
		return errno;
	    }
#endif
	    lcount++;
	    continue;
	}
	num -= a;
	wbuf += a;

	if ( timeout > 0 ) {
	    endtime = SL_Wtime();
	    if ( (endtime - starttime) > timeout ) {
		PRINTF(("SL_socket_read: data transfer operation timed out after %lf secs\n",
			(endtime-starttime)));
		return SL_ERR_PROC_UNREACHABLE;
	    }
	}
    } while ( num > 0 );
    PRINTF (("SL_socket_read: read %d bytes from %d lcount=%d num=%d\n", 
	     a, hdl, lcount, num ));
    
    return SL_SUCCESS;
}

int SL_socket_read_nb ( int hdl, char *buf, int num, int* numread )
{
    int ret;
#ifdef MINGW
    ret = recv ( hdl, buf, num, 0 );
#else
    ret = read ( hdl, buf, num);
#endif
#ifdef MINGW
    if ( ret == -1 ) {
	*numread = 0;
	errno = WSAGetLastError();
	if ( errno == WSAEINTR  || errno == WSAEINPROGRESS ||
	     errno == WSAEWOULDBLOCK) {
	    ret = SL_SUCCESS;
	}
	else {
	    PRINTF(("SL_socket_read_nb: error while reading from socket %s\n", 
		    strerror(errno)));
	    return errno;
		}
	}
#else
	if ( ret == -1 ) {
	    *numread = 0;
	    if ( errno == EINTR || errno == EAGAIN || errno == EINPROGRESS ||
		 errno == EWOULDBLOCK) {
		ret = SL_SUCCESS;
	    }
	    else {
		PRINTF(("SL_socket_read_nb: error while reading from socket %s\n", 
			strerror(errno)));
		return errno;
	    }
	}
#endif
	
    else {
	*numread = ret;
	ret = SL_SUCCESS;
    }
    
    return ret;
}
