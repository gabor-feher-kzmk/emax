/*			The structure of an emax fs directory
 *****************************************************************************/

#define		FS_WHAT		0	/*the parent dir of the dir*/
#define		FS_DIR_PARENT	4	/*the parent dir of the dir*/
/*			here will be the defense			     */
#define		FS_DIR_STRUCT	58	/*the dir structure */

#define		FS_DIR_NAME	0	/*the name of the file/dir*/
#define		FS_DIR_INODE	4	/*the name of the file/dir*/

#define		FS_DIR_NEXT	508	/*the next address of the list*/
/*PARAMS*/
#define		FS_NAME_SIZE	46	/*size of the file name*/
#define		FS_DFD_SIZE	50

