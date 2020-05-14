/*			The structure of an emax fs directory
 *****************************************************************************/

#define		FS_WHAT		0	/*the parent dir of the dir*/
#define		FS_IN_PARENT	4	/*the parent dir of the dir*/
#define		FS_OPEN_BY	8	/*the proc that opened the file (0 if closed)*/
#define		FS_FILE_SIZE	12	/*the size of the file in bytes*/
/*			here will be the defense			     */
#define		FS_IN_STRUCT	60	/*the dir structure */
#define		FS_IN_1D	500	/*1x indirect block*/
#define		FS_IN_2D	504	/*2x indirect block*/
#define		FS_IN_3D	508	/*3x indirect block*/


