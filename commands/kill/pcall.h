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
#define PUSH(var)\
	asm("pushl %0\
		"\
		: \
		: "r" (var));
#define CALL(num)\
	asm("ljmp $" #num " ,$0");

#define PUSHN(num)\
	asm("pushl $"#num);

#define ADDESP(num)\
	asm("addl $" #num ",%esp ");

#define START\
	asm("pushl %ebp\n\
    	movl %esp,%ebp");

#define END\
        asm("movl %ebp,%esp\n\
        popl %ebp");

#define PUSHEAX\
	asm("pushl	%eax");

#define PUSHESP\
	asm("pushl	%esp");

#define RES(var)\
	PUSHEAX\
	asm("popl %0\
		"\
		: "=r" (var)\
		: );
			
#define LOCK\
	asm(" cli ");

#define UNLOCK\
	asm(" sti ");

#define ENTRY(str)\
asm(".globl "#str );\
asm(#str": ")

#define LONG(lnm)\
asm(".long "#lnm );

#define BYTE(bnm)\
asm(".byte "#bnm );

#define STRING(lnm)\
asm(".ascii "#lnm );
/*
 * These are the interface for the protected mode routines
 */
/* void printd(char * cstr); */
#define printe(cst)\
    PUSH(cst)\
    PUSHN(1)\
    CALL(56)\
    ADDESP(8)

/* void pr_xy(unsigned x,unsigned y,unsigned c,char * cstr); */
#define print_xy(x,y,c,cst)\
    PUSH(cst)\
    PUSH(c)\
    PUSH(y)\
    PUSH(x)\
    PUSHN(2)\
    CALL(56)\
    ADDESP(20)

#define TRUE	1   
#define FALSE	0   

     