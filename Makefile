#
# Copyright (c) 2008-2012      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include Makefile.defs

HEADER = $(VOLPEX_INC_DIR)/SL.h

ifeq ($(CLUSTER),1)
all:
	cd src/SockLib ; make 
	cd src/volpex ; make
	cd src/contrib; make
	cd src/startup; make
else
all:
	cd src/SockLib ; make
	cd src/volpex ; make
	cd src/startup; make
endif


clean:
	cd src/SockLib ; make clean
	cd src/volpex ; make clean
	cd src/contrib; make clean
	cd src/startup; make clean

	$(RM) $(RMFLAGS) *~ $(VOLPEX_INC_DIR)/*~ *.out 
	$(RM) $(RMFLAGS) $(VOLPEX_LIB_DIR)/$(VOLPEX_LIB)
	

install:
	test -z $(VOLPEX_INSTALL_DIR)/lib || mkdir -p $(VOLPEX_INSTALL_DIR)/lib	
	install -c -m 644 $(VOLPEX_LIB_DIR)/$(VOLPEX_LIB)  $(VOLPEX_INSTALL_DIR)/lib/
	test -z $(VOLPEX_INSTALL_DIR)/include || mkdir -p $(VOLPEX_INSTALL_DIR)/include	
	install -c -m 644 $(VOLPEX_INC_DIR)/VOLPEX.h  $(VOLPEX_INSTALL_DIR)/include/
	install -c -m 644 $(ADCL_INC_DIR)/SL.h    $(VOLPEX_INSTALL_DIR)/include/
