#!/bin/sh
#
# build the system
#
# Version:  	0.00.08.07    
#
CFLAGS="-c -malign-loops=0 -malign-jumps=0 -malign-functions=0"

echo "Compiling cat"
gcc -D_KERNEL_ $CFLAGS ./cat.c
gcc -D_KERNEL_ $CFLAGS ./proc.c
#--------------------------------------------------------------------------
ld -N -X -x -e main -Ttext 0x0 ./cat.o -o ./cat.elf
ld -N -X -x -s -e SEF -Ttext 0x0 ./proc.o -o ./proc.elf
./epl ./cat.elf ./proc.elf ./cat.sef
#--------------------------------------------------------------------------
