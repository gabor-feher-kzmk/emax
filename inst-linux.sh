#!/bin/sh
#
# Make the system, root & boot
#
# Version:  	0.01.02.25    
#
echo "Now you started to install Emax        ;-) Good choice"
echo "<[1]===================MAKE A BOOT FLOPPY========================>"
echo "Please put a floppy in the drive this will be the boot floppy "
echo "Press Enter to continue"
read line
cd ./emax
cat ./COMPILE/k32.img > /dev/fd0
cd ..
echo "<[2]==================MAKE A ROOT FLOPPY=========================>"
echo "Please put a floppy in the drive this will be the root floppy "
echo "Press Enter to continue"
read line
cd ./fstest
./make.sh
cd ..