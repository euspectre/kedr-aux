
delegate_base.ko:     file format elf32-i386


Disassembly of section .text:

00000000 <report_error>:
report_error():
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	e8 fc ff ff ff       	call   4 <report_error+0x4>
			4: R_386_PC32	mcount
   8:	8b 15 08 00 00 00    	mov    0x8,%edx
			a: R_386_32	.bss
   e:	85 d2                	test   %edx,%edx
  10:	75 05                	jne    17 <report_error+0x17>
  12:	a3 08 00 00 00       	mov    %eax,0x8
			13: R_386_32	.bss
  17:	5d                   	pop    %ebp
  18:	c3                   	ret    
  19:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00000020 <control_file_write>:
control_file_write():
  20:	55                   	push   %ebp
  21:	89 e5                	mov    %esp,%ebp
  23:	53                   	push   %ebx
  24:	83 ec 04             	sub    $0x4,%esp
  27:	e8 fc ff ff ff       	call   28 <control_file_write+0x8>
			28: R_386_PC32	mcount
  2c:	a1 00 00 00 00       	mov    0x0,%eax
			2d: R_386_32	current_delegate
  31:	85 c0                	test   %eax,%eax
  33:	89 cb                	mov    %ecx,%ebx
  35:	74 11                	je     48 <control_file_write+0x28>
  37:	8b 50 04             	mov    0x4(%eax),%edx
  3a:	ff 12                	call   *(%edx)
  3c:	85 c0                	test   %eax,%eax
  3e:	78 02                	js     42 <control_file_write+0x22>
  40:	89 d8                	mov    %ebx,%eax
  42:	83 c4 04             	add    $0x4,%esp
  45:	5b                   	pop    %ebx
  46:	5d                   	pop    %ebp
  47:	c3                   	ret    
  48:	c7 04 24 1e 00 00 00 	movl   $0x1e,(%esp)
			4b: R_386_32	.rodata.str1.1
  4f:	e8 fc ff ff ff       	call   50 <control_file_write+0x30>
			50: R_386_PC32	printk
  54:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
  59:	eb e7                	jmp    42 <control_file_write+0x22>
  5b:	90                   	nop
  5c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00000060 <delegate_unregister>:
delegate_unregister():
  60:	55                   	push   %ebp
  61:	89 e5                	mov    %esp,%ebp
  63:	83 ec 04             	sub    $0x4,%esp
  66:	e8 fc ff ff ff       	call   67 <delegate_unregister+0x7>
			67: R_386_PC32	mcount
  6b:	89 c2                	mov    %eax,%edx
  6d:	a1 00 00 00 00       	mov    0x0,%eax
			6e: R_386_32	current_delegate
  72:	39 d0                	cmp    %edx,%eax
  74:	75 1d                	jne    93 <delegate_unregister+0x33>
  76:	c7 05 00 00 00 00 00 	movl   $0x0,0x0
  7d:	00 00 00 
			78: R_386_32	current_delegate
  80:	e8 fc ff ff ff       	call   81 <delegate_unregister+0x21>
			81: R_386_PC32	delegate_operations_restore
  85:	b8 00 00 00 00       	mov    $0x0,%eax
			86: R_386_32	__this_module
  8a:	e8 fc ff ff ff       	call   8b <delegate_unregister+0x2b>
			8b: R_386_PC32	delegate_operations_target_unload_callback
  8f:	31 c0                	xor    %eax,%eax
  91:	c9                   	leave  
  92:	c3                   	ret    
  93:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
			96: R_386_32	.rodata.str1.4
  9a:	e8 fc ff ff ff       	call   9b <delegate_unregister+0x3b>
			9b: R_386_PC32	printk
  9f:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
  a4:	c9                   	leave  
  a5:	c3                   	ret    
  a6:	8d 76 00             	lea    0x0(%esi),%esi
  a9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000000b0 <delegate_register>:
