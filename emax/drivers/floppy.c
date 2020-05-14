/* *   		floppy.c includes the floppy driver for the kernel	  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "../include/pcall.h"
#include "../include/tss.h"
#include "../include/pis.h"

#define DC		6
/* I/O Ports used by floppy disk task. */
#define DOR            0x3F2	/* motor drive control bits */
#define FDC_STATUS     0x3F4	/* floppy disk controller status register */
#define FDC_DATA       0x3F5	/* floppy disk controller data register */
#define FDC_RATE       0x3F7	/* transfer rate register */
#define DMA_ADDR       0x004	/* port for low 16 bits of DMA address */
#define DMA_TOP        0x081	/* port for top 4 bits of 20-bit DMA addr */
#define DMA_COUNT      0x005	/* port for DMA count (count =  bytes - 1) */
#define DMA_FLIPFLOP   0x00C	/* DMA byte pointer flip-flop */
#define DMA_MODE       0x00B	/* DMA mode port */
#define DMA_INIT       0x00A	/* DMA init port */
#define DMA_RESET_VAL   0x06

/* Status registers returned as result of operation. */
#define ST0             0x00	/* status register 0 */
#define ST1             0x01	/* status register 1 */
#define ST2             0x02	/* status register 2 */
#define ST3             0x00	/* status register 3 (return by DRIVE_SENSE) */
#define ST_CYL          0x03	/* slot where controller reports cylinder */
#define ST_HEAD         0x04	/* slot where controller reports head */
#define ST_SEC          0x05	/* slot where controller reports sector */
#define ST_PCN          0x01	/* slot where controller reports present cyl */

/* Fields within the I/O ports. */
/* Main status register. */
#define CTL_BUSY        0x10	/* bit is set when read or write in progress */
#define DIRECTION       0x40	/* bit is set when reading data reg is valid */
#define MASTER          0x80	/* bit is set when data reg can be accessed */

/* Digital output port (DOR). */
#define MOTOR_SHIFT        4	/* high 4 bits control the motors in DOR */
#define ENABLE_INT      0x0C	/* used for setting DOR port */

/* ST0. */
#define ST0_BITS        0xF8	/* check top 5 bits of seek status */
#define TRANS_ST0       0x00	/* top 5 bits of ST0 for READ/WRITE */
#define SEEK_ST0        0x20	/* top 5 bits of ST0 for SEEK */

/* ST1. */
#define BAD_SECTOR      0x05	/* if these bits are set in ST1, recalibrate */
#define WRITE_PROTECT   0x02	/* bit is set if diskette is write protected */

/* ST2. */
#define BAD_CYL         0x1F	/* if any of these bits are set, recalibrate */

/* ST3 (not used). */
#define ST3_FAULT       0x80	/* if this bit is set, drive is sick */
#define ST3_WR_PROTECT  0x40	/* set when diskette is write protected */
#define ST3_READY       0x20	/* set when drive is ready */

/* Floppy disk controller command bytes. */
#define FDC_SEEK        0x0F	/* command the drive to seek */
#define FDC_READ        0xE6	/* command the drive to read */
#define FDC_WRITE       0xC5	/* command the drive to write */
#define FDC_SENSE       0x08	/* command the controller to tell its status */
#define FDC_RECALIBRATE 0x07	/* command the drive to go to cyl 0 */
#define FDC_SPECIFY     0x03	/* command the drive to accept params */
#define FDC_READ_ID     0x4A	/* command the drive to read sector identity */
#define FDC_FORMAT      0x4D	/* command the drive to format a track */

/* DMA channel commands. */
#define DMA_READ        0x46	/* DMA p. card  to mem*/
#define DMA_WRITE       0x4A	/* DMA write opcode */

