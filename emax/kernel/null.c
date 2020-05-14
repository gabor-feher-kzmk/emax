/* * 	                 null.c includes the null proc		  	  *
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

/*
    The Null task is ran when there are no other procs to run
*/

#include "null.h"
ENTRY(null);
void null_loop()
{
 while(TRUE) { 
 /*		DO NOTHING		*/
 }
}

ENTRY(null_end);

