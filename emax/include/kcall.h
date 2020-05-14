/* *   kcall.h includes the definition of the routines for the kernel     *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
/*
 *  These are C language routines for the kernel
 */
void print_dec(unsigned long num,unsigned long x,unsigned long y);
unsigned long get_mem_size();
void printd(char * cstr);
void mk_gdt32(unsigned int num,unsigned long offset,unsigned long hat,
 unsigned int type);
void mk_idt32(unsigned int num,unsigned long offset,unsigned int select,
 unsigned int type);
void new_task(unsigned long tnm,unsigned long tss,unsigned long pri);
void load_preg(unsigned long pn,unsigned long reg,unsigned long data);
unsigned long get_preg(unsigned long pn,unsigned long reg);
void sys_call(unsigned long sysc_num,unsigned long proc_stack);

/*
 *These are C language routines for the kernel witch are not included in main.c
 */
 extern void init_clock();
 extern void load_IDT(); 
 extern void clock();			//The interface for the clock task
 extern void clk_head();		//The interface for the clock task
 extern void tick();			//The interface for the clock task
 extern void clock_end();		//The interface for the end of the clock task
 extern void scheduler();		//The interface for the scheduler task
 extern void scheduler_end();		//The interface for the end of the scheduler task
 extern void scheduler_head();		//The interface for the scheduler task
 extern void pinit();			//The interface for the scheduler task
 extern void pinit_end();		//The interface for the scheduler task
 extern void pinit_head();		//The interface for the scheduler task
 extern void pinit_t1_head();		//The interface for the scheduler task
 extern void pinit_sig3();
 extern void getty1();			//The interface for the scheduler task
 extern void getty1_end();		//The interface for the scheduler task
 extern void getty1_head();		//The interface for the scheduler task
 extern void getty2();			//The interface for the scheduler task
 extern void getty2_end();		//The interface for the scheduler task
 extern void getty2_head();		//The interface for the scheduler task
 extern void message();
 extern void message_end();
 extern void message_head();
 extern void dma();
 extern void dma_end();
 extern void dma_head();
 extern void null();
 extern void null_end();
 extern void null_head();
 extern void keyboard();
 extern void keyboard_end();
 extern void keyboard_head();
 extern void drvsrv();
 extern void drvsrv_end();
 extern void drvsrv_head();
 extern void console();
 extern void console_end();
 extern void console_head();
 extern void kbd();
 extern void kbd_end();
 extern void kbd_head();
 extern void kbd_t1_head();
 extern void floppy();
 extern void floppy_end();
 extern void floppy_head();
 extern void floppy_t1_head();
 extern void floppy_sig16();
 extern void floppy_sig6();
 extern void int6();
 extern void int6_end();
 extern void int6_head();
 extern void mm();
 extern void mm_end();
 extern void mm_head();
 extern void sys_head();
 extern void signal();
 extern void signal_end();
 extern void signal_head();
 extern void fs();
 extern void fs_end();
 extern void fs_head();
 extern void fs_t1_head();
 extern void fs_sig16();
 extern void kbd_sig1();
 extern void sysproc();
 extern void sysproc_end();
 extern void sysproc_head();

extern void end_session();

