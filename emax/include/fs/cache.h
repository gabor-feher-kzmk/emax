/*			The structure of the fs cacheing system
 *****************************************************************************/

#include "../../CONFIG/config.h"

#define CACHE_SIZE	C_CACHE_SIZE*2	/*11*/
#define SYNC_TIME	C_SYNC_TIME	/*30*/

struct cache_type {
    unsigned long accesed;
    int modify;
};