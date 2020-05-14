/* *   		console.c includes console driver for the kernel	  *
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

 struct ctext {
    unsigned long buff_add;
    unsigned long str_to;
    unsigned int c_x;
    unsigned int c_y;
    unsigned int key_enabled;
    unsigned int str_con;
    unsigned long vt_str[260];
 };

#define CURSOR            14	/* 6845's cursor register */
#define INDEX              0	/* 6845's index register */
#define DATA               1	/* 6845's data register */
#define BYTE		0377	/* mask for 8 bits */

#define DO_READ		3
#define DO_WRITE	4
#define SWITCH_VT	5

#define MAX_VT		2

static int do_read(unsigned long proc,unsigned int rvt); 
static int do_write(unsigned long proc,unsigned int rvt); 
static int write_vt(unsigned long proc,unsigned int rvt); 
static int write_cvt(unsigned long proc); 
static int str_ready_vt(unsigned long proc,unsigned int rvt); 
static int str_ready(unsigned long proc);
static int write_char(char chr);
static int write_char_vt(char chr,unsigned int rvt);
static void out_char(unsigned long chr,char cur);
static void out_char_vt(unsigned long chr,unsigned int rvt);
static void scroll(unsigned long num);
static void scroll_vt(unsigned long num,unsigned int rvt);
static void vid_copy(unsigned long src,unsigned long dest,unsigned long size);
static void vid_copy_vt(unsigned long src,unsigned long dest,unsigned long size,unsigned int rvt);
static void msg_copy(unsigned long src,unsigned long dest,unsigned long size);
static unsigned long get_mem(unsigned long size);
static void set_6845(int reg,unsigned val);
static void pout_byte(unsigned long port,unsigned long data);
static int switch_vt(int nvt);
static void vt_alloc();
static void init_console();
static void mem_wstr(unsigned long offset,unsigned int data);
#include "console.h"
/*
    This is the console driver of  the kernel. It can handle lots of
    Virtual Terminals on one PC. If the NET will be included to the 
    kernel I'm sure it will handle Telnet too.
*/
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
ENTRY(console);
static unsigned long c_x;
static unsigned long c_y;
static unsigned long key_enabled;
static unsigned long str_to;
extern unsigned long str_con;
static struct ctext vt[MAX_VT+1];
static unsigned long ch_str[260];
static int cvt;
static int alloc;

