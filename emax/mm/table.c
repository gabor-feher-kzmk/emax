/* *   		table.c includes the memory table for the mm server	  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/

#include "memutils.c"
/*===========================================================================*/
/*                                 FREE_BLOCK                                */
/*===========================================================================*/
static void free_block(unsigned long block)
{
unsigned long rol,byte;
    byte = block / 8;
    rol = block % 8;
    memtab[byte] = memtab[byte] & (0xFF^(1 << rol));
}
/*===========================================================================*/
/*                                  GET_BLOCK                                */
/*===========================================================================*/
static void get_block(unsigned long block)
{
unsigned long rol,byte;
    byte = block / 8;
    rol = block % 8;
    memtab[byte] = memtab[byte] | (1 << rol);
}
/*===========================================================================*/
/*                                  CEAR_BLOCK                               */
/*===========================================================================*/
static void clear_block(unsigned long block)
{
unsigned long offset;
    offset = block * 512;
    fill_up(offset,0xFECABABA,512);
}
/*===========================================================================*/
/*                                 GET_REGION                                */
/*===========================================================================*/
static int get_region(unsigned long from,unsigned long to)
{
 extern unsigned long mem_size;
 unsigned long fb,tb,i;
     if (to > mem_size) {return(0);}
 fb = from / 512;
 tb = to / 512;
 if ((to%512) != 0) { tb++; }
     for (i=fb;i<=tb;i++) {
         get_block(i);
     }
 return(1);
}
/*===========================================================================*/
/*                                 FREE_REGION                               */
/*===========================================================================*/
static int free_region(unsigned long from,unsigned long to)
{
 extern unsigned long mem_size;
 unsigned long fb,tb,i;
     if (to > mem_size) {return(0);}
     if (to == from) {return(0);}
 fb = from / 512;
 tb = to / 512;
 if ((to%512) != 0) { tb++; }
     for (i=fb;i<=tb;i++) {
         free_block(i);
         clear_block(i);
     }
 return(1);
}
/*===========================================================================*/
/*                                  GET_MEM                                  */
/*===========================================================================*/
static int test(unsigned long by,unsigned long size)
{
unsigned long i;
    for (i=by;i<=size;i++) {
    if (memtab[i] != 0x00) {return(0);}
    }
 return(1);
}

static unsigned long get_mem(unsigned long size)
{
 extern unsigned long mem_size;
 unsigned long bn,i;
 
 if (size == 0) {return(0);}
 bn = (size / 4096)+1;
 
 i=0;
 while (TRUE) {
    if (memtab[i] == 0x00) {
	if (test(i,bn)==1) {get_region(i*4096,(i*4096)+size);return(i*4096);}
    }i++;
 if ((i*4096) > mem_size) {return(0);}
 }
}



