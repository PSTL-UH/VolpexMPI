#
# Copyright (c) 2008-2009      University of Houston. All rights reserved.
# Additional copyrights may follow
#
include ../Makefile.defs

temp: 
	$(CC) $(CFLAGS) -o temp temp.c $(LDFLAGS)

tempf:
	$(FC) $(FFLAGS) -o temp temp.f $(LDFLAGS)

bt:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/bt.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/make_set.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/initialize.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/exact_solution.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/exact_rhs.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/set_constants.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/adi.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/define.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/copy_faces.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/rhs.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/lhsx.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/lhsy.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/lhsz.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/x_solve.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/y_solve.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/z_solve.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/add.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/error.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/verify.f  
	$(FC) $(FFLAGS) -c ./TestData/BT/setup_mpi.f  
	$(FC) $(FFLAGS) -o bt bt.o make_set.o initialize.o exact_solution.o exact_rhs.o set_constants.o adi.o define.o copy_faces.o  rhs.o lhsx.o lhsy.o lhsz.o x_solve.o y_solve.o z_solve.o add.o error.o verify.o setup_mpi.o print_results.o timers.o $(LDFLAGS)
	make clean

cg:
	$(FC) $(FFLAGS) ./TestData/common/timers.f  
	$(FC) $(FFLAGS) ./TestData/common/randi8.f  
	$(FC) $(FFLAGS) ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) ./TestData/CG/cg.f  
	$(FC) $(FFLAGS) -o cg cg.o timers.o randi8.o print_results.o $(LDFLAGS)

ep:
	$(FC) $(FFLAGS) ./TestData/common/timers.f  
	$(FC) $(FFLAGS) ./TestData/common/randdp.f  
	$(FC) $(FFLAGS) ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) ./TestData/EP/ep.f  
	$(FC) $(FFLAGS) -o ep ep.o timers.o randdp.o print_results.o $(LDFLAGS)

ft:
	$(FC) $(FFLAGS) ./TestData/common/timers.f  
	$(FC) $(FFLAGS) ./TestData/common/randi8.f  
	$(FC) $(FFLAGS) ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) ./TestData/FT/ft.f  	
	$(FC) $(FFLAGS)  -o ft ft.o timers.o randi8.o print_results.o $(LDFLAGS)
	make clean

is:	
	$(CC) $(CFLAGS) ./TestData/common/c_timers.c 
	$(CC) $(CFLAGS) ./TestData/common/c_print_results.c 
	$(CC) $(CFLAGS) ./TestData/IS/is.c 
	$(CC) $(CFLAGS) -o is is.o c_timers.o c_print_results.o $(LDFLAGS)
	make clean

lu:
	$(FC) $(FFLAGS) ./TestData/common/timers.f  
	$(FC) $(FFLAGS) ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) ./TestData/LU/lu.f  
	$(FC) $(FFLAGS) ./TestData/LU/init_comm.f  
	$(FC) $(FFLAGS) ./TestData/LU/read_input.f  
	$(FC) $(FFLAGS) ./TestData/LU/bcast_inputs.f  
	$(FC) $(FFLAGS) ./TestData/LU/proc_grid.f  
	$(FC) $(FFLAGS) ./TestData/LU/neighbors.f  
	$(FC) $(FFLAGS) ./TestData/LU/nodedim.f  
	$(FC) $(FFLAGS) ./TestData/LU/subdomain.f  
	$(FC) $(FFLAGS) ./TestData/LU/setcoeff.f  
	$(FC) $(FFLAGS) ./TestData/LU/sethyper.f  
	$(FC) $(FFLAGS) ./TestData/LU/setbv.f  
	$(FC) $(FFLAGS) ./TestData/LU/exact.f  
	$(FC) $(FFLAGS) ./TestData/LU/setiv.f  
	$(FC) $(FFLAGS) ./TestData/LU/erhs.f  
	$(FC) $(FFLAGS) ./TestData/LU/ssor.f  
	$(FC) $(FFLAGS) ./TestData/LU/exchange_1.f  
	$(FC) $(FFLAGS) ./TestData/LU/exchange_3.f  
	$(FC) $(FFLAGS) ./TestData/LU/exchange_4.f  
	$(FC) $(FFLAGS) ./TestData/LU/exchange_5.f  
	$(FC) $(FFLAGS) ./TestData/LU/exchange_6.f  
	$(FC) $(FFLAGS) ./TestData/LU/rhs.f  
	$(FC) $(FFLAGS) ./TestData/LU/l2norm.f   
	$(FC) $(FFLAGS) ./TestData/LU/jacld.f  
	$(FC) $(FFLAGS) ./TestData/LU/blts.f  
	$(FC) $(FFLAGS) ./TestData/LU/jacu.f  
	$(FC) $(FFLAGS) ./TestData/LU/buts.f  
	$(FC) $(FFLAGS) ./TestData/LU/error.f   
	$(FC) $(FFLAGS) ./TestData/LU/pintgr.f  
	$(FC) $(FFLAGS) ./TestData/LU/verify.f  
	$(FC) $(FFLAGS) -o lu lu.o timers.o print_results.o init_comm.o read_input.o bcast_inputs.o proc_grid.o neighbors.o nodedim.o subdomain.o setcoeff.o sethyper.o setbv.o exact.o setiv.o erhs.o ssor.o exchange_1.o exchange_3.o exchange_4.o exchange_5.o exchange_6.o rhs.o l2norm.o jacld.o blts.o jacu.o buts.o error.o pintgr.o verify.o $(LDFLAGS)
	make clean 

