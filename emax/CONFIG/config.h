/* *            config.h includes the main header of the  system          *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************
/*
    This is the main header of the system. You can modify the systems main
    features by changeing the values of the defined constants
*/
#define C_CACHE_SIZE		25	/* The size of the disk cache in KiloBytes */  

#define C_SYNC_TIME		30      /* This is the time in seconds, thats between to sync call */
					/* A sync call writes the cache to the disk */

#define C_CLOCK_FREQ		100     /* This is the timers frequency in Hertz */
					/* Slower CPU (386,486) =>	50Hz - 60Hz	*/
					/* Faster CPU (>Pentium) => 	100Hz 		*/
