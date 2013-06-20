
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

2. type make clean all

For application
1. Put your application in dir VolpexMPI/trunk/test
2.Add a line in VolpexMPI/trunk/test/Makefile
application_name:application_name.c
        $(CC) $(CFLAGS) -o application_name application_name.c $(LDFLAGS)
3. in dir VolpexMPI/trunk/test create softlinks
    ln -s ../src/startup/mcfarun
    ln -s ../src/startup/mcfastart_d

command to execute:
in dir VolpexMPI/trunk/test
./mcfarun -np [# of processes] -hostfile [hostfile] ./application

hostfile: list of nodes where application to be executed
if hostfile is not specified all processes execute on front node

