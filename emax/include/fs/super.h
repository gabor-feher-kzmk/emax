/*			The structure of the emax fs superblock
 ****************************************************************************/

#define		FS_VER		0	/*the version of the fs*/ 
#define		FS_SIZE		4	/*the size of the fs in blocks (512 b)*/ 
#define		FS_NSUPER	8	/*the next superblock's address*/ 
#define		FS_ROOT		12	/*the first block for dir struct*/ 
#define		FS_FREEB	16	/*the free block bitmap*/ 


