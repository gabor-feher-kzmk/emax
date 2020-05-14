@echo off
echo Now you started to install Emax (Good choice)
echo [1]===================MAKE A BOOT FLOPPY========================
echo Please put a floppy in the drive this will be the boot floppy
pause
fdvol 1440 A: .\emax\compile\k32.img
echo [2]===================MAKE A ROOT FLOPPY========================
echo Please put a floppy in the drive this will be the root floppy
pause
fdvol 1440 A: .\fstest\emaxfs.img

