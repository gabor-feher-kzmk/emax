/* *   		rw.c includes the file management for the kernel	  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
/*read write utilities for the fs server*/

static void write_cache(unsigned long num);
static unsigned long find_cache(unsigned long block);
static unsigned long make_free(unsigned long block);

/*===========================================================================*/
/*                                 F_READ                                    */
/*===========================================================================*/
static unsigned char *f_read(unsigned long block)
{
char tmp[60];
unsigned long h,c,s,fb,tfb;
unsigned char *ch;
int ret;
 fb = find_cache(block);
 if (fb != 0xFFFFFFFF) {
    return(&cache[(fb*512)+0]);
    if (c_prop[fb].accesed != 0xFFFFFFFF) {c_prop[fb].accesed++;}
} else { 
    tfb = make_free(block);
    ch = &cache[(tfb*512)+0];
    if (c_prop[tfb].accesed != 0xFFFFFFFF) {c_prop[tfb].accesed++;}
    PUSHN(10);PUSH(ch);PUSHN(130);PUSHN(0x109);CALL(105);ADDESP(16);
    PUSH(block);PUSHN(0);PUSHN(2);PUSHN(3);PUSHESP;
    PUSHN(4);PUSHN(0x603);CALL(105);ADDESP(28);
    RES(ret);
    return(ch);
 }
}
/*===========================================================================*/
/*                                F_WRITE                                    */
/*===========================================================================*/
static int f_write(unsigned char * ch,unsigned long block)
{
unsigned long i,cb,mb;
unsigned char *cc;
 cb = find_cache(block);
 if (cb != 0xFFFFFFFF) {
    if (c_prop[cb].accesed != 0xFFFFFFFF) {c_prop[cb].accesed++;}
    c_prop[cb].modify = 1;
    cc = &cache[(cb*512)+0];
         if (cc != ch) {
	     for (i=0;i<512;i++) {cc[i] = ch[i];}
	 }
 } else {
    mb = make_free(block);
    cc = &cache[(mb*512)+0];
    if (c_prop[mb].accesed != 0xFFFFFFFF) {c_prop[mb].accesed++;}
    c_prop[mb].modify = 1;
         if (cc != ch) {
	     for (i=0;i<512;i++) {cc[i] = ch[i];}
	 }
 }    
return(1);
}
/*===========================================================================*/
/*                                 F_READ                                    */
/*===========================================================================*/
static unsigned char fd_read(unsigned long block,unsigned long byte)
{
unsigned char ret;
unsigned char *buff;
    buff = f_read(block);
    ret = buff[byte];
    return ret;
}
/*===========================================================================*/
/*                                F_WRITE                                    */
/*===========================================================================*/
static void fd_write(unsigned long block,unsigned long byte,unsigned char data)
{
unsigned char *buff;
unsigned char ret;
    buff = f_read(block);
    buff[byte] = data;
    f_write(buff,block);
}
static unsigned long fd_rlong(unsigned long block,unsigned long byte)
{
unsigned char *buff;
unsigned long ret;
    buff = f_read(block);
    ret = 0;
    ret += buff[byte];byte++;
    ret += (buff[byte]*0x100);byte++;
    ret += (buff[byte]*0x10000);byte++;
    ret += (buff[byte]*0x1000000);
    return ret;
}

static void fd_wlong(unsigned long block,unsigned long byte,unsigned long data)
{
unsigned char *buff;
unsigned char lp;
unsigned char h,c,s;
    buff = f_read(block);
    
    lp = data;
    buff[byte]=lp;
    lp = data >> 8;byte++;
    buff[byte]=lp;
    lp = data >> 16;byte++;
    buff[byte]=lp;
    lp = data >> 24;byte++;
    buff[byte]=lp;
    f_write(buff,block);
}
/*===========================================================================*/
/*                                WRITE_CACHE                                */
/*===========================================================================*/
static void write_cache(unsigned long num)
{
unsigned long block,ret;
unsigned char *ch;
    block = free[num];
    ch = &cache[(num*512)+0];
    PUSHN(10);PUSH(ch);PUSHN(128);PUSHN(0x009);CALL(105);ADDESP(16);
    PUSH(block);PUSHN(0);PUSHN(2);PUSHN(4);PUSHESP;
    PUSHN(4);PUSHN(0x603);CALL(105);ADDESP(28);
    RES(ret);
}
/*===========================================================================*/
/*                                 FIND_CACHE                                */
/*===========================================================================*/
static unsigned long find_cache(unsigned long block)
{
unsigned long i;
	for (i=0;i<CACHE_SIZE;i++) {
	    if (free[i] == block) { return(i); }
	}
    return(0xFFFFFFFF);
}
/*===========================================================================*/
/*                                 MAKE_FREE                                 */
/*===========================================================================*/
static unsigned long make_free(unsigned long block)
{
unsigned long i;
    	for (i=0;i<CACHE_SIZE;i++) {
	    if (free[i] == 0) { free[i]=block;return(i); }
	}
	circ++; if (circ>(CACHE_SIZE-1)) { circ=0; } else { }
	write_cache(circ);
	free[circ] = block;
	c_prop[circ].accesed = 0;
	c_prop[circ].modify = 0;
	return(circ); 
}
/*===========================================================================*/
/*                                WRITE_ALL                                  */
/*===========================================================================*/
static void write_all()
{
unsigned long i;
    	for (i=0;i<CACHE_SIZE;i++) {
	    if (free[i] != 0) {
		if (c_prop[i].modify == 1) {
		    write_cache(i);
		    c_prop[i].modify = 0;
		}
	    }
	}
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
/*===========================================================================*/
/*                                MEM_STR                                    */
/*===========================================================================*/
static void mem_str(unsigned long offset,unsigned long data)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("pushl	%0"
		: 
		: "r" (data));
	asm("popl	%eax
	     popl	%edi\n\t
	     movl	%eax,%es:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
}


