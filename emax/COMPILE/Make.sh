#!/bin/sh
#
# build the system
#
# Version:  	0.00.08.07    
#
CFLAGS="-c -malign-loops=0 -malign-jumps=0 -malign-functions=0 "

echo "Compileing..."
cc -D__ASSEMBLY__ -traditional -c ../kernel/head.S -o head.o
cc -D__ASSEMBLY__ -traditional -c ../kernel/klib386.S -o klib386.o
gcc -D_KERNEL_ $CFLAGS ../kernel/main.c
gcc -D_KERNEL_ $CFLAGS ../kernel/int.c
gcc -D_KERNEL_ $CFLAGS ../kernel/clock.c
gcc -D_KERNEL_ $CFLAGS ../kernel/scheduler.c
gcc -D_KERNEL_ $CFLAGS ../kernel/message.c
gcc -D_KERNEL_ $CFLAGS ../kernel/null.c
gcc -D_KERNEL_ $CFLAGS ../kernel/pinit.c
gcc -D_KERNEL_ $CFLAGS ../kernel/dma.c
gcc -D_KERNEL_ $CFLAGS ../kernel/drvsrv.c
gcc -D_KERNEL_ $CFLAGS ../kernel/sysproc.c
gcc -D_KERNEL_ $CFLAGS ../drivers/floppy.c
gcc -D_KERNEL_ $CFLAGS ../drivers/vt/getty1.c
gcc -D_KERNEL_ $CFLAGS ../drivers/vt/getty2.c
gcc -D_KERNEL_ $CFLAGS ../drivers/keyboard.c
gcc -D_KERNEL_ $CFLAGS ../drivers/console.c
gcc -D_KERNEL_ $CFLAGS ../signal/signal.c
gcc -D_KERNEL_ $CFLAGS ../mm/memsrv.c
gcc -D_KERNEL_ $CFLAGS ../fs/fs.c 
echo "Linking..."
#--------------------------------------------------------------------------
ld -Ttext 0x11000 -e st_32 -s ./head.o ./klib386.o ./int.o \
./clock.o ./scheduler.o ./message.o ./dma.o ./floppy.o ./keyboard.o \
./drvsrv.o ./sysproc.o ./console.o ./signal.o ./fs.o ./memsrv.o ./pinit.o \
./getty1.o ./getty2.o ./null.o ./main.o
#--------------------------------------------------------------------------
./lnk32
