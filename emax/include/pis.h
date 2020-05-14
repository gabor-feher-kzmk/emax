/* *   pis.h  the constats for the process information for the kernel     *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/

#define		proc_GDT	0
#define		SFC_id		4
#define		LDT_off		8
#define		TSS_off		12
#define		PS_start	16
#define		PS_size		20
#define		proc_TYPE	24
#define		THR_NEXT	28

#define		VT_ID		32
#define		free1		36
#define		free2		40
#define		free3		44

#define		proc_PRI	48
#define		p_PRI_data	52
#define		proc_STATUS	56

#define		mess_STK	60
#define		mess_DEST	64
#define		mess_SIZE	68
#define		mesr_STK	72
#define		mesr_DEST	76
#define		mesr_SIZE	80

#define		dma_OFF		84
#define		dma_SIZE	88
#define		dma_PTO		92
#define		dma_MODE	96

#define		dma2_OFF	100
#define		dma2_SIZE	104
#define		dma2_PTO	108
#define		dma2_MODE	112

#define		KILL_STAT	116

#define		U_SPACE		120

#define		THR_MAX		224
#define		THR_MOTHER	228
#define		THR_NUM		232

#define		LDT_SPACE	236

#define		SIGNAL		316
#define		DEF_SIG		320

#define		SIG_NUM		0
#define		SIG_MASK	4
#define		SIG_THREAD	8
          