mg:
	$(FC) $(FFLAGS) ./TestData/common/timers.f  
	$(FC) $(FFLAGS) ./TestData/common/randi8.f  
	$(FC) $(FFLAGS) ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) ./TestData/MG/mg.f  
	$(FC) $(FFLAGS) -o mg mg.o timers.o randi8.o print_results.o $(LDFLAGS)
	make clean 

sp:
	$(FC) $(FFLAGS) ./TestData/common/timers.f  
	$(FC) $(FFLAGS) ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) ./TestData/SP/sp.f  
	$(FC) $(FFLAGS) ./TestData/SP/make_set.f  
	$(FC) $(FFLAGS) ./TestData/SP/initialize.f  
	$(FC) $(FFLAGS) ./TestData/SP/exact_solution.f  
	$(FC) $(FFLAGS) ./TestData/SP/exact_rhs.f  
	$(FC) $(FFLAGS) ./TestData/SP/set_constants.f  
	$(FC) $(FFLAGS) ./TestData/SP/adi.f  
	$(FC) $(FFLAGS) ./TestData/SP/define.f  
	$(FC) $(FFLAGS) ./TestData/SP/copy_faces.f  
	$(FC) $(FFLAGS) ./TestData/SP/rhs.f  
	$(FC) $(FFLAGS) ./TestData/SP/lhsx.f  
	$(FC) $(FFLAGS) ./TestData/SP/lhsy.f  
	$(FC) $(FFLAGS) ./TestData/SP/lhsz.f  
	$(FC) $(FFLAGS) ./TestData/SP/x_solve.f  
	$(FC) $(FFLAGS) ./TestData/SP/ninvr.f  
	$(FC) $(FFLAGS) ./TestData/SP/y_solve.f  
	$(FC) $(FFLAGS) ./TestData/SP/pinvr.f  
	$(FC) $(FFLAGS) ./TestData/SP/z_solve.f  
	$(FC) $(FFLAGS) ./TestData/SP/tzetar.f  
	$(FC) $(FFLAGS) ./TestData/SP/add.f  
	$(FC) $(FFLAGS) ./TestData/SP/txinvr.f  
	$(FC) $(FFLAGS) ./TestData/SP/error.f  
	$(FC) $(FFLAGS) ./TestData/SP/verify.f  
	$(FC) $(FFLAGS) ./TestData/SP/setup_mpi.f  
	$(FC) $(FFLAGS) -o sp sp.o timers.o print_results.o make_set.o initialize.o exact_solution.o exact_rhs.o set_constants.o adi.o define.o copy_faces.o rhs.o lhsx.o lhsy.o lhsz.o x_solve.o ninvr.o y_solve.o pinvr.o z_solve.o tzetar.o add.o txinvr.o error.o verify.o setup_mpi.o $(LDFLAGS)


clean:
	$(RM) $(RMFLAGS) *.o *~ *.out