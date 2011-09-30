#
# Copyright (c) 2006-2008      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs
#OBJ1 =  MCFA_API.o

OBJ =	MCFA_pack_unpack.o	\
	MCFA_create_lists.o	\
	MCFA_printf.o		\
	MCFA_pack_unpack_lists.o \
	MCFA_getID.o		\
	MCFA_connection.o	\
	MCFA_env.o		\
	MCFA_event_handling.o	\
	MCFA_eventcontrol.o	\
	MCFA_spawn_procs.o	\
	MCFA_node_selection.o   \
        MCFA_clustering.o	\
	MCFA_set_list.o		\
	MCFA_boinc.o		\
	MCFA_help.o		\
	MCFA_API.o	
	
#	MCFA_socket_comm.o	\
#	MCFA_socket_comm.o	\

.SUFFIXES: .o.c

all:    
	make $(VOLPEX_LIB) 
	make MCFA_startprocs
#	make MCFA_control
#	make MCFA_monitor
	make MCFA_startup_d
	make MCFA_proxy

$(VOLPEX_LIB): $(HEADER) $(OBJ) 
	$(AR) $(AR_FLAGS) $(VOLPEX_LIB_DIR)/$(VOLPEX_LIB) $(OBJ)


$(OBJ):	$(HEADER)
.c.o:	$(HEADER)
	$(CC) $(CFLAGS)  -c $<

MCFA_startprocs: $(HEADER)  $(OBJ)
	$(CC) $(CFLAGS) -o mcfarun MCFA_startprocs.c $(LDFLAGS)

MCFA_control: $(HEADER) $(OBJ)
	$(CC) $(CFLAGS) -o mcfacontrol MCFA_control.c $(LDFLAGS)

MCFA_monitor: $(HEADER) $(OBJ)
	$(CC) $(CFLAGS) -o mcfamonitor MCFA_monitor.c $(LDFLAGS)

MCFA_startup_d: $(HEADER) $(OBJ)
	$(CC) $(CFLAGS) -o mcfastart_d MCFA_startup_d.c $(LDFLAGS)

MCFA_proxy: $(HEADER) $(OBJ)
	$(CC) $(CFLAGS) -o mcfa_proxy MCFA_proxy.c $(LDFLAGS)

clean:
	rm -rf *.o *~ *.out 
	rm -f mcfacontrol mcfamonitor mcfarun mcfastart_d mcfa_proxy
