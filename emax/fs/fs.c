/* *   		fs.c includes the file management for the kernel	  *
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
#include "../include/fs/cache.h"
#include "../include/fs/super.h"
#include "../include/fs/inode.h"
#include "../include/fs/dir.h"
#include "../include/pis.h"

#define SCT	SYNC_TIME*1000

static void writeln(char * ch);
static void readln(char * ch);
static void ltoa(char *str,unsigned long lnum);
static unsigned long get_mem(unsigned long size);

static inline char * strrchr(const char * s, int c);
static inline char * strcpy(char * dest,const char *src);
static inline char * strncpy(char * dest,const char *src,unsigned long count);
static inline int strcmp(const char * cs,const char * ct);
static inline char * strchr(const char * s, int c);
static char *strnset(char * s,int ch,unsigned long n);
static inline char * strcat(char * dest,const char * src);
static inline char * strncat(char * dest,const char * src,unsigned long count);

static void write_all();
static unsigned char fd_read(unsigned long block,unsigned long byte);
static unsigned long fd_rlong(unsigned long block,unsigned long byte);
static void fd_write(unsigned long block,unsigned long byte,unsigned char data);
static void fd_wlong(unsigned long block,unsigned long byte,unsigned long data);
static unsigned long mem_load(unsigned long offset);
static void mem_str(unsigned long offset,unsigned long data);

static void init_fs();
static int mount();
static unsigned long cmp_name(char *name,unsigned long node,unsigned long add);
static unsigned long mknode(char *dir);
static unsigned long get_add(char *name,unsigned long node);
static unsigned long get_node(char *dir);
static unsigned long get_block();
static int mkdir(char *dir);
static unsigned long find_next(char *name,unsigned long num,unsigned long block);
static unsigned long open_r(unsigned long proc,char *name);
static unsigned long get_nblock(unsigned long inode,unsigned long pos);
static int fs_open_dev(unsigned long master,unsigned long slave);
static int fs_close_dev(unsigned long master,unsigned long slave);

static unsigned long do_find_next(unsigned long proc,char *name,unsigned long num);
static int do_mkdir(unsigned long proc);
static int do_close(unsigned long proc,unsigned long fp);
static unsigned long do_open(unsigned long proc,unsigned long ac);
static unsigned long do_read(unsigned long proc,unsigned long fp,unsigned long pos);
static int do_fsize(unsigned long proc,unsigned long fp);
static unsigned long do_get_dev(unsigned long proc,unsigned long fp);

#include "fs.h"
ENTRY(fs);
static char nm[52];
static unsigned long circ;
static unsigned long free[CACHE_SIZE+2];
static unsigned char *cache;
static struct cache_type c_prop[CACHE_SIZE+2];
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
static void who_w()
{
 extern unsigned long p_head[200];
 unsigned long *get,back,to,proc,c;
 unsigned long rc[99];
    init_fs();
    while(TRUE) {
    get = &rc[1]; back = 0;
    PUSH(get);PUSHN(24);PUSHN(0x405);CALL(105);ADDESP(12);
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
    mem_str(p_head[14]+VT_ID,mem_load(p_head[rc[1]]+VT_ID)); 	
	switch (rc[3]) {
	 case 1: back=1;write_all();				break;
	 case 2: back=do_find_next(rc[1],nm,rc[4]);		break;
	 case 3: back=do_mkdir(rc[1]);				break;
	 case 4: back=do_open(rc[1],rc[4]);			break;
	default:
	switch (rc[3]) {
	 case 5: back=do_close(rc[1],rc[4]);			break;
	 case 6: back=do_read(rc[1],rc[4],rc[5]);		break;
	 case 7: back=do_fsize(rc[1],rc[4]);			break;
	 case 8: back=do_get_dev(rc[1],rc[4]);			break;
	}
	}
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
    to = (rc[1]*0x100)+4;
    PUSH(back);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
    }/*while forever*/
}
/*===========================================================================*/
/*                                 INIT_FS                                   */
/*===========================================================================*/
static void init_fs()
{
extern unsigned long mem_max;
unsigned long i,ver;
unsigned char tmp[260];
    for (i=0;i<(CACHE_SIZE+2);i++) {
    free[i] = 0;
    c_prop[i].accesed = 0;
    c_prop[i].modify = 0;
    }
    circ = 0;
    i = get_mem(512*(CACHE_SIZE+2));
    cache = (unsigned char *) i;
/* This routine is not really the component of the file system it's only 
 * for to tell the user to change the foppy in the drive */ver=0;
    while (ver == 0) {
	writeln("Please put the 'ROOT' disk into the drive and press ENTER\0");
	readln(tmp);
	writeln("Mounting root filesystem from /dev/fd0...\0");
	ver = mount();
	    if (ver != 0) {
		writeln(" mounted emax fs version \0");
		if (ver < 10) {writeln("0.\0");}
		ltoa(tmp,ver);
		writeln(tmp);writeln("\n\0");
	    } else {
		writeln("ERROR: not a vailed fs\n\0");
		 write_all();
		 for (i=0;i<CACHE_SIZE;i++) {free[i]=0;c_prop[i].accesed=0;c_prop[i].modify=0;}
	    }
    }
    /* Make a sync and write all changed block's in cache to the disk 
     * in every 30 seconds */
    PUSHN(30000);PUSHN(3);PUSHESP;PUSHN(2);PUSHN(0x403);CALL(105);ADDESP(20);
    PUSHN(3);PUSHN(38);PUSHN(5);PUSHESP;PUSHN(3);PUSHN(0x403);
    CALL(105);ADDESP(24);/*send sig to init*/
}
/*===========================================================================*/
/*                                  MOUNT                                    */
/*===========================================================================*/
static int mount()
{
unsigned long version;
	version = fd_rlong(1,FS_VER);
	if (version != 1) {return(0);}
    return(version);
}
/*===========================================================================*/
/*                                  MKDIR                                    */
/*===========================================================================*/
static int mkdir(char *dir)
{
unsigned long dnode;
	dnode = mknode(dir);
	if (dnode == 0) {return(0);}
	fd_wlong(dnode,FS_WHAT,1);
    return(1);
}
/*===========================================================================*/
/*                                  MKFILE                                   */
/*===========================================================================*/
static unsigned long mkfile(char *dir)
{
unsigned long dnode;
	dnode = mknode(dir);
	if (dnode == 0) {return(0);}
	fd_wlong(dnode,FS_WHAT,2);
    return(dnode);
}
/*===========================================================================*/
/*                                DO_FIND_NEXT                               */
/*===========================================================================*/
static unsigned long do_find_next(unsigned long proc,char *name,unsigned long num)
{
static char *xp;
unsigned long node,ret;
static char dir[260];
    xp = &dir[0];
    PUSH(proc);PUSH(xp);PUSHN(65);PUSHN(0x109);CALL(105);ADDESP(16);
    PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
    node = get_node(dir);
    if (node == 0) {return(0);}
    ret = find_next(name,num,node);
    PUSH(proc);PUSH(name);PUSHN(13);PUSHN(0x209);CALL(105);ADDESP(16);
    PUSH(proc);PUSHN(12);CALL(105);ADDESP(8);//con dma
    return(ret);
}
/*===========================================================================*/
/*                                   DO_MKDIR                                */
/*===========================================================================*/
static int do_mkdir(unsigned long proc)
{
static char *xp;
unsigned long ret;
static char dir[260];
    xp = &dir[0];
    PUSH(proc);PUSH(xp);PUSHN(65);PUSHN(0x109);CALL(105);ADDESP(16);
    PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
    ret = mkdir(dir);
    return(ret);
}
/*===========================================================================*/
/*                                 DO_CLOSE                                  */
/*===========================================================================*/
static int do_close(unsigned long proc,unsigned long fp)
{
 int r;
    if (fp == 0) {return(0);}
    if (fd_rlong(fp,FS_OPEN_BY) != proc) {return(0);}
	//if (fd_rlong(fp,FS_WHAT) == FS_DEV_FILE) {
	// r = fs_close_dev(fd_rlong(fp,FS_DEV_MASTER),fd_rlong(fp,FS_DEV_SLAVE));
	// if (r != 1) {return(r);}
	//} else { 
	 if (fd_rlong(fp,FS_WHAT) != 2) {return(0);}
	// }
    fd_wlong(fp,FS_OPEN_BY,0);
    return(1);
}
/*===========================================================================*/
/*                                 DO_OPEN                                   */
/*===========================================================================*/
static unsigned long do_open(unsigned long proc,unsigned long ac)
{
static char *xp;
unsigned long ret;
static char dir[260];
    xp = &dir[0];
    PUSH(proc);PUSH(xp);PUSHN(65);PUSHN(0x109);CALL(105);ADDESP(16);
    PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
    switch (ac) {
    case 1:	ret=open_r(proc,dir); break;
    default : return(0);
    }
    return(ret);
}
/*===========================================================================*/
/*                                  DO_READ                                  */
/*===========================================================================*/
static unsigned long do_read(unsigned long proc,unsigned long fp,unsigned long pos)
{
unsigned char tmp[260];
 unsigned long block;
 unsigned long byte;
    if (fp == 0) {return(0);}
    if (fd_rlong(fp,FS_WHAT) != 2) {return(0);}
    if (fd_rlong(fp,FS_OPEN_BY) != proc) {return(0);}
    
    block = get_nblock(fp,pos);
    byte = pos % 512;
    return(fd_read(block,byte));
}
/*===========================================================================*/
/*                                 DO_FSIZE                                  */
/*===========================================================================*/
static int do_fsize(unsigned long proc,unsigned long fp)
{
    if (fp == 0) {return(0);}
    if (fd_rlong(fp,FS_WHAT) != 2) {return(0);}
    if (fd_rlong(fp,FS_OPEN_BY) != proc) {return(0);}
    return(fd_rlong(fp,FS_FILE_SIZE));
}
/*===========================================================================*/
/*                                 OPEN_R                                    */
/*===========================================================================*/
static unsigned long open_r(unsigned long proc,char *name)
{
 unsigned long inode;
 int r;
    inode = get_node(name);
    if (inode == 0) {return(0);}
    if (fd_rlong(inode,FS_OPEN_BY) != 0) {return(0);}
	//if (fd_rlong(inode,FS_WHAT) == FS_DEV_FILE) {
	// r = fs_open_dev(fd_rlong(inode,FS_DEV_MASTER),fd_rlong(inode,FS_DEV_SLAVE));
	// if (r != 1) {return(r);}
	//}
    fd_wlong(inode,FS_OPEN_BY,proc);
    return(inode);
}
/*===========================================================================*/
/*                                FS_OPEN_DEV                                */
/*===========================================================================*/
static int fs_open_dev(unsigned long master,unsigned long slave)
{
int ret;
    PUSH(slave);PUSH(master);PUSHN(6);PUSHESP;PUSHN(3);
    PUSHN(0x603);CALL(105);ADDESP(24);
    RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                FS_CLOSE_DEV                                */
/*===========================================================================*/
static int fs_close_dev(unsigned long master,unsigned long slave)
{
int ret;
    PUSH(slave);PUSH(master);PUSHN(7);PUSHESP;PUSHN(3);
    PUSHN(0x603);CALL(105);ADDESP(24);
    RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                 DO_GET_DEV                                */
/*===========================================================================*/
static unsigned long do_get_dev(unsigned long proc,unsigned long fp)
{
unsigned long master,slave,ret;
    if (fp == 0) {return(0);}
    if (fd_rlong(fp,FS_WHAT) != FS_DEV_FILE) {return(0);}
    if (fd_rlong(fp,FS_OPEN_BY) != proc) {return(0);}
    master = fd_rlong(fp,FS_DEV_MASTER);
    slave = fd_rlong(fp,FS_DEV_SLAVE);
    ret = (master << 16) + slave;
return(ret);
}
/*===========================================================================*/
/*                                GET_NBLOCK                                 */
/*===========================================================================*/
static unsigned long get_nblock(unsigned long inode,unsigned long pos)
{
 unsigned long ret,abs_b;

    abs_b = pos / 512;
    if (abs_b < 136) {
	ret = fd_rlong(inode,FS_IN_STRUCT+(abs_b*4));	
	return(ret);
    } else {writeln("File too big\n\0");}         
}
/*===========================================================================*/
/*                                GET_BLOCK                                  */
/*===========================================================================*/
static unsigned long get_block()
{
unsigned long b,rol;
unsigned char free;
    b=FS_FREEB;
    while (fd_read(1,b) == 0xFF)
    { b++;
	if (b > 511) {writeln("no free space on root device\0");}
    }
    free = fd_read(1,b);
	rol = 0;
	while ( ((free >> rol)&1) == 1)
	{rol++;}
    free = free + (1 << rol);
    fd_write(1,b,free);
return(rol+((b-FS_FREEB)*8)+2);
}
/*===========================================================================*/
/*                               GET_ABS_NODE                                */
/*===========================================================================*/
static unsigned long get_abs_node(char *dir)
{
static char dira[250];
char ch,*dnm,*dch;
unsigned long add;
    strcpy(dira,dir);dch=dira;
 if (dir[0]=='/') {
     add = fd_rlong(1,FS_ROOT);
    if (dir[1] == 0) {return(add);}
    while (strchr(dch,'/')!=0) {
	dch=strchr(dch,'/')+1;
	dnm = strchr(dch,'/');
	if (dnm != 0) {ch=dnm[0];dnm[0]=0;}
	add=get_add(dch,add);
	    if (add==0) {return(0);}
	if (dnm != 0) {dnm[0] = ch;}
    }		 
    return(add);
 } else { return(0);}
}
/*===========================================================================*/
/*                               GET_REL_NODE                                */
/*===========================================================================*/
static unsigned long get_rel_node(char *dir)
{
char *pp,*adir;
char *pt = ":/bin/\0";
static char ntmp[260];
unsigned long ret;
pp = &pt[0];
//    writeln("the PATH : '\0");writeln(pp);writeln("'\n\0");
    while (strchr(pp,':') != 0) {
	pp = strchr(pp,':')+1;
	strnset(ntmp,0,260);
	if (strchr(pp,':') != 0) {   
	    strncpy(ntmp,pp,strchr(pp,':')-pp);
	} else {
	    strcpy(ntmp,pp);
	}
	adir = strcat(ntmp,dir);
	ret = get_abs_node(adir);
	if (ret != 0) {return(ret);}
    }
 return(0);
}
/*===========================================================================*/
/*                                GET_NODE                                   */
/*===========================================================================*/
static unsigned long get_node(char *dir)
{
    if (dir[0] == '/') {
	return(get_abs_node(dir));
    } else {
	return(get_rel_node(dir));
    }
}
/*===========================================================================*/
/*                                CMP_NAME                                   */
/*===========================================================================*/
static unsigned long cmp_name(char *name,unsigned long node,unsigned long add)
{
 unsigned long i;
 char fname[50]; 
 i=0;   
    while (fd_read(node,add+i)!=0)
    {
     fname[i] = fd_read(node,add+i);
     i++;
     if (i > 46) {return(0);}
    }fname[i] = 0;
return(strcmp(name,fname));
}
/*===========================================================================*/
/*                                  GET_ADD                                  */
/*===========================================================================*/
static unsigned long get_add(char *name,unsigned long node)
{
unsigned long i;
    for (i=0;i<10;i++)
    {
	if (cmp_name(name,node,FS_DIR_STRUCT+4+(i*50))==0) {
	    return(fd_rlong(node,FS_DIR_STRUCT+(i*50)));
	}
    }
    return(0);
}
/*===========================================================================*/
/*                                  MKNODE                                   */
/*===========================================================================*/
static unsigned long mknode(char *dir)
{
unsigned long dir_add,i,d,td,dirnode;
char *path,*tp;
char name[50];
     if (get_node(dir)!=0) {return(0);}
     path = dir;
     dir = strrchr(dir,'/');dir++;
     strcpy(name,dir);
     if (strrchr(path,'/') == strchr(path,'/'))	{
        path = strrchr(path,'/');path[1]=0;
	dir_add = fd_rlong(1,FS_ROOT);
     } else {
        tp = strrchr(path,'/');tp[0]=0;
        dir_add = get_node(path);
     }
    if (dir_add==0) {return(0);}
 d=0;
    for (i=FS_DIR_STRUCT;i<500;i+=50)
    {
	if (fd_rlong(dir_add,i)==0) {
	d=i;
	}
    }
    if (d==0) {return(0);}
    dirnode = get_block();
    fd_wlong(dir_add,d,dirnode);d+=4;
 i=0;
 while (name[i]!=0)
 {
    fd_write(dir_add,d,name[i]);	/*writeing dir name in the current*/
    i++;d++;				/*dirnode*/
    if (i > 42) {name[i]=0;}
 }td = d;
    for (i=td;i<50;i++) {
    fd_write(dir_add,d,0);
    d++;
    }
/* creating a new dirnode*/
	fd_wlong(dirnode,FS_WHAT,0);
	fd_wlong(dirnode,FS_DIR_PARENT,dir_add);
        for (i=8;i<512;i++) {
	    fd_write(dirnode,i,0);	/*new node*/
	}
    return(dirnode);
}
/*===========================================================================*/
/*                                 FIND_NEXT                                 */
/*===========================================================================*/
static unsigned long find_next(char *name,unsigned long num,unsigned long block)
{
unsigned long i,c,add;
char ch;
    for (i=num;i<10;i++)
    {
	if (fd_rlong(block,FS_DIR_STRUCT+(i*50))!=0) {
    	 add = FS_DIR_STRUCT+4+(i*50);
	     c = 0;
	     ch = fd_read(block,add+c);
	    while (ch!=0)
             {
	     name[c] = ch;
	     c++;if (c > 49) { break;} 
	     ch = fd_read(block,add+c);
	     }name[c] = 0;
	    return(i);
	}
    }    
    return(0);
}
#include "../include/string.h"
#include "../lib/stdlib.c"
#include "../lib/emax.c"
#include "rw.c"
/*===========================================================================*/
/*                                 WRITELN                                   */
/*===========================================================================*/
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
/*                                 READLN                                    */
/*===========================================================================*/
static void readln(char * ch)
{
unsigned long buff[267];
unsigned long i,*cp;
cp=&buff[1];
PUSHN(6);PUSH(cp);PUSHN(255);PUSHN(0x108);CALL(105);ADDESP(16);
PUSHN(0);PUSHN(0);PUSHN(3);PUSHESP;PUSHN(3);PUSHN(0x603);CALL(105);ADDESP(24);
    i=1;    
    while (buff[i] != 0) {
    ch[i-1] = buff[i];
    i++;
    if (i == 255) {buff[i] = 0;}
    }ch[i-1] = 0;ch[i] = 0;
}
/*===========================================================================*/
/*                                   SYNC                                    */
/*===========================================================================*/
ENTRY(fs_sig16);
static void sync()
{
unsigned long sct;
 while (TRUE) {
    write_all();
    sct = SCT;
    PUSH(sct);PUSHN(3);PUSHESP;PUSHN(2);PUSHN(0x403);CALL(105);ADDESP(20);
    PUSHN(0x10A);CALL(105);ADDESP(4);/* end of signal*/
 }
}

ENTRY(fs_end);
