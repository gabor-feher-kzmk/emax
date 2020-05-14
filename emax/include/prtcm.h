#define __KERNEL_CS	0x10			/*Address of 0x000 for GDT*/
#define __KERNEL_DS	0x18			/*Address of 0x000 for GDT*/
#define	__KERNEL_SS	0x28
#define	__K_STACK_START	0x98000			/*The kernel stck segment */
#define __K_STACK	0x7FFF			/*Top of the kernel stack*/
#define __I_STACK	0x1FFF0			/*Top of the interrupt stack*/
#define __K_VIDMEM	0x20			/*Address of the video mem seg*/ 
#define __GDT_ADDR	0x200			/*Address of the GDT*/ 
#define __IDT_ADDR	0x00			/*Address of the GDT*/ 

#define __L0_IRQ	0x8E00			/*Type of the IRQ descriptor*/