static void who_w()
{
 unsigned long get,back,to;
 unsigned long rc[99];
    init_console();
    while(TRUE) { 
    get = (unsigned long) &rc[1]; back = 0;
    PUSH(get);PUSHN(24);PUSHN(0x602);CALL(105);ADDESP(12);
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
	switch (rc[3]) {
	case 1 : if (rc[4] == 38) {vt_alloc();}			break;
	case DO_READ:	back = do_read(rc[4],rc[5]);		break;
	case DO_WRITE:	back = do_write(rc[4],rc[5]);		break;
	case SWITCH_VT:	back = switch_vt(rc[4]);		break;
	}
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
    }
}
/*===========================================================================*/
/*                                DO_READ                                    */
/*===========================================================================*/
static int do_read(unsigned long proc,unsigned int rvt) 
{
unsigned long i;
if (rvt == cvt) {
      for (i=1;i<255;i++) {ch_str[i]=0;} 
      key_enabled=1;
      str_con=1;
      str_to=proc;
} else {
      for (i=1;i<255;i++) {vt[rvt].vt_str[i]=0;} 
      vt[rvt].key_enabled=1;
      vt[rvt].str_con=1;
      vt[rvt].str_to=proc;
}
return(1);
}
/*===========================================================================*/
/*                                DO_WRITE                                   */
/*===========================================================================*/
static int do_write(unsigned long proc,unsigned int rvt)
{
if (rvt == 0) {write_cvt(proc);return(1);}
if (rvt == cvt) {
    write_cvt(proc);
} else {
    write_vt(proc,rvt);
}
} 
/*===========================================================================*/
/*                               WRITE_CVT                                   */
/*===========================================================================*/
static int write_cvt(unsigned long proc) 
{
unsigned long buff[257];
unsigned long *xp,i,com,ret,to;
char f_ch;
ret=1;xp = &buff[1];
PUSH(proc);PUSH(xp);PUSHN(255);PUSHN(0x108);CALL(105);ADDESP(16);
PUSH(proc);PUSHN(6);CALL(105);ADDESP(8);//con dma
i=1;buff[255]=0;
    while (buff[i] != 0) { f_ch = buff[i]; 
	if (key_enabled == 1) {
	    if (f_ch == 13) {
	    ret=str_ready(str_to);
	    }
	}
	write_char(f_ch);i++;
    }
    to = (proc*0x100)+4;
    PUSH(ret);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
}
/*===========================================================================*/
/*                                STR_READY                                  */
/*===========================================================================*/
static int str_ready(unsigned long proc) 
{
unsigned long buff[256];
unsigned long com,size;
unsigned long i,*cp,to;
cp=&buff[1];ch_str[str_con] = 0;size = str_con;
i=1;while (ch_str[i] != 0) {buff[i] = ch_str[i];i++;if(i>= 250){ch_str[i]=0;}}
buff[i]=0;
PUSH(proc);PUSH(cp);PUSH(size);PUSHN(0x08);CALL(105);ADDESP(16);
PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
to = (proc*0x100)+4;
PUSHN(1);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
      for (i=str_con;i<255;i++) {ch_str[i]=0;} 
      key_enabled=0;
      str_con=0;
      str_to=0;
return(0);
}
/*===========================================================================*/
/*                               WRITE_CHAR                                  */
/*===========================================================================*/
static int write_char(char chr)
{
if (chr != 0 ) {
 if (chr > 31) {
    if (key_enabled == 1){if(str_con < 250){ch_str[str_con]=chr;str_con++;}}
    if(str_con < 250){
    	if (c_x == 79) {
	 out_char(chr,0);
         scroll(1);
        } else {
         out_char(chr,0);
         c_x++;
         set_6845(CURSOR,(c_y * 80) + c_x);
        }
    } else {if (str_con == 250) {str_con--;}} 
 } else {
    switch (chr) {
    case 13 : case 10 : scroll(1); break; /* ENTER [CR] is pressed*/
    case 14 : /* BKSP is pressed*/
	if (key_enabled == 1) { if (str_con!=1) { str_con--;ch_str[str_con]=0;
	if (c_x != 0) { 
	c_x--; out_char(0,1); } else {
	if (c_y != 0) { 
	c_y--;c_x = 79;out_char(0,1); } } } }
	break;
    }
 }
} 
return 1;
}
/*===========================================================================*/
/*                                OUT_CHAR                                   */
/*===========================================================================*/
static void out_char(unsigned long bet,char cur)
{
unsigned long addr;
    if (cur == 1) { 
    set_6845(CURSOR,(c_y * 80) + c_x);
    }
addr = (c_y * 160) + (c_x * 2) + 0xB8000;
asm("pushl	%edi");
asm("pushl	%0"
	: 
	: "r" (addr));
asm("pushl	%0"
	: 
	: "r" (bet));
asm("
    popl	%eax
    addl	$0x0700,%eax
    popl	%edi
    movw	%ax,%es:(%edi)
    popl	%edi
    ");
}
/*===========================================================================*/
/*                                 SCROLL                                    */
/*===========================================================================*/
static void scroll(unsigned long num)
{
 if (c_y > 24 - num) { 
    vid_copy(num*160,0,4000-(num*160)); c_x = 0;
    vid_copy(4000+((num-1)*160),4000-(num*160),(num*160));
 } else { c_y++; c_x = 0; }
    set_6845(CURSOR,(c_y * 80) + c_x);
}
/*===========================================================================*/
/*                                WRITE_VT                                   */
/*===========================================================================*/
static int write_vt(unsigned long proc,unsigned int rvt) 
{
unsigned long buff[257];
unsigned long *xp,i,com,ret,to;
char f_ch;
ret=1;xp = &buff[1];
PUSH(proc);PUSH(xp);PUSHN(255);PUSHN(0x108);CALL(105);ADDESP(16);
PUSH(proc);PUSHN(6);CALL(105);ADDESP(8);//con dma
i=1;buff[255]=0;
    while (buff[i] != 0) { f_ch = buff[i]; 
	if (vt[rvt].key_enabled == 1) {
	    if (f_ch == 13) {
	    ret=str_ready_vt(vt[rvt].str_to,rvt);
	    }
	}
	write_char_vt(f_ch,rvt);i++;
    }
    to = (proc*0x100)+4;
    PUSH(ret);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
}
/*===========================================================================*/
/*                              STR_READY_VT                                 */
/*===========================================================================*/
static int str_ready_vt(unsigned long proc,unsigned int rvt) 
{
unsigned long buff[256];
unsigned long com,size;
unsigned long i,*cp,to;
cp=&buff[1]; vt[rvt].vt_str[vt[rvt].str_con] = 0; size = vt[rvt].str_con;
i=1;while (vt[rvt].vt_str[i] != 0) {buff[i] = vt[rvt].vt_str[i];i++;if(i>= 250){vt[rvt].vt_str[i]=0;}}
buff[i]=0;
PUSH(proc);PUSH(cp);PUSH(size);PUSHN(0x08);CALL(105);ADDESP(16);
PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
to = (proc*0x100)+4;
PUSHN(1);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
      for (i=vt[rvt].str_con;i<255;i++) {vt[rvt].vt_str[i]=0;} 
      vt[rvt].key_enabled=0;
      vt[rvt].str_con=0;
      vt[rvt].str_to=0;
return(0);
}
/*===========================================================================*/
/*                              WRITE_CHAR_VT                                */
/*===========================================================================*/
static int write_char_vt(char chr,unsigned int rvt)
{
if (chr != 0 ) {
 if (chr > 31) {
    if (vt[rvt].key_enabled == 1){if(vt[rvt].str_con < 250){vt[rvt].vt_str[vt[rvt].str_con]=chr;vt[rvt].str_con++;}}
    if (vt[rvt].str_con < 250) {
    	if (vt[rvt].c_x == 79) {
	 out_char_vt(chr,rvt);
         scroll_vt(1,rvt);
        } else {
         out_char_vt(chr,rvt);
         vt[rvt].c_x++;
        }
    } else {if (vt[rvt].str_con == 250) {vt[rvt].str_con--;}} 
 } else {
    switch (chr) {
    case 13 : case 10 : scroll_vt(1,rvt); break; /* ENTER [CR] is pressed*/
    case 14 : /* BKSP is pressed*/
	if (vt[rvt].key_enabled == 1) { if (vt[rvt].str_con!=1) { vt[rvt].str_con--;vt[rvt].vt_str[vt[rvt].str_con]=0;
	if (vt[rvt].c_x != 0) { 
	vt[rvt].c_x--; out_char_vt(0,rvt); } else {
	if (vt[rvt].c_y != 0) { 
	vt[rvt].c_y--;vt[rvt].c_x = 79;out_char_vt(0,rvt); } } } }
	break;
    }
 }
} 
return 1;
}
/*===========================================================================*/
/*                               OUT_CHAR_VT                                 */
/*===========================================================================*/
static void out_char_vt(unsigned long bet,unsigned int rvt)
{
unsigned long addr;
addr = (vt[rvt].c_y * 160) + (vt[rvt].c_x * 2) + vt[rvt].buff_add;
asm("pushl	%edi");
asm("pushl	%0"
	: 
	: "r" (addr));
asm("pushl	%0"
	: 
	: "r" (bet));
asm("
    popl	%eax
    addl	$0x0700,%eax
    popl	%edi
    movw	%ax,%es:(%edi)
    popl	%edi
    ");
}
/*===========================================================================*/
/*                                MEM_WSTR                                    */
/*===========================================================================*/
static void mem_wstr(unsigned long offset,unsigned int data)
{
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("pushl	%0"
		: 
		: "r" (data));
	asm("popl	%eax
	     popl	%edi\n\t
	     movw	%ax,%es:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
}
/*===========================================================================*/
/*                                SCROLL_VT                                  */
/*===========================================================================*/
static void scroll_vt(unsigned long num,unsigned int rvt)
{
 if (vt[rvt].c_y > 24 - num) { 
    vid_copy_vt(num*160,0,4000-(num*160),rvt); vt[rvt].c_x = 0;
    vid_copy_vt(4000+((num-1)*160),4000-(num*160),(num*160),rvt);
 } else { vt[rvt].c_y++; vt[rvt].c_x = 0; }
}
/*===========================================================================*/
/*                                SWITCH_VT                                  */
/*===========================================================================*/
static int switch_vt(int nvt)
{
unsigned long to,i;
if (alloc) {
    if (nvt > MAX_VT) {return(0);}
    if (nvt == cvt) {return(1);}
    vt[cvt].c_x = c_x;
    vt[cvt].c_y = c_y;
    vt[cvt].key_enabled = key_enabled;
    vt[cvt].str_con = str_con;
    vt[cvt].str_to = str_to;
    for (i=0;i<259;i++) { vt[cvt].vt_str[i] = ch_str[i]; }
    c_x = vt[nvt].c_x;
    c_y = vt[nvt].c_y;
    key_enabled = vt[nvt].key_enabled;
    str_con = vt[nvt].str_con;
    str_to = vt[nvt].str_to;
    for (i=0;i<259;i++) { ch_str[i] = vt[nvt].vt_str[i]; }
    //LOCK;
    	msg_copy(0xB8000,vt[cvt].buff_add,1050);
	msg_copy(vt[nvt].buff_add,0xB8000,1050);
    //UNLOCK;
    set_6845(CURSOR,(c_y * 80) + c_x);
    cvt = nvt;
return(1);
}
return(0);
}
/*===========================================================================*/
/*                               MSG_COPY                                    */
/*===========================================================================*/
static void msg_copy(unsigned long src,unsigned long dest,unsigned long size)
{
asm("pushl	%edi
     pushl	%esi
    ");
asm("pushl	%0"
	: 
	: "r" (src));
asm("pushl	%0"
	: 
	: "r" (dest));
asm("pushl	%0"
	: 
	: "r" (size));
asm("popl	%ecx
     popl	%edi
     popl	%esi
     cld
     rep
     movsl
    ");
asm("popl	%esi
     popl	%edi
    ");	
		
}
/*===========================================================================*/
/*                               VID_COPY                                    */
/*===========================================================================*/
static void vid_copy(unsigned long src,unsigned long dest,unsigned long size)
{
msg_copy(src+0xB8000,dest+0xB8000,size/4);
}
/*===========================================================================*/
/*                              VID_COPY_VT                                  */
/*===========================================================================*/
static void vid_copy_vt(unsigned long src,unsigned long dest,unsigned long size,unsigned int rvt)
{
msg_copy(src+vt[rvt].buff_add,dest+vt[rvt].buff_add,size/4);
}
/*===========================================================================*/
/*                               SET_6845                                    */
/*===========================================================================*/
static void set_6845(int reg,unsigned val)
{
/* Set a register pair inside the 6845.
 * Registers 12-13 tell the 6845 where in video ram to start
 * Registers 14-15 tell the 6845 where to put the cursor
 */
  pout_byte(0x3D4 + INDEX, reg);		/* set the index register */
  pout_byte(0x3D4 + DATA, (val>>8) & BYTE);	/* output high byte */
  pout_byte(0x3D4 + INDEX, reg + 1);		/* again */
  pout_byte(0x3D4 + DATA, val&BYTE);		/* output low byte */
}

static void pout_byte(unsigned long port,unsigned long data)
{
PUSH(port);
PUSH(data);
asm("
    popl	%eax
    popl	%edx
    outb	%al,%dx    
");
}
/*===========================================================================*/
/*                               INIT_CONSOLE                                */
/*===========================================================================*/
static void init_console()
{
 str_con = 0;
 key_enabled = 0;
 c_x = 0;
 c_y = 24;
 cvt = 1;
 scroll(1);
 alloc = 0;
}
/*===========================================================================*/
/*                                 VT_ALLOC                                  */
/*===========================================================================*/
static void vt_alloc()
{
unsigned long i;
 vt[1].buff_add = get_mem(4500);
 vt[2].buff_add = get_mem(4500);
 vt[2].c_x = 0;
 vt[2].c_y = 0;
 vt[2].key_enabled = 0;
 vt[2].str_con = 0;
 vt[2].str_to = 0;
 for (i=0;i<4400;i=i+2) {mem_wstr(vt[2].buff_add+i,0x0720);}
 alloc = 1;
}
/*===========================================================================*/
/*                                 GET_MEM                                   */
/*===========================================================================*/
static unsigned long get_mem(unsigned long size)
{
 unsigned long ret;
 PUSH(size);PUSHN(1);PUSHESP;PUSHN(2);PUSHN(0xC03);CALL(105);ADDESP(20);
 RES(ret);
 return(ret);
}

ENTRY(console_end);
