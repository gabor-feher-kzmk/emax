
#include "../CONFIG/config.h"

#define HZ C_CLOCK_FREQ /*100*/		/* MAX 2400 on 25MHZ /AT isa */
#define	TIMER_COUNT ((unsigned) (TIMER_FREQ/HZ))
#define TIMER_FREQ 1193182L
#define SQUARE_WAVE 0x36
#define TIMER0 0x40
#define TIMER2 0x42
#define TIMER_MODE 0x43
