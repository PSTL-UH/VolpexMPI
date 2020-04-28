Following are the steps for VolpexMPI

To compile VolpexMPI
1.In dir VolpexMPI/trunk change file Makefile.defs
   VOLPEX_DIR    = $current working path$
   CC           = add the compiler to compile C code, default is gcc
   CFLAGS       = add optimization flags for C code, deafult is O3
   LDFLAGS      = add link, dafulat is statically linked
   FC           = compiler to compile fortran code, default is gfortran
   FFLAGS       = add optimization flags for Fortran code, deafult is O3
   CLUSTER	= 1 (to enable clustering), default is 0
   
   In order to enable debug options
   add -DPRINTF -O0 -g options to CFLAGS

2. make all


hostfile: list of nodes where application to be executed
edit this file with the node names you want to use
if hostfile is not specified all processes execute on front node

