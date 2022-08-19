set pagination off
catch throw
commands
	backtrace
	continue
end

**************************************************************************************
Debugging memory leaks:

valgrind --vgdb=yes --vgdb-error=0 ./run ../../instancias/2e-vrp-tw/

gdb run:

 set non-stop off
 target remote | vgdb