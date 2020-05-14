/*===========================================================================*/
/*                                    POW                                    */
/*===========================================================================*/
static unsigned long pow(unsigned long x,unsigned long y)
{
 unsigned long i,ret;
    if (y == 1) {return(x);}
    if (y == 0) {return(1);}
    
    ret = x;
    for (i=1;i<y;i++) {
	ret *= x;
    }
 return(ret);
}