/* Parameters for the disk drive. */
#define HC_SIZE         2880	/* # sectors on largest legal disk (1.44MB) */
#define NR_HEADS        0x02	/* two heads (i.e., two tracks/cylinder) */
#define MAX_SECTORS	  18	/* largest # sectors per track */
#define DTL             0xFF	/* determines data length (sector size) */
#define SPEC2           0x02	/* second parameter to SPECIFY */
#define MOTOR_OFF       3*HZ	/* how long to wait before stopping motor */
#define WAKEUP		2*HZ	/* timeout on I/O, FDC won't quit. */

/* Error codes */
#define ERR_SEEK         (-1)	/* bad seek */
#define ERR_TRANSFER     (-2)	/* bad transfer */
#define ERR_STATUS       (-3)	/* something wrong when getting status */
#define ERR_READ_ID      (-4)	/* bad read id */
#define ERR_RECALIBRATE  (-5)	/* recalibrate didn't work properly */
#define ERR_DRIVE        (-6)	/* something wrong with a drive */
#define ERR_WR_PROTECT   (-7)	/* diskette is write protected */
#define ERR_TIMEOUT      (-8)	/* interrupt timeout */

/* No retries on some errors. */
#define err_no_retry(err)	((err) <= ERR_WR_PROTECT)

/* Encoding of drive type in minor device number. */
#define DEV_TYPE_BITS   0x7C	/* drive type + 1, if nonzero */
#define DEV_TYPE_SHIFT     2	/* right shift to normalize type bits */
#define FORMAT_DEV_BIT  0x80	/* bit in minor to turn write into format */

/* Miscellaneous. */
#define MAX_ERRORS         6	/* how often to try rd/wt before quitting */
#define MAX_RESULTS        7	/* max number of bytes controller returns */
#define NR_DRIVES          2	/* maximum number of drives */
#define DIVISOR          128	/* used for sector size encoding */
#define SECTOR_SIZE_CODE   2	/* code to say "512" to the controller */
#define TIMEOUT		1500	/* milliseconds waiting for FDC */
#define NT                 7	/* number of diskette/drive combinations */
#define UNCALIBRATED       0	/* drive needs to be calibrated at next use */
#define CALIBRATED         1	/* no calibration needed */
#define BASE_SECTOR        1	/* sectors are numbered starting at 1 */
#define NO_SECTOR        (-1)	/* current sector unknown */
#define NO_CYL		 (-1)	/* current cylinder unknown, must seek */
#define NO_DENS		 100	/* current media unknown */
#define BSY_IDLE	   0	/* busy doing nothing */
#define BSY_IO		   1	/* doing I/O */
#define BSY_WAKEN	   2	/* got a wakeup call */
#define OK		   1    /* every things OK*/

#define DO_READ		3
#define DO_WRITE	4

#include "floppy.h"
ENTRY(floppy);
static void do_sig6();
static void do_sig16();
static void writeln(char * ch);
static void init_floppy();
static int do_read(unsigned long proc,unsigned long block);
static int do_write(unsigned long proc,unsigned long block);
static int f_read(int hd,int cyl,int sect);
static int f_write(int hd,int cyl,int sect);
static int position();
static int seek(int hd,int Hcyl);
static void stop_motor();
static void start_motor();
static void set_dma(int mode,unsigned long tr_dma,unsigned long size);
static int fdc_results();
static void fdc_reset();
static int recalibrate();
static void fdc_out(int val);
static int f_intr_wait();
static unsigned char pin_byte(unsigned long port);
static void pout_byte(unsigned long port,unsigned long data);
static unsigned long mem_load(unsigned long offset);
static void mem_str(unsigned long offset,unsigned long data);
/* these arrays incule the information about the different disk types*/
static char gap[NT] =
	{0x2A, 0x1B, 0x2A, 0x2A, 0x23, 0x23, 0x1B}; /* gap size */
static char spec1[NT] =
	{0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF}; /* step rate, etc. */
static char rate[NT] =
	{0x02, 0x00, 0x02, 0x02, 0x01, 0x01, 0x00}; /* 2=250,1=300,0=500 kbps*/
