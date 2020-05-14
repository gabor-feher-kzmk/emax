#define __SAVE_INT	0x1000		/*The save point of the INT*/

#define NULL_PROC	37

#define tss_0		10 

/*Length =*/                               
#define	_Back Link	0
#define	_ESP0		4
#define	_SS0		8
#define	_ESP1		12
#define	_SS1		16
#define	_ESP2		20
#define	_SS2		24
#define	_CR3		28
#define	_EIP		32
#define	_EFLAGS		36

#define	_EAX		40
#define	_ECX		44
#define	_EDX		48
#define	_EBX		52
#define	_ESP		56
#define	_EBP		60
#define	_ESI		64
#define	_EDI		68
#define	_ES		72
#define	_CS		76

#define	_SS		80
#define	_DS		84
#define	_FS		88
#define	_GS		92
#define	_LDT_select	96
#define	_IO_map		100
/**/

 struct devs {
     int type;	// 0=not a vaild device 1=stream 2=blockz
     unsigned long owner_proc;
     unsigned long driver_proc;
 };

 struct dmas {
     unsigned long off;
     unsigned long size;
     unsigned long p_to;
     char rw; //1=write 0=read  
 };

 struct dwq {
     unsigned long s_proc;
     unsigned long d_proc;
     char usg;
     char slp;
    };
 struct time_tab {
     unsigned long start;
     unsigned long tout;
     unsigned long to;
     unsigned long sig;
    };
#define TASK_N 37  
#define MAX_PROC 200
