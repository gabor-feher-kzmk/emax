/*			linux utilities to write on emax fs 
 ****************************************************************************/

#include <stdio.h>  
 
unsigned char fd_read(FILE *fp,unsigned long block,unsigned long byte)
{
unsigned char ret;
    fseek(fp,(block*512)+byte,0);
    ret = fgetc(fp);
    return ret;
}

void fd_write(FILE *fp,unsigned long block,unsigned long byte,unsigned char data)
{
    fseek(fp,(block*512)+byte,0);
    fputc(data,fp);
}

unsigned long fd_rlong(FILE *fp,unsigned long block,unsigned long byte)
{
unsigned long ret,lp;
    fseek(fp,(block*512)+byte,0);ret=0;
    ret += fgetc(fp);byte++;
	fseek(fp,(block*512)+byte,0);lp=fgetc(fp);
    ret += (lp*0x100);byte++;
	fseek(fp,(block*512)+byte,0);lp=fgetc(fp);
    ret += (lp*0x10000);byte++;
	fseek(fp,(block*512)+byte,0);lp=fgetc(fp);
    ret += (lp*0x1000000);
    return ret;
}

void fd_wlong(FILE *fp,unsigned long block,unsigned long byte,unsigned long data)
{
unsigned char lp;
    
    lp = data;
    fseek(fp,(block*512)+byte,0);
    fputc(lp,fp);

    lp = data >> 8;byte++;
    fseek(fp,(block*512)+byte,0);
    fputc(lp,fp);

    lp = data >> 16;byte++;
    fseek(fp,(block*512)+byte,0);
    fputc(lp,fp);

    lp = data >> 24;byte++;
    fseek(fp,(block*512)+byte,0);
    fputc(lp,fp);
}
 