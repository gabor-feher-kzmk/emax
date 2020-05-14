	.file	"cat.c"
	.version	"01.01"
gcc2_compiled.:
.text
.globl main
	.type	 main,@function
main:
	pushl %ebp
	movl %esp,%ebp
	subl $4,%esp
	movl 12(%ebp),%edx
	incl %edx
	movl %edx,-4(%ebp)
	movl -4(%ebp),%eax
	pushl %eax
	call cat
	addl $4,%esp
#APP
	pushl $3
	pushl	%esp
	pushl $1
	pushl $0x1303
	ljmp $105 ,$0
	addl $20,%esp 
#NO_APP
.L1:
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe1:
	.size	 main,.Lfe1-main
	.type	 writeln,@function
writeln:
	pushl %ebp
	movl %esp,%ebp
	subl $1032,%esp
	pushl %ebx
	leal -1020(%ebp),%eax
	leal 4(%eax),%ebx
	movl %ebx,-1028(%ebp)
	movl $1,-1024(%ebp)
.L3:
	movl -1024(%ebp),%eax
	addl 8(%ebp),%eax
	leal -1(%eax),%edx
	cmpb $0,(%edx)
	jne .L5
	jmp .L4
.L5:
	movl -1024(%ebp),%eax
	movl -1024(%ebp),%edx
	addl 8(%ebp),%edx
	leal -1(%edx),%ecx
	movsbl (%ecx),%ebx
	movl %ebx,-1020(%ebp,%eax,4)
	incl -1024(%ebp)
	jmp .L3
.L4:
	movl -1024(%ebp),%eax
	movl $0,-1020(%ebp,%eax,4)
	movl -1024(%ebp),%ebx
	addl $3,%ebx
	movl %ebx,-1032(%ebp)
#APP
	pushl $6
#NO_APP
	movl -1028(%ebp),%ebx
#APP
	pushl %ebx		
#NO_APP
	movl -1032(%ebp),%ebx
#APP
	pushl %ebx		
	pushl $0x08
	ljmp $105 ,$0
	addl $16,%esp 
	pushl $0x4
	pushl $0x2
	pushl	%esp
	pushl $2
	pushl $0x603
	ljmp $105 ,$0
	addl $20,%esp 
#NO_APP
.L2:
	movl -1036(%ebp),%ebx
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe2:
	.size	 writeln,.Lfe2-writeln
.section	.rodata
.LC0:
	.string	"Error opening file : "
	.string	""
.LC1:
	.string	"\r"
	.string	""
.text
	.type	 cat,@function
cat:
	pushl %ebp
	movl %esp,%ebp
	subl $24,%esp
	pushl %ebx
	pushl $1
	movl 8(%ebp),%eax
	pushl %eax
	call fopen
	addl $8,%esp
	movl %eax,%eax
	movl %eax,-4(%ebp)
	cmpl $0,-4(%ebp)
	jne .L7
	pushl $.LC0
	call writeln
	addl $4,%esp
	movl 8(%ebp),%eax
	pushl %eax
	call writeln
	addl $4,%esp
	pushl $.LC1
	call writeln
	addl $4,%esp
	jmp .L8
.L7:
	movl -4(%ebp),%eax
	pushl %eax
	call fsize
	addl $4,%esp
	movl %eax,%eax
	movl %eax,-16(%ebp)
	movl -16(%ebp),%ecx
	movl %ecx,%eax
	movl $200,%ebx
	xorl %edx,%edx
	divl %ebx
	movl %eax,-20(%ebp)
	movl $0,-12(%ebp)
.L9:
	movl -12(%ebp),%eax
	cmpl %eax,-20(%ebp)
	ja .L12
	jmp .L10
.L12:
	movl $0,-8(%ebp)
.L13:
	cmpl $199,-8(%ebp)
	jbe .L16
	jmp .L14
.L16:
	movl -12(%ebp),%edx
	movl %edx,%eax
	sall $2,%eax
	addl %edx,%eax
	leal 0(,%eax,4),%edx
	addl %edx,%eax
	leal 0(,%eax,8),%edx
	movl %edx,%eax
	addl -8(%ebp),%eax
	pushl %eax
	movl -4(%ebp),%eax
	pushl %eax
	call fread
	addl $8,%esp
	movb %al,%al
	movl -8(%ebp),%edx
	addl $fs_buff,%edx
	movb %al,(%edx)
.L15:
	incl -8(%ebp)
	jmp .L13
