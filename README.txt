- used mimi.cs.mcgill.ca
- compiled using the following 2 commands: 
gcc -c cpu.c kernel.c pcb.c ram.c interpreter.c shell.c shellmemory.c  memorymanager.c IOSCHEDULER.c DISK_driver.c


gcc -o mykernel memorymanager.o ram.o cpu.o kernel.o pcb.o interpreter.o shell.o shellmemory.o IOSCHEDULER.o DISK_driver.o