delegate_register():
  b0:	55                   	push   %ebp
  b1:	89 e5                	mov    %esp,%ebp
  b3:	53                   	push   %ebx
  b4:	83 ec 04             	sub    $0x4,%esp
  b7:	e8 fc ff ff ff       	call   b8 <delegate_register+0x8>
			b8: R_386_PC32	mcount
  bc:	8b 0d 00 00 00 00    	mov    0x0,%ecx
			be: R_386_32	current_delegate
  c2:	85 c9                	test   %ecx,%ecx
  c4:	89 c3                	mov    %eax,%ebx
  c6:	75 1f                	jne    e7 <delegate_register+0x37>
  c8:	b8 00 00 00 00       	mov    $0x0,%eax
			c9: R_386_32	__this_module
  cd:	e8 fc ff ff ff       	call   ce <delegate_register+0x1e>
			ce: R_386_PC32	delegate_operations_target_load_callback
  d2:	89 d8                	mov    %ebx,%eax
  d4:	e8 fc ff ff ff       	call   d5 <delegate_register+0x25>
			d5: R_386_PC32	delegate_operations_replace
  d9:	31 c0                	xor    %eax,%eax
  db:	89 1d 00 00 00 00    	mov    %ebx,0x0
			dd: R_386_32	current_delegate
  e1:	83 c4 04             	add    $0x4,%esp
  e4:	5b                   	pop    %ebx
  e5:	5d                   	pop    %ebp
  e6:	c3                   	ret    
  e7:	c7 04 24 3c 00 00 00 	movl   $0x3c,(%esp)
			ea: R_386_32	.rodata.str1.4
  ee:	e8 fc ff ff ff       	call   ef <delegate_register+0x3f>
			ef: R_386_PC32	printk
  f3:	b8 f0 ff ff ff       	mov    $0xfffffff0,%eax
  f8:	eb e7                	jmp    e1 <delegate_register+0x31>
  fa:	90                   	nop
  fb:	90                   	nop

Disassembly of section .exit.text:

00000000 <cleanup_module>:
cleanup_module():
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	e8 fc ff ff ff       	call   4 <cleanup_module+0x4>
			4: R_386_PC32	mcount
   8:	a1 00 00 00 00       	mov    0x0,%eax
			9: R_386_32	control_file_name
   d:	31 d2                	xor    %edx,%edx
   f:	e8 fc ff ff ff       	call   10 <cleanup_module+0x10>
			10: R_386_PC32	remove_proc_entry
  14:	5d                   	pop    %ebp
  15:	c3                   	ret    

Disassembly of section .init.text:

00000000 <init_module>:
init_module():
   0:	55                   	push   %ebp
   1:	31 c9                	xor    %ecx,%ecx
   3:	89 e5                	mov    %esp,%ebp
   5:	83 ec 08             	sub    $0x8,%esp
   8:	a1 00 00 00 00       	mov    0x0,%eax
			9: R_386_32	control_file_name
   d:	ba 92 00 00 00       	mov    $0x92,%edx
  12:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
  19:	00 
  1a:	c7 04 24 20 00 00 00 	movl   $0x20,(%esp)
			1d: R_386_32	.data
  21:	e8 fc ff ff ff       	call   22 <init_module+0x22>
			22: R_386_PC32	proc_create_data
  26:	31 d2                	xor    %edx,%edx
  28:	85 c0                	test   %eax,%eax
  2a:	a3 00 00 00 00       	mov    %eax,0x0
			2b: R_386_32	control_file
  2f:	75 11                	jne    42 <init_module+0x42>
  31:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
			34: R_386_32	.rodata.str1.1
  38:	e8 fc ff ff ff       	call   39 <init_module+0x39>
			39: R_386_PC32	printk
  3d:	ba ea ff ff ff       	mov    $0xffffffea,%edx
  42:	89 d0                	mov    %edx,%eax
  44:	c9                   	leave  
  45:	c3                   	ret    
