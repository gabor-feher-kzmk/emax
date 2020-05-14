/* The emax program linker
 *****************************************************************************/
#include <stdio.h>
#include "rw.c"

#define			DEL_REG		0x74

void main(int argc, char *argv[])
{
FILE *pp,*hp,*sef;
unsigned long i,size_p,size_h,start_h;

    pp = fopen(argv[1],"a");size_p = ftell(pp);fclose(pp);
    pp = fopen(argv[1],"r+");
	hp = fopen(argv[2],"r+");
	size_h = fd_rlong(hp,0,DEL_REG);
	start_h = DEL_REG + 4;
	    sef = fopen(argv[3],"w+");
		for (i=0;i<size_h;i++) {
		    fseek(hp,i+start_h,0);
		    fputc(fgetc(hp),sef);
		}
		for (i=DEL_REG;i<size_p;i++) {
		    fseek(pp,i,0);
		    fputc(fgetc(pp),sef);
		}
	    fclose(sef);
	fclose(hp);
    fclose(pp);
}

 
 
 