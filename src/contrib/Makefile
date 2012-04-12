#
# Copyright (c) 2006-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs

OBJ =	clustering.o	\
	mpipp.o		\
	MCFA_node_selection.o	

.SUFFIXES: .o.c

all:    
	make $(VOLPEX_LIB) 

$(VOLPEX_LIB): $(HEADER) $(OBJ) 
	$(AR) $(AR_FLAGS) $(VOLPEX_LIB_DIR)/$(VOLPEX_LIB) $(OBJ)


$(OBJ):	$(HEADER)
.c.o:	$(HEADER)
	$(CC) $(CFLAGS)  -c $<


clean:
	rm -rf *.o *~ *.out 
