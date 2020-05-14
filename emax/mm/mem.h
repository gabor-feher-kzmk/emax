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
/* The main structures of the memory server
 */
 struct sig_tab {
 unsigned long offset;
 
 };

/* This  part of the file containes the routines to link the files together 
 * in the memory server. 
 **/
 
/* in memsrv.c*/ 
 static void init_mem();

/* in signal.c*/
 static int do_set_signal(unsigned long event);