static int need_reset;		/* set to 1 when controller must be reset */
static int motor_status;	/* the motor's running */
static int recalib;
static int f_drive;
static int motor_down;
static int active,time_out,act;
static int current_spec1;	/* latest spec1 sent to the controller */
static char f_results[MAX_RESULTS];/* the controller can give lots of output */
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
static void who_w()
{
 extern unsigned long p_head[200];
 unsigned long *get,back,to,proc;
 unsigned long rc[99];
    init_floppy();
    while(TRUE) {
    get = &rc[1]; back = 0;LOCK;
active=0;PUSH(get);PUSHN(24);PUSHN(0x602);CALL(105);ADDESP(12);active=1;UNLOCK;
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
    mem_str(p_head[10]+VT_ID,mem_load(p_head[rc[1]]+VT_ID)); 	
	switch (rc[3]) {
	case DO_READ:	start_motor();				break;
	case DO_WRITE:	stop_motor();				break;
	case 5:		back=do_read(rc[4],rc[5]);		break;
	case 6:		back=do_write(rc[4],rc[5]);		break;
	}
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
	to = (rc[4]*0x100)+4;
	PUSH(back);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
    LOCK;motor_down++;UNLOCK;
    PUSHN(5000);PUSHN(3);PUSHESP;PUSHN(2);PUSHN(0x403);CALL(105);ADDESP(20);
    }/*while forever*/
}
/*===========================================================================*/
/*                                 DO_READ                                   */
/*===========================================================================*/
static int do_read(unsigned long proc,unsigned long block)
{
 unsigned long size,i,offset,to;
 unsigned long r;
 int hd,cyl,sect;
        cyl = block/36;
	sect = ((block-(block/36))%18)+1;  /* [chs => lba] */
	hd = (block/18)%2;
    if (sect == 0) return(ERR_SEEK);/* what a fast job this was */
	r = f_read(hd,cyl,sect);
	    if (r != 1) {r = f_read(hd,cyl,sect);
	    	if ( r!=1 ) {writeln("Floppy device error\r\0");}	
	    }
	    /*routine to send the DMA buffer */
	PUSH(proc);PUSHN(0x70000);PUSHN(128);PUSHN(0x009);CALL(105);ADDESP(16);
    /* |>> */    PUSH(proc);PUSHN(0x006);CALL(105);ADDESP(8);//con dma BUGGG !!!!!!!
    /* some times the floppy task goes to a never ending sleep here */
return(r);
}
/*===========================================================================*/
/*                                DO_WRITE                                   */
/*===========================================================================*/
static int do_write(unsigned long proc,unsigned long block)
{
 unsigned long size,i,to;
 unsigned long r;
 int hd,cyl,sect;
        cyl = block/36;
	sect = ((block-(block/36))%18)+1;  /* [chs => lba] */
	hd = (block/18)%2;
    if (sect == 0) return(ERR_SEEK);/* what a fast job this was */	    
    /*routine to recieve the DMA buffer */
	PUSH(proc);PUSHN(0x70000);PUSHN(130);PUSHN(0x109);CALL(105);ADDESP(16);
        PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
	r = f_write(hd,cyl,sect);
	    if (r != 1) {r = f_write(hd,cyl,sect);
		if ( r!=1 ) {writeln("Floppy device error\r\0");}	
	    }
return(r);
}
/*===========================================================================*/
/*                               INIT_FLOPPY                                 */
/*===========================================================================*/
static void init_floppy()
{
unsigned long sc;
UNLOCK;
recalib=0;f_drive=0;motor_status=0;current_spec1=0;motor_down=0;active=0;
fdc_reset();
}
/*===========================================================================*/
/*				START_MOTOR				     */
/*===========================================================================*/
static void start_motor()
{
/* Control of the floppy disk motors is a big pain.  If a motor is off, you
 * have to turn it on first, which takes 1/2 second.  You can't leave it on
 * all the time, since that would wear out the diskette.  However, if you turn
 * the motor off after each operation, the system performance will be awful.
 * The compromise used here is to leave it on for a few seconds after each
 * operation.  If a new operation is started in that interval, it need not be
 * turned on again.  If no new operation is started, a timer goes off and the
 * motor is turned off.  I/O port DOR has bits to control each of 4 drives.
 * The timer cannot go off while we are changing with the bits, since the
 * clock task cannot run while another (this) task is active, so there is no
 * need to lock().
 */
 extern unsigned long uct_time[3];
 unsigned long std;
 int running;

  running = motor_status;		/* nonzero if this motor is running */
  motor_status = 1;
  pout_byte(DOR,(motor_status << MOTOR_SHIFT) | ENABLE_INT | f_drive);

  if (running) { return; } else {
  std = uct_time[0];
  while ((uct_time[0] - std) < 1000) {}
  }
}
/*===========================================================================*/
/*				 STOP_MOTOR				     */
/*===========================================================================*/
static void stop_motor()
{
/* This routine is called by the clock interrupt after several seconds have
 * elapsed with no floppy disk activity.  It checks to see if any drives are
 * supposed to be turned off, and if so, turns them off.
 */
  pout_byte(DOR,0);
  motor_status = 0;
  recalib = 0;
}
/*===========================================================================*/
/*				DMA_SETUP				     */
/*===========================================================================*/
static void set_dma(int mode,unsigned long tr_dma,unsigned long size)
{
/* The IBM PC can perform DMA operations by using the DMA chip.  To use it,
 * the DMA (Direct Memory Access) chip is loaded with the 20-bit memory address
 * to be read from or written to, the byte count minus 1, and a read or write
 * opcode.  This routine sets up the DMA chip.  Note that the chip is not
 * capable of doing a DMA across a 64K boundary (e.g., you can't read a
 * 512-byte block starting at physical address 65520).
 */
  /* Set up the DMA registers.  (The comment on the reset is a bit strong,
   * it probably only resets the floppy channel.)
   */
  pout_byte(DMA_INIT, DMA_RESET_VAL);    /* reset the dma controller */
  pout_byte(DMA_FLIPFLOP, 0);		/* write anything to reset it */
  pout_byte(DMA_MODE, mode);
  pout_byte(DMA_ADDR, tr_dma >>  0);
  pout_byte(DMA_ADDR, tr_dma >>  8);
  pout_byte(DMA_TOP,  tr_dma >> 16);
  pout_byte(DMA_COUNT, (size - 1) >> 0);
  pout_byte(DMA_COUNT, (size - 1) >> 8);
  pout_byte(DMA_INIT, 2);	/* some sort of enable */
}
/*===========================================================================*/
/*				    SEEK				     */
/*===========================================================================*/
static int seek(int hd,int Hcyl)
{
/* To seek the head of the floppy drive you need to write to the command port
 * (0x3F5 or 0x375) the 0x0F command , and the HEADDRV and CYLINDER number
 **/
 extern unsigned long uct_time[3];
 unsigned long std;
 int r;
 if (recalib == UNCALIBRATED) if (recalibrate() != OK) {return(ERR_SEEK);}
 fdc_out(FDC_SEEK);
 fdc_out((hd << 2) | f_drive);
 fdc_out(Hcyl);
  if (need_reset) {return(ERR_SEEK);}/* if controller is sick, abort seek */
  if (f_intr_wait() != OK) return(ERR_TIMEOUT);

/* Interrupt has been received.  Check drive status. */
  fdc_out(FDC_SENSE);		/* probe FDC to make it return status */
  r = fdc_results();		/* get controller status bytes */
  if (r != OK || (f_results[ST0] & ST0_BITS) != SEEK_ST0
				|| f_results[ST1] != Hcyl) {
	/* seek failed, may need a recalibrate */
	recalib=0;
	return(ERR_SEEK);
  }
  /* give head time to settle on a format, no retrying here! */
  return(OK);
}
/*===========================================================================*/
/*                                F_READ                                     */
/*===========================================================================*/
static int f_read(int hd,int cyl,int sect)
{
 extern unsigned long uct_time[3];
 unsigned long std;
 int r;
    if (need_reset) {fdc_reset();}
		/* Set the stepping rate */
		/*if (current_spec1 != spec1[DC]) {
			fdc_out(FDC_SPECIFY);
			current_spec1 = spec1[DC];
			fdc_out(current_spec1);
			fdc_out(SPEC2);
		}
    pout_byte(FDC_RATE, rate[DC]);*/
    r = seek(hd,cyl);
    if (r!=OK) return(ERR_SEEK);
  set_dma(DMA_READ,0x70000,0x200);
  if (recalib == UNCALIBRATED) return(ERR_TRANSFER);
  if (motor_status == 0) return(ERR_TRANSFER);
    	fdc_out(FDC_READ);
	fdc_out((hd << 2) | f_drive);
	fdc_out(cyl);
	fdc_out(hd);
	fdc_out(sect);
	fdc_out(SECTOR_SIZE_CODE);
	fdc_out(18/*f_sectors*/);
	fdc_out(gap[DC]);	/* sector gap */
	fdc_out(DTL);		/* data length */
  /* Block, waiting for disk interrupt. */
  if (need_reset) return(ERR_TRANSFER);	/* if controller is sick, abort op */
  if (f_intr_wait() != OK)return(ERR_TIMEOUT);
  r = fdc_results();
  if (r != OK) return(r); 
  if ((f_results[ST0] & ST0_BITS) != TRANS_ST0)return(ERR_TRANSFER);
  if (f_results[ST1] | f_results[ST2]) return(ERR_TRANSFER);
  return(OK);
}
/*===========================================================================*/
/*                                F_WRITE                                    */
/*===========================================================================*/
static int f_write(int hd,int cyl,int sect)
{
 extern unsigned long uct_time[3];
 unsigned long std;
 int r;
    if (need_reset) {fdc_reset();}
		/* Set the stepping rate */
		/*if (current_spec1 != spec1[DC]) {
			fdc_out(FDC_SPECIFY);
			current_spec1 = spec1[DC];
			fdc_out(current_spec1);
			fdc_out(SPEC2);
		}
    pout_byte(FDC_RATE, rate[DC]);*/
    r = seek(hd,cyl);
    if (r!=OK) return(ERR_SEEK);
  set_dma(DMA_WRITE,0x70000,0x200);
  if (recalib == UNCALIBRATED) return(ERR_TRANSFER);
  if (motor_status == 0) return(ERR_TRANSFER);
    	fdc_out(FDC_WRITE);
	fdc_out((hd << 2) | f_drive);
	fdc_out(cyl);
	fdc_out(hd);
	fdc_out(sect);
	fdc_out(SECTOR_SIZE_CODE);
	fdc_out(18/*f_sectors*/);
	fdc_out(gap[DC]);	/* sector gap */
	fdc_out(DTL);		/* data length */
  /* Block, waiting for disk interrupt. */
  if (need_reset) return(ERR_TRANSFER);	/* if controller is sick, abort op */
  if (f_intr_wait() != OK)return(ERR_TIMEOUT);
  r = fdc_results();
  if (r != OK) return(r);
    if (f_results[ST1] & WRITE_PROTECT) {
	writeln("\rdiskette is write protected.\n");
	return(ERR_WR_PROTECT);
    }
   if ((f_results[ST0] & ST0_BITS) != TRANS_ST0)return(ERR_TRANSFER);
  if (f_results[ST1] | f_results[ST2]) return(ERR_TRANSFER);
  return(OK);
}
/*===========================================================================*/
/*				  FDC_OUT				     */
/*===========================================================================*/
static void fdc_out(int val)/* write this byte to floppy disk controller */
{
/* Output a byte to the controller.  This is not entirely trivial, since you
 * can only write to it when it is listening, and it decides when to listen.
 * If the controller refuses to listen, the FDC chip is given a hard reset.
 */
 extern unsigned long uct_time[3];
 unsigned long std;

 if (need_reset){return;}	/* if controller is not listening, return */

 std = uct_time[0];
  while ((pin_byte(FDC_STATUS) & (MASTER | DIRECTION)) != (MASTER | 0)) {
	if ((uct_time[0] - std) > TIMEOUT) {
		/* Controller is not listening.  Hit it over the head. */
    		need_reset = TRUE;
		return;
	}
  }
  pout_byte(FDC_DATA, val);
}
/*===========================================================================*/
/*                                RECALIBRATE                                */
/*===========================================================================*/
static int recalibrate()
{
/* The floppy disk controller has no way of determining its absolute arm
 * position (cylinder).  Instead, it steps the arm a cylinder at a time and
 * keeps track of where it thinks it is (in software).  However, after a
 * SEEK, the hardware reads information from the diskette telling where the
 * arm actually is.  If the arm is in the wrong place, a recalibration is done,
 * which forces the arm to cylinder 0.  This way the controller can get back
 * into sync with reality.
 */
  int r;
  /* Issue the RECALIBRATE command and wait for the interrupt. */
  start_motor();		/* can't recalibrate with motor off */
  fdc_out(FDC_RECALIBRATE);	/* tell drive to recalibrate itself */
  fdc_out(f_drive);			/* specify drive */
  if (need_reset){ return(ERR_SEEK);}/* don't wait if controller is sick */
  if (f_intr_wait() != OK) return(ERR_TIMEOUT);
  /* Determine if the recalibration succeeded. */
  fdc_out(FDC_SENSE);		/* issue SENSE command to request results */
  r = fdc_results();		/* get results of the FDC_RECALIBRATE command*/
  if (r != OK ||		/* controller would not respond */
     (f_results[ST0] & ST0_BITS) != SEEK_ST0 || f_results[ST_PCN] != 0) {
	/* Recalibration failed.  FDC must be reset. */
	need_reset = TRUE;
	return(ERR_RECALIBRATE);
  } else {
	/* Recalibration succeeded. */
        recalib = CALIBRATED;
	return(OK);
  }
}
/*===========================================================================*/
/*                               FDC_RESULTS                                 */
/*===========================================================================*/
static int fdc_results()
{
/* Extract results from the controller after an operation, then allow floppy
 * interrupts again.
 */
  extern unsigned long uct_time[3];
  unsigned long std;
  int result_nr, status;
  /* Extract bytes from FDC until it says it has no more.  The loop is
   * really an outer loop on result_nr and an inner loop on status.
   */
  std = uct_time[0];
  result_nr = 0;
  do {
	/* Reading one byte is almost a mirror of fdc_out() - the DIRECTION
	 * bit must be set instead of clear, but the CTL_BUSY bit destroys
	 * the perfection of the mirror.
	 */
	status = pin_byte(FDC_STATUS) & (MASTER | DIRECTION | CTL_BUSY);
	if (status == (MASTER | DIRECTION | CTL_BUSY)) {
		if (result_nr >= MAX_RESULTS) break;	/* too many results */
		f_results[result_nr++] = pin_byte(FDC_DATA);
		continue;
	}
	if (status == MASTER) {	/* all read */
		return(OK);	/* only good exit */
	}
  } while ((uct_time[0] - std) < TIMEOUT);
  need_reset = TRUE;		/* controller chip must be reset */
  return(ERR_STATUS);
}
/*===========================================================================*/
/*                                FDC_RESET                                  */
/*===========================================================================*/
static void fdc_reset()
{
 extern unsigned long fdc_int_wait;
 extern unsigned long uct_time[3];
 unsigned long std,i;
  need_reset = 0;
  LOCK;
  motor_status = 0;
  pout_byte(DOR, 0);		/* strobe reset bit low */
  pout_byte(DOR, ENABLE_INT);	/* strobe it high again */
  fdc_int_wait = 1;
  std = uct_time[0];
  UNLOCK;
  for (i = 0; i < 4; i++) {
	fdc_out(FDC_SENSE);	/* probe FDC to make it return status */
	(void) fdc_results();	/* flush controller */
  }
  recalib = 0;			/* we need to calibrate after a hard reset*/
  /* The current timing parameters must be specified again. */
  current_spec1 = 0;
  while (fdc_int_wait == 1){		/*Let's have our afternoon sleep before the*/
                          	/*INT wakes us up*/
    if ((uct_time[0] - std) > TIMEOUT) {
	return;
    }
    CALL(88);
  }
}
/*===========================================================================*/
/*                                F_INTR_WAIT                                */
/*===========================================================================*/
static int f_intr_wait()
{
 extern unsigned long uct_time[3];
 extern unsigned long fdc_int_wait;
 unsigned long std;
/* Wait for an interrupt, but not forever.  The FDC may have all the time of
 * the world, but we humans do not.
 */
LOCK;
  fdc_int_wait = 1;
  std = uct_time[0];
UNLOCK;
//  act = 1;
  //int6 = 0;time_out=0;
    //PUSHN(0x00A);CALL(105);
  while (fdc_int_wait){
    if ((uct_time[0] - std) > 2000) {
	/* No interrupt from the FDC, this means that there is probably no
	 * floppy in the drive.  Get the FDC down to earth and return error.
	 */
	  fdc_reset();
     return ERR_TIMEOUT;
    }
    CALL(88);
  }
  return(OK);
}
/*===========================================================================*/
/*                                  UTILS                                    */
/*===========================================================================*/
static unsigned char pin_byte(unsigned long port)
{
unsigned long ret;
	PUSH(port);
	asm( "	popl	%edx
		xorl	%eax,%eax
		inb	%dx,%al ");
	RES(ret);
return ret;	
}
static void pout_byte(unsigned long port,unsigned long data)
{
PUSH(port);
PUSH(data);
asm("
    popl	%eax
    popl	%edx
    outb	%al,%dx    
");
}
static void writeln(char * ch)
{
unsigned long buff[255];
unsigned long i,*cp,size;
cp=&buff[1];
i=1;    
    while (ch[i-1] != 0) {
    buff[i] = ch[i-1];
    i++;}buff[i]=0;
    size = i+3;
PUSHN(6);PUSH(cp);PUSH(size);PUSHN(0x08);CALL(105);ADDESP(16);
PUSHN(1);PUSHN(0);PUSHN(4);PUSHESP;PUSHN(3);PUSHN(0x603);CALL(105);ADDESP(24);
}
/*===========================================================================*/
/*                               MEM_LOAD                                    */
/*===========================================================================*/
static unsigned long mem_load(unsigned long offset)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("popl	%edi\n\t
	     movl	%es:(%edi),%eax\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}
/*===========================================================================*/
/*                                MEM_STR                                    */
/*===========================================================================*/
static void mem_str(unsigned long offset,unsigned long data)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("pushl	%0"
		: 
		: "r" (data));
	asm("popl	%eax
	     popl	%edi\n\t
	     movl	%eax,%es:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
}
/*===========================================================================*/
/*                                 SIGNALS                                   */
/*===========================================================================*/
ENTRY(floppy_sig6);
//static void do_sig6()
// {
//  while (TRUE) {
//     if (act) {int6=1;act=0;PUSHN(0x00A);CALL(105);}
//	PUSHN(0x60B);CALL(105);/* end of signal*/
//  }
// }
ENTRY(floppy_sig16);
static void do_sig16()
{
LOCK;
 while (TRUE) {
    if (motor_down == 1) {
	if (active == 0) {     
	    UNLOCK;stop_motor();LOCK;
        }
    }
    if (motor_down!=0) {motor_down--;}
 PUSHN(0x10A);CALL(105);ADDESP(4);/* end of signal*/
 } 
}
ENTRY(floppy_end);
#include "./ints/int6.c"

