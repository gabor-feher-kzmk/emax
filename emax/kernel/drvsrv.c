/* *   		drvsrv.c includes the drive server for the kernel	  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "../include/pcall.h"
#include "../include/tss.h"
#include "../include/pis.h"

#define HARD_INT	2
#define DEV_READ	3
#define DEV_WRITE	4
#define DEV_OPEN	6
#define DEV_CLOSE	7
#define DEV_RESULT	8
#define SWITCH_VT	9

static int do_dev_open(unsigned long proc,unsigned long master,int slave);
static int do_dev_close(unsigned long proc,unsigned long master,int slave);
static int do_dev_write(unsigned long proc,unsigned long master,unsigned long slave,unsigned long *data);
static int do_dev_read(unsigned long proc,unsigned long master,unsigned long slave,unsigned long *data);
static int do_hard_int(unsigned long proc,unsigned long dev);
static int do_init_vt(unsigned long proc);
static int do_dev_result(unsigned long proc,unsigned long res);
static int writein_stream(unsigned long proc,unsigned long master,unsigned long slave);
static int writein_1D(unsigned long proc,unsigned long master,unsigned long slave,unsigned long *data);
static int writein_2D(unsigned long proc,unsigned long master,unsigned long slave);
static int writein_3D(unsigned long proc,unsigned long master,unsigned long slave);
static int readfrom_stream(unsigned long proc,unsigned long master,unsigned long slave);
static int readfrom_1D(unsigned long proc,unsigned long master,unsigned long slave,unsigned long *data);
static int readfrom_2D(unsigned long proc,unsigned long master,unsigned long slave);
static int readfrom_3D(unsigned long proc,unsigned long master,unsigned long slave);
static int do_sw_vt(unsigned long proc,int vt);
static void sos(unsigned long ch);
static unsigned long mem_load(unsigned long offset);
#include "drvsrv.h"
/*
    The Driver Server is created to manage the drivers of the kernel.
    A proc can't acces the drive or the driver directly. This job can
    only be done through the DrvSrv, wich checks if the proc has enough
    premiissons to access the drive.
*/
ENTRY(drvsrv);
static unsigned long drc[99];
static int wkup,lvt;
static unsigned long secp;
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
static void who_w()
{
 unsigned long *get,back,com,i;
 unsigned long rc[99];
 
LOCK; /* This lock will speed up the driver access, but it's still slow */
 wkup = 0;lvt = 0;
 while(TRUE) { 
    get = &rc[1]; back = 0;
    PUSH(get);PUSHN(24);PUSHN(0x405);CALL(105);ADDESP(12);
    for (i=0;i<25;i++) {drc[i] = rc[i];}
    secp = rc[1];
    if (rc[1] == 17) {secp = 14;} /* This is not the finall way that threads are handled ;-) */
	/* The new kernel 0.02 will do it with a TID in the message server */
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
    if (rc[3] < 5) {
	switch (rc[3]) {
	case 1:		back = do_init_vt(rc[1]);			break;
	case HARD_INT:	back = do_hard_int(rc[1],rc[4]);		break;
	case DEV_READ:	back = do_dev_read(rc[1],rc[4],rc[5],drc);	break;
	case DEV_WRITE:	back = do_dev_write(rc[1],rc[4],rc[5],drc);	break;
	}
    } else {
	if (secp == 14) {
	switch (rc[3]) {
	case DEV_OPEN:  back = do_dev_open(rc[1],rc[4],rc[5]);		break;
	case DEV_CLOSE:	back = do_dev_close(rc[1],rc[4],rc[5]);		break;
	}
	} else { 
	    if (secp == 15) {
	    switch (rc[3]) {
	    case SWITCH_VT:  back = do_sw_vt(rc[1],rc[4]);		break;
	    }
	    } else { back = 0;wkup = 1;}
	}
    }	    
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
    if (wkup) {com = (rc[1]*0x100)+4;wkup = 0;
    PUSH(back);PUSHN(1);PUSH(com);CALL(105);ADDESP(12);}
 }
}
/*===========================================================================*/
/*                                DO_DEV_OPEN                                */
/*===========================================================================*/
static int do_dev_open(unsigned long proc,unsigned long master,int slave)
{
extern struct devs devlib[10,5];

 if (devlib[master,slave].type == 0) {wkup=1;return(0);}
    if (devlib[master,slave].owner_proc != 0) {wkup=1;return(0);}
    devlib[master,slave].owner_proc = proc;
wkup=1;return(1);
}
/*===========================================================================*/
/*                               DO_DEV_CLOSE                                */
/*===========================================================================*/
static int do_dev_close(unsigned long proc,unsigned long master,int slave)
{
extern struct devs devlib[10,5];

 if (devlib[master,slave].type == 0) {wkup=1;return(0);} 
    if (devlib[master,slave].owner_proc != proc) {wkup=1;return(0);}
    devlib[master,slave].owner_proc = 0;
wkup=1;return(1);
}
/*===========================================================================*/
/*                               DO_DEV_WRITE                                */
/*===========================================================================*/
static int do_dev_write(unsigned long proc,unsigned long master
,unsigned long slave,unsigned long *data)
{
extern struct devs devlib[10,5];
int ret;
ret = 0;
 /* try out the std streams */
 if (master == 0) {ret = writein_stream(proc,master,slave);return(ret);}
 if (devlib[master,slave].type != 0) {
    if (devlib[master,slave].owner_proc == secp) {
        switch (devlib[master,slave].type) {
        case 1: ret = writein_stream(proc,master,slave);	break;
        case 2: ret = writein_1D(proc,master,slave,data);	break;
        case 3: ret = writein_2D(proc,master,slave);		break;
        case 4: ret = writein_3D(proc,master,slave);		break;
        }
    }
 }
return(ret);
}
/*===========================================================================*/
/*                                DO_DEV_READ                                */
/*===========================================================================*/
static int do_dev_read(unsigned long proc,unsigned long master
,unsigned long slave,unsigned long *data)
{
extern struct devs devlib[10,5];
int ret;
ret=0;
 /* try out the std streams */
 if (master == 0) {ret = readfrom_stream(proc,master,slave);return(ret);}
 if (devlib[master,slave].type != 0) {
    if (devlib[master,slave].owner_proc == proc) {
        switch (devlib[master,slave].type) {
        case 1: ret = readfrom_stream(proc,master,slave);	break;
        case 2: ret = readfrom_1D(proc,master,slave,data);	break;
        case 3: ret = readfrom_2D(proc,master,slave);		break;
        case 4: ret = readfrom_3D(proc,master,slave);		break;
        }
    }
 }
return(ret);
}
/*===========================================================================*/
/*                                DO_HARD_INT                                */
/*===========================================================================*/
static int do_hard_int(unsigned long proc,unsigned long dev)
{
extern unsigned long doing_proc;
extern struct devs devlib[10,5];
unsigned long com;
com = (proc*0x100)+7;
PUSHN(7);PUSH(com);CALL(105);ADDESP(8);//set dma to proc
PUSHN(0);PUSH(proc);PUSHN(4);PUSHESP;PUSHN(3);PUSHN(0x701);
CALL(105);ADDESP(24);/*send buff to console */
return(1);
}
/*===========================================================================*/
/*                                DO_INIT_VT                                 */
/*===========================================================================*/
static int do_init_vt(unsigned long proc)
{
PUSH(proc);PUSHN(1);PUSHESP;PUSHN(2);PUSHN(0x701);CALL(105);ADDESP(20);/*send buff to console */
wkup = 1;
return(1);
}
/*===========================================================================*/
/*                              WRITEIN_STREAM                               */
/*===========================================================================*/
static int writein_stream(unsigned long proc,unsigned long master,unsigned long slave)
{
extern unsigned long p_head[200];
extern struct devs devlib[10,5];
unsigned long com;
int ret,vt;
 if (master == 0) { /* The proc wants to write to one of the std streams */
    switch (slave) {
    case 1: /* This is the stdout to the predefined VT*/
	vt = mem_load(p_head[proc]+VT_ID); /* this doesn't work now, but it'll work soon */
        com = (proc*0x100)+7;
	PUSHN(7);PUSH(com);CALL(105);ADDESP(8);//set dma to proc
	PUSH(vt);PUSH(proc);PUSHN(4);PUSHESP;PUSHN(3);PUSHN(0x701);
	CALL(105);ADDESP(24);/*send buff to console */ break;
    case 4: break; /* this will be the printer */ 
    default : return(0);
    }
 }
return(1);
}
/*===========================================================================*/
/*                                 WRITEIN_1D                                */
/*===========================================================================*/
static int writein_1D(unsigned long proc,unsigned long master
,unsigned long slave,unsigned long *data)
{
extern struct devs devlib[10,5];
unsigned long driver,com,bl;
    driver = devlib[master,slave].driver_proc;
    com = (10*0x100)+1;
    bl = drc[6];
    //sos(bl+48);
    PUSH(bl);PUSH(proc);PUSHN(6);PUSHESP;PUSHN(3);//is very buggy here !!!!
    PUSH(com);CALL(105);ADDESP(24);    
return(1);
}
/*===========================================================================*/
/*                                 WRITEIN_2D                                */
/*===========================================================================*/
static int writein_2D(unsigned long proc,unsigned long master,unsigned long slave)
{
}
/*===========================================================================*/
/*                                 WRITEIN_3D                                */
/*===========================================================================*/
static int writein_3D(unsigned long proc,unsigned long master,unsigned long slave)
{
}
/*===========================================================================*/
/*                              READFROM_STREAM                              */
/*===========================================================================*/
static int readfrom_stream(unsigned long proc,unsigned long master,unsigned long slave)
{
extern unsigned long p_head[200];
int ret,vt;
extern struct devs devlib[10,5];
unsigned long com;
 if (master == 0) { /* The proc wants to write to one of the std streams */
    switch (slave) {
    case 0: /* This is the stdin from the predefined VT*/
	com = (proc*0x100)+7;
	PUSHN(7);PUSH(com);CALL(105);ADDESP(8); /* set dma to proc */
	vt = mem_load(p_head[proc]+VT_ID); /* this doesn't work now, but it'll work soon */
	PUSH(vt);PUSH(proc);PUSHN(3);PUSHESP;PUSHN(3);PUSHN(0x701);
	CALL(105);ADDESP(24);/*send buff to console */ break;
    default : return(0);
    }
 }
return(1);
}
/*===========================================================================*/
/*                                READFROM_1D                                */
/*===========================================================================*/
static int readfrom_1D(unsigned long proc,unsigned long master
,unsigned long slave,unsigned long *data)
{
extern struct devs devlib[10,5];
unsigned long driver,com,bl;
    driver = devlib[master,slave].driver_proc;
    com = (10*0x100)+1;
    bl = drc[6];
    //sos(bl+48);
    PUSH(bl);PUSH(proc);PUSHN(5);PUSHESP;PUSHN(3);
    PUSH(com);CALL(105);ADDESP(24);    
return(1);
}
/*===========================================================================*/
/*                                READFROM_2D                                */
/*===========================================================================*/
static int readfrom_2D(unsigned long proc,unsigned long master,unsigned long slave)
{
}
/*===========================================================================*/
/*                                READFROM_3D                                */
/*===========================================================================*/
static int readfrom_3D(unsigned long proc,unsigned long master,unsigned long slave)
{
}
/*===========================================================================*/
/*                                 DO_SW_VT                                  */
/*===========================================================================*/
static int do_sw_vt(unsigned long proc,int vt)
{
PUSH(vt);PUSHN(5);PUSHESP;PUSHN(2);PUSHN(0x701);
CALL(105);ADDESP(20);
    //send vt to console
wkup = 1;
return(1);
}
static void sos(unsigned long ch)
{
asm("pushl	%edi");
asm("pushl	%0"
	: 
	: "r" (ch));
asm("
    popl	%edx
    movl	$0xB8522,%edi
    movl	$0x0700,%eax
    addl	%edx,%eax
    movl	%eax,%es:(%edi)
    popl	%edi
    ");
}
/*===========================================================================*/
/*                               MEM_LOAD                                    */
/*===========================================================================*/
static unsigned long mem_load(unsigned long offset)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("popl	%edi\n\t
	     movl	%es:(%edi),%eax\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}

ENTRY(drvsrv_end);