.L14:
	movb $0,fs_buff+200
	pushl $fs_buff
	call writeln
	addl $4,%esp
.L11:
	incl -12(%ebp)
	jmp .L9
.L10:
	movl -16(%ebp),%ecx
	movl %ecx,%eax
	movl $200,%ebx
	xorl %edx,%edx
	divl %ebx
	movl %edx,-12(%ebp)
	movl $0,-8(%ebp)
.L17:
	movl -8(%ebp),%eax
	cmpl %eax,-12(%ebp)
	ja .L20
	jmp .L18
.L20:
	movl -20(%ebp),%edx
	movl %edx,%eax
	sall $2,%eax
	addl %edx,%eax
	leal 0(,%eax,4),%edx
	addl %edx,%eax
	leal 0(,%eax,8),%edx
	movl %edx,%eax
	addl -8(%ebp),%eax
	pushl %eax
	movl -4(%ebp),%eax
	pushl %eax
	call fread
	addl $8,%esp
	movb %al,%al
	movl -8(%ebp),%edx
	addl $fs_buff,%edx
	movb %al,(%edx)
.L19:
	incl -8(%ebp)
	jmp .L17
.L18:
	movl -12(%ebp),%eax
	addl $fs_buff,%eax
	movb $0,(%eax)
	pushl $fs_buff
	call writeln
	addl $4,%esp
	pushl $.LC1
	call writeln
	addl $4,%esp
.L8:
	movl -4(%ebp),%eax
	pushl %eax
	call fclose
	addl $4,%esp
	movl -24(%ebp),%eax
	jmp .L6
.L6:
	movl -28(%ebp),%ebx
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe3:
	.size	 cat,.Lfe3-cat
	.type	 fopen,@function
fopen:
	pushl %ebp
	movl %esp,%ebp
	subl $8,%esp
#APP
	pushl $14
#NO_APP
	movl 8(%ebp),%edx
#APP
	pushl %edx		
	pushl $15
	pushl $0x09
	ljmp $105 ,$0
	addl $16,%esp 
#NO_APP
	movl 12(%ebp),%edx
#APP
	pushl %edx		
	pushl $4
	pushl	%esp
	pushl $2
	pushl $0xE03
	ljmp $105 ,$0
	addl $20,%esp 
	pushl	%eax
	popl %edx		
#NO_APP
	movl %edx,-4(%ebp)
	movl -4(%ebp),%eax
	jmp .L21
.L21:
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe4:
	.size	 fopen,.Lfe4-fopen
	.type	 fclose,@function
fclose:
	pushl %ebp
	movl %esp,%ebp
	subl $8,%esp
	movl 8(%ebp),%edx
#APP
	pushl %edx		
	pushl $5
	pushl	%esp
	pushl $2
	pushl $0xE03
	ljmp $105 ,$0
	addl $20,%esp 
	pushl	%eax
	popl %edx		
#NO_APP
	movl %edx,-4(%ebp)
	movl -4(%ebp),%eax
	jmp .L22
.L22:
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe5:
	.size	 fclose,.Lfe5-fclose
	.type	 fread,@function
fread:
	pushl %ebp
	movl %esp,%ebp
	subl $4,%esp
	movl 12(%ebp),%ecx
#APP
	pushl %ecx		
#NO_APP
	movl 8(%ebp),%ecx
#APP
	pushl %ecx		
	pushl $6
	pushl	%esp
	pushl $3
	pushl $0xE03
	ljmp $105 ,$0
	addl $24,%esp 
	pushl	%eax
	popl %ecx		
#NO_APP
	movl %ecx,-4(%ebp)
	movzbl -4(%ebp),%edx
	movl %edx,%eax
	jmp .L23
.L23:
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe6:
	.size	 fread,.Lfe6-fread
	.type	 fsize,@function
fsize:
	pushl %ebp
	movl %esp,%ebp
	subl $4,%esp
	movl 8(%ebp),%edx
#APP
	pushl %edx		
	pushl $7
	pushl	%esp
	pushl $2
	pushl $0xE03
	ljmp $105 ,$0
	addl $20,%esp 
	pushl	%eax
	popl %edx		
#NO_APP
	movl %edx,-4(%ebp)
	movl -4(%ebp),%eax
	jmp .L24
.L24:
	movl %ebp,%esp
	popl %ebp
	ret
.Lfe7:
	.size	 fsize,.Lfe7-fsize
	.local	fs_buff
	.comm	fs_buff,263,1
	.ident	"GCC: (GNU) 2.7.2.3"
