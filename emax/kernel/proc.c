/* * 	       proc.c includes the routines to handle processes	  	  *
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
/*                                NEW_PROC                                   */
/*===========================================================================*/
void new_task(unsigned long tnm,unsigned long tss,unsigned long pri)
{
 extern unsigned long p_head[200];

 mk_gdt32(((tnm * 2)+7-1),tss+LDT_SPACE,0x50,LDT_SYS(1,0,0));
 p_head[tnm] = tss;
 mem_vsl(p_head[tnm]+proc_TYPE,1);
 mem_vsl(p_head[tnm]+THR_NEXT,tnm);
 mem_vsl(p_head[tnm]+LDT_off,p_head[tnm]+LDT_SPACE);
 mk_gdt32((tnm * 2)+7,tss+U_SPACE,104,TSS_DESC(1,pri,0));
}
/*===========================================================================*/
/*                                NEW_THREAD                                 */
/*===========================================================================*/
void new_thread(unsigned long tnm,unsigned long mother,unsigned long tss
,unsigned long pri)
{
 extern unsigned long p_head[200];

 mk_gdt32(((tnm * 2)+7-1),p_head[mother]+LDT_SPACE,0x50,LDT_SYS(1,0,0));
 p_head[tnm] = tss;
 mem_vsl(p_head[tnm]+proc_TYPE,2);
 mem_vsl(p_head[tnm]+THR_NEXT,tnm);
 mem_vsl(p_head[tnm]+THR_MOTHER,mother);
 mem_vsl(p_head[tnm]+LDT_off,mem_vll(p_head[mother]+LDT_off));
 mk_gdt32((tnm * 2)+7,tss+U_SPACE,104,TSS_DESC(1,pri,0));
}
/*===========================================================================*/
/*                               LOAD_PREG                                   */
/*===========================================================================*/
void load_preg(unsigned long pn,unsigned long reg,unsigned long data)
{
 extern unsigned long p_head[200];

 mem_vsl(reg+p_head[pn]+U_SPACE,data);
} 
/*===========================================================================*/
/*                               GET_PREG                                    */
/*===========================================================================*/
unsigned long get_preg(unsigned long pn,unsigned long reg)
{
 extern unsigned long p_head[200];

 return mem_vll(reg+p_head[pn]+U_SPACE);
} 
/*===========================================================================*/
/*                               FREE_TASK                                   */
/*===========================================================================*/
void free_task(unsigned task)
{
 unsigned toff;
 toff = 0x238+(((2*task))*8)+5;
 mem_vsb(toff,mem_vlb(toff)^2);
}

 