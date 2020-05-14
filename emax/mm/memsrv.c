/* *   		memsrv.c includes the memory management for the kernel	  *
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
#include "mem.h"

#include "mm.h"

static void free_block(unsigned long block);
static void get_block(unsigned long block);
static void clear_block(unsigned long block);
static int get_region(unsigned long from,unsigned long to);
static int free_region(unsigned long from,unsigned long to);
static unsigned long get_mem(unsigned long size);
static void readln(char * ch);

static unsigned long pow(unsigned long x,unsigned long y);

static void writeln(char * ch);
static void ltoa(char *str,unsigned long lnum);

ENTRY(mm);
static unsigned long max_block;
static unsigned char *memtab;
static char sbuff[200];
/*
    The Memory Server manages the free and the used memory. It will handle
    swap in the future. ;-)
*/
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
static void who_w()
{
 unsigned long *get,back,to,proc,i;
 unsigned long rc[99];
    init_mem();
    while(TRUE) {
    get = &rc[1]; back = 0;
    PUSH(get);PUSHN(24);PUSHN(0x405);CALL(105);ADDESP(12);
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
	switch (rc[3]) {
	case 1:	back = get_mem(rc[4]);				break;
	case 2:	back = free_region(rc[4],rc[5]);		break;
	}
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
	to = (rc[1]*0x100)+4;
	PUSH(back);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
    }/*while forever*/
}
/*===========================================================================*/
/*                                INIT_MEM                                   */
/*===========================================================================*/
static void init_mem()
{
extern unsigned long mem_max,mem_size;
unsigned long i,to;

 max_block = mem_size / 512;
 memtab = (unsigned char *) mem_max;
 mem_max += (mem_size / (8 * 512))+512;
 writeln("Initializing memory... \0");
    get_region(0,mem_max);
    free_region(mem_max,mem_size);
 ltoa(sbuff,mem_size);
 writeln("Initialized \0");writeln(sbuff);writeln(" bytes of memory\r\0");
 PUSHN(3);PUSHN(38);PUSHN(5);PUSHESP;PUSHN(3);PUSHN(0x403);
 CALL(105);ADDESP(24);/*send sig to init*/
}

#include "table.c"
#include "../lib/stdlib.c"
#include "../lib/math.c"

/*===========================================================================*/
/*                                 WRITELN                                   */
/*===========================================================================*/
static void writeln(char * ch)
{
unsigned long buff[255];
unsigned long i,*cp,size;
cp=&buff[1];
i=1;    
    while (ch[i-1] != 0) {
    buff[i] = ch[i-1];
    i++;}buff[i]=0;
    size = i+3;
PUSHN(6);PUSH(cp);PUSH(size);PUSHN(0x08);CALL(105);ADDESP(16);
PUSHN(1);PUSHN(0);PUSHN(4);PUSHESP;PUSHN(3);PUSHN(0x603);CALL(105);ADDESP(24);
}
ENTRY(mm_end);

