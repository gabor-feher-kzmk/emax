#define PR_IDT(n) \
 dec_hex(mem_vl(8*n),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+2),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+4),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+6),hex);\
 printd(hex);

#define PR_GDT(n) \
 dec_hex(mem_vl(8*n+__GDT_ADDR),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+2+__GDT_ADDR),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+4+__GDT_ADDR),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+6+__GDT_ADDR),hex);\
 printd(hex);

#define PR_HEXL(l) \
 scroll_screen_up();\
 dec_hex(axh(l),hex);\
 pr_xy(0,24,7,hex);\
 dec_hex(axl(l),hex);\
 pr_xy(4,24,7,hex);

#define PR_XY_HEXL(l,x,y) \
 dec_hex(axh(l),hex);\
 pr_xy(x+0,y,7,hex);\
 dec_hex(axl(l),hex);\
 pr_xy(x+4,y,7,hex);

#define PR_DT(n,addr) \
 dec_hex(mem_vl(8*n+addr),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+2+addr),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+4+addr),hex);\
 printd(hex);\
 dec_hex(mem_vl(8*n+6+addr),hex);\
 printd(hex);
