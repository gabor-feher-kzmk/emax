#!/bin/sh
#
# build the system
#
# Version:  	0.00.08.07    
#
CFLAGS="-c -malign-loops=0 -malign-jumps=0 -malign-functions=0"

echo "Compiling mkdir"
gcc -D_KERNEL_ $CFLAGS ./mkdir.c
gcc -D_KERNEL_ $CFLAGS ./proc.c
#--------------------------------------------------------------------------
ld -N -x -X -s -e main -Ttext 0x0 ./mkdir.o -o ./mkdir.elf
ld -N -x -X -s -e SEF -Ttext 0x0 ./proc.o -o ./proc.elf
./epl ./mkdir.elf ./proc.elf ./mkdir.sef
#--------------------------------------------------------------------------
