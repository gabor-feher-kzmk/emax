/*		The linux test of the emax fs
 ****************************************************************************/
#include <string.h>
#include "rw.c"
#include "dir.h"
#include "inode.h"
#include "super.h"

static unsigned long get_block(FILE *dp);
static void mkfs(FILE *dp,unsigned long size);
static void do_command(FILE *dp,char *com[]);
static int mkdir(FILE *dp,char *dir);
static unsigned long cmp_name(FILE *dp,char *name,unsigned long node,
unsigned long add);
static unsigned long get_add(FILE *dp,char *name,unsigned long node);
static unsigned long get_node(FILE *dp,char *dir);
static int mknode(FILE *dp,char *dir);
static int cp_to_fb(FILE *dp,unsigned long dfp,unsigned long block
,unsigned long size,unsigned char *fp);
static unsigned long mkfile(FILE *dp,char *dir);
static int cp_to_disk(FILE *dp,char *dir,FILE *fp,unsigned long size);
static unsigned long find_next(FILE *dp,char *name,
unsigned long num,unsigned long block);

void main(int argc, char *argv[])
{
FILE *dp;
unsigned long i;
char ch[20];

    dp = fopen("/dev/fd0","r+");
    do_command(dp,argv);
    fclose(dp);
}

static void do_command(FILE *dp,char *com[])
{
unsigned long size;
FILE *fp;
    if (strcmp(com[1],"mkfs")==0) {
	printf("makeing filesystem on fd0 ...\0");
	mkfs(dp,2880);
	printf("done\n");
    return;}
    if (strcmp(com[1],"mkdir")==0) {
	printf("makeing dir %s ...\0",com[2]);
	if (mkdir(dp,com[2])==1) {
	printf("done\n");} else {printf("fail\n");}
    return;}
    if (strcmp(com[1],"cp")==0) {
	printf("copying file %s ...\0",com[2]);
	fp = fopen(com[2],"a");size = ftell(fp);fclose(fp);
	fp = fopen(com[2],"r+");
	if (cp_to_disk(dp,com[3],fp,size)==1) {
	printf("done\n");} else {printf("fail\n");}
	fclose(fp);
    return;}
    if (strcmp(com[1],"ls")==0) {
	list(dp,com[2]);
    return;}
}

static void mkfs(FILE *dp,unsigned long size)
{
unsigned long i,root;
    for (i=0;i<512;i++) {fd_write(dp,0,i,0xFF);} /*clear the boot block*/
    for (i=FS_FREEB;i<512;i++) {
	fd_write(dp,1,i,0);			/*free all spaces*/
    }
    fd_wlong(dp,1,FS_VER,1);
    fd_wlong(dp,1,FS_SIZE,size);
    fd_wlong(dp,1,FS_NSUPER,0);
    root=get_block(dp);
    fd_wlong(dp,1,FS_ROOT,root);
    for (i=0;i<512;i++) {
	fd_write(dp,root,i,0);			/*free root dir*/
    }
}

static int mkdir(FILE *dp,char *dir)
{
unsigned long dnode;
	dnode = mknode(dp,dir);
	if (dnode == 0) {return(0);}
	fd_wlong(dp,dnode,FS_WHAT,1);
    return(1);
}

static int cp_to_disk(FILE *dp,char *dir,FILE *fp,unsigned long size)
{
unsigned long file,mc,fc,end,ret;
unsigned char buff[513];
 mc = 0;
 file = mkfile(dp,dir);if (file == 0) {return(0);}
 while ((mc+1)*512 < size) {
	for (fc=0;fc<512;fc++) {
	    fseek(fp,fc+(mc*512),0);
	    buff[fc] = fgetc(fp);
	}
	ret = cp_to_fb(dp,file,mc,512,buff);if (ret == 0) {return(0);}
 mc++;
 } end = size - (mc*512);
  	for (fc=0;fc<end;fc++) {
	    fseek(fp,fc+(mc*512),0);
	    buff[fc] = fgetc(fp);
	}
	ret = cp_to_fb(dp,file,mc,end,buff);if (ret == 0) {return(0);}
    fd_wlong(dp,file,FS_FILE_SIZE,size);
    return(1);
}

static int cp_to_fb(FILE *dp,unsigned long dfp,unsigned long block
,unsigned long size,unsigned char *fp)
{
unsigned long bmap,datab,b;
    bmap = FS_IN_STRUCT+(block*4);
    if (bmap > 499) {return(0);}
	if (fd_rlong(dp,dfp,bmap) == 0) {
	    datab = get_block(dp);
	    fd_wlong(dp,dfp,bmap,datab);
	} else {
	    datab = fd_rlong(dp,dfp,bmap);
	}    
    if (size > 512) {return(0);}
	for (b=0;b<size;b++) {
	fd_write(dp,datab,b,fp[b]);
	}
    return(1);
}

