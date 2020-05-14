/*These are the descriptor types*/
#define TASK_GATE(p,dpl)\
	(0 + (2 * dpl) + (8 * p))*0x10 + 5

#define TSS_DESC(p,dpl,g)\
	(2048*g) + ((0 + (2 * dpl) + (8 * p))*0x10 + 9)

#define STACK_SEG(p,dpl,g)\
	(2048*g) + 1024 + (128*p) + (32*dpl) + 22

#define LDT_SYS(p,dpl,g)\
	(2048*g) + 1024 + (128*p) + (32*dpl) + 2
                 
#define DATA_SEG(p,dpl,g,w)\
	(2048*g) + 1024 + (128*p) + (32*dpl) + 16 + (2*w)

#define CODE_SEG(p,dpl,g,w)\
	(2048*g) + 1024 + (128*p) + (32*dpl) + 16 + 8 + (2*w)

/*These macros make privileg levels easyer*/
#define	SEL(indx,ti,rpl)\
	(8*indx) + (4*ti) + rpl
	