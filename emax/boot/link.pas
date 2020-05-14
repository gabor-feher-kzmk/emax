program link;

uses crt,dos;

var
f,c,k : file of byte;
fil : string;
con,fs,i : longint;
b : byte;

begin
writeln('Kozmik OS Pascal linker  v0.01');
readln(fil);
 assign(f,fil);
 reset(f);
    fs := filesize(f);
     assign(c,'krnl.com');
     rewrite(c);
      for i := 32 to fs-2 do
       begin
       seek(f,i);
       read(f,b);
       seek(c,i-32);
       write(c,b);
       end;
     close(c);
 close(f);

 con := 0;
 assign(k,'kernel.img');
  rewrite(k);
     assign(f,'bootsect.com');
     reset(f);
      for i := 0 to filesize(f)-1 do
       begin
        seek(f,i);
        read(f,b);
	seek(k,con);
	write(k,b);
	con := con+1;
       end;
     close(f);
 repeat
	seek(k,con);
	write(k,b);
	con := con+1;
 until con = 510;
	seek(k,con);
	b:=85;
	write(k,b);
	con := con+1;
	seek(k,con);
	b :=170;
	write(k,b);
	con := con+1;
 assign(c,'krnl.com');
  reset(c);       
     for i := 0 to filesize(c)-1 do
      begin
       seek(c,i);
       read(c,b);
       seek(k,con);
       write(k,b);
       con := con+1;
      end; 
    b := 144;
 repeat
	seek(k,con);
	write(k,b);
	con := con+1;
 until con = 4096;
  close(c);     
 close(k);
 writeln('Kernel size : ',con,' byte'); 
 Writeln('-------------------------');
end.