static unsigned long mkfile(FILE *dp,char *dir)
{
unsigned long dnode;
	dnode = mknode(dp,dir);
	if (dnode == 0) {return(0);}
	fd_wlong(dp,dnode,FS_WHAT,2);
    return(dnode);
}

static unsigned long get_block(FILE *dp)
{
unsigned long b,rol;
unsigned char free;
    b=FS_FREEB;
    while (fd_read(dp,1,b) == 0xFF)
    { b++;
	if (b > 511) {printf("no free space on root device\n");}
    }
    free = fd_read(dp,1,b);
	rol = 0;
	while ( ((free >> rol)&1) == 1)
	{rol++;}
    free = free + (1 << rol);
    fd_write(dp,1,b,free);
return(rol+((b-FS_FREEB)*8)+2);
}

static unsigned long get_node(FILE *dp,char *dir)
{
static char dira[250];
char ch,*dnm,*dch;
unsigned long add;
    strcpy(dira,dir);dch=dira;
 if (dir[0]=='/') {
    add = fd_rlong(dp,1,FS_ROOT);
    if (dir[1] == 0) {return(add);}
    while (strchr(dch,'/')!=0) {
	dch=strchr(dch,'/')+1;
	dnm = strchr(dch,'/');
	if (dnm != 0) {ch=dnm[0];dnm[0]=0;}
	add=get_add(dp,dch,add);
	    if (add==0) {return(0);}
	if (dnm != 0) {dnm[0] = ch;}
    }		 
    return(add);
 } else { return(0);}
}

static unsigned long get_add(FILE *dp,char *name,unsigned long node)
{
unsigned long i;
    for (i=0;i<10;i++)
    {
	if (cmp_name(dp,name,node,FS_DIR_STRUCT+4+(i*50))==0) {
	    return(fd_rlong(dp,node,FS_DIR_STRUCT+(i*50)));
	}
    }
    return(0);
}

static unsigned long cmp_name(FILE *dp,char *name,unsigned long node,
unsigned long add)
{
 unsigned long i;
 char fname[50]; 
 i=0;   
    while (fd_read(dp,node,add+i)!=0)
    {
     fname[i] = fd_read(dp,node,add+i);
     i++;
     if (i > 46) {return(0);}
    }fname[i] = 0;
return(strcmp(name,fname));
}

static int mknode(FILE *dp,char *dir)
{
unsigned long dir_add,i,d,td,dirnode;
char *path,*tp;
char name[50];
     if (get_node(dp,dir)!=0) {return(0);}
     path = dir;
     dir = strrchr(dir,'/');dir++;
     strcpy(name,dir);
     if (strrchr(path,'/') == strchr(path,'/'))	{
        path = strrchr(path,'/');path[1]=0;
	dir_add = fd_rlong(dp,1,FS_ROOT);
     } else {
        tp = strrchr(path,'/');tp[0]=0;
        dir_add = get_node(dp,path);
     }
    if (dir_add==0) {return(0);}
 d=0;
    for (i=FS_DIR_STRUCT;i<500;i+=50)
    {
	if (fd_rlong(dp,dir_add,i)==0) {
	d=i;
	}
    }
    if (d==0) {return(0);}
    dirnode = get_block(dp);
    fd_wlong(dp,dir_add,d,dirnode);d+=4;
 i=0;
 while (name[i]!=0)
 {
    fd_write(dp,dir_add,d,name[i]);	/*writeing dir name in the current*/
    i++;d++;				/*dirnode*/
    if (i > 46) {return(0);}
 }td = d;
    for (i=td;i<50;i++) {
    fd_write(dp,dir_add,d,0);
    d++;
    }
/* creating a new dirnode*/
	fd_wlong(dp,dirnode,FS_WHAT,0);
	fd_wlong(dp,dirnode,FS_DIR_PARENT,dir_add);
        for (i=8;i<512;i++) {
	    fd_write(dp,dirnode,i,0);	/*new node*/
	}
    return(dirnode);
}

static unsigned long find_next(FILE *dp,char *name,
unsigned long num,unsigned long block)
{
unsigned long i,c,add;
char ch;
    for (i=num;i<10;i++)
    {
	if (fd_rlong(dp,block,FS_DIR_STRUCT+(i*50))!=0) {
    	 add = FS_DIR_STRUCT+4+(i*50);
	     c = 0;
	     ch = fd_read(dp,block,add+c);
	    while (ch!=0)
             {
	     name[c] = ch;
	     c++;if (c > 49) { break;} 
	     ch = fd_read(dp,block,add+c);
	     }name[c] = 0;
	    return(i);
	}
    }    
    return(0);
}
static unsigned long list(FILE *dp,char *dir)
{
char ch[52];
unsigned long p,node;
node = get_node(dp,dir);
    if (node == 0) {return(0);}
	p = find_next(dp,ch,0,node);
     while (p != 0) {	
	printf(" %s \n",ch);
	p++;
	p = find_next(dp,ch,p,node);
     }
}



