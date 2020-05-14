#!/bin/sh
#
# make the root floppy structure
#
# Version:  	0.01.01.02    
#
./fs mkfs
./fs mkdir /bin
./fs cp ../commands/sh/sh.sef /bin/sh
./fs cp ../commands/ls/ls.sef /bin/ls
./fs cp ../commands/cat/cat.sef /bin/cat
./fs cp ../commands/kill/kill.sef /bin/kill
./fs cp ../commands/mkdir/mkdir.sef /bin/mkdir
./fs mkdir /man
./fs cp ./README /man/emax.txt
