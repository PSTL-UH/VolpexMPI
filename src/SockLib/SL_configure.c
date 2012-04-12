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
#include "SL.h"


/* The implementation of SL_print_socket_options is based on the source code
   given in Stevens:"UNIX Network Programming, Volume 1" */

static char strres[128];

union val {
  int                   i_val;
  long                  l_val;
  char                  c_val[10];
  struct linger         linger_val;
  struct timeval        timeval_val;
} val;

static char     *sock_str_flag(union val *, int);
static char     *sock_str_int(union val *, int);
static char     *sock_str_linger(union val *, int);
static char     *sock_str_timeval(union val *, int);


struct sock_opts {
  char     *opt_str;
  int     opt_level;
  int      opt_name;
  char   *(*opt_val_str)(union val *, int);
} sock_opts[] = {
    {"SO_BROADCAST",         SOL_SOCKET,     SO_BROADCAST,   sock_str_flag},
    {    "SO_DEBUG",             SOL_SOCKET,     SO_DEBUG,       sock_str_flag},
    {    "SO_DONTROUTE",         SOL_SOCKET,     SO_DONTROUTE,   sock_str_flag},
    {    "SO_ERROR",             SOL_SOCKET,     SO_ERROR,       sock_str_int},
    {    "SO_KEEPALIVE",         SOL_SOCKET,     SO_KEEPALIVE,   sock_str_flag},
    {    "SO_LINGER",            SOL_SOCKET,     SO_LINGER,      sock_str_linger},
    {    "SO_OOBINLINE",         SOL_SOCKET,     SO_OOBINLINE,   sock_str_flag},
    {    "SO_RCVBUF",            SOL_SOCKET,     SO_RCVBUF,      sock_str_int},
    {    "SO_SNDBUF",            SOL_SOCKET,     SO_SNDBUF,      sock_str_int},
    {    "SO_RCVLOWAT",          SOL_SOCKET,     SO_RCVLOWAT,    sock_str_int},
    {    "SO_SNDLOWAT",          SOL_SOCKET,     SO_SNDLOWAT,    sock_str_int},
    {    "SO_RCVTIMEO",          SOL_SOCKET,     SO_RCVTIMEO,    sock_str_timeval},
    {    "SO_SNDTIMEO",          SOL_SOCKET,     SO_SNDTIMEO,    sock_str_timeval},
    {    "SO_REUSEADDR",         SOL_SOCKET,     SO_REUSEADDR,   sock_str_flag},
#ifdef  SO_REUSEPORT
        {"SO_REUSEPORT",         SOL_SOCKET,     SO_REUSEPORT,   sock_str_flag},
#else
    {    "SO_REUSEPORT",         0,              0,              NULL},
#endif
    {    "SO_TYPE",              SOL_SOCKET,     SO_TYPE,        sock_str_int},
#if 0
    {    "SO_USELOOPBACK",       SOL_SOCKET,     SO_USELOOPBACK, sock_str_flag},
#endif
    {    "IP_TOS",               IPPROTO_IP,     IP_TOS,         sock_str_int},
    {    "IP_TTL",               IPPROTO_IP,     IP_TTL,         sock_str_int},
    {    "TCP_MAXSEG",           IPPROTO_TCP,    TCP_MAXSEG,     sock_str_int},
    {    "TCP_NODELAY",          IPPROTO_TCP,    TCP_NODELAY,    sock_str_flag},
    {    NULL,                   0,              0,              NULL}
};



