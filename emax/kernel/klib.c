/* *   		kilb.c includes the C language kernel routines		  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/

/*===========================================================================*/
/*                                PRINT_DEC                                  */
/*===========================================================================*/
void print_dec(unsigned long num,unsigned long x,unsigned long y)
{
unsigned long xc,dig,den;
char * dec = "x\0" ;
den = num;
xc = x;
    	dig = den / 10000;
	den = den - (dig*10000); 
     dec[0]=dig+48;pr_xy(xc,y,7,dec);xc++;
    	dig = den / 1000;
	den = den - (dig*1000); 
     dec[0]=dig+48;pr_xy(xc,y,7,dec);xc++;
    	dig = den / 100;
	den = den - (dig*100); 
     dec[0]=dig+48;pr_xy(xc,y,7,dec);xc++;
    	dig = den / 10;
	den = den - (dig*10); 
     dec[0]=dig+48;pr_xy(xc,y,7,dec);xc++;
     dec[0]=den+48;pr_xy(xc,y,7,dec);
}
/*===========================================================================*/
/*                               GET_MEM_SIZE                                */
/*===========================================================================*/
unsigned long get_mem_size()
{
unsigned long mem;
unsigned int al,ah; 
    out_byte(0x70,0x17);
    al = in_byte(0x71);

    out_byte(0x70,0x18);
    ah = in_byte(0x71);

mem = (ah * 0x100) + al;
return mem;    
}
/*===========================================================================*/
/*                                GET_MEM                                    */
/*===========================================================================*/
static unsigned long get_smem(unsigned long size)
{
extern unsigned long mem_max;
unsigned long ret;    
    ret = mem_max;
    mem_max += size;
    return(ret);
}
/*===========================================================================*/
/*                                ST_COPY                                    */
/*===========================================================================*/
void st_copy(char * a,char * b)
 {
 unsigned i;

     for(i = 0;i <= 20;i++)
     {
     a[i] = b[i];
     }
 }
/*===========================================================================*/
/*                                SEL_OFF                                    */
/*===========================================================================*/
unsigned long sel_off(unsigned long ss,unsigned long proc)
 {
 extern unsigned long p_head[200];
 unsigned long foff,ret;

 foff = ss / 8;
if(proc==0){foff=(foff*8)+0x200;
}else{foff=(foff*8)+mem_vll(p_head[proc]+LDT_off);}
 foff = foff + 2;
 ret = mem_vl(foff);
 foff = foff + 2;
 ret = ret + al(mem_vl(foff))*0x10000;
 foff = foff + 2;
 ret = ret + ah(mem_vl(foff))*0x1000000;
 return ret;
 }
/*===========================================================================*/
/*                                 PRINTD                                    */
/*===========================================================================*/
void printd(char * cstr)
 {
 scroll_screen_up();
 pr_xy(0,24,7,cstr);
 }
/*===========================================================================*/
/*                                MK_IDT32                                   */
/*===========================================================================*/
void mk_idt32(unsigned int num,unsigned long offset,unsigned int select,
 unsigned int type)
 {
 unsigned long foff;
 
 foff = (num * 8)/*+__IDT_ADDR*/;
 mem_vs(foff,axl(offset));
 foff = foff + 2;
 mem_vs(foff,select);
 foff = foff + 2;
 mem_vs(foff,type);
 foff = foff + 2;
 mem_vs(foff,axh(offset));         
 }    
/*===========================================================================*/
/*                                MK_LDT32                                   */
/*===========================================================================*/
void mk_ldt32(unsigned int pn,unsigned int num,unsigned long offset,
unsigned long hat,unsigned int type)
 {
 extern unsigned long p_head[200];
 unsigned long foff;

 foff = (num * 8)+mem_vll(p_head[pn]+LDT_off);
 mem_vs(foff,axl(hat));
 foff = foff + 2;
 mem_vs(foff,axl(offset));
 foff = foff + 2;
 mem_vsb(foff,al(axh(offset)));
 foff = foff + 1;
 mem_vsb(foff,al(type));
 foff = foff + 1;
 mem_vsb(foff,((ah(type)*0x10)+axh(hat)) );
 foff = foff + 1;
 mem_vsb(foff,ah(axh(offset)));
 }    
/*===========================================================================*/
/*                                MK_GDT32                                   */
/*===========================================================================*/
void mk_gdt32(unsigned int num,unsigned long offset,unsigned long hat,
 unsigned int type)
 {
 unsigned long foff;

 foff = (num * 8)+__GDT_ADDR;
 mem_vs(foff,axl(hat));
 foff = foff + 2;
 mem_vs(foff,axl(offset));
 foff = foff + 2;
 mem_vsb(foff,al(axh(offset)));
 foff = foff + 1;
 mem_vsb(foff,al(type));
 foff = foff + 1;
 mem_vsb(foff,((ah(type)*0x10)+axh(hat)) );
 foff = foff + 1;
 mem_vsb(foff,ah(axh(offset)));
 }    


    