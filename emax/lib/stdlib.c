/*===========================================================================*/
/*                                   LTOA                                    */
/*===========================================================================*/
static void ltoa(char *str,unsigned long lnum)
{
unsigned long i,div,num,cnum,digit,set;
if (lnum == 0) {str[0] = 48;str[1] = 0;return;}
 div = 1000000000;
 num = lnum;
 cnum = 0;
 set = 0;
    for (i=0;i<10;i++) {
	digit = (num / div);
	num = num % div;
	div = div / 10;
	    if (digit != 0) { 
		str[cnum] = 48 + digit;
		cnum++;
		set=1;
	    } else {
		if (set != 0) {str[cnum] = 48 + digit;cnum++;}
	    }
    }
    str[cnum] = 0;
}
/*===========================================================================*/
/*                                   ATOL                                    */
/*===========================================================================*/
static unsigned long atol(char *str)
{
 unsigned long i,end,ret,dig;
    end = 0;
    ret = 0;
    dig = 1;
    while (str[end] != 0) {end++;}end--;
	for (i=0;i<=end;i++) {
	    ret += (str[end-i]-48) * dig;
	    dig *= 10;
	}
 return(ret);
}