void SL_print_socket_options ( int fd )
{
    socklen_t         len;
    struct sock_opts *ptr;
    int              flag;
    
    flag=1; 
    len=sizeof(flag);

#ifdef MINGW
    char *winflag;
    sprintf(winflag, "%d", flag);
    if(getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, winflag, &len)<0) {
#else    
    if(getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &flag, &len)<0) {
#endif
	PRINTF(("[%d]:SL_print_socket_options: could not get configuration for TCP_MAXSEG\n",SL_this_procid));
    } else {
	PRINTF(("[%d]:SL_print_socket_options: TCP_MAXSEG = %d\n",SL_this_procid,flag));
    }
    
    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++) {
	PRINTF(("[%d]:%s: ", SL_this_procid,ptr->opt_str));
	if (ptr->opt_val_str == NULL)
	    PRINTF(("(undefined)\n"));
	else {
	    len = sizeof(val);
#ifdef MINGW
    	    sprintf(winflag, "%d", val);
          if (getsockopt(fd, ptr->opt_level, ptr->opt_name, winflag, &len) == -1) {
#else
	    if (getsockopt(fd, ptr->opt_level, ptr->opt_name,
			   &val, &len) == -1) {
#endif
		PRINTF(("getsockopt error"));
	    } else {
		PRINTF(("value = %s\n", (*ptr->opt_val_str)(&val, len)));
	    }
	}
    }

    return;
}


static char* sock_str_flag(union val *ptr, int len)
{
    if (len != sizeof(int))
	snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    else
	snprintf(strres, sizeof(strres),
		 "%s", (ptr->i_val == 0) ? "off" : "on");
    return(strres);
}

static char* sock_str_int(union val *ptr, int len)
{
    if (len != sizeof(int))
	snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    else
	snprintf(strres, sizeof(strres), "%d", ptr->i_val);
    return(strres);
}

static char* sock_str_linger(union val *ptr, int len)
{ 
    struct linger   *lptr = &ptr->linger_val;
    
    if (len != sizeof(struct linger))
	snprintf(strres, sizeof(strres),
		 "size (%d) not sizeof(struct linger)", len);
    else
	snprintf(strres, sizeof(strres), "l_onoff = %d, l_linger = %d",
		 lptr->l_onoff, lptr->l_linger);
    return(strres);
}

static char* sock_str_timeval(union val *ptr, int len)
{
        struct timeval  *tvptr = &ptr->timeval_val;

        if (len != sizeof(struct timeval))
                snprintf(strres, sizeof(strres),
                                 "size (%d) not sizeof(struct timeval)", len);
        else
                snprintf(strres, sizeof(strres), "%ld sec, %ld usec",
                                 tvptr->tv_sec, tvptr->tv_usec);
        return(strres);
}


void SL_configure_socket ( int sd ) 
{
    
    int flag;
    struct linger ling;

    flag=1;
    if(setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag))<0) {
        PRINTF(("[%d]:SL_configure_socket: could not configure socket\n",SL_this_procid));
    }
    
    flag=SL_TCP_BUFFER_SIZE;
    if(setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char*)&flag, sizeof(flag))<0) {
	PRINTF(("[%d]:SL_configure_socket: could not configure socket\n",SL_this_procid));
    }
   
    flag=SL_TCP_BUFFER_SIZE;
    if(setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char*)&flag, sizeof(flag))<0) {
	PRINTF(("[%d]:SL_configure_socket: could not configure socket\n",SL_this_procid));
    }

    flag=1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag))<0) {
        PRINTF(("[%d]:SL_configure_socket: Could not set SO_REUSEADDR\n",SL_this_procid));
    }

    ling.l_onoff  = 1;
    ling.l_linger = 0;
    if(setsockopt(sd, SOL_SOCKET, SO_LINGER, &ling, sizeof(struct linger))<0) {
	PRINTF(("[%d]:SL_configure_socket: Could not set SO_LINGER\n",SL_this_procid));
    }


    return;
}

void SL_configure_socket_nb ( int sd ) 
{
    int flag;

    SL_configure_socket ( sd );
#ifdef MINGW
    u_long iMode = 1; /*non-zero iMode means NONBLOCK*/
    ioctlsocket(sd, FIONBIO, &iMode);
#else
    flag = fcntl ( sd, F_GETFL, 0 );
    fcntl ( sd, F_SETFL, flag | O_NONBLOCK );
#endif
    return;
}
