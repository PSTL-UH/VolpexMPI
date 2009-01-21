#
# Copyright (c) 2008-2009      University of Houston. All rights reserved.
# Additional copyrights may follow
#
include ../../Makefile.defs

#W_INC_DIR = ..\include
#W_LIB_DIR = ..\lib

#WCLIB		= libvcdll.a
#WFLIB		= libvfdll.a
#CDLL		= vc.dll
#FDLL		= vf.dll

#WCFLAGS = -c -DBUILD_DLL -DMINGW 
#WFFLAGS	= -c -DBUILD_DLL -DMINGW -DVOLPEXFORTRAN -w
#WCDLLFLAGS  = -shared -o ..\lib\vc.dll -Wl,--out-implib,..\lib\libvcdll.a
#WFDLLFLAGS  = -shared -o ..\lib\vf.dll -Wl,--out-implib,..\lib\libvfdll.a

OBJ = 	volpex_API.o		\
	volpex_collective.o	\
	volpex_comm.o		\
	volpex_global.o		\
	volpex_p2p.o

.SUFFIXES: .o.c

all: 
	make $(VOLPEX_LIB)

$(VOLPEX_LIB): $(HEADER) $(OBJ)
	$(AR) $(AR_FLAGS) $(VOLPEX_LIB_DIR)/$(VOLPEX_LIB) $(OBJ)

$(OBJ): $(HEADER)
.c.o:   $(HEADER)
	$(CC) $(CFLAGS) -DPRINTF -DVOLPEXFORTRAN -c $<

#winclib:
#	mingw32-make winclean
#	gcc $(WCFLAGS) $(SRC) -I$(W_INC_DIR)
#	gcc $(WCDLLFLAGS) $(OBJ) -lws2_32
#	copy ..\lib\vc.dll ..\..\vc.dll
#	mingw32-make winclean
#
#winflib:
#	mingw32-make winclean
#	gcc $(WFFLAGS) $(SRC) -I$(W_INC_DIR)
#	gcc $(WFDLLFLAGS) $(OBJ) -lws2_32
#	copy ..\lib\vf.dll ..\..\vf.dll
#	mingw32-make winclean

clean:
	$(RM) $(RMFLAGS) *.o *~
