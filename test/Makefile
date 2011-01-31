#
# Copyright (c) 2008-2009      University of Houston. All rights reserved.
# Additional copyrights may follow
#
include ../Makefile.defs

test_status:
	$(CC) $(CFLAGS) -o test_status test_status.c $(LDFLAGS)

target_selection:
	 $(CC) $(CFLAGS) -o target_selection target_selection.c $(LDFLAGS)

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
	make cleanish

cg:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/randi8.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/CG/cg.f  
	$(FC) $(FFLAGS) -o cg cg.o timers.o randi8.o print_results.o $(LDFLAGS)
	make cleanish

ep:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/randdp.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/EP/ep.f  
	$(FC) $(FFLAGS) -o ep ep.o timers.o randdp.o print_results.o $(LDFLAGS)
	make cleanish

ft:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/randi8.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/FT/ft.f  	
	$(FC) $(FFLAGS)  -o ft ft.o timers.o randi8.o print_results.o $(LDFLAGS)
	make cleanish

is:	
	$(CC) $(CFLAGS) -c ./TestData/common/c_timers.c 
	$(CC) $(CFLAGS) -c ./TestData/common/c_print_results.c 
	$(CC) $(CFLAGS) -c ./TestData/IS/is.c 
	$(CC) $(CFLAGS) -o is is.o c_timers.o c_print_results.o $(LDFLAGS)
	make cleanish

lu:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/lu.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/init_comm.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/read_input.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/bcast_inputs.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/proc_grid.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/neighbors.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/nodedim.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/subdomain.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/setcoeff.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/sethyper.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/setbv.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/exact.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/setiv.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/erhs.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/ssor.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/exchange_1.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/exchange_3.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/exchange_4.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/exchange_5.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/exchange_6.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/rhs.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/l2norm.f   
	$(FC) $(FFLAGS) -c ./TestData/LU/jacld.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/blts.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/jacu.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/buts.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/error.f   
	$(FC) $(FFLAGS) -c ./TestData/LU/pintgr.f  
	$(FC) $(FFLAGS) -c ./TestData/LU/verify.f  
	$(FC) $(FFLAGS) -o lu lu.o timers.o print_results.o init_comm.o read_input.o bcast_inputs.o proc_grid.o neighbors.o nodedim.o subdomain.o setcoeff.o sethyper.o setbv.o exact.o setiv.o erhs.o ssor.o exchange_1.o exchange_3.o exchange_4.o exchange_5.o exchange_6.o rhs.o l2norm.o jacld.o blts.o jacu.o buts.o error.o pintgr.o verify.o $(LDFLAGS)
	make cleanish

mg:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/randi8.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/MG/mg.f  
	$(FC) $(FFLAGS) -o mg mg.o timers.o randi8.o print_results.o $(LDFLAGS)
	make cleanish

sp:
	$(FC) $(FFLAGS) -c ./TestData/common/timers.f  
	$(FC) $(FFLAGS) -c ./TestData/common/print_results.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/sp.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/make_set.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/initialize.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/exact_solution.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/exact_rhs.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/set_constants.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/adi.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/define.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/copy_faces.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/rhs.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/lhsx.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/lhsy.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/lhsz.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/x_solve.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/ninvr.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/y_solve.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/pinvr.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/z_solve.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/tzetar.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/add.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/txinvr.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/error.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/verify.f  
	$(FC) $(FFLAGS) -c ./TestData/SP/setup_mpi.f  
	$(FC) $(FFLAGS) -o sp sp.o timers.o print_results.o make_set.o initialize.o exact_solution.o exact_rhs.o set_constants.o adi.o define.o copy_faces.o rhs.o lhsx.o lhsy.o lhsz.o x_solve.o ninvr.o y_solve.o pinvr.o z_solve.o tzetar.o add.o txinvr.o error.o verify.o setup_mpi.o $(LDFLAGS)
	make cleanish

cleanish:
	$(RM) $(RMFLAGS) *.o *~ *.out

clean:
	$(RM) $(RMFLAGS) *.o *~ *.out
	$(RM) -f bt sp mg ft lu is ep cg temp
