/* *   main.c includes the C language kernel main things, like process sw *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "../include/kcall.h"
#include "../include/prtcm.h"
#include "../include/debug.h"
#include "../include/dtypes.h"
#include "../include/tss.h"
#include "../include/pcall.h"
#include "../include/pis.h"
#include "klib.c"
#include "proc.c"
#include "sys_call.c"

#define KS_START	0x110000
/*===========================================================================*
 *				   main					     *
 *===========================================================================*/	
 volatile unsigned long mintime;
 volatile unsigned long uct_time[3];

 volatile unsigned long doing_proc,sys_proc,dm1;
 volatile unsigned long proc_1[100];
 volatile struct devs devlib[10,5];
 volatile struct dwq dma_wq[120];
 volatile unsigned long dma_p,dma_w,int1;
 volatile unsigned long p_head[200];

 volatile unsigned long utfmap[128];
 volatile unsigned long utfsmap[128];
 volatile unsigned long utfcmap[128];
 volatile unsigned long fdc_int_wait;
 volatile unsigned long g_sig,sig_num,sig_to;
 volatile struct time_tab titab[200];
 volatile unsigned long tit_p,tit_max;

volatile unsigned long str_con,mem_max,mem_size,max_proc,max_pid;

/*------------------------------------------*/
/*-->*/ static unsigned long last_byte;    
/*
    This is the first C language routine that's is run. This sets the 
    tasks context and puts them in the scheduler's queue. Finally it
    sends the size of memory that is not used by the kernel.  
*/
 void main()
 {
 unsigned long i;
 char * hex = "xxxx\0";
 fdc_int_wait=0;g_sig=0;sig_num=0;sig_to=0;titab[0].tout=0;titab[1].tout=0; 
 tit_p=0;tit_max=0;int1=0;dm1 = 0;dma_p=0;dma_w=0;
 mem_store(&devlib[10,5]-&devlib[0,0],&devlib[0,0],0);
 devlib[2,0].type = 2;
 devlib[2,0].owner_proc = 14;
 devlib[2,0].driver_proc = 10;
 for (i=0;i<101;i++){dma_wq[1].usg = 0;dma_wq[1].slp = 0;dma_wq[1].d_proc = 0;dma_wq[1].s_proc = 0;}
 mem_max = KS_START;
 
 printd("System memory :       K \0");
 mem_size = get_mem_size()+1024;
 print_dec(mem_size,16,24);mem_size = mem_size * 1024;
 printd("Last kernel data byte: \0");
 PR_XY_HEXL(&last_byte,24,24);
 printd("Last kernel code byte: \0");
 PR_XY_HEXL(&end_session,24,24);

 mk_gdt32(7,p_int(&sys_head)+U_SPACE,104,TSS_DESC(1,0,0));
 ltr(56);
 doing_proc=0;
 p_head[0]=p_int(&sys_head);
 load_IDT();
 
 new_task(1,p_int(&clk_head),0);
 mk_ldt32(1,2,p_int(&clock),p_int(&clock_end)-p_int(&clock),CODE_SEG(1,1,0,1));
 mk_ldt32(1,3,get_smem(0x400),0x400,DATA_SEG(1,1,0,1));
 mk_ldt32(1,4,0,0xFFFFF,DATA_SEG(1,1,0,1));
 load_preg(1,_CS,SEL(2,1,1));
 load_preg(1,_EIP,0);
 load_preg(1,_SS,SEL(3,1,1));
 load_preg(1,_ESP,0x400);
 load_preg(1,_ES,SEL(4,1,1));
 load_preg(1,_DS,SEL(4,1,1));
 load_preg(1,_EFLAGS,0x2000);
 load_preg(1,_LDT_select,SEL((((/**/1/**/ *2)+7)-1),0,0));

 new_task(2,p_int(&scheduler_head),2);
 mk_ldt32(2,2,p_int(&scheduler),p_int(&scheduler_end)-p_int(&scheduler),CODE_SEG(1,0,0,1));
 mk_ldt32(2,3,get_smem(0x1000),0x1000,DATA_SEG(1,0,0,1));
 load_preg(2,_CS,SEL(2,1,0));
 load_preg(2,_EIP,0);
 load_preg(2,_SS,SEL(3,1,0));
 load_preg(2,_ESP,0x1000);
 load_preg(2,_ES,__KERNEL_DS);
 load_preg(2,_DS,__KERNEL_DS);
 load_preg(2,_EFLAGS,0);
 load_preg(2,_LDT_select,SEL((((/**/2/**/ *2)+7)-1),0,0));

 new_task(3,p_int(&message_head),3);
 mk_ldt32(3,2,p_int(&message),p_int(&message_end)-p_int(&message),CODE_SEG(1,0,0,1));
 mk_ldt32(3,3,get_smem(0x1000),0x1000,DATA_SEG(1,0,0,1));
 load_preg(3,_CS,SEL(2,1,0));
 load_preg(3,_EIP,0);
 load_preg(3,_SS,SEL(3,1,0));
 load_preg(3,_ESP,0x1000);
 load_preg(3,_DS,__KERNEL_DS);
 load_preg(3,_ES,__KERNEL_DS);
 load_preg(3,_EFLAGS,0);
 load_preg(3,_LDT_select,SEL((((/**/3/**/ *2)+7)-1),0,0));

 new_task(5,p_int(&keyboard_head),0);
 mk_ldt32(5,2,p_int(&keyboard),p_int(&keyboard_end)-p_int(&keyboard),CODE_SEG(1,1,0,1));
 mk_ldt32(5,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(5,4,0,0xFFFFF,DATA_SEG(1,1,0,1));
 load_preg(5,_CS,SEL(2,1,1));
 load_preg(5,_EIP,0);
 load_preg(5,_SS,SEL(3,1,1));
 load_preg(5,_ESP,0x1000);
 load_preg(5,_DS,SEL(4,1,1));
 load_preg(5,_ES,SEL(4,1,1));
 load_preg(5,_EFLAGS,0x2000);
 load_preg(5,_LDT_select,SEL((((/**/5/**/ *2)+7)-1),0,0));

 new_task(6,p_int(&drvsrv_head),0);
 mk_ldt32(6,2,p_int(&drvsrv),p_int(&drvsrv_end)-p_int(&drvsrv),CODE_SEG(1,1,0,1));
 mk_ldt32(6,3,get_smem(0x3000),0x3000,DATA_SEG(1,1,0,1));
 mk_ldt32(6,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 load_preg(6,_CS,SEL(2,1,1));
 load_preg(6,_EIP,0);
 load_preg(6,_SS,SEL(3,1,1));
 load_preg(6,_ESP,0x3000);
 load_preg(6,_DS,SEL(4,1,1));
 load_preg(6,_ES,SEL(4,1,1));
 load_preg(6,_EFLAGS,0x1200);
 load_preg(6,_LDT_select,SEL((((/**/6/**/ *2)+7)-1),0,0));

 new_task(9,p_int(&kbd_head),0);
 mk_ldt32(9,2,p_int(&kbd),p_int(&kbd_end)-p_int(&kbd),CODE_SEG(1,1,0,1));
 mk_ldt32(9,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(9,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 mk_ldt32(9,5,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 load_preg(9,_CS,SEL(2,1,1));
 load_preg(9,_EIP,0);
 load_preg(9,_SS,SEL(3,1,1));
 load_preg(9,_ESP,0x1000);
 load_preg(9,_DS,SEL(4,1,1));
 load_preg(9,_ES,SEL(4,1,1));
 load_preg(9,_EFLAGS,0x1200);
 load_preg(9,_LDT_select,SEL((((/**/9/**/ *2)+7)-1),0,0));
    new_thread(15,9,p_int(&kbd_t1_head),0);
     load_preg(15,_CS,SEL(2,1,1));
     load_preg(15,_EIP,p_int(&kbd_sig1)-p_int(&kbd));
     load_preg(15,_SS,SEL(5,1,1));
     load_preg(15,_ESP,0x1000);
     load_preg(15,_DS,SEL(4,1,1));
     load_preg(15,_ES,SEL(4,1,1));
     load_preg(15,_EFLAGS,0x1200);
     load_preg(15,_LDT_select,SEL((((/**/9/**/ *2)+7)-1),0,0));

 new_task(11,p_int(&int6_head),0);
 mk_ldt32(11,2,p_int(&int6),p_int(&int6_end)-p_int(&int6),CODE_SEG(1,0,0,1));
 mk_ldt32(11,3,get_smem(0x200),0x200,DATA_SEG(1,0,0,1));
 mk_ldt32(11,4,0,0xFFFFF,DATA_SEG(1,0,0,1));
 load_preg(11,_CS,SEL(2,1,0));
 load_preg(11,_EIP,0);
 load_preg(11,_SS,SEL(3,1,0));
 load_preg(11,_ESP,0x200);
 load_preg(11,_DS,SEL(4,1,0));
 load_preg(11,_ES,SEL(4,1,0));
 load_preg(11,_EFLAGS,0);
 load_preg(11,_LDT_select,SEL((((/**/11/**/ *2)+7)-1),0,0));

 new_task(7,p_int(&console_head),0);
 mk_ldt32(7,2,p_int(&console),p_int(&console_end)-p_int(&console),CODE_SEG(1,1,0,1));
 mk_ldt32(7,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(7,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 load_preg(7,_CS,SEL(2,1,1));
 load_preg(7,_EIP,0);
 load_preg(7,_SS,SEL(3,1,1));
 load_preg(7,_ESP,0x1000);
 load_preg(7,_DS,SEL(4,1,1));
 load_preg(7,_ES,SEL(4,1,1));
 load_preg(7,_EFLAGS,0x1200);
 load_preg(7,_LDT_select,SEL((((/**/7/**/ *2)+7)-1),0,0));

 new_task(8,p_int(&dma_head),0);
 mk_ldt32(8,2,p_int(&dma),p_int(&dma_end)-p_int(&dma),CODE_SEG(1,0,0,1));
 mk_ldt32(8,3,get_smem(0x1000),0x1000,DATA_SEG(1,0,0,1));
 mk_ldt32(8,4,0,0xFFFFF,DATA_SEG(1,0,1,1));
 load_preg(8,_CS,SEL(2,1,0));
 load_preg(8,_EIP,0);
 load_preg(8,_SS,SEL(3,1,0));
 load_preg(8,_ESP,0x1000);
 load_preg(8,_DS,SEL(4,1,0));
 load_preg(8,_ES,__KERNEL_DS);
 load_preg(8,_EFLAGS,512);
 load_preg(8,_LDT_select,SEL((((/**/8/**/ *2)+7)-1),0,0));

 new_task(10,p_int(&floppy_head),0);
 mk_ldt32(10,2,p_int(&floppy),p_int(&floppy_end)-p_int(&floppy),CODE_SEG(1,1,0,1));
 mk_ldt32(10,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(10,4,0,0xFF000,DATA_SEG(1,1,0,1));
 mk_ldt32(10,5,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 load_preg(10,_CS,SEL(2,1,1));

 load_preg(10,_EIP,0);
 load_preg(10,_SS,SEL(3,1,1));
 load_preg(10,_ESP,0x1000);
 load_preg(10,_ES,SEL(4,1,1));
 load_preg(10,_DS,SEL(4,1,1));
 load_preg(10,_EFLAGS,0x1200);
 load_preg(10,_LDT_select,SEL((((/**/10/**/ *2)+7)-1),0,0));
    new_thread(16,10,p_int(&floppy_t1_head),0);
     load_preg(16,_CS,SEL(2,1,1));
     load_preg(16,_EIP,&floppy_sig16-&floppy);
     load_preg(16,_SS,SEL(5,1,1));
     load_preg(16,_ESP,0x1000);
     load_preg(16,_ES,SEL(4,1,1));
     load_preg(16,_DS,SEL(4,1,1));
     load_preg(16,_EFLAGS,0x1200);
     load_preg(16,_LDT_select,SEL((((/**/10/**/ *2)+7)-1),0,0));
    
 new_task(12,p_int(&mm_head),0);
 mk_ldt32(12,2,p_int(&mm),p_int(&mm_end)-p_int(&mm),CODE_SEG(1,1,0,1));
 mk_ldt32(12,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(12,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 load_preg(12,_CS,SEL(2,1,1));
 load_preg(12,_EIP,0);
 load_preg(12,_SS,SEL(3,1,1));
 load_preg(12,_ESP,0x1000);
 load_preg(12,_ES,SEL(4,1,1));
 load_preg(12,_DS,SEL(4,1,1));
 load_preg(12,_EFLAGS,0x1200);
 load_preg(12,_LDT_select,SEL((((/**/12/**/ *2)+7)-1),0,0));

 new_task(13,p_int(&signal_head),2);
 mk_ldt32(13,2,p_int(&signal),p_int(&signal_end)-p_int(&signal),CODE_SEG(1,0,0,1));
 mk_ldt32(13,3,get_smem(0x1000),0x1000,DATA_SEG(1,0,0,1));
 load_preg(13,_CS,SEL(2,1,0));
 load_preg(13,_EIP,0);
 load_preg(13,_SS,SEL(3,1,0));
 load_preg(13,_ESP,0x1000);
 load_preg(13,_ES,__KERNEL_DS);
 load_preg(13,_DS,__KERNEL_DS);
 load_preg(13,_EFLAGS,0);
 load_preg(13,_LDT_select,SEL((((/**/13/**/ *2)+7)-1),0,0));

 new_task(14,p_int(&fs_head),0);
 mk_ldt32(14,2,p_int(&fs),p_int(&fs_end)-p_int(&fs),CODE_SEG(1,1,0,1));
 mk_ldt32(14,3,get_smem(0x2000),0x2000,DATA_SEG(1,1,0,1));
 mk_ldt32(14,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 mk_ldt32(14,5,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 load_preg(14,_CS,SEL(2,1,1));
 load_preg(14,_EIP,0);
 load_preg(14,_SS,SEL(3,1,1));
 load_preg(14,_ESP,0x2000);
 load_preg(14,_ES,SEL(4,1,1));
 load_preg(14,_DS,SEL(4,1,1));
 load_preg(14,_EFLAGS,0x1200);
 load_preg(14,_LDT_select,SEL((((/**/14/**/ *2)+7)-1),0,0));
    new_thread(17,14,p_int(&fs_t1_head),0);
     load_preg(17,_CS,SEL(2,1,1));
     load_preg(17,_EIP,&fs_sig16-&fs);
     load_preg(17,_SS,SEL(5,1,1));
     load_preg(17,_ESP,0x1000);
     load_preg(17,_ES,SEL(4,1,1));
     load_preg(17,_DS,SEL(4,1,1));
     load_preg(17,_EFLAGS,0x1200);
     load_preg(17,_LDT_select,SEL((((/**/14/**/ *2)+7)-1),0,0));

 new_task(37,p_int(&null_head),0);
 mk_ldt32(37,2,p_int(&null),p_int(&null_end)-p_int(&null),CODE_SEG(1,1,0,1));
 mk_ldt32(37,3,get_smem(0x100),0x100,DATA_SEG(1,1,0,1));
 mk_ldt32(37,4,0,0xFFFFF,DATA_SEG(1,1,0,1));
 load_preg(37,_CS,SEL(2,1,1));
 load_preg(37,_EIP,0);
 load_preg(37,_SS,SEL(3,1,1));
 load_preg(37,_DS,SEL(4,1,1));
 load_preg(37,_ESP,0x100);
 load_preg(37,_EFLAGS,512);
 load_preg(37,_LDT_select,SEL((((/**/37/**/ *2)+7)-1),0,0));

 new_task(38,p_int(&pinit_head),0);
 mk_ldt32(38,2,p_int(&pinit),p_int(&pinit_end)-p_int(&pinit),CODE_SEG(1,1,0,1));
 mk_ldt32(38,3,get_smem(0x3000),0x3000,DATA_SEG(1,1,0,1));
 mk_ldt32(38,4,0,0xFF000,DATA_SEG(1,1,0,1));
 mk_ldt32(38,5,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 load_preg(38,_CS,SEL(2,1,1));
 load_preg(38,_EIP,0);
 load_preg(38,_SS,SEL(3,1,1));
 load_preg(38,_ESP,0x3000);
 load_preg(38,_ES,SEL(4,1,1));
 load_preg(38,_DS,SEL(4,1,1));
 load_preg(38,_EFLAGS,512);
 load_preg(38,_LDT_select,SEL((((/**/38/**/ *2)+7)-1),0,0));
    new_thread(18,38,p_int(&pinit_t1_head),0);
     load_preg(18,_CS,SEL(2,1,1));
     load_preg(18,_EIP,&pinit_sig3-&pinit);
     load_preg(18,_SS,SEL(5,1,1));
     load_preg(18,_ESP,0x1000);
     load_preg(18,_ES,SEL(4,1,1));
     load_preg(18,_DS,SEL(4,1,1));
     load_preg(18,_EFLAGS,0x1200);
     load_preg(18,_LDT_select,SEL((((/**/38/**/ *2)+7)-1),0,0));

 new_task(19,p_int(&sysproc_head),0);
 mk_ldt32(19,2,p_int(&sysproc),p_int(&sysproc_end)-p_int(&sysproc),CODE_SEG(1,0,0,1));
 mk_ldt32(19,3,get_smem(0x2000),0x2000,DATA_SEG(1,0,0,1));
 mk_ldt32(19,4,0,0xFFFFF,DATA_SEG(1,0,1,1));
 load_preg(19,_CS,SEL(2,1,0));
 load_preg(19,_EIP,0);
 load_preg(19,_SS,SEL(3,1,0));
 load_preg(19,_ESP,0x2000);
 load_preg(19,_ES,SEL(4,1,0));
 load_preg(19,_DS,SEL(4,1,0));
 load_preg(19,_EFLAGS,0);
 load_preg(19,_LDT_select,SEL((((/**/19/**/ *2)+7)-1),0,0));

 new_task(21,p_int(&getty1_head),0);
 mk_ldt32(21,2,p_int(&getty1),p_int(&getty1_end)-p_int(&getty1),CODE_SEG(1,1,0,1));
 mk_ldt32(21,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(21,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 load_preg(21,_CS,SEL(2,1,1));
 load_preg(21,_EIP,0);
 load_preg(21,_SS,SEL(3,1,1));
 load_preg(21,_ESP,0x1000);
 load_preg(21,_DS,SEL(4,1,1));
 load_preg(21,_ES,SEL(4,1,1));
 load_preg(21,_EFLAGS,512);
 load_preg(21,_LDT_select,SEL((((/**/21/**/ *2)+7)-1),0,0));

 new_task(22,p_int(&getty2_head),0);
 mk_ldt32(22,2,p_int(&getty2),p_int(&getty2_end)-p_int(&getty2),CODE_SEG(1,1,0,1));
 mk_ldt32(22,3,get_smem(0x1000),0x1000,DATA_SEG(1,1,0,1));
 mk_ldt32(22,4,0,0xFFFFF,DATA_SEG(1,1,1,1));
 load_preg(22,_CS,SEL(2,1,1));
 load_preg(22,_EIP,0);
 load_preg(22,_SS,SEL(3,1,1));
 load_preg(22,_ESP,0x1000);
 load_preg(22,_DS,SEL(4,1,1));
 load_preg(22,_ES,SEL(4,1,1));
 load_preg(22,_EFLAGS,512);
 load_preg(22,_LDT_select,SEL((((/**/22/**/ *2)+7)-1),0,0));

 init_clock();
 mk_idt32(0x21,0,136,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x26,0,232,(TASK_GATE(1,0))*0x100);

 out_byte(0x60,0xFF);
 out_byte(0x60,0xFA);
 out_byte(0x60,0xF3);
 out_byte(0x60,0x00);

 enable_irq(6);
 enable_irq(1);
 enable_irq(0);
 CALL(88);

 UNLOCK;
 sys_loop();
ENTRY(end_session);  
 halt();
 }
