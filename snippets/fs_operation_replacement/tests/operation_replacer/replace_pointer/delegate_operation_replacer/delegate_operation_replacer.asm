
delegate_operation_replacer.ko:     file format elf32-i386


Disassembly of section .text:

00000000 <delegate_operations_get_orig_special_f>:
delegate_operations_get_orig_special_f():
       0:	55                   	push   %ebp
       1:	89 e5                	mov    %esp,%ebp
       3:	53                   	push   %ebx
       4:	83 ec 04             	sub    $0x4,%esp
       7:	e8 fc ff ff ff       	call   8 <delegate_operations_get_orig_special_f+0x8>
			8: R_386_PC32	mcount
       c:	89 cb                	mov    %ecx,%ebx
       e:	8d 4a 04             	lea    0x4(%edx),%ecx
      11:	89 c2                	mov    %eax,%edx
      13:	a1 00 00 00 00       	mov    0x0,%eax
			14: R_386_32	.bss
      18:	89 1c 24             	mov    %ebx,(%esp)
      1b:	e8 fc ff ff ff       	call   1c <delegate_operations_get_orig_special_f+0x1c>
			1c: R_386_PC32	operation_get_orig_special
      20:	83 c4 04             	add    $0x4,%esp
      23:	5b                   	pop    %ebx
      24:	5d                   	pop    %ebp
      25:	c3                   	ret    
      26:	8d 76 00             	lea    0x0(%esi),%esi
      29:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000030 <delegate_operations_get_orig_f>:
delegate_operations_get_orig_f():
      30:	55                   	push   %ebp
      31:	89 e5                	mov    %esp,%ebp
      33:	e8 fc ff ff ff       	call   34 <delegate_operations_get_orig_f+0x4>
			34: R_386_PC32	mcount
      38:	8d 4a 04             	lea    0x4(%edx),%ecx
      3b:	89 c2                	mov    %eax,%edx
      3d:	a1 00 00 00 00       	mov    0x0,%eax
			3e: R_386_32	.bss
      42:	e8 fc ff ff ff       	call   43 <delegate_operations_get_orig_f+0x13>
			43: R_386_PC32	operation_get_orig
      47:	5d                   	pop    %ebp
      48:	c3                   	ret    
      49:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00000050 <delegate_operations_restore>:
delegate_operations_restore():
      50:	55                   	push   %ebp
      51:	89 e5                	mov    %esp,%ebp
      53:	e8 fc ff ff ff       	call   54 <delegate_operations_restore+0x4>
			54: R_386_PC32	mcount
      58:	8d 50 04             	lea    0x4(%eax),%edx
      5b:	a1 00 00 00 00       	mov    0x0,%eax
			5c: R_386_32	.bss
      60:	e8 fc ff ff ff       	call   61 <delegate_operations_restore+0x11>
			61: R_386_PC32	operation_restore
      65:	5d                   	pop    %ebp
      66:	c3                   	ret    
      67:	89 f6                	mov    %esi,%esi
      69:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000070 <delegate_operations_replace>:
delegate_operations_replace():
      70:	55                   	push   %ebp
      71:	89 e5                	mov    %esp,%ebp
      73:	e8 fc ff ff ff       	call   74 <delegate_operations_replace+0x4>
			74: R_386_PC32	mcount
      78:	8d 50 04             	lea    0x4(%eax),%edx
      7b:	a1 00 00 00 00       	mov    0x0,%eax
			7c: R_386_32	.bss
      80:	e8 fc ff ff ff       	call   81 <delegate_operations_replace+0x11>
			81: R_386_PC32	operation_replace
      85:	5d                   	pop    %ebp
      86:	c3                   	ret    
      87:	89 f6                	mov    %esi,%esi
      89:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000090 <delegate_operations_target_unload_callback>:
delegate_operations_target_unload_callback():
      90:	55                   	push   %ebp
      91:	89 e5                	mov    %esp,%ebp
      93:	e8 fc ff ff ff       	call   94 <delegate_operations_target_unload_callback+0x4>
			94: R_386_PC32	mcount
      98:	31 c9                	xor    %ecx,%ecx
      9a:	89 c2                	mov    %eax,%edx
      9c:	a1 00 00 00 00       	mov    0x0,%eax
			9d: R_386_32	.bss
      a1:	e8 fc ff ff ff       	call   a2 <delegate_operations_target_unload_callback+0x12>
			a2: R_386_PC32	operation_target_unload_callback
      a6:	5d                   	pop    %ebp
      a7:	c3                   	ret    
      a8:	90                   	nop
      a9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

000000b0 <delegate_operations_target_load_callback>:
delegate_operations_target_load_callback():
      b0:	55                   	push   %ebp
      b1:	89 e5                	mov    %esp,%ebp
      b3:	e8 fc ff ff ff       	call   b4 <delegate_operations_target_load_callback+0x4>
			b4: R_386_PC32	mcount
      b8:	89 c2                	mov    %eax,%edx
      ba:	a1 00 00 00 00       	mov    0x0,%eax
			bb: R_386_32	.bss
      bf:	e8 fc ff ff ff       	call   c0 <delegate_operations_target_load_callback+0x10>
			c0: R_386_PC32	operation_target_load_callback
      c4:	5d                   	pop    %ebp
      c5:	c3                   	ret    
      c6:	8d 76 00             	lea    0x0(%esi),%esi
      c9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000000d0 <delegate_operations_payload_unregister_special>:
delegate_operations_payload_unregister_special():
      d0:	55                   	push   %ebp
      d1:	89 e5                	mov    %esp,%ebp
      d3:	e8 fc ff ff ff       	call   d4 <delegate_operations_payload_unregister_special+0x4>
			d4: R_386_PC32	mcount
      d8:	89 c2                	mov    %eax,%edx
      da:	a1 00 00 00 00       	mov    0x0,%eax
			db: R_386_32	.bss
      df:	e8 fc ff ff ff       	call   e0 <delegate_operations_payload_unregister_special+0x10>
			e0: R_386_PC32	operation_payload_unregister_special
      e4:	5d                   	pop    %ebp
      e5:	c3                   	ret    
      e6:	8d 76 00             	lea    0x0(%esi),%esi
      e9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000000f0 <delegate_operations_payload_register_special>:
delegate_operations_payload_register_special():
      f0:	55                   	push   %ebp
      f1:	89 e5                	mov    %esp,%ebp
      f3:	e8 fc ff ff ff       	call   f4 <delegate_operations_payload_register_special+0x4>
			f4: R_386_PC32	mcount
      f8:	89 c2                	mov    %eax,%edx
      fa:	a1 00 00 00 00       	mov    0x0,%eax
			fb: R_386_32	.bss
      ff:	e8 fc ff ff ff       	call   100 <delegate_operations_payload_register_special+0x10>
			100: R_386_PC32	operation_payload_register_special
     104:	5d                   	pop    %ebp
     105:	c3                   	ret    
     106:	8d 76 00             	lea    0x0(%esi),%esi
     109:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000110 <delegate_operations_payload_unregister>:
delegate_operations_payload_unregister():
     110:	55                   	push   %ebp
     111:	89 e5                	mov    %esp,%ebp
     113:	e8 fc ff ff ff       	call   114 <delegate_operations_payload_unregister+0x4>
			114: R_386_PC32	mcount
     118:	89 c2                	mov    %eax,%edx
     11a:	a1 00 00 00 00       	mov    0x0,%eax
			11b: R_386_32	.bss
     11f:	e8 fc ff ff ff       	call   120 <delegate_operations_payload_unregister+0x10>
			120: R_386_PC32	operation_payload_unregister
     124:	5d                   	pop    %ebp
     125:	c3                   	ret    
     126:	8d 76 00             	lea    0x0(%esi),%esi
     129:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000130 <delegate_operations_payload_register>:
delegate_operations_payload_register():
     130:	55                   	push   %ebp
     131:	89 e5                	mov    %esp,%ebp
     133:	e8 fc ff ff ff       	call   134 <delegate_operations_payload_register+0x4>
			134: R_386_PC32	mcount
     138:	89 c2                	mov    %eax,%edx
     13a:	a1 00 00 00 00       	mov    0x0,%eax
			13b: R_386_32	.bss
     13f:	e8 fc ff ff ff       	call   140 <delegate_operations_payload_register+0x10>
			140: R_386_PC32	operation_payload_register
     144:	5d                   	pop    %ebp
     145:	c3                   	ret    
     146:	90                   	nop
     147:	90                   	nop
     148:	90                   	nop
     149:	90                   	nop
     14a:	90                   	nop
     14b:	90                   	nop
     14c:	90                   	nop
     14d:	90                   	nop
     14e:	90                   	nop
     14f:	90                   	nop

00000150 <fix_payload>:
fix_payload():
     150:	55                   	push   %ebp
     151:	89 e5                	mov    %esp,%ebp
     153:	83 ec 18             	sub    $0x18,%esp
     156:	89 5d f4             	mov    %ebx,-0xc(%ebp)
     159:	89 75 f8             	mov    %esi,-0x8(%ebp)
     15c:	89 7d fc             	mov    %edi,-0x4(%ebp)
     15f:	e8 fc ff ff ff       	call   160 <fix_payload+0x10>
			160: R_386_PC32	mcount
     164:	89 d3                	mov    %edx,%ebx
     166:	8b 52 10             	mov    0x10(%edx),%edx
     169:	85 d2                	test   %edx,%edx
     16b:	75 52                	jne    1bf <fix_payload+0x6f>
     16d:	8b 43 14             	mov    0x14(%ebx),%eax
     170:	8b 30                	mov    (%eax),%esi
     172:	85 f6                	test   %esi,%esi
     174:	74 33                	je     1a9 <fix_payload+0x59>
     176:	64 8b 15 00 00 00 00 	mov    %fs:0x0,%edx
			179: R_386_32	per_cpu__cpu_number
     17d:	83 3e 02             	cmpl   $0x2,(%esi)
     180:	74 70                	je     1f2 <fix_payload+0xa2>
     182:	8b 86 5c 01 00 00    	mov    0x15c(%esi),%eax
     188:	03 04 95 00 00 00 00 	add    0x0(,%edx,4),%eax
			18b: R_386_32	__per_cpu_offset
     18f:	ff 00                	incl   (%eax)
     191:	8b 86 5c 01 00 00    	mov    0x15c(%esi),%eax
     197:	03 04 95 00 00 00 00 	add    0x0(,%edx,4),%eax
			19a: R_386_32	__per_cpu_offset
     19e:	8b 38                	mov    (%eax),%edi
     1a0:	a1 04 00 00 00       	mov    0x4,%eax
			1a1: R_386_32	__tracepoint_module_get
     1a5:	85 c0                	test   %eax,%eax
     1a7:	75 1a                	jne    1c3 <fix_payload+0x73>
     1a9:	c7 43 10 01 00 00 00 	movl   $0x1,0x10(%ebx)
     1b0:	31 c0                	xor    %eax,%eax
     1b2:	8b 5d f4             	mov    -0xc(%ebp),%ebx
     1b5:	8b 75 f8             	mov    -0x8(%ebp),%esi
     1b8:	8b 7d fc             	mov    -0x4(%ebp),%edi
     1bb:	89 ec                	mov    %ebp,%esp
     1bd:	5d                   	pop    %ebp
     1be:	c3                   	ret    
     1bf:	0f 0b                	ud2a   
     1c1:	eb fe                	jmp    1c1 <fix_payload+0x71>
     1c3:	a1 10 00 00 00       	mov    0x10,%eax
			1c4: R_386_32	__tracepoint_module_get
     1c8:	85 c0                	test   %eax,%eax
     1ca:	89 45 f0             	mov    %eax,-0x10(%ebp)
     1cd:	74 da                	je     1a9 <fix_payload+0x59>
     1cf:	8b 00                	mov    (%eax),%eax
     1d1:	89 45 ec             	mov    %eax,-0x14(%ebp)
     1d4:	89 f0                	mov    %esi,%eax
     1d6:	89 f9                	mov    %edi,%ecx
     1d8:	ba 82 01 00 00       	mov    $0x182,%edx
			1d9: R_386_32	.text
     1dd:	ff 55 ec             	call   *-0x14(%ebp)
     1e0:	83 45 f0 04          	addl   $0x4,-0x10(%ebp)
     1e4:	8b 45 f0             	mov    -0x10(%ebp),%eax
     1e7:	8b 00                	mov    (%eax),%eax
     1e9:	85 c0                	test   %eax,%eax
     1eb:	89 45 ec             	mov    %eax,-0x14(%ebp)
     1ee:	75 e4                	jne    1d4 <fix_payload+0x84>
     1f0:	eb b7                	jmp    1a9 <fix_payload+0x59>
     1f2:	c7 04 24 bc 00 00 00 	movl   $0xbc,(%esp)
			1f5: R_386_32	.rodata.str1.4
     1f9:	e8 fc ff ff ff       	call   1fa <fix_payload+0xaa>
			1fa: R_386_PC32	printk
     1fe:	b8 f0 ff ff ff       	mov    $0xfffffff0,%eax
     203:	eb ad                	jmp    1b2 <fix_payload+0x62>
     205:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     209:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000210 <operation_restore_at_place>:
operation_restore_at_place():
     210:	55                   	push   %ebp
     211:	89 e5                	mov    %esp,%ebp
     213:	57                   	push   %edi
     214:	56                   	push   %esi
     215:	53                   	push   %ebx
     216:	83 ec 18             	sub    $0x18,%esp
     219:	e8 fc ff ff ff       	call   21a <operation_restore_at_place+0xa>
			21a: R_386_PC32	mcount
     21e:	8b 5d 08             	mov    0x8(%ebp),%ebx
     221:	8b 75 0c             	mov    0xc(%ebp),%esi
     224:	89 45 f0             	mov    %eax,-0x10(%ebp)
     227:	89 4d ec             	mov    %ecx,-0x14(%ebp)
     22a:	01 de                	add    %ebx,%esi
     22c:	39 de                	cmp    %ebx,%esi
     22e:	76 18                	jbe    248 <operation_restore_at_place+0x38>
     230:	8b 03                	mov    (%ebx),%eax
     232:	85 c0                	test   %eax,%eax
     234:	74 08                	je     23e <operation_restore_at_place+0x2e>
     236:	eb 1a                	jmp    252 <operation_restore_at_place+0x42>
     238:	8b 3b                	mov    (%ebx),%edi
     23a:	85 ff                	test   %edi,%edi
     23c:	75 14                	jne    252 <operation_restore_at_place+0x42>
     23e:	83 c3 04             	add    $0x4,%ebx
     241:	39 f3                	cmp    %esi,%ebx
     243:	72 f3                	jb     238 <operation_restore_at_place+0x28>
     245:	8d 76 00             	lea    0x0(%esi),%esi
     248:	83 c4 18             	add    $0x18,%esp
     24b:	31 c0                	xor    %eax,%eax
     24d:	5b                   	pop    %ebx
     24e:	5e                   	pop    %esi
     24f:	5f                   	pop    %edi
     250:	5d                   	pop    %ebp
     251:	c3                   	ret    
     252:	85 db                	test   %ebx,%ebx
     254:	74 f2                	je     248 <operation_restore_at_place+0x38>
     256:	8b 7d f0             	mov    -0x10(%ebp),%edi
     259:	89 75 e4             	mov    %esi,-0x1c(%ebp)
     25c:	83 c7 04             	add    $0x4,%edi
     25f:	90                   	nop
     260:	8b 4d f0             	mov    -0x10(%ebp),%ecx
     263:	89 d8                	mov    %ebx,%eax
     265:	2b 45 08             	sub    0x8(%ebp),%eax
     268:	8b 75 ec             	mov    -0x14(%ebp),%esi
     26b:	8b 09                	mov    (%ecx),%ecx
     26d:	89 ca                	mov    %ecx,%edx
     26f:	01 c2                	add    %eax,%edx
     271:	89 4d e8             	mov    %ecx,-0x18(%ebp)
     274:	8b 0a                	mov    (%edx),%ecx
     276:	3b 0c 06             	cmp    (%esi,%eax,1),%ecx
     279:	75 3f                	jne    2ba <operation_restore_at_place+0xaa>
     27b:	8b 04 07             	mov    (%edi,%eax,1),%eax
     27e:	89 02                	mov    %eax,(%edx)
     280:	83 c3 04             	add    $0x4,%ebx
     283:	3b 5d e4             	cmp    -0x1c(%ebp),%ebx
     286:	73 c0                	jae    248 <operation_restore_at_place+0x38>
     288:	8b 33                	mov    (%ebx),%esi
     28a:	85 f6                	test   %esi,%esi
     28c:	75 1d                	jne    2ab <operation_restore_at_place+0x9b>
     28e:	8b 75 e4             	mov    -0x1c(%ebp),%esi
     291:	eb 0b                	jmp    29e <operation_restore_at_place+0x8e>
     293:	90                   	nop
     294:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     298:	8b 0b                	mov    (%ebx),%ecx
     29a:	85 c9                	test   %ecx,%ecx
     29c:	75 0a                	jne    2a8 <operation_restore_at_place+0x98>
     29e:	83 c3 04             	add    $0x4,%ebx
     2a1:	39 f3                	cmp    %esi,%ebx
     2a3:	72 f3                	jb     298 <operation_restore_at_place+0x88>
     2a5:	eb a1                	jmp    248 <operation_restore_at_place+0x38>
     2a7:	90                   	nop
     2a8:	89 75 e4             	mov    %esi,-0x1c(%ebp)
     2ab:	85 db                	test   %ebx,%ebx
     2ad:	8d 76 00             	lea    0x0(%esi),%esi
     2b0:	75 ae                	jne    260 <operation_restore_at_place+0x50>
     2b2:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     2b8:	eb 8e                	jmp    248 <operation_restore_at_place+0x38>
     2ba:	2b 55 e8             	sub    -0x18(%ebp),%edx
     2bd:	c7 04 24 04 01 00 00 	movl   $0x104,(%esp)
			2c0: R_386_32	.rodata.str1.4
     2c4:	89 54 24 04          	mov    %edx,0x4(%esp)
     2c8:	e8 fc ff ff ff       	call   2c9 <operation_restore_at_place+0xb9>
			2c9: R_386_PC32	printk
     2cd:	eb b1                	jmp    280 <operation_restore_at_place+0x70>
     2cf:	90                   	nop

000002d0 <free_data>:
free_data():
     2d0:	55                   	push   %ebp
     2d1:	89 e5                	mov    %esp,%ebp
     2d3:	83 ec 08             	sub    $0x8,%esp
     2d6:	89 1c 24             	mov    %ebx,(%esp)
     2d9:	89 74 24 04          	mov    %esi,0x4(%esp)
     2dd:	e8 fc ff ff ff       	call   2de <free_data+0xe>
			2de: R_386_PC32	mcount
     2e2:	89 ce                	mov    %ecx,%esi
     2e4:	89 d3                	mov    %edx,%ebx
     2e6:	e8 fc ff ff ff       	call   2e7 <free_data+0x17>
			2e7: R_386_PC32	kfree
     2eb:	85 f6                	test   %esi,%esi
     2ed:	74 0a                	je     2f9 <free_data+0x29>
     2ef:	8b 16                	mov    (%esi),%edx
     2f1:	85 d2                	test   %edx,%edx
     2f3:	74 04                	je     2f9 <free_data+0x29>
     2f5:	89 d8                	mov    %ebx,%eax
     2f7:	ff d2                	call   *%edx
     2f9:	8b 1c 24             	mov    (%esp),%ebx
     2fc:	8b 74 24 04          	mov    0x4(%esp),%esi
     300:	89 ec                	mov    %ebp,%esp
     302:	5d                   	pop    %ebp
     303:	c3                   	ret    
     304:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     30a:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

00000310 <unregister_payloads_at_delete>:
unregister_payloads_at_delete():
     310:	55                   	push   %ebp
     311:	89 e5                	mov    %esp,%ebp
     313:	83 ec 18             	sub    $0x18,%esp
     316:	89 5d f8             	mov    %ebx,-0x8(%ebp)
     319:	89 75 fc             	mov    %esi,-0x4(%ebp)
     31c:	e8 fc ff ff ff       	call   31d <unregister_payloads_at_delete+0xd>
			31d: R_386_PC32	mcount
     321:	8b 1a                	mov    (%edx),%ebx
     323:	39 da                	cmp    %ebx,%edx
     325:	75 0a                	jne    331 <unregister_payloads_at_delete+0x21>
     327:	8b 5d f8             	mov    -0x8(%ebp),%ebx
     32a:	8b 75 fc             	mov    -0x4(%ebp),%esi
     32d:	89 ec                	mov    %ebp,%esp
     32f:	5d                   	pop    %ebp
     330:	c3                   	ret    
     331:	8b 43 14             	mov    0x14(%ebx),%eax
     334:	b9 00 00 00 00       	mov    $0x0,%ecx
			335: R_386_32	.rodata.str1.1
     339:	8b 30                	mov    (%eax),%esi
     33b:	85 f6                	test   %esi,%esi
     33d:	74 03                	je     342 <unregister_payloads_at_delete+0x32>
     33f:	8d 4e 0c             	lea    0xc(%esi),%ecx
     342:	89 55 f4             	mov    %edx,-0xc(%ebp)
     345:	89 4c 24 08          	mov    %ecx,0x8(%esp)
     349:	89 44 24 04          	mov    %eax,0x4(%esp)
     34d:	c7 04 24 40 01 00 00 	movl   $0x140,(%esp)
			350: R_386_32	.rodata.str1.4
     354:	e8 fc ff ff ff       	call   355 <unregister_payloads_at_delete+0x45>
			355: R_386_PC32	printk
     359:	8b 43 04             	mov    0x4(%ebx),%eax
     35c:	8b 0b                	mov    (%ebx),%ecx
     35e:	89 41 04             	mov    %eax,0x4(%ecx)
     361:	89 08                	mov    %ecx,(%eax)
     363:	89 d8                	mov    %ebx,%eax
     365:	c7 03 00 01 10 00    	movl   $0x100100,(%ebx)
     36b:	c7 43 04 00 02 20 00 	movl   $0x200200,0x4(%ebx)
     372:	e8 fc ff ff ff       	call   373 <unregister_payloads_at_delete+0x63>
			373: R_386_PC32	kfree
     377:	8b 55 f4             	mov    -0xc(%ebp),%edx
     37a:	8b 1a                	mov    (%edx),%ebx
     37c:	39 da                	cmp    %ebx,%edx
     37e:	74 a7                	je     327 <unregister_payloads_at_delete+0x17>
     380:	eb af                	jmp    331 <unregister_payloads_at_delete+0x21>
     382:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     389:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000390 <operation_payload_unregister_special>:
operation_payload_unregister_special():
     390:	55                   	push   %ebp
     391:	89 e5                	mov    %esp,%ebp
     393:	57                   	push   %edi
     394:	56                   	push   %esi
     395:	53                   	push   %ebx
     396:	83 ec 08             	sub    $0x8,%esp
     399:	e8 fc ff ff ff       	call   39a <operation_payload_unregister_special+0xa>
			39a: R_386_PC32	mcount
     39e:	bf fc ff ff ff       	mov    $0xfffffffc,%edi
     3a3:	89 c3                	mov    %eax,%ebx
     3a5:	89 d6                	mov    %edx,%esi
     3a7:	8d 40 18             	lea    0x18(%eax),%eax
     3aa:	89 45 f0             	mov    %eax,-0x10(%ebp)
     3ad:	e8 fc ff ff ff       	call   3ae <operation_payload_unregister_special+0x1e>
			3ae: R_386_PC32	mutex_lock_killable
     3b2:	85 c0                	test   %eax,%eax
     3b4:	75 4e                	jne    404 <operation_payload_unregister_special+0x74>
     3b6:	8b 43 0c             	mov    0xc(%ebx),%eax
     3b9:	8b 10                	mov    (%eax),%edx
     3bb:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     3bf:	83 c3 0c             	add    $0xc,%ebx
     3c2:	39 d8                	cmp    %ebx,%eax
     3c4:	75 0e                	jne    3d4 <operation_payload_unregister_special+0x44>
     3c6:	eb 46                	jmp    40e <operation_payload_unregister_special+0x7e>
     3c8:	89 d0                	mov    %edx,%eax
     3ca:	8b 12                	mov    (%edx),%edx
     3cc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     3d0:	39 d8                	cmp    %ebx,%eax
     3d2:	74 3a                	je     40e <operation_payload_unregister_special+0x7e>
     3d4:	39 70 14             	cmp    %esi,0x14(%eax)
     3d7:	75 ef                	jne    3c8 <operation_payload_unregister_special+0x38>
     3d9:	8b 48 10             	mov    0x10(%eax),%ecx
     3dc:	85 c9                	test   %ecx,%ecx
     3de:	75 41                	jne    421 <operation_payload_unregister_special+0x91>
     3e0:	8b 48 04             	mov    0x4(%eax),%ecx
     3e3:	31 ff                	xor    %edi,%edi
     3e5:	89 4a 04             	mov    %ecx,0x4(%edx)
     3e8:	89 11                	mov    %edx,(%ecx)
     3ea:	c7 00 00 01 10 00    	movl   $0x100100,(%eax)
     3f0:	c7 40 04 00 02 20 00 	movl   $0x200200,0x4(%eax)
     3f7:	e8 fc ff ff ff       	call   3f8 <operation_payload_unregister_special+0x68>
			3f8: R_386_PC32	kfree
     3fc:	8b 45 f0             	mov    -0x10(%ebp),%eax
     3ff:	e8 fc ff ff ff       	call   400 <operation_payload_unregister_special+0x70>
			400: R_386_PC32	mutex_unlock
     404:	83 c4 08             	add    $0x8,%esp
     407:	89 f8                	mov    %edi,%eax
     409:	5b                   	pop    %ebx
     40a:	5e                   	pop    %esi
     40b:	5f                   	pop    %edi
     40c:	5d                   	pop    %ebp
     40d:	c3                   	ret    
     40e:	c7 04 24 fc 01 00 00 	movl   $0x1fc,(%esp)
			411: R_386_32	.rodata.str1.4
     415:	bf ea ff ff ff       	mov    $0xffffffea,%edi
     41a:	e8 fc ff ff ff       	call   41b <operation_payload_unregister_special+0x8b>
			41b: R_386_PC32	printk
     41f:	eb db                	jmp    3fc <operation_payload_unregister_special+0x6c>
     421:	c7 04 24 a4 01 00 00 	movl   $0x1a4,(%esp)
			424: R_386_32	.rodata.str1.4
     428:	bf f0 ff ff ff       	mov    $0xfffffff0,%edi
     42d:	e8 fc ff ff ff       	call   42e <operation_payload_unregister_special+0x9e>
			42e: R_386_PC32	printk
     432:	eb c8                	jmp    3fc <operation_payload_unregister_special+0x6c>
     434:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     43a:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

00000440 <operation_payload_unregister>:
operation_payload_unregister():
     440:	55                   	push   %ebp
     441:	89 e5                	mov    %esp,%ebp
     443:	57                   	push   %edi
     444:	56                   	push   %esi
     445:	53                   	push   %ebx
     446:	83 ec 08             	sub    $0x8,%esp
     449:	e8 fc ff ff ff       	call   44a <operation_payload_unregister+0xa>
			44a: R_386_PC32	mcount
     44e:	bf fc ff ff ff       	mov    $0xfffffffc,%edi
     453:	89 c3                	mov    %eax,%ebx
     455:	89 d6                	mov    %edx,%esi
     457:	8d 40 18             	lea    0x18(%eax),%eax
     45a:	89 45 f0             	mov    %eax,-0x10(%ebp)
     45d:	e8 fc ff ff ff       	call   45e <operation_payload_unregister+0x1e>
			45e: R_386_PC32	mutex_lock_killable
     462:	85 c0                	test   %eax,%eax
     464:	0f 85 85 00 00 00    	jne    4ef <operation_payload_unregister+0xaf>
     46a:	8b 43 04             	mov    0x4(%ebx),%eax
     46d:	8b 10                	mov    (%eax),%edx
     46f:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     473:	8d 4b 04             	lea    0x4(%ebx),%ecx
     476:	39 c8                	cmp    %ecx,%eax
     478:	75 16                	jne    490 <operation_payload_unregister+0x50>
     47a:	e9 b8 00 00 00       	jmp    537 <operation_payload_unregister+0xf7>
     47f:	90                   	nop
     480:	89 d0                	mov    %edx,%eax
     482:	8b 12                	mov    (%edx),%edx
     484:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     488:	39 c8                	cmp    %ecx,%eax
     48a:	0f 84 a7 00 00 00    	je     537 <operation_payload_unregister+0xf7>
     490:	39 70 14             	cmp    %esi,0x14(%eax)
     493:	75 eb                	jne    480 <operation_payload_unregister+0x40>
     495:	8b 48 10             	mov    0x10(%eax),%ecx
     498:	8b 76 08             	mov    0x8(%esi),%esi
     49b:	85 c9                	test   %ecx,%ecx
     49d:	0f 85 ab 00 00 00    	jne    54e <operation_payload_unregister+0x10e>
     4a3:	8b 48 04             	mov    0x4(%eax),%ecx
     4a6:	89 4a 04             	mov    %ecx,0x4(%edx)
     4a9:	89 11                	mov    %edx,(%ecx)
     4ab:	89 f2                	mov    %esi,%edx
     4ad:	c7 00 00 01 10 00    	movl   $0x100100,(%eax)
     4b3:	c7 40 04 00 02 20 00 	movl   $0x200200,0x4(%eax)
     4ba:	8b 4b 34             	mov    0x34(%ebx),%ecx
     4bd:	01 f1                	add    %esi,%ecx
     4bf:	39 f1                	cmp    %esi,%ecx
     4c1:	76 1d                	jbe    4e0 <operation_payload_unregister+0xa0>
     4c3:	8b 3e                	mov    (%esi),%edi
     4c5:	85 ff                	test   %edi,%edi
     4c7:	74 0d                	je     4d6 <operation_payload_unregister+0x96>
     4c9:	eb 35                	jmp    500 <operation_payload_unregister+0xc0>
     4cb:	90                   	nop
     4cc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     4d0:	8b 3a                	mov    (%edx),%edi
     4d2:	85 ff                	test   %edi,%edi
     4d4:	75 2a                	jne    500 <operation_payload_unregister+0xc0>
     4d6:	83 c2 04             	add    $0x4,%edx
     4d9:	39 ca                	cmp    %ecx,%edx
     4db:	72 f3                	jb     4d0 <operation_payload_unregister+0x90>
     4dd:	8d 76 00             	lea    0x0(%esi),%esi
     4e0:	e8 fc ff ff ff       	call   4e1 <operation_payload_unregister+0xa1>
			4e1: R_386_PC32	kfree
     4e5:	31 ff                	xor    %edi,%edi
     4e7:	8b 45 f0             	mov    -0x10(%ebp),%eax
     4ea:	e8 fc ff ff ff       	call   4eb <operation_payload_unregister+0xab>
			4eb: R_386_PC32	mutex_unlock
     4ef:	83 c4 08             	add    $0x8,%esp
     4f2:	89 f8                	mov    %edi,%eax
     4f4:	5b                   	pop    %ebx
     4f5:	5e                   	pop    %esi
     4f6:	5f                   	pop    %edi
     4f7:	5d                   	pop    %ebp
     4f8:	c3                   	ret    
     4f9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     500:	85 d2                	test   %edx,%edx
     502:	74 dc                	je     4e0 <operation_payload_unregister+0xa0>
     504:	89 d1                	mov    %edx,%ecx
     506:	29 f1                	sub    %esi,%ecx
     508:	03 4b 14             	add    0x14(%ebx),%ecx
     50b:	8b 39                	mov    (%ecx),%edi
     50d:	85 ff                	test   %edi,%edi
     50f:	74 39                	je     54a <operation_payload_unregister+0x10a>
     511:	c7 01 00 00 00 00    	movl   $0x0,(%ecx)
     517:	8b 4b 34             	mov    0x34(%ebx),%ecx
     51a:	83 c2 04             	add    $0x4,%edx
     51d:	01 f1                	add    %esi,%ecx
     51f:	39 d1                	cmp    %edx,%ecx
     521:	76 bd                	jbe    4e0 <operation_payload_unregister+0xa0>
     523:	90                   	nop
     524:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     528:	8b 3a                	mov    (%edx),%edi
     52a:	85 ff                	test   %edi,%edi
     52c:	75 d2                	jne    500 <operation_payload_unregister+0xc0>
     52e:	83 c2 04             	add    $0x4,%edx
     531:	39 ca                	cmp    %ecx,%edx
     533:	72 f3                	jb     528 <operation_payload_unregister+0xe8>
     535:	eb a9                	jmp    4e0 <operation_payload_unregister+0xa0>
     537:	c7 04 24 b0 02 00 00 	movl   $0x2b0,(%esp)
			53a: R_386_32	.rodata.str1.4
     53e:	bf ea ff ff ff       	mov    $0xffffffea,%edi
     543:	e8 fc ff ff ff       	call   544 <operation_payload_unregister+0x104>
			544: R_386_PC32	printk
     548:	eb 9d                	jmp    4e7 <operation_payload_unregister+0xa7>
     54a:	0f 0b                	ud2a   
     54c:	eb fe                	jmp    54c <operation_payload_unregister+0x10c>
     54e:	c7 04 24 60 02 00 00 	movl   $0x260,(%esp)
			551: R_386_32	.rodata.str1.4
     555:	bf f0 ff ff ff       	mov    $0xfffffff0,%edi
     55a:	e8 fc ff ff ff       	call   55b <operation_payload_unregister+0x11b>
			55b: R_386_PC32	printk
     55f:	eb 86                	jmp    4e7 <operation_payload_unregister+0xa7>
     561:	eb 0d                	jmp    570 <operation_replace_replace_pointer>
     563:	90                   	nop
     564:	90                   	nop
     565:	90                   	nop
     566:	90                   	nop
     567:	90                   	nop
     568:	90                   	nop
     569:	90                   	nop
     56a:	90                   	nop
     56b:	90                   	nop
     56c:	90                   	nop
     56d:	90                   	nop
     56e:	90                   	nop
     56f:	90                   	nop

00000570 <operation_replace_replace_pointer>:
operation_replace_replace_pointer():
     570:	55                   	push   %ebp
     571:	89 e5                	mov    %esp,%ebp
     573:	57                   	push   %edi
     574:	56                   	push   %esi
     575:	53                   	push   %ebx
     576:	83 ec 18             	sub    $0x18,%esp
     579:	e8 fc ff ff ff       	call   57a <operation_replace_replace_pointer+0xa>
			57a: R_386_PC32	mcount
     57e:	89 55 ec             	mov    %edx,-0x14(%ebp)
     581:	89 cb                	mov    %ecx,%ebx
     583:	89 45 f0             	mov    %eax,-0x10(%ebp)
     586:	8b 32                	mov    (%edx),%esi
     588:	89 c2                	mov    %eax,%edx
     58a:	83 c2 04             	add    $0x4,%edx
     58d:	89 d7                	mov    %edx,%edi
     58f:	89 30                	mov    %esi,(%eax)
     591:	8b 4d 0c             	mov    0xc(%ebp),%ecx
     594:	c1 e9 02             	shr    $0x2,%ecx
     597:	f3 a5                	rep movsl %ds:(%esi),%es:(%edi)
     599:	8b 4d 0c             	mov    0xc(%ebp),%ecx
     59c:	83 e1 03             	and    $0x3,%ecx
     59f:	74 02                	je     5a3 <operation_replace_replace_pointer+0x33>
     5a1:	f3 a4                	rep movsb %ds:(%esi),%es:(%edi)
     5a3:	c7 04 24 0c 03 00 00 	movl   $0x30c,(%esp)
			5a6: R_386_32	.rodata.str1.4
     5aa:	89 55 e8             	mov    %edx,-0x18(%ebp)
     5ad:	e8 fc ff ff ff       	call   5ae <operation_replace_replace_pointer+0x3e>
			5ae: R_386_PC32	printk
     5b2:	8b 45 ec             	mov    -0x14(%ebp),%eax
     5b5:	89 44 24 08          	mov    %eax,0x8(%esp)
     5b9:	8b 4d f0             	mov    -0x10(%ebp),%ecx
     5bc:	c7 04 24 07 00 00 00 	movl   $0x7,(%esp)
			5bf: R_386_32	.rodata.str1.1
     5c3:	89 4c 24 04          	mov    %ecx,0x4(%esp)
     5c7:	e8 fc ff ff ff       	call   5c8 <operation_replace_replace_pointer+0x58>
			5c8: R_386_PC32	printk
     5cc:	8b 75 08             	mov    0x8(%ebp),%esi
     5cf:	89 5c 24 04          	mov    %ebx,0x4(%esp)
     5d3:	89 74 24 08          	mov    %esi,0x8(%esp)
     5d7:	c7 04 24 1d 00 00 00 	movl   $0x1d,(%esp)
			5da: R_386_32	.rodata.str1.1
     5de:	e8 fc ff ff ff       	call   5df <operation_replace_replace_pointer+0x6f>
			5df: R_386_PC32	printk
     5e3:	8b 55 e8             	mov    -0x18(%ebp),%edx
     5e6:	89 54 24 08          	mov    %edx,0x8(%esp)
     5ea:	8b 4d f0             	mov    -0x10(%ebp),%ecx
     5ed:	8b 01                	mov    (%ecx),%eax
     5ef:	c7 04 24 34 03 00 00 	movl   $0x334,(%esp)
			5f2: R_386_32	.rodata.str1.4
     5f6:	89 44 24 04          	mov    %eax,0x4(%esp)
     5fa:	e8 fc ff ff ff       	call   5fb <operation_replace_replace_pointer+0x8b>
			5fb: R_386_PC32	printk
     5ff:	8b 4d 0c             	mov    0xc(%ebp),%ecx
     602:	89 f0                	mov    %esi,%eax
     604:	8b 55 e8             	mov    -0x18(%ebp),%edx
     607:	01 f1                	add    %esi,%ecx
     609:	39 f1                	cmp    %esi,%ecx
     60b:	76 18                	jbe    625 <operation_replace_replace_pointer+0xb5>
     60d:	8b 36                	mov    (%esi),%esi
     60f:	85 f6                	test   %esi,%esi
     611:	74 0b                	je     61e <operation_replace_replace_pointer+0xae>
     613:	eb 23                	jmp    638 <operation_replace_replace_pointer+0xc8>
     615:	8d 76 00             	lea    0x0(%esi),%esi
     618:	8b 38                	mov    (%eax),%edi
     61a:	85 ff                	test   %edi,%edi
     61c:	75 1a                	jne    638 <operation_replace_replace_pointer+0xc8>
     61e:	83 c0 04             	add    $0x4,%eax
     621:	39 c8                	cmp    %ecx,%eax
     623:	72 f3                	jb     618 <operation_replace_replace_pointer+0xa8>
     625:	8b 75 ec             	mov    -0x14(%ebp),%esi
     628:	31 c0                	xor    %eax,%eax
     62a:	89 16                	mov    %edx,(%esi)
     62c:	83 c4 18             	add    $0x18,%esp
     62f:	5b                   	pop    %ebx
     630:	5e                   	pop    %esi
     631:	5f                   	pop    %edi
     632:	5d                   	pop    %ebp
     633:	c3                   	ret    
     634:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     638:	85 c0                	test   %eax,%eax
     63a:	74 e9                	je     625 <operation_replace_replace_pointer+0xb5>
     63c:	89 c6                	mov    %eax,%esi
     63e:	2b 75 08             	sub    0x8(%ebp),%esi
     641:	8b 3c 33             	mov    (%ebx,%esi,1),%edi
     644:	89 3c 32             	mov    %edi,(%edx,%esi,1)
     647:	eb 0d                	jmp    656 <operation_replace_replace_pointer+0xe6>
     649:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     650:	8b 30                	mov    (%eax),%esi
     652:	85 f6                	test   %esi,%esi
     654:	75 e2                	jne    638 <operation_replace_replace_pointer+0xc8>
     656:	83 c0 04             	add    $0x4,%eax
     659:	39 c8                	cmp    %ecx,%eax
     65b:	72 f3                	jb     650 <operation_replace_replace_pointer+0xe0>
     65d:	8b 75 ec             	mov    -0x14(%ebp),%esi
     660:	31 c0                	xor    %eax,%eax
     662:	89 16                	mov    %edx,(%esi)
     664:	83 c4 18             	add    $0x18,%esp
     667:	5b                   	pop    %ebx
     668:	5e                   	pop    %esi
     669:	5f                   	pop    %edi
     66a:	5d                   	pop    %ebp
     66b:	c3                   	ret    
     66c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00000670 <operation_replace_at_place>:
operation_replace_at_place():
     670:	55                   	push   %ebp
     671:	89 e5                	mov    %esp,%ebp
     673:	57                   	push   %edi
     674:	56                   	push   %esi
     675:	53                   	push   %ebx
     676:	83 ec 04             	sub    $0x4,%esp
     679:	e8 fc ff ff ff       	call   67a <operation_replace_at_place+0xa>
			67a: R_386_PC32	mcount
     67e:	89 c3                	mov    %eax,%ebx
     680:	89 c8                	mov    %ecx,%eax
     682:	8b 4d 0c             	mov    0xc(%ebp),%ecx
     685:	89 13                	mov    %edx,(%ebx)
     687:	8d 7b 04             	lea    0x4(%ebx),%edi
     68a:	89 d6                	mov    %edx,%esi
     68c:	c1 e9 02             	shr    $0x2,%ecx
     68f:	f3 a5                	rep movsl %ds:(%esi),%es:(%edi)
     691:	8b 4d 0c             	mov    0xc(%ebp),%ecx
     694:	83 e1 03             	and    $0x3,%ecx
     697:	74 02                	je     69b <operation_replace_at_place+0x2b>
     699:	f3 a4                	rep movsb %ds:(%esi),%es:(%edi)
     69b:	8b 55 08             	mov    0x8(%ebp),%edx
     69e:	8b 4d 0c             	mov    0xc(%ebp),%ecx
     6a1:	01 d1                	add    %edx,%ecx
     6a3:	39 d1                	cmp    %edx,%ecx
     6a5:	76 19                	jbe    6c0 <operation_replace_at_place+0x50>
     6a7:	8b 3a                	mov    (%edx),%edi
     6a9:	85 ff                	test   %edi,%edi
     6ab:	74 09                	je     6b6 <operation_replace_at_place+0x46>
     6ad:	eb 1b                	jmp    6ca <operation_replace_at_place+0x5a>
     6af:	90                   	nop
     6b0:	8b 32                	mov    (%edx),%esi
     6b2:	85 f6                	test   %esi,%esi
     6b4:	75 14                	jne    6ca <operation_replace_at_place+0x5a>
     6b6:	83 c2 04             	add    $0x4,%edx
     6b9:	39 ca                	cmp    %ecx,%edx
     6bb:	72 f3                	jb     6b0 <operation_replace_at_place+0x40>
     6bd:	8d 76 00             	lea    0x0(%esi),%esi
     6c0:	83 c4 04             	add    $0x4,%esp
     6c3:	31 c0                	xor    %eax,%eax
     6c5:	5b                   	pop    %ebx
     6c6:	5e                   	pop    %esi
     6c7:	5f                   	pop    %edi
     6c8:	5d                   	pop    %ebp
     6c9:	c3                   	ret    
     6ca:	85 d2                	test   %edx,%edx
     6cc:	74 f2                	je     6c0 <operation_replace_at_place+0x50>
     6ce:	89 4d f0             	mov    %ecx,-0x10(%ebp)
     6d1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     6d8:	89 d6                	mov    %edx,%esi
     6da:	8b 3b                	mov    (%ebx),%edi
     6dc:	83 c2 04             	add    $0x4,%edx
     6df:	2b 75 08             	sub    0x8(%ebp),%esi
     6e2:	8b 0c 30             	mov    (%eax,%esi,1),%ecx
     6e5:	89 0c 37             	mov    %ecx,(%edi,%esi,1)
     6e8:	3b 55 f0             	cmp    -0x10(%ebp),%edx
     6eb:	73 d3                	jae    6c0 <operation_replace_at_place+0x50>
     6ed:	8b 0a                	mov    (%edx),%ecx
     6ef:	85 c9                	test   %ecx,%ecx
     6f1:	75 18                	jne    70b <operation_replace_at_place+0x9b>
     6f3:	8b 4d f0             	mov    -0x10(%ebp),%ecx
     6f6:	eb 06                	jmp    6fe <operation_replace_at_place+0x8e>
     6f8:	8b 3a                	mov    (%edx),%edi
     6fa:	85 ff                	test   %edi,%edi
     6fc:	75 0a                	jne    708 <operation_replace_at_place+0x98>
     6fe:	83 c2 04             	add    $0x4,%edx
     701:	39 ca                	cmp    %ecx,%edx
     703:	72 f3                	jb     6f8 <operation_replace_at_place+0x88>
     705:	eb b9                	jmp    6c0 <operation_replace_at_place+0x50>
     707:	90                   	nop
     708:	89 4d f0             	mov    %ecx,-0x10(%ebp)
     70b:	85 d2                	test   %edx,%edx
     70d:	8d 76 00             	lea    0x0(%esi),%esi
     710:	75 c6                	jne    6d8 <operation_replace_at_place+0x68>
     712:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     718:	eb a6                	jmp    6c0 <operation_replace_at_place+0x50>
     71a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

00000720 <operation_get_orig>:
operation_get_orig():
     720:	55                   	push   %ebp
     721:	89 e5                	mov    %esp,%ebp
     723:	56                   	push   %esi
     724:	53                   	push   %ebx
     725:	e8 fc ff ff ff       	call   726 <operation_get_orig+0x6>
			726: R_386_PC32	mcount
     72a:	89 c3                	mov    %eax,%ebx
     72c:	8b 40 4c             	mov    0x4c(%eax),%eax
     72f:	89 d6                	mov    %edx,%esi
     731:	89 ca                	mov    %ecx,%edx
     733:	85 c0                	test   %eax,%eax
     735:	74 3a                	je     771 <operation_get_orig+0x51>
     737:	8b 40 08             	mov    0x8(%eax),%eax
     73a:	83 3c 30 00          	cmpl   $0x0,(%eax,%esi,1)
     73e:	74 3d                	je     77d <operation_get_orig+0x5d>
     740:	8b 03                	mov    (%ebx),%eax
     742:	e8 fc ff ff ff       	call   743 <operation_get_orig+0x23>
			743: R_386_PC32	data_map_get
     747:	3d 00 f0 ff ff       	cmp    $0xfffff000,%eax
     74c:	77 2b                	ja     779 <operation_get_orig+0x59>
     74e:	85 c0                	test   %eax,%eax
     750:	74 23                	je     775 <operation_get_orig+0x55>
     752:	8b 53 38             	mov    0x38(%ebx),%edx
     755:	85 d2                	test   %edx,%edx
     757:	75 09                	jne    762 <operation_get_orig+0x42>
     759:	8b 00                	mov    (%eax),%eax
     75b:	8b 04 30             	mov    (%eax,%esi,1),%eax
     75e:	5b                   	pop    %ebx
     75f:	5e                   	pop    %esi
     760:	5d                   	pop    %ebp
     761:	c3                   	ret    
     762:	83 fa 01             	cmp    $0x1,%edx
     765:	74 04                	je     76b <operation_get_orig+0x4b>
     767:	0f 0b                	ud2a   
     769:	eb fe                	jmp    769 <operation_get_orig+0x49>
     76b:	8b 44 30 04          	mov    0x4(%eax,%esi,1),%eax
     76f:	eb ed                	jmp    75e <operation_get_orig+0x3e>
     771:	0f 0b                	ud2a   
     773:	eb fe                	jmp    773 <operation_get_orig+0x53>
     775:	0f 0b                	ud2a   
     777:	eb fe                	jmp    777 <operation_get_orig+0x57>
     779:	0f 0b                	ud2a   
     77b:	eb fe                	jmp    77b <operation_get_orig+0x5b>
     77d:	0f 0b                	ud2a   
     77f:	eb fe                	jmp    77f <operation_get_orig+0x5f>
     781:	eb 0d                	jmp    790 <operation_get_orig_special>
     783:	90                   	nop
     784:	90                   	nop
     785:	90                   	nop
     786:	90                   	nop
     787:	90                   	nop
     788:	90                   	nop
     789:	90                   	nop
     78a:	90                   	nop
     78b:	90                   	nop
     78c:	90                   	nop
     78d:	90                   	nop
     78e:	90                   	nop
     78f:	90                   	nop

00000790 <operation_get_orig_special>:
operation_get_orig_special():
     790:	55                   	push   %ebp
     791:	89 e5                	mov    %esp,%ebp
     793:	57                   	push   %edi
     794:	56                   	push   %esi
     795:	53                   	push   %ebx
     796:	83 ec 0c             	sub    $0xc,%esp
     799:	e8 fc ff ff ff       	call   79a <operation_get_orig_special+0xa>
			79a: R_386_PC32	mcount
     79e:	8b 7d 08             	mov    0x8(%ebp),%edi
     7a1:	89 45 ec             	mov    %eax,-0x14(%ebp)
     7a4:	89 55 f0             	mov    %edx,-0x10(%ebp)
     7a7:	89 4d e8             	mov    %ecx,-0x18(%ebp)
     7aa:	8b 40 4c             	mov    0x4c(%eax),%eax
     7ad:	8b 30                	mov    (%eax),%esi
     7af:	8b 58 0c             	mov    0xc(%eax),%ebx
     7b2:	85 f6                	test   %esi,%esi
     7b4:	7e 24                	jle    7da <operation_get_orig_special+0x4a>
     7b6:	39 3b                	cmp    %edi,(%ebx)
     7b8:	74 26                	je     7e0 <operation_get_orig_special+0x50>
     7ba:	8d 43 0c             	lea    0xc(%ebx),%eax
     7bd:	31 d2                	xor    %edx,%edx
     7bf:	eb 12                	jmp    7d3 <operation_get_orig_special+0x43>
     7c1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     7c8:	8b 08                	mov    (%eax),%ecx
     7ca:	89 c3                	mov    %eax,%ebx
     7cc:	83 c0 0c             	add    $0xc,%eax
     7cf:	39 f9                	cmp    %edi,%ecx
     7d1:	74 0d                	je     7e0 <operation_get_orig_special+0x50>
     7d3:	83 c2 01             	add    $0x1,%edx
     7d6:	39 f2                	cmp    %esi,%edx
     7d8:	7c ee                	jl     7c8 <operation_get_orig_special+0x38>
     7da:	0f 0b                	ud2a   
     7dc:	eb fe                	jmp    7dc <operation_get_orig_special+0x4c>
     7de:	66 90                	xchg   %ax,%ax
     7e0:	8b 43 04             	mov    0x4(%ebx),%eax
     7e3:	8b 55 f0             	mov    -0x10(%ebp),%edx
     7e6:	8b 04 10             	mov    (%eax,%edx,1),%eax
     7e9:	85 c0                	test   %eax,%eax
     7eb:	74 11                	je     7fe <operation_get_orig_special+0x6e>
     7ed:	8b 43 08             	mov    0x8(%ebx),%eax
     7f0:	8b 55 f0             	mov    -0x10(%ebp),%edx
     7f3:	8b 04 10             	mov    (%eax,%edx,1),%eax
     7f6:	83 c4 0c             	add    $0xc,%esp
     7f9:	5b                   	pop    %ebx
     7fa:	5e                   	pop    %esi
     7fb:	5f                   	pop    %edi
     7fc:	5d                   	pop    %ebp
     7fd:	c3                   	ret    
     7fe:	8b 4d e8             	mov    -0x18(%ebp),%ecx
     801:	8b 55 f0             	mov    -0x10(%ebp),%edx
     804:	8b 45 ec             	mov    -0x14(%ebp),%eax
     807:	e8 fc ff ff ff       	call   808 <operation_get_orig_special+0x78>
			808: R_386_PC32	operation_get_orig
     80c:	83 c4 0c             	add    $0xc,%esp
     80f:	5b                   	pop    %ebx
     810:	5e                   	pop    %esi
     811:	5f                   	pop    %edi
     812:	5d                   	pop    %ebp
     813:	c3                   	ret    
     814:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     81a:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

00000820 <operation_target_unload_callback>:
operation_target_unload_callback():
     820:	55                   	push   %ebp
     821:	89 e5                	mov    %esp,%ebp
     823:	57                   	push   %edi
     824:	56                   	push   %esi
     825:	53                   	push   %ebx
     826:	83 ec 18             	sub    $0x18,%esp
     829:	e8 fc ff ff ff       	call   82a <operation_target_unload_callback+0xa>
			82a: R_386_PC32	mcount
     82e:	89 4d ec             	mov    %ecx,-0x14(%ebp)
     831:	8b 70 30             	mov    0x30(%eax),%esi
     834:	89 c3                	mov    %eax,%ebx
     836:	89 d7                	mov    %edx,%edi
     838:	85 f6                	test   %esi,%esi
     83a:	75 1d                	jne    859 <operation_target_unload_callback+0x39>
     83c:	c7 40 30 01 00 00 00 	movl   $0x1,0x30(%eax)
     843:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
     848:	f0 0f c1 43 2c       	lock xadd %eax,0x2c(%ebx)
     84d:	83 f8 01             	cmp    $0x1,%eax
     850:	74 16                	je     868 <operation_target_unload_callback+0x48>
     852:	c7 43 30 00 00 00 00 	movl   $0x0,0x30(%ebx)
     859:	83 c4 18             	add    $0x18,%esp
     85c:	5b                   	pop    %ebx
     85d:	5e                   	pop    %esi
     85e:	5f                   	pop    %edi
     85f:	5d                   	pop    %ebp
     860:	c3                   	ret    
     861:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     868:	8b 53 40             	mov    0x40(%ebx),%edx
     86b:	8d 72 f8             	lea    -0x8(%edx),%esi
     86e:	8b 46 0c             	mov    0xc(%esi),%eax
     871:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     875:	8d 4b 3c             	lea    0x3c(%ebx),%ecx
     878:	39 d1                	cmp    %edx,%ecx
     87a:	89 4d e4             	mov    %ecx,-0x1c(%ebp)
     87d:	74 2d                	je     8ac <operation_target_unload_callback+0x8c>
     87f:	89 5d e8             	mov    %ebx,-0x18(%ebp)
     882:	89 cb                	mov    %ecx,%ebx
     884:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     888:	8b 56 14             	mov    0x14(%esi),%edx
     88b:	8b 52 10             	mov    0x10(%edx),%edx
     88e:	85 d2                	test   %edx,%edx
     890:	74 07                	je     899 <operation_target_unload_callback+0x79>
     892:	89 f8                	mov    %edi,%eax
     894:	ff d2                	call   *%edx
     896:	8b 46 0c             	mov    0xc(%esi),%eax
     899:	8d 70 f8             	lea    -0x8(%eax),%esi
     89c:	89 c2                	mov    %eax,%edx
     89e:	8b 46 0c             	mov    0xc(%esi),%eax
     8a1:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     8a5:	39 d3                	cmp    %edx,%ebx
     8a7:	75 df                	jne    888 <operation_target_unload_callback+0x68>
     8a9:	8b 5d e8             	mov    -0x18(%ebp),%ebx
     8ac:	8b 53 48             	mov    0x48(%ebx),%edx
     8af:	8d 72 f8             	lea    -0x8(%edx),%esi
     8b2:	8b 46 0c             	mov    0xc(%esi),%eax
     8b5:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     8b9:	8d 4b 44             	lea    0x44(%ebx),%ecx
     8bc:	39 d1                	cmp    %edx,%ecx
     8be:	89 4d e8             	mov    %ecx,-0x18(%ebp)
     8c1:	74 29                	je     8ec <operation_target_unload_callback+0xcc>
     8c3:	89 5d e0             	mov    %ebx,-0x20(%ebp)
     8c6:	89 cb                	mov    %ecx,%ebx
     8c8:	8b 56 14             	mov    0x14(%esi),%edx
     8cb:	8b 52 10             	mov    0x10(%edx),%edx
     8ce:	85 d2                	test   %edx,%edx
     8d0:	74 07                	je     8d9 <operation_target_unload_callback+0xb9>
     8d2:	89 f8                	mov    %edi,%eax
     8d4:	ff d2                	call   *%edx
     8d6:	8b 46 0c             	mov    0xc(%esi),%eax
     8d9:	8d 70 f8             	lea    -0x8(%eax),%esi
     8dc:	89 c2                	mov    %eax,%edx
     8de:	8b 46 0c             	mov    0xc(%esi),%eax
     8e1:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     8e5:	39 d3                	cmp    %edx,%ebx
     8e7:	75 df                	jne    8c8 <operation_target_unload_callback+0xa8>
     8e9:	8b 5d e0             	mov    -0x20(%ebp),%ebx
     8ec:	8b 53 4c             	mov    0x4c(%ebx),%edx
     8ef:	85 d2                	test   %edx,%edx
     8f1:	74 68                	je     95b <operation_target_unload_callback+0x13b>
     8f3:	8b 0a                	mov    (%edx),%ecx
     8f5:	85 c9                	test   %ecx,%ecx
     8f7:	7e 37                	jle    930 <operation_target_unload_callback+0x110>
     8f9:	31 c9                	xor    %ecx,%ecx
     8fb:	31 f6                	xor    %esi,%esi
     8fd:	89 5d e0             	mov    %ebx,-0x20(%ebp)
     900:	89 cb                	mov    %ecx,%ebx
     902:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     908:	8b 7a 0c             	mov    0xc(%edx),%edi
     90b:	83 c3 01             	add    $0x1,%ebx
     90e:	01 f7                	add    %esi,%edi
     910:	83 c6 0c             	add    $0xc,%esi
     913:	8b 47 08             	mov    0x8(%edi),%eax
     916:	89 55 dc             	mov    %edx,-0x24(%ebp)
     919:	e8 fc ff ff ff       	call   91a <operation_target_unload_callback+0xfa>
			91a: R_386_PC32	kfree
     91e:	8b 47 04             	mov    0x4(%edi),%eax
     921:	e8 fc ff ff ff       	call   922 <operation_target_unload_callback+0x102>
			922: R_386_PC32	kfree
     926:	8b 55 dc             	mov    -0x24(%ebp),%edx
     929:	3b 1a                	cmp    (%edx),%ebx
     92b:	7c db                	jl     908 <operation_target_unload_callback+0xe8>
     92d:	8b 5d e0             	mov    -0x20(%ebp),%ebx
     930:	8b 42 0c             	mov    0xc(%edx),%eax
     933:	89 55 dc             	mov    %edx,-0x24(%ebp)
     936:	e8 fc ff ff ff       	call   937 <operation_target_unload_callback+0x117>
			937: R_386_PC32	kfree
     93b:	8b 55 dc             	mov    -0x24(%ebp),%edx
     93e:	8b 42 08             	mov    0x8(%edx),%eax
     941:	e8 fc ff ff ff       	call   942 <operation_target_unload_callback+0x122>
			942: R_386_PC32	kfree
     946:	8b 55 dc             	mov    -0x24(%ebp),%edx
     949:	8b 42 04             	mov    0x4(%edx),%eax
     94c:	e8 fc ff ff ff       	call   94d <operation_target_unload_callback+0x12d>
			94d: R_386_PC32	kfree
     951:	8b 55 dc             	mov    -0x24(%ebp),%edx
     954:	89 d0                	mov    %edx,%eax
     956:	e8 fc ff ff ff       	call   957 <operation_target_unload_callback+0x137>
			957: R_386_PC32	kfree
     95b:	8d 7b 18             	lea    0x18(%ebx),%edi
     95e:	89 f8                	mov    %edi,%eax
     960:	e8 fc ff ff ff       	call   961 <operation_target_unload_callback+0x141>
			961: R_386_PC32	mutex_lock
     965:	8b 45 ec             	mov    -0x14(%ebp),%eax
     968:	8d 4d f0             	lea    -0x10(%ebp),%ecx
     96b:	ba d0 02 00 00       	mov    $0x2d0,%edx
			96c: R_386_32	.text
     970:	89 45 f0             	mov    %eax,-0x10(%ebp)
     973:	8b 03                	mov    (%ebx),%eax
     975:	e8 fc ff ff ff       	call   976 <operation_target_unload_callback+0x156>
			976: R_386_PC32	data_map_delete_all
     97a:	8b 43 3c             	mov    0x3c(%ebx),%eax
     97d:	8d 70 f8             	lea    -0x8(%eax),%esi
     980:	8b 56 08             	mov    0x8(%esi),%edx
     983:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     987:	39 45 e4             	cmp    %eax,-0x1c(%ebp)
     98a:	74 4a                	je     9d6 <operation_target_unload_callback+0x1b6>
     98c:	8b 46 10             	mov    0x10(%esi),%eax
     98f:	85 c0                	test   %eax,%eax
     991:	0f 84 9f 00 00 00    	je     a36 <operation_target_unload_callback+0x216>
     997:	89 5d ec             	mov    %ebx,-0x14(%ebp)
     99a:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
     99d:	eb 0c                	jmp    9ab <operation_target_unload_callback+0x18b>
     99f:	90                   	nop
     9a0:	8b 4e 10             	mov    0x10(%esi),%ecx
     9a3:	85 c9                	test   %ecx,%ecx
     9a5:	0f 84 8b 00 00 00    	je     a36 <operation_target_unload_callback+0x216>
     9ab:	8b 46 14             	mov    0x14(%esi),%eax
     9ae:	8b 00                	mov    (%eax),%eax
     9b0:	85 c0                	test   %eax,%eax
     9b2:	74 08                	je     9bc <operation_target_unload_callback+0x19c>
     9b4:	e8 fc ff ff ff       	call   9b5 <operation_target_unload_callback+0x195>
			9b5: R_386_PC32	module_put
     9b9:	8b 56 08             	mov    0x8(%esi),%edx
     9bc:	c7 46 10 00 00 00 00 	movl   $0x0,0x10(%esi)
     9c3:	8d 72 f8             	lea    -0x8(%edx),%esi
     9c6:	89 d0                	mov    %edx,%eax
     9c8:	8b 56 08             	mov    0x8(%esi),%edx
     9cb:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     9cf:	39 c3                	cmp    %eax,%ebx
     9d1:	75 cd                	jne    9a0 <operation_target_unload_callback+0x180>
     9d3:	8b 5d ec             	mov    -0x14(%ebp),%ebx
     9d6:	8b 43 44             	mov    0x44(%ebx),%eax
     9d9:	8d 70 f8             	lea    -0x8(%eax),%esi
     9dc:	8b 56 08             	mov    0x8(%esi),%edx
     9df:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     9e3:	39 45 e8             	cmp    %eax,-0x18(%ebp)
     9e6:	74 42                	je     a2a <operation_target_unload_callback+0x20a>
     9e8:	8b 46 10             	mov    0x10(%esi),%eax
     9eb:	85 c0                	test   %eax,%eax
     9ed:	74 4b                	je     a3a <operation_target_unload_callback+0x21a>
     9ef:	89 5d ec             	mov    %ebx,-0x14(%ebp)
     9f2:	8b 5d e8             	mov    -0x18(%ebp),%ebx
     9f5:	eb 08                	jmp    9ff <operation_target_unload_callback+0x1df>
     9f7:	90                   	nop
     9f8:	8b 4e 10             	mov    0x10(%esi),%ecx
     9fb:	85 c9                	test   %ecx,%ecx
     9fd:	74 3b                	je     a3a <operation_target_unload_callback+0x21a>
     9ff:	8b 46 14             	mov    0x14(%esi),%eax
     a02:	8b 00                	mov    (%eax),%eax
     a04:	85 c0                	test   %eax,%eax
     a06:	74 08                	je     a10 <operation_target_unload_callback+0x1f0>
     a08:	e8 fc ff ff ff       	call   a09 <operation_target_unload_callback+0x1e9>
			a09: R_386_PC32	module_put
     a0d:	8b 56 08             	mov    0x8(%esi),%edx
     a10:	c7 46 10 00 00 00 00 	movl   $0x0,0x10(%esi)
     a17:	8d 72 f8             	lea    -0x8(%edx),%esi
     a1a:	89 d0                	mov    %edx,%eax
     a1c:	8b 56 08             	mov    0x8(%esi),%edx
     a1f:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     a23:	39 c3                	cmp    %eax,%ebx
     a25:	75 d1                	jne    9f8 <operation_target_unload_callback+0x1d8>
     a27:	8b 5d ec             	mov    -0x14(%ebp),%ebx
     a2a:	89 f8                	mov    %edi,%eax
     a2c:	e8 fc ff ff ff       	call   a2d <operation_target_unload_callback+0x20d>
			a2d: R_386_PC32	mutex_unlock
     a31:	e9 1c fe ff ff       	jmp    852 <operation_target_unload_callback+0x32>
     a36:	0f 0b                	ud2a   
     a38:	eb fe                	jmp    a38 <operation_target_unload_callback+0x218>
     a3a:	0f 0b                	ud2a   
     a3c:	eb fe                	jmp    a3c <operation_target_unload_callback+0x21c>
     a3e:	66 90                	xchg   %ax,%ax

00000a40 <operation_replacement_clean>:
operation_replacement_clean():
     a40:	55                   	push   %ebp
     a41:	89 e5                	mov    %esp,%ebp
     a43:	83 ec 14             	sub    $0x14,%esp
     a46:	89 5d f4             	mov    %ebx,-0xc(%ebp)
     a49:	89 75 f8             	mov    %esi,-0x8(%ebp)
     a4c:	89 7d fc             	mov    %edi,-0x4(%ebp)
     a4f:	e8 fc ff ff ff       	call   a50 <operation_replacement_clean+0x10>
			a50: R_386_PC32	mcount
     a54:	89 c3                	mov    %eax,%ebx
     a56:	89 d7                	mov    %edx,%edi
     a58:	8b 53 4c             	mov    0x4c(%ebx),%edx
     a5b:	31 c0                	xor    %eax,%eax
     a5d:	85 d2                	test   %edx,%edx
     a5f:	74 28                	je     a89 <operation_replacement_clean+0x49>
     a61:	8b 03                	mov    (%ebx),%eax
     a63:	89 fa                	mov    %edi,%edx
     a65:	e8 fc ff ff ff       	call   a66 <operation_replacement_clean+0x26>
			a66: R_386_PC32	data_map_get
     a6a:	3d 00 f0 ff ff       	cmp    $0xfffff000,%eax
     a6f:	89 c6                	mov    %eax,%esi
     a71:	77 23                	ja     a96 <operation_replacement_clean+0x56>
     a73:	85 c0                	test   %eax,%eax
     a75:	74 36                	je     aad <operation_replacement_clean+0x6d>
     a77:	8b 03                	mov    (%ebx),%eax
     a79:	89 fa                	mov    %edi,%edx
     a7b:	e8 fc ff ff ff       	call   a7c <operation_replacement_clean+0x3c>
			a7c: R_386_PC32	data_map_delete
     a80:	89 f0                	mov    %esi,%eax
     a82:	e8 fc ff ff ff       	call   a83 <operation_replacement_clean+0x43>
			a83: R_386_PC32	kfree
     a87:	31 c0                	xor    %eax,%eax
     a89:	8b 5d f4             	mov    -0xc(%ebp),%ebx
     a8c:	8b 75 f8             	mov    -0x8(%ebp),%esi
     a8f:	8b 7d fc             	mov    -0x4(%ebp),%edi
     a92:	89 ec                	mov    %ebp,%esp
     a94:	5d                   	pop    %ebp
     a95:	c3                   	ret    
     a96:	89 7c 24 04          	mov    %edi,0x4(%esp)
     a9a:	c7 04 24 5c 03 00 00 	movl   $0x35c,(%esp)
			a9d: R_386_32	.rodata.str1.4
     aa1:	e8 fc ff ff ff       	call   aa2 <operation_replacement_clean+0x62>
			aa2: R_386_PC32	printk
     aa6:	b8 01 00 00 00       	mov    $0x1,%eax
     aab:	eb dc                	jmp    a89 <operation_replacement_clean+0x49>
     aad:	0f 0b                	ud2a   
     aaf:	eb fe                	jmp    aaf <operation_replacement_clean+0x6f>
     ab1:	eb 0d                	jmp    ac0 <operation_restore>
     ab3:	90                   	nop
     ab4:	90                   	nop
     ab5:	90                   	nop
     ab6:	90                   	nop
     ab7:	90                   	nop
     ab8:	90                   	nop
     ab9:	90                   	nop
     aba:	90                   	nop
     abb:	90                   	nop
     abc:	90                   	nop
     abd:	90                   	nop
     abe:	90                   	nop
     abf:	90                   	nop

00000ac0 <operation_restore>:
operation_restore():
     ac0:	55                   	push   %ebp
     ac1:	89 e5                	mov    %esp,%ebp
     ac3:	57                   	push   %edi
     ac4:	56                   	push   %esi
     ac5:	53                   	push   %ebx
     ac6:	83 ec 0c             	sub    $0xc,%esp
     ac9:	e8 fc ff ff ff       	call   aca <operation_restore+0xa>
			aca: R_386_PC32	mcount
     ace:	8b 48 4c             	mov    0x4c(%eax),%ecx
     ad1:	89 c7                	mov    %eax,%edi
     ad3:	31 c0                	xor    %eax,%eax
     ad5:	85 c9                	test   %ecx,%ecx
     ad7:	89 d3                	mov    %edx,%ebx
     ad9:	74 44                	je     b1f <operation_restore+0x5f>
     adb:	8b 07                	mov    (%edi),%eax
     add:	89 4d f0             	mov    %ecx,-0x10(%ebp)
     ae0:	e8 fc ff ff ff       	call   ae1 <operation_restore+0x21>
			ae1: R_386_PC32	data_map_get
     ae5:	8b 4d f0             	mov    -0x10(%ebp),%ecx
     ae8:	3d 00 f0 ff ff       	cmp    $0xfffff000,%eax
     aed:	89 c6                	mov    %eax,%esi
     aef:	77 70                	ja     b61 <operation_restore+0xa1>
     af1:	85 c0                	test   %eax,%eax
     af3:	74 68                	je     b5d <operation_restore+0x9d>
     af5:	8b 47 38             	mov    0x38(%edi),%eax
     af8:	8b 51 04             	mov    0x4(%ecx),%edx
     afb:	8b 49 08             	mov    0x8(%ecx),%ecx
     afe:	85 c0                	test   %eax,%eax
     b00:	75 25                	jne    b27 <operation_restore+0x67>
     b02:	8d 46 04             	lea    0x4(%esi),%eax
     b05:	39 03                	cmp    %eax,(%ebx)
     b07:	75 42                	jne    b4b <operation_restore+0x8b>
     b09:	8b 06                	mov    (%esi),%eax
     b0b:	89 03                	mov    %eax,(%ebx)
     b0d:	8b 07                	mov    (%edi),%eax
     b0f:	89 da                	mov    %ebx,%edx
     b11:	e8 fc ff ff ff       	call   b12 <operation_restore+0x52>
			b12: R_386_PC32	data_map_delete
     b16:	89 f0                	mov    %esi,%eax
     b18:	e8 fc ff ff ff       	call   b19 <operation_restore+0x59>
			b19: R_386_PC32	kfree
     b1d:	31 c0                	xor    %eax,%eax
     b1f:	83 c4 0c             	add    $0xc,%esp
     b22:	5b                   	pop    %ebx
     b23:	5e                   	pop    %esi
     b24:	5f                   	pop    %edi
     b25:	5d                   	pop    %ebp
     b26:	c3                   	ret    
     b27:	83 f8 01             	cmp    $0x1,%eax
     b2a:	74 04                	je     b30 <operation_restore+0x70>
     b2c:	0f 0b                	ud2a   
     b2e:	eb fe                	jmp    b2e <operation_restore+0x6e>
     b30:	8b 47 34             	mov    0x34(%edi),%eax
     b33:	89 0c 24             	mov    %ecx,(%esp)
     b36:	89 d1                	mov    %edx,%ecx
     b38:	89 da                	mov    %ebx,%edx
     b3a:	89 44 24 04          	mov    %eax,0x4(%esp)
     b3e:	89 f0                	mov    %esi,%eax
     b40:	e8 cb f6 ff ff       	call   210 <operation_restore_at_place>
     b45:	85 c0                	test   %eax,%eax
     b47:	75 d6                	jne    b1f <operation_restore+0x5f>
     b49:	eb c2                	jmp    b0d <operation_restore+0x4d>
     b4b:	89 5c 24 04          	mov    %ebx,0x4(%esp)
     b4f:	c7 04 24 d8 03 00 00 	movl   $0x3d8,(%esp)
			b52: R_386_32	.rodata.str1.4
     b56:	e8 fc ff ff ff       	call   b57 <operation_restore+0x97>
			b57: R_386_PC32	printk
     b5b:	eb b0                	jmp    b0d <operation_restore+0x4d>
     b5d:	0f 0b                	ud2a   
     b5f:	eb fe                	jmp    b5f <operation_restore+0x9f>
     b61:	89 5c 24 04          	mov    %ebx,0x4(%esp)
     b65:	c7 04 24 a0 03 00 00 	movl   $0x3a0,(%esp)
			b68: R_386_32	.rodata.str1.4
     b6c:	e8 fc ff ff ff       	call   b6d <operation_restore+0xad>
			b6d: R_386_PC32	printk
     b71:	b8 01 00 00 00       	mov    $0x1,%eax
     b76:	eb a7                	jmp    b1f <operation_restore+0x5f>
     b78:	90                   	nop
     b79:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00000b80 <operation_replacer_destroy>:
operation_replacer_destroy():
     b80:	55                   	push   %ebp
     b81:	89 e5                	mov    %esp,%ebp
     b83:	53                   	push   %ebx
     b84:	e8 fc ff ff ff       	call   b85 <operation_replacer_destroy+0x5>
			b85: R_386_PC32	mcount
     b89:	89 c3                	mov    %eax,%ebx
     b8b:	8b 40 2c             	mov    0x2c(%eax),%eax
     b8e:	85 c0                	test   %eax,%eax
     b90:	75 2d                	jne    bbf <operation_replacer_destroy+0x3f>
     b92:	8d 53 04             	lea    0x4(%ebx),%edx
     b95:	89 d8                	mov    %ebx,%eax
     b97:	e8 74 f7 ff ff       	call   310 <unregister_payloads_at_delete>
     b9c:	8d 53 0c             	lea    0xc(%ebx),%edx
     b9f:	89 d8                	mov    %ebx,%eax
     ba1:	e8 6a f7 ff ff       	call   310 <unregister_payloads_at_delete>
     ba6:	8b 43 14             	mov    0x14(%ebx),%eax
     ba9:	e8 fc ff ff ff       	call   baa <operation_replacer_destroy+0x2a>
			baa: R_386_PC32	kfree
     bae:	8b 03                	mov    (%ebx),%eax
     bb0:	e8 fc ff ff ff       	call   bb1 <operation_replacer_destroy+0x31>
			bb1: R_386_PC32	data_map_destroy
     bb5:	89 d8                	mov    %ebx,%eax
     bb7:	e8 fc ff ff ff       	call   bb8 <operation_replacer_destroy+0x38>
			bb8: R_386_PC32	kfree
     bbc:	5b                   	pop    %ebx
     bbd:	5d                   	pop    %ebp
     bbe:	c3                   	ret    
     bbf:	0f 0b                	ud2a   
     bc1:	eb fe                	jmp    bc1 <operation_replacer_destroy+0x41>
     bc3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     bc9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000bd0 <operation_payload_register>:
operation_payload_register():
     bd0:	55                   	push   %ebp
     bd1:	89 e5                	mov    %esp,%ebp
     bd3:	57                   	push   %edi
     bd4:	56                   	push   %esi
     bd5:	53                   	push   %ebx
     bd6:	83 ec 20             	sub    $0x20,%esp
     bd9:	e8 fc ff ff ff       	call   bda <operation_payload_register+0xa>
			bda: R_386_PC32	mcount
     bde:	89 55 e8             	mov    %edx,-0x18(%ebp)
     be1:	89 c6                	mov    %eax,%esi
     be3:	8b 40 2c             	mov    0x2c(%eax),%eax
     be6:	8b 7a 08             	mov    0x8(%edx),%edi
     be9:	85 c0                	test   %eax,%eax
     beb:	0f 85 74 01 00 00    	jne    d65 <operation_payload_register+0x195>
     bf1:	8d 46 18             	lea    0x18(%esi),%eax
     bf4:	bb fc ff ff ff       	mov    $0xfffffffc,%ebx
     bf9:	89 45 ec             	mov    %eax,-0x14(%ebp)
     bfc:	e8 fc ff ff ff       	call   bfd <operation_payload_register+0x2d>
			bfd: R_386_PC32	mutex_lock_killable
     c01:	85 c0                	test   %eax,%eax
     c03:	0f 85 c9 00 00 00    	jne    cd2 <operation_payload_register+0x102>
     c09:	8b 56 34             	mov    0x34(%esi),%edx
     c0c:	89 fb                	mov    %edi,%ebx
     c0e:	01 fa                	add    %edi,%edx
     c10:	39 fa                	cmp    %edi,%edx
     c12:	76 25                	jbe    c39 <operation_payload_register+0x69>
     c14:	8b 0f                	mov    (%edi),%ecx
     c16:	89 f8                	mov    %edi,%eax
     c18:	85 c9                	test   %ecx,%ecx
     c1a:	74 16                	je     c32 <operation_payload_register+0x62>
     c1c:	e9 bf 00 00 00       	jmp    ce0 <operation_payload_register+0x110>
     c21:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     c28:	8b 08                	mov    (%eax),%ecx
     c2a:	85 c9                	test   %ecx,%ecx
     c2c:	0f 85 ae 00 00 00    	jne    ce0 <operation_payload_register+0x110>
     c32:	83 c0 04             	add    $0x4,%eax
     c35:	39 d0                	cmp    %edx,%eax
     c37:	72 ef                	jb     c28 <operation_payload_register+0x58>
     c39:	31 c0                	xor    %eax,%eax
     c3b:	85 c0                	test   %eax,%eax
     c3d:	89 5d e4             	mov    %ebx,-0x1c(%ebp)
     c40:	0f 85 a5 00 00 00    	jne    ceb <operation_payload_register+0x11b>
     c46:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
     c49:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     c50:	ba d0 00 00 00       	mov    $0xd0,%edx
     c55:	b8 34 03 00 00       	mov    $0x334,%eax
			c56: R_386_32	kmalloc_caches
     c5a:	e8 fc ff ff ff       	call   c5b <operation_payload_register+0x8b>
			c5b: R_386_PC32	kmem_cache_alloc
     c5f:	8b 15 04 00 00 00    	mov    0x4,%edx
			c61: R_386_32	__tracepoint_kmalloc
     c65:	85 d2                	test   %edx,%edx
     c67:	89 45 f0             	mov    %eax,-0x10(%ebp)
     c6a:	a1 38 03 00 00       	mov    0x338,%eax
			c6b: R_386_32	kmalloc_caches
     c6f:	89 45 e0             	mov    %eax,-0x20(%ebp)
     c72:	0f 85 19 01 00 00    	jne    d91 <operation_payload_register+0x1c1>
     c78:	8b 45 f0             	mov    -0x10(%ebp),%eax
     c7b:	85 c0                	test   %eax,%eax
     c7d:	0f 84 f8 00 00 00    	je     d7b <operation_payload_register+0x1ab>
     c83:	8b 45 e8             	mov    -0x18(%ebp),%eax
     c86:	8b 4d f0             	mov    -0x10(%ebp),%ecx
     c89:	89 41 14             	mov    %eax,0x14(%ecx)
     c8c:	c7 41 10 00 00 00 00 	movl   $0x0,0x10(%ecx)
     c93:	8b 46 04             	mov    0x4(%esi),%eax
     c96:	89 48 04             	mov    %ecx,0x4(%eax)
     c99:	89 01                	mov    %eax,(%ecx)
     c9b:	8d 46 04             	lea    0x4(%esi),%eax
     c9e:	89 41 04             	mov    %eax,0x4(%ecx)
     ca1:	8b 46 34             	mov    0x34(%esi),%eax
     ca4:	89 4e 04             	mov    %ecx,0x4(%esi)
     ca7:	01 f8                	add    %edi,%eax
     ca9:	39 f8                	cmp    %edi,%eax
     cab:	76 1b                	jbe    cc8 <operation_payload_register+0xf8>
     cad:	8b 0f                	mov    (%edi),%ecx
     caf:	85 c9                	test   %ecx,%ecx
     cb1:	74 0b                	je     cbe <operation_payload_register+0xee>
     cb3:	eb 7b                	jmp    d30 <operation_payload_register+0x160>
     cb5:	8d 76 00             	lea    0x0(%esi),%esi
     cb8:	8b 13                	mov    (%ebx),%edx
     cba:	85 d2                	test   %edx,%edx
     cbc:	75 72                	jne    d30 <operation_payload_register+0x160>
     cbe:	83 c3 04             	add    $0x4,%ebx
     cc1:	39 c3                	cmp    %eax,%ebx
     cc3:	72 f3                	jb     cb8 <operation_payload_register+0xe8>
     cc5:	8d 76 00             	lea    0x0(%esi),%esi
     cc8:	31 db                	xor    %ebx,%ebx
     cca:	8b 45 ec             	mov    -0x14(%ebp),%eax
     ccd:	e8 fc ff ff ff       	call   cce <operation_payload_register+0xfe>
			cce: R_386_PC32	mutex_unlock
     cd2:	83 c4 20             	add    $0x20,%esp
     cd5:	89 d8                	mov    %ebx,%eax
     cd7:	5b                   	pop    %ebx
     cd8:	5e                   	pop    %esi
     cd9:	5f                   	pop    %edi
     cda:	5d                   	pop    %ebp
     cdb:	c3                   	ret    
     cdc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     ce0:	89 5d e4             	mov    %ebx,-0x1c(%ebp)
     ce3:	85 c0                	test   %eax,%eax
     ce5:	0f 84 5b ff ff ff    	je     c46 <operation_payload_register+0x76>
     ceb:	8b 5e 14             	mov    0x14(%esi),%ebx
     cee:	89 c1                	mov    %eax,%ecx
     cf0:	29 f9                	sub    %edi,%ecx
     cf2:	8b 0c 0b             	mov    (%ebx,%ecx,1),%ecx
     cf5:	85 c9                	test   %ecx,%ecx
     cf7:	0f 85 dd 00 00 00    	jne    dda <operation_payload_register+0x20a>
     cfd:	83 c0 04             	add    $0x4,%eax
     d00:	39 d0                	cmp    %edx,%eax
     d02:	0f 83 3e ff ff ff    	jae    c46 <operation_payload_register+0x76>
     d08:	8b 18                	mov    (%eax),%ebx
     d0a:	85 db                	test   %ebx,%ebx
     d0c:	75 d5                	jne    ce3 <operation_payload_register+0x113>
     d0e:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
     d11:	eb 0b                	jmp    d1e <operation_payload_register+0x14e>
     d13:	90                   	nop
     d14:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     d18:	8b 08                	mov    (%eax),%ecx
     d1a:	85 c9                	test   %ecx,%ecx
     d1c:	75 c2                	jne    ce0 <operation_payload_register+0x110>
     d1e:	83 c0 04             	add    $0x4,%eax
     d21:	39 d0                	cmp    %edx,%eax
     d23:	72 f3                	jb     d18 <operation_payload_register+0x148>
     d25:	e9 26 ff ff ff       	jmp    c50 <operation_payload_register+0x80>
     d2a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     d30:	85 db                	test   %ebx,%ebx
     d32:	74 94                	je     cc8 <operation_payload_register+0xf8>
     d34:	8b 0b                	mov    (%ebx),%ecx
     d36:	89 d8                	mov    %ebx,%eax
     d38:	83 c3 04             	add    $0x4,%ebx
     d3b:	8b 56 14             	mov    0x14(%esi),%edx
     d3e:	29 f8                	sub    %edi,%eax
     d40:	89 0c 02             	mov    %ecx,(%edx,%eax,1)
     d43:	8b 46 34             	mov    0x34(%esi),%eax
     d46:	01 f8                	add    %edi,%eax
     d48:	39 d8                	cmp    %ebx,%eax
     d4a:	0f 86 78 ff ff ff    	jbe    cc8 <operation_payload_register+0xf8>
     d50:	8b 0b                	mov    (%ebx),%ecx
     d52:	85 c9                	test   %ecx,%ecx
     d54:	75 da                	jne    d30 <operation_payload_register+0x160>
     d56:	83 c3 04             	add    $0x4,%ebx
     d59:	39 c3                	cmp    %eax,%ebx
     d5b:	72 f3                	jb     d50 <operation_payload_register+0x180>
     d5d:	8d 76 00             	lea    0x0(%esi),%esi
     d60:	e9 63 ff ff ff       	jmp    cc8 <operation_payload_register+0xf8>
     d65:	c7 04 24 44 04 00 00 	movl   $0x444,(%esp)
			d68: R_386_32	.rodata.str1.4
     d6c:	bb f0 ff ff ff       	mov    $0xfffffff0,%ebx
     d71:	e8 fc ff ff ff       	call   d72 <operation_payload_register+0x1a2>
			d72: R_386_PC32	printk
     d76:	e9 57 ff ff ff       	jmp    cd2 <operation_payload_register+0x102>
     d7b:	c7 04 24 2c 05 00 00 	movl   $0x52c,(%esp)
			d7e: R_386_32	.rodata.str1.4
     d82:	bb f4 ff ff ff       	mov    $0xfffffff4,%ebx
     d87:	e8 fc ff ff ff       	call   d88 <operation_payload_register+0x1b8>
			d88: R_386_PC32	printk
     d8c:	e9 39 ff ff ff       	jmp    cca <operation_payload_register+0xfa>
     d91:	8b 15 10 00 00 00    	mov    0x10,%edx
			d93: R_386_32	__tracepoint_kmalloc
     d97:	85 d2                	test   %edx,%edx
     d99:	89 55 e4             	mov    %edx,-0x1c(%ebp)
     d9c:	0f 84 d6 fe ff ff    	je     c78 <operation_payload_register+0xa8>
     da2:	8b 0a                	mov    (%edx),%ecx
     da4:	89 4d dc             	mov    %ecx,-0x24(%ebp)
     da7:	8b 45 e0             	mov    -0x20(%ebp),%eax
     daa:	b9 18 00 00 00       	mov    $0x18,%ecx
     daf:	8b 55 f0             	mov    -0x10(%ebp),%edx
     db2:	c7 44 24 04 d0 00 00 	movl   $0xd0,0x4(%esp)
     db9:	00 
     dba:	89 04 24             	mov    %eax,(%esp)
     dbd:	b8 50 0c 00 00       	mov    $0xc50,%eax
			dbe: R_386_32	.text
     dc2:	ff 55 dc             	call   *-0x24(%ebp)
     dc5:	83 45 e4 04          	addl   $0x4,-0x1c(%ebp)
     dc9:	8b 55 e4             	mov    -0x1c(%ebp),%edx
     dcc:	8b 12                	mov    (%edx),%edx
     dce:	85 d2                	test   %edx,%edx
     dd0:	89 55 dc             	mov    %edx,-0x24(%ebp)
     dd3:	75 d2                	jne    da7 <operation_payload_register+0x1d7>
     dd5:	e9 9e fe ff ff       	jmp    c78 <operation_payload_register+0xa8>
     dda:	c7 04 24 a4 04 00 00 	movl   $0x4a4,(%esp)
			ddd: R_386_32	.rodata.str1.4
     de1:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
     de6:	e8 fc ff ff ff       	call   de7 <operation_payload_register+0x217>
			de7: R_386_PC32	printk
     deb:	e9 da fe ff ff       	jmp    cca <operation_payload_register+0xfa>

00000df0 <operation_payload_register_special>:
operation_payload_register_special():
     df0:	55                   	push   %ebp
     df1:	89 e5                	mov    %esp,%ebp
     df3:	83 ec 24             	sub    $0x24,%esp
     df6:	89 5d f4             	mov    %ebx,-0xc(%ebp)
     df9:	89 75 f8             	mov    %esi,-0x8(%ebp)
     dfc:	89 7d fc             	mov    %edi,-0x4(%ebp)
     dff:	e8 fc ff ff ff       	call   e00 <operation_payload_register_special+0x10>
			e00: R_386_PC32	mcount
     e04:	89 c3                	mov    %eax,%ebx
     e06:	89 55 f0             	mov    %edx,-0x10(%ebp)
     e09:	8b 40 2c             	mov    0x2c(%eax),%eax
     e0c:	85 c0                	test   %eax,%eax
     e0e:	75 77                	jne    e87 <operation_payload_register_special+0x97>
     e10:	8d 7b 18             	lea    0x18(%ebx),%edi
     e13:	be fc ff ff ff       	mov    $0xfffffffc,%esi
     e18:	89 f8                	mov    %edi,%eax
     e1a:	e8 fc ff ff ff       	call   e1b <operation_payload_register_special+0x2b>
			e1b: R_386_PC32	mutex_lock_killable
     e1f:	85 c0                	test   %eax,%eax
     e21:	74 15                	je     e38 <operation_payload_register_special+0x48>
     e23:	89 f0                	mov    %esi,%eax
     e25:	8b 5d f4             	mov    -0xc(%ebp),%ebx
     e28:	8b 75 f8             	mov    -0x8(%ebp),%esi
     e2b:	8b 7d fc             	mov    -0x4(%ebp),%edi
     e2e:	89 ec                	mov    %ebp,%esp
     e30:	5d                   	pop    %ebp
     e31:	c3                   	ret    
     e32:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     e38:	ba d0 00 00 00       	mov    $0xd0,%edx
     e3d:	b8 34 03 00 00       	mov    $0x334,%eax
			e3e: R_386_32	kmalloc_caches
     e42:	e8 fc ff ff ff       	call   e43 <operation_payload_register_special+0x53>
			e43: R_386_PC32	kmem_cache_alloc
     e47:	89 c6                	mov    %eax,%esi
     e49:	a1 38 03 00 00       	mov    0x338,%eax
			e4a: R_386_32	kmalloc_caches
     e4e:	89 45 ec             	mov    %eax,-0x14(%ebp)
     e51:	a1 04 00 00 00       	mov    0x4,%eax
			e52: R_386_32	__tracepoint_kmalloc
     e56:	85 c0                	test   %eax,%eax
     e58:	75 40                	jne    e9a <operation_payload_register_special+0xaa>
     e5a:	85 f6                	test   %esi,%esi
     e5c:	74 7c                	je     eda <operation_payload_register_special+0xea>
     e5e:	8b 45 f0             	mov    -0x10(%ebp),%eax
     e61:	c7 46 10 00 00 00 00 	movl   $0x0,0x10(%esi)
     e68:	89 46 14             	mov    %eax,0x14(%esi)
     e6b:	8b 43 0c             	mov    0xc(%ebx),%eax
     e6e:	89 70 04             	mov    %esi,0x4(%eax)
     e71:	89 06                	mov    %eax,(%esi)
     e73:	8d 43 0c             	lea    0xc(%ebx),%eax
     e76:	89 46 04             	mov    %eax,0x4(%esi)
     e79:	89 73 0c             	mov    %esi,0xc(%ebx)
     e7c:	31 f6                	xor    %esi,%esi
     e7e:	89 f8                	mov    %edi,%eax
     e80:	e8 fc ff ff ff       	call   e81 <operation_payload_register_special+0x91>
			e81: R_386_PC32	mutex_unlock
     e85:	eb 9c                	jmp    e23 <operation_payload_register_special+0x33>
     e87:	c7 04 24 44 04 00 00 	movl   $0x444,(%esp)
			e8a: R_386_32	.rodata.str1.4
     e8e:	be f0 ff ff ff       	mov    $0xfffffff0,%esi
     e93:	e8 fc ff ff ff       	call   e94 <operation_payload_register_special+0xa4>
			e94: R_386_PC32	printk
     e98:	eb 89                	jmp    e23 <operation_payload_register_special+0x33>
     e9a:	a1 10 00 00 00       	mov    0x10,%eax
			e9b: R_386_32	__tracepoint_kmalloc
     e9f:	85 c0                	test   %eax,%eax
     ea1:	89 45 e8             	mov    %eax,-0x18(%ebp)
     ea4:	74 b4                	je     e5a <operation_payload_register_special+0x6a>
     ea6:	8b 00                	mov    (%eax),%eax
     ea8:	89 45 e4             	mov    %eax,-0x1c(%ebp)
     eab:	8b 45 ec             	mov    -0x14(%ebp),%eax
     eae:	b9 18 00 00 00       	mov    $0x18,%ecx
     eb3:	89 f2                	mov    %esi,%edx
     eb5:	c7 44 24 04 d0 00 00 	movl   $0xd0,0x4(%esp)
     ebc:	00 
     ebd:	89 04 24             	mov    %eax,(%esp)
     ec0:	b8 38 0e 00 00       	mov    $0xe38,%eax
			ec1: R_386_32	.text
     ec5:	ff 55 e4             	call   *-0x1c(%ebp)
     ec8:	83 45 e8 04          	addl   $0x4,-0x18(%ebp)
     ecc:	8b 45 e8             	mov    -0x18(%ebp),%eax
     ecf:	8b 00                	mov    (%eax),%eax
     ed1:	85 c0                	test   %eax,%eax
     ed3:	89 45 e4             	mov    %eax,-0x1c(%ebp)
     ed6:	75 d3                	jne    eab <operation_payload_register_special+0xbb>
     ed8:	eb 80                	jmp    e5a <operation_payload_register_special+0x6a>
     eda:	c7 04 24 70 05 00 00 	movl   $0x570,(%esp)
			edd: R_386_32	.rodata.str1.4
     ee1:	be f4 ff ff ff       	mov    $0xfffffff4,%esi
     ee6:	e8 fc ff ff ff       	call   ee7 <operation_payload_register_special+0xf7>
			ee7: R_386_PC32	printk
     eeb:	eb 91                	jmp    e7e <operation_payload_register_special+0x8e>
     eed:	8d 76 00             	lea    0x0(%esi),%esi

00000ef0 <operation_replace>:
operation_replace():
     ef0:	55                   	push   %ebp
     ef1:	89 e5                	mov    %esp,%ebp
     ef3:	57                   	push   %edi
     ef4:	56                   	push   %esi
     ef5:	53                   	push   %ebx
     ef6:	83 ec 18             	sub    $0x18,%esp
     ef9:	e8 fc ff ff ff       	call   efa <operation_replace+0xa>
			efa: R_386_PC32	mcount
     efe:	bf f4 ff ff ff       	mov    $0xfffffff4,%edi
     f03:	89 55 f0             	mov    %edx,-0x10(%ebp)
     f06:	8b 48 4c             	mov    0x4c(%eax),%ecx
     f09:	89 c3                	mov    %eax,%ebx
     f0b:	85 c9                	test   %ecx,%ecx
     f0d:	0f 84 81 00 00 00    	je     f94 <operation_replace+0xa4>
     f13:	8b 40 38             	mov    0x38(%eax),%eax
     f16:	85 c0                	test   %eax,%eax
     f18:	74 19                	je     f33 <operation_replace+0x43>
     f1a:	83 f8 01             	cmp    $0x1,%eax
     f1d:	74 14                	je     f33 <operation_replace+0x43>
     f1f:	89 44 24 04          	mov    %eax,0x4(%esp)
     f23:	c7 04 24 bc 05 00 00 	movl   $0x5bc,(%esp)
			f26: R_386_32	.rodata.str1.4
     f2a:	e8 fc ff ff ff       	call   f2b <operation_replace+0x3b>
			f2b: R_386_PC32	printk
     f2f:	0f 0b                	ud2a   
     f31:	eb fe                	jmp    f31 <operation_replace+0x41>
     f33:	8b 43 34             	mov    0x34(%ebx),%eax
     f36:	ba d0 00 00 00       	mov    $0xd0,%edx
     f3b:	89 4d e4             	mov    %ecx,-0x1c(%ebp)
     f3e:	83 c0 04             	add    $0x4,%eax
     f41:	e8 fc ff ff ff       	call   f42 <operation_replace+0x52>
			f42: R_386_PC32	__kmalloc
     f46:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
     f49:	85 c0                	test   %eax,%eax
     f4b:	89 c6                	mov    %eax,%esi
     f4d:	0f 84 b7 00 00 00    	je     100a <operation_replace+0x11a>
     f53:	8b 41 04             	mov    0x4(%ecx),%eax
     f56:	89 45 e8             	mov    %eax,-0x18(%ebp)
     f59:	8b 43 38             	mov    0x38(%ebx),%eax
     f5c:	8b 49 08             	mov    0x8(%ecx),%ecx
     f5f:	85 c0                	test   %eax,%eax
     f61:	89 4d ec             	mov    %ecx,-0x14(%ebp)
     f64:	0f 84 82 00 00 00    	je     fec <operation_replace+0xfc>
     f6a:	83 f8 01             	cmp    $0x1,%eax
     f6d:	74 2f                	je     f9e <operation_replace+0xae>
     f6f:	0f 0b                	ud2a   
     f71:	eb fe                	jmp    f71 <operation_replace+0x81>
     f73:	8b 55 ec             	mov    -0x14(%ebp),%edx
     f76:	8b 43 34             	mov    0x34(%ebx),%eax
     f79:	8b 4d e8             	mov    -0x18(%ebp),%ecx
     f7c:	89 14 24             	mov    %edx,(%esp)
     f7f:	8b 55 f0             	mov    -0x10(%ebp),%edx
     f82:	89 44 24 04          	mov    %eax,0x4(%esp)
     f86:	89 f0                	mov    %esi,%eax
     f88:	e8 83 f2 ff ff       	call   210 <operation_restore_at_place>
     f8d:	89 f0                	mov    %esi,%eax
     f8f:	e8 fc ff ff ff       	call   f90 <operation_replace+0xa0>
			f90: R_386_PC32	kfree
     f94:	83 c4 18             	add    $0x18,%esp
     f97:	89 f8                	mov    %edi,%eax
     f99:	5b                   	pop    %ebx
     f9a:	5e                   	pop    %esi
     f9b:	5f                   	pop    %edi
     f9c:	5d                   	pop    %ebp
     f9d:	c3                   	ret    
     f9e:	8b 43 34             	mov    0x34(%ebx),%eax
     fa1:	8b 4d e8             	mov    -0x18(%ebp),%ecx
     fa4:	8b 55 f0             	mov    -0x10(%ebp),%edx
     fa7:	89 44 24 04          	mov    %eax,0x4(%esp)
     fab:	8b 45 ec             	mov    -0x14(%ebp),%eax
     fae:	89 04 24             	mov    %eax,(%esp)
     fb1:	89 f0                	mov    %esi,%eax
     fb3:	e8 b8 f6 ff ff       	call   670 <operation_replace_at_place>
     fb8:	89 c7                	mov    %eax,%edi
     fba:	85 ff                	test   %edi,%edi
     fbc:	75 75                	jne    1033 <operation_replace+0x143>
     fbe:	8b 55 f0             	mov    -0x10(%ebp),%edx
     fc1:	89 f1                	mov    %esi,%ecx
     fc3:	8b 03                	mov    (%ebx),%eax
     fc5:	e8 fc ff ff ff       	call   fc6 <operation_replace+0xd6>
			fc6: R_386_PC32	data_map_add
     fca:	85 c0                	test   %eax,%eax
     fcc:	89 c7                	mov    %eax,%edi
     fce:	74 c4                	je     f94 <operation_replace+0xa4>
     fd0:	c7 04 24 70 06 00 00 	movl   $0x670,(%esp)
			fd3: R_386_32	.rodata.str1.4
     fd7:	e8 fc ff ff ff       	call   fd8 <operation_replace+0xe8>
			fd8: R_386_PC32	printk
     fdc:	8b 43 38             	mov    0x38(%ebx),%eax
     fdf:	85 c0                	test   %eax,%eax
     fe1:	74 3d                	je     1020 <operation_replace+0x130>
     fe3:	83 f8 01             	cmp    $0x1,%eax
     fe6:	74 8b                	je     f73 <operation_replace+0x83>
     fe8:	0f 0b                	ud2a   
     fea:	eb fe                	jmp    fea <operation_replace+0xfa>
     fec:	8b 55 ec             	mov    -0x14(%ebp),%edx
     fef:	8b 43 34             	mov    0x34(%ebx),%eax
     ff2:	8b 4d e8             	mov    -0x18(%ebp),%ecx
     ff5:	89 14 24             	mov    %edx,(%esp)
     ff8:	8b 55 f0             	mov    -0x10(%ebp),%edx
     ffb:	89 44 24 04          	mov    %eax,0x4(%esp)
     fff:	89 f0                	mov    %esi,%eax
    1001:	e8 6a f5 ff ff       	call   570 <operation_replace_replace_pointer>
    1006:	89 c7                	mov    %eax,%edi
    1008:	eb b0                	jmp    fba <operation_replace+0xca>
    100a:	c7 04 24 ec 05 00 00 	movl   $0x5ec,(%esp)
			100d: R_386_32	.rodata.str1.4
    1011:	bf f4 ff ff ff       	mov    $0xfffffff4,%edi
    1016:	e8 fc ff ff ff       	call   1017 <operation_replace+0x127>
			1017: R_386_PC32	printk
    101b:	e9 74 ff ff ff       	jmp    f94 <operation_replace+0xa4>
    1020:	8b 55 f0             	mov    -0x10(%ebp),%edx
    1023:	8d 46 04             	lea    0x4(%esi),%eax
    1026:	39 02                	cmp    %eax,(%edx)
    1028:	75 21                	jne    104b <operation_replace+0x15b>
    102a:	8b 06                	mov    (%esi),%eax
    102c:	89 02                	mov    %eax,(%edx)
    102e:	e9 5a ff ff ff       	jmp    f8d <operation_replace+0x9d>
    1033:	89 f0                	mov    %esi,%eax
    1035:	e8 fc ff ff ff       	call   1036 <operation_replace+0x146>
			1036: R_386_PC32	kfree
    103a:	c7 04 24 30 06 00 00 	movl   $0x630,(%esp)
			103d: R_386_32	.rodata.str1.4
    1041:	e8 fc ff ff ff       	call   1042 <operation_replace+0x152>
			1042: R_386_PC32	printk
    1046:	e9 49 ff ff ff       	jmp    f94 <operation_replace+0xa4>
    104b:	8b 45 f0             	mov    -0x10(%ebp),%eax
    104e:	c7 04 24 d8 03 00 00 	movl   $0x3d8,(%esp)
			1051: R_386_32	.rodata.str1.4
    1055:	89 44 24 04          	mov    %eax,0x4(%esp)
    1059:	e8 fc ff ff ff       	call   105a <operation_replace+0x16a>
			105a: R_386_PC32	printk
    105e:	e9 2a ff ff ff       	jmp    f8d <operation_replace+0x9d>
    1063:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1069:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001070 <operation_replacement_update>:
operation_replacement_update():
    1070:	55                   	push   %ebp
    1071:	89 e5                	mov    %esp,%ebp
    1073:	57                   	push   %edi
    1074:	56                   	push   %esi
    1075:	53                   	push   %ebx
    1076:	83 ec 18             	sub    $0x18,%esp
    1079:	e8 fc ff ff ff       	call   107a <operation_replacement_update+0xa>
			107a: R_386_PC32	mcount
    107e:	be f4 ff ff ff       	mov    $0xfffffff4,%esi
    1083:	89 55 f0             	mov    %edx,-0x10(%ebp)
    1086:	89 c3                	mov    %eax,%ebx
    1088:	83 78 4c 00          	cmpl   $0x0,0x4c(%eax)
    108c:	74 68                	je     10f6 <operation_replacement_update+0x86>
    108e:	8b 00                	mov    (%eax),%eax
    1090:	e8 fc ff ff ff       	call   1091 <operation_replacement_update+0x21>
			1091: R_386_PC32	data_map_get
    1095:	3d 00 f0 ff ff       	cmp    $0xfffff000,%eax
    109a:	89 c7                	mov    %eax,%edi
    109c:	0f 87 d6 00 00 00    	ja     1178 <operation_replacement_update+0x108>
    10a2:	85 c0                	test   %eax,%eax
    10a4:	0f 84 ca 00 00 00    	je     1174 <operation_replacement_update+0x104>
    10aa:	8b 43 4c             	mov    0x4c(%ebx),%eax
    10ad:	8b 50 04             	mov    0x4(%eax),%edx
    10b0:	89 55 e8             	mov    %edx,-0x18(%ebp)
    10b3:	8b 40 08             	mov    0x8(%eax),%eax
    10b6:	89 45 ec             	mov    %eax,-0x14(%ebp)
    10b9:	8b 43 38             	mov    0x38(%ebx),%eax
    10bc:	85 c0                	test   %eax,%eax
    10be:	75 40                	jne    1100 <operation_replacement_update+0x90>
    10c0:	8b 55 f0             	mov    -0x10(%ebp),%edx
    10c3:	31 f6                	xor    %esi,%esi
    10c5:	8b 4b 34             	mov    0x34(%ebx),%ecx
    10c8:	8b 02                	mov    (%edx),%eax
    10ca:	8d 57 04             	lea    0x4(%edi),%edx
    10cd:	39 d0                	cmp    %edx,%eax
    10cf:	74 25                	je     10f6 <operation_replacement_update+0x86>
    10d1:	3b 07                	cmp    (%edi),%eax
    10d3:	0f 84 94 00 00 00    	je     116d <operation_replacement_update+0xfd>
    10d9:	8b 55 ec             	mov    -0x14(%ebp),%edx
    10dc:	89 f8                	mov    %edi,%eax
    10de:	89 4c 24 04          	mov    %ecx,0x4(%esp)
    10e2:	8b 4d e8             	mov    -0x18(%ebp),%ecx
    10e5:	89 14 24             	mov    %edx,(%esp)
    10e8:	8b 55 f0             	mov    -0x10(%ebp),%edx
    10eb:	e8 80 f4 ff ff       	call   570 <operation_replace_replace_pointer>
    10f0:	89 c6                	mov    %eax,%esi
    10f2:	85 f6                	test   %esi,%esi
    10f4:	75 46                	jne    113c <operation_replacement_update+0xcc>
    10f6:	83 c4 18             	add    $0x18,%esp
    10f9:	89 f0                	mov    %esi,%eax
    10fb:	5b                   	pop    %ebx
    10fc:	5e                   	pop    %esi
    10fd:	5f                   	pop    %edi
    10fe:	5d                   	pop    %ebp
    10ff:	c3                   	ret    
    1100:	83 f8 01             	cmp    $0x1,%eax
    1103:	74 14                	je     1119 <operation_replacement_update+0xa9>
    1105:	89 44 24 04          	mov    %eax,0x4(%esp)
    1109:	c7 04 24 bc 05 00 00 	movl   $0x5bc,(%esp)
			110c: R_386_32	.rodata.str1.4
    1110:	e8 fc ff ff ff       	call   1111 <operation_replacement_update+0xa1>
			1111: R_386_PC32	printk
    1115:	0f 0b                	ud2a   
    1117:	eb fe                	jmp    1117 <operation_replacement_update+0xa7>
    1119:	8b 55 ec             	mov    -0x14(%ebp),%edx
    111c:	8b 43 34             	mov    0x34(%ebx),%eax
    111f:	8b 4d e8             	mov    -0x18(%ebp),%ecx
    1122:	89 14 24             	mov    %edx,(%esp)
    1125:	8b 55 f0             	mov    -0x10(%ebp),%edx
    1128:	89 45 e4             	mov    %eax,-0x1c(%ebp)
    112b:	89 44 24 04          	mov    %eax,0x4(%esp)
    112f:	89 f8                	mov    %edi,%eax
    1131:	e8 da f0 ff ff       	call   210 <operation_restore_at_place>
    1136:	85 c0                	test   %eax,%eax
    1138:	89 c6                	mov    %eax,%esi
    113a:	74 13                	je     114f <operation_replacement_update+0xdf>
    113c:	8b 55 f0             	mov    -0x10(%ebp),%edx
    113f:	8b 03                	mov    (%ebx),%eax
    1141:	e8 fc ff ff ff       	call   1142 <operation_replacement_update+0xd2>
			1142: R_386_PC32	data_map_delete
    1146:	89 f8                	mov    %edi,%eax
    1148:	e8 fc ff ff ff       	call   1149 <operation_replacement_update+0xd9>
			1149: R_386_PC32	kfree
    114d:	eb a7                	jmp    10f6 <operation_replacement_update+0x86>
    114f:	8b 55 ec             	mov    -0x14(%ebp),%edx
    1152:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    1155:	8b 4d e8             	mov    -0x18(%ebp),%ecx
    1158:	89 14 24             	mov    %edx,(%esp)
    115b:	8b 55 f0             	mov    -0x10(%ebp),%edx
    115e:	89 44 24 04          	mov    %eax,0x4(%esp)
    1162:	89 f8                	mov    %edi,%eax
    1164:	e8 07 f5 ff ff       	call   670 <operation_replace_at_place>
    1169:	89 c6                	mov    %eax,%esi
    116b:	eb 85                	jmp    10f2 <operation_replacement_update+0x82>
    116d:	8b 45 f0             	mov    -0x10(%ebp),%eax
    1170:	89 10                	mov    %edx,(%eax)
    1172:	eb 82                	jmp    10f6 <operation_replacement_update+0x86>
    1174:	0f 0b                	ud2a   
    1176:	eb fe                	jmp    1176 <operation_replacement_update+0x106>
    1178:	8b 55 f0             	mov    -0x10(%ebp),%edx
    117b:	89 d8                	mov    %ebx,%eax
    117d:	e8 fc ff ff ff       	call   117e <operation_replacement_update+0x10e>
			117e: R_386_PC32	operation_replace
    1182:	89 c6                	mov    %eax,%esi
    1184:	e9 6d ff ff ff       	jmp    10f6 <operation_replacement_update+0x86>
    1189:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00001190 <operation_replacer_create_common>:
operation_replacer_create_common():
    1190:	55                   	push   %ebp
    1191:	89 e5                	mov    %esp,%ebp
    1193:	57                   	push   %edi
    1194:	56                   	push   %esi
    1195:	53                   	push   %ebx
    1196:	83 ec 14             	sub    $0x14,%esp
    1199:	e8 fc ff ff ff       	call   119a <operation_replacer_create_common+0xa>
			119a: R_386_PC32	mcount
    119e:	89 c7                	mov    %eax,%edi
    11a0:	89 d6                	mov    %edx,%esi
    11a2:	b8 a4 00 00 00       	mov    $0xa4,%eax
			11a3: R_386_32	kmalloc_caches
    11a7:	ba d0 00 00 00       	mov    $0xd0,%edx
    11ac:	e8 fc ff ff ff       	call   11ad <operation_replacer_create_common+0x1d>
			11ad: R_386_PC32	kmem_cache_alloc
    11b1:	8b 15 04 00 00 00    	mov    0x4,%edx
			11b3: R_386_32	__tracepoint_kmalloc
    11b7:	85 d2                	test   %edx,%edx
    11b9:	89 c3                	mov    %eax,%ebx
    11bb:	a1 a8 00 00 00       	mov    0xa8,%eax
			11bc: R_386_32	kmalloc_caches
    11c0:	89 45 f0             	mov    %eax,-0x10(%ebp)
    11c3:	0f 85 89 00 00 00    	jne    1252 <operation_replacer_create_common+0xc2>
    11c9:	85 db                	test   %ebx,%ebx
    11cb:	0f 84 01 01 00 00    	je     12d2 <operation_replacer_create_common+0x142>
    11d1:	89 f8                	mov    %edi,%eax
    11d3:	e8 fc ff ff ff       	call   11d4 <operation_replacer_create_common+0x44>
			11d4: R_386_PC32	data_map_create
    11d8:	85 c0                	test   %eax,%eax
    11da:	89 03                	mov    %eax,(%ebx)
    11dc:	0f 84 d6 00 00 00    	je     12b8 <operation_replacer_create_common+0x128>
    11e2:	ba d0 00 00 00       	mov    $0xd0,%edx
    11e7:	89 f0                	mov    %esi,%eax
    11e9:	e8 fc ff ff ff       	call   11ea <operation_replacer_create_common+0x5a>
			11ea: R_386_PC32	__kmalloc
    11ee:	85 c0                	test   %eax,%eax
    11f0:	89 43 14             	mov    %eax,0x14(%ebx)
    11f3:	0f 84 a1 00 00 00    	je     129a <operation_replacer_create_common+0x10a>
    11f9:	89 f1                	mov    %esi,%ecx
    11fb:	89 c7                	mov    %eax,%edi
    11fd:	c1 e9 02             	shr    $0x2,%ecx
    1200:	31 c0                	xor    %eax,%eax
    1202:	89 f2                	mov    %esi,%edx
    1204:	f3 ab                	rep stos %eax,%es:(%edi)
    1206:	f6 c2 02             	test   $0x2,%dl
    1209:	74 02                	je     120d <operation_replacer_create_common+0x7d>
    120b:	66 ab                	stos   %ax,%es:(%edi)
    120d:	f6 c2 01             	test   $0x1,%dl
    1210:	74 01                	je     1213 <operation_replacer_create_common+0x83>
    1212:	aa                   	stos   %al,%es:(%edi)
    1213:	8d 43 04             	lea    0x4(%ebx),%eax
    1216:	b9 04 00 00 00       	mov    $0x4,%ecx
			1217: R_386_32	.bss
    121b:	89 43 04             	mov    %eax,0x4(%ebx)
    121e:	ba 34 00 00 00       	mov    $0x34,%edx
			121f: R_386_32	.rodata.str1.1
    1223:	89 43 08             	mov    %eax,0x8(%ebx)
    1226:	8d 43 0c             	lea    0xc(%ebx),%eax
    1229:	89 43 0c             	mov    %eax,0xc(%ebx)
    122c:	89 43 10             	mov    %eax,0x10(%ebx)
    122f:	8d 43 18             	lea    0x18(%ebx),%eax
    1232:	c7 43 2c 00 00 00 00 	movl   $0x0,0x2c(%ebx)
    1239:	c7 43 30 00 00 00 00 	movl   $0x0,0x30(%ebx)
    1240:	e8 fc ff ff ff       	call   1241 <operation_replacer_create_common+0xb1>
			1241: R_386_PC32	__mutex_init
    1245:	89 73 34             	mov    %esi,0x34(%ebx)
    1248:	83 c4 14             	add    $0x14,%esp
    124b:	89 d8                	mov    %ebx,%eax
    124d:	5b                   	pop    %ebx
    124e:	5e                   	pop    %esi
    124f:	5f                   	pop    %edi
    1250:	5d                   	pop    %ebp
    1251:	c3                   	ret    
    1252:	8b 15 10 00 00 00    	mov    0x10,%edx
			1254: R_386_32	__tracepoint_kmalloc
    1258:	85 d2                	test   %edx,%edx
    125a:	89 55 ec             	mov    %edx,-0x14(%ebp)
    125d:	0f 84 66 ff ff ff    	je     11c9 <operation_replacer_create_common+0x39>
    1263:	8b 02                	mov    (%edx),%eax
    1265:	89 45 e8             	mov    %eax,-0x18(%ebp)
    1268:	8b 55 f0             	mov    -0x10(%ebp),%edx
    126b:	b8 90 11 00 00       	mov    $0x1190,%eax
			126c: R_386_32	.text
    1270:	b9 50 00 00 00       	mov    $0x50,%ecx
    1275:	c7 44 24 04 d0 00 00 	movl   $0xd0,0x4(%esp)
    127c:	00 
    127d:	89 14 24             	mov    %edx,(%esp)
    1280:	89 da                	mov    %ebx,%edx
    1282:	ff 55 e8             	call   *-0x18(%ebp)
    1285:	83 45 ec 04          	addl   $0x4,-0x14(%ebp)
    1289:	8b 45 ec             	mov    -0x14(%ebp),%eax
    128c:	8b 00                	mov    (%eax),%eax
    128e:	85 c0                	test   %eax,%eax
    1290:	89 45 e8             	mov    %eax,-0x18(%ebp)
    1293:	75 d3                	jne    1268 <operation_replacer_create_common+0xd8>
    1295:	e9 2f ff ff ff       	jmp    11c9 <operation_replacer_create_common+0x39>
    129a:	c7 04 24 4c 07 00 00 	movl   $0x74c,(%esp)
			129d: R_386_32	.rodata.str1.4
    12a1:	e8 fc ff ff ff       	call   12a2 <operation_replacer_create_common+0x112>
			12a2: R_386_PC32	printk
    12a6:	8b 03                	mov    (%ebx),%eax
    12a8:	e8 fc ff ff ff       	call   12a9 <operation_replacer_create_common+0x119>
			12a9: R_386_PC32	data_map_destroy
    12ad:	89 d8                	mov    %ebx,%eax
    12af:	31 db                	xor    %ebx,%ebx
    12b1:	e8 fc ff ff ff       	call   12b2 <operation_replacer_create_common+0x122>
			12b2: R_386_PC32	kfree
    12b6:	eb 90                	jmp    1248 <operation_replacer_create_common+0xb8>
    12b8:	c7 04 24 00 07 00 00 	movl   $0x700,(%esp)
			12bb: R_386_32	.rodata.str1.4
    12bf:	e8 fc ff ff ff       	call   12c0 <operation_replacer_create_common+0x130>
			12c0: R_386_PC32	printk
    12c4:	89 d8                	mov    %ebx,%eax
    12c6:	31 db                	xor    %ebx,%ebx
    12c8:	e8 fc ff ff ff       	call   12c9 <operation_replacer_create_common+0x139>
			12c9: R_386_PC32	kfree
    12cd:	e9 76 ff ff ff       	jmp    1248 <operation_replacer_create_common+0xb8>
    12d2:	c7 04 24 b4 06 00 00 	movl   $0x6b4,(%esp)
			12d5: R_386_32	.rodata.str1.4
    12d9:	e8 fc ff ff ff       	call   12da <operation_replacer_create_common+0x14a>
			12da: R_386_PC32	printk
    12de:	66 90                	xchg   %ax,%ax
    12e0:	e9 63 ff ff ff       	jmp    1248 <operation_replacer_create_common+0xb8>
    12e5:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    12e9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000012f0 <operation_replacer_create_at_place>:
operation_replacer_create_at_place():
    12f0:	55                   	push   %ebp
    12f1:	89 e5                	mov    %esp,%ebp
    12f3:	e8 fc ff ff ff       	call   12f4 <operation_replacer_create_at_place+0x4>
			12f4: R_386_PC32	mcount
    12f8:	e8 93 fe ff ff       	call   1190 <operation_replacer_create_common>
    12fd:	85 c0                	test   %eax,%eax
    12ff:	74 07                	je     1308 <operation_replacer_create_at_place+0x18>
    1301:	c7 40 38 01 00 00 00 	movl   $0x1,0x38(%eax)
    1308:	5d                   	pop    %ebp
    1309:	c3                   	ret    
    130a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

00001310 <operation_replacer_create>:
operation_replacer_create():
    1310:	55                   	push   %ebp
    1311:	89 e5                	mov    %esp,%ebp
    1313:	e8 fc ff ff ff       	call   1314 <operation_replacer_create+0x4>
			1314: R_386_PC32	mcount
    1318:	e8 73 fe ff ff       	call   1190 <operation_replacer_create_common>
    131d:	85 c0                	test   %eax,%eax
    131f:	74 07                	je     1328 <operation_replacer_create+0x18>
    1321:	c7 40 38 00 00 00 00 	movl   $0x0,0x38(%eax)
    1328:	5d                   	pop    %ebp
    1329:	c3                   	ret    
    132a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

00001330 <operation_target_load_callback>:
operation_target_load_callback():
    1330:	55                   	push   %ebp
    1331:	89 e5                	mov    %esp,%ebp
    1333:	57                   	push   %edi
    1334:	56                   	push   %esi
    1335:	53                   	push   %ebx
    1336:	83 ec 3c             	sub    $0x3c,%esp
    1339:	e8 fc ff ff ff       	call   133a <operation_target_load_callback+0xa>
			133a: R_386_PC32	mcount
    133e:	89 45 e4             	mov    %eax,-0x1c(%ebp)
    1341:	89 55 c8             	mov    %edx,-0x38(%ebp)
    1344:	8b 78 30             	mov    0x30(%eax),%edi
    1347:	85 ff                	test   %edi,%edi
    1349:	75 22                	jne    136d <operation_target_load_callback+0x3d>
    134b:	c7 40 30 01 00 00 00 	movl   $0x1,0x30(%eax)
    1352:	b8 01 00 00 00       	mov    $0x1,%eax
    1357:	8b 55 e4             	mov    -0x1c(%ebp),%edx
    135a:	f0 0f c1 42 2c       	lock xadd %eax,0x2c(%edx)
    135f:	85 c0                	test   %eax,%eax
    1361:	74 12                	je     1375 <operation_target_load_callback+0x45>
    1363:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
    1366:	c7 43 30 00 00 00 00 	movl   $0x0,0x30(%ebx)
    136d:	83 c4 3c             	add    $0x3c,%esp
    1370:	5b                   	pop    %ebx
    1371:	5e                   	pop    %esi
    1372:	5f                   	pop    %edi
    1373:	5d                   	pop    %ebp
    1374:	c3                   	ret    
    1375:	89 d7                	mov    %edx,%edi
    1377:	83 c7 18             	add    $0x18,%edi
    137a:	89 f8                	mov    %edi,%eax
    137c:	e8 fc ff ff ff       	call   137d <operation_target_load_callback+0x4d>
			137d: R_386_PC32	mutex_lock
    1381:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
    1384:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
    1387:	83 c1 3c             	add    $0x3c,%ecx
    138a:	89 4d d8             	mov    %ecx,-0x28(%ebp)
    138d:	89 4b 3c             	mov    %ecx,0x3c(%ebx)
    1390:	89 4b 40             	mov    %ecx,0x40(%ebx)
    1393:	8b 75 e4             	mov    -0x1c(%ebp),%esi
    1396:	8b 5e 04             	mov    0x4(%esi),%ebx
    1399:	8b 03                	mov    (%ebx),%eax
    139b:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    139f:	83 c6 04             	add    $0x4,%esi
    13a2:	39 f3                	cmp    %esi,%ebx
    13a4:	74 31                	je     13d7 <operation_target_load_callback+0xa7>
    13a6:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    13a9:	89 da                	mov    %ebx,%edx
    13ab:	e8 a0 ed ff ff       	call   150 <fix_payload>
    13b0:	85 c0                	test   %eax,%eax
    13b2:	75 17                	jne    13cb <operation_target_load_callback+0x9b>
    13b4:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
    13b7:	8d 53 08             	lea    0x8(%ebx),%edx
    13ba:	8b 41 40             	mov    0x40(%ecx),%eax
    13bd:	89 51 40             	mov    %edx,0x40(%ecx)
    13c0:	8b 4d d8             	mov    -0x28(%ebp),%ecx
    13c3:	89 43 0c             	mov    %eax,0xc(%ebx)
    13c6:	89 4b 08             	mov    %ecx,0x8(%ebx)
    13c9:	89 10                	mov    %edx,(%eax)
    13cb:	8b 1b                	mov    (%ebx),%ebx
    13cd:	8b 03                	mov    (%ebx),%eax
    13cf:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    13d3:	39 f3                	cmp    %esi,%ebx
    13d5:	75 cf                	jne    13a6 <operation_target_load_callback+0x76>
    13d7:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
    13da:	8b 75 e4             	mov    -0x1c(%ebp),%esi
    13dd:	83 c3 44             	add    $0x44,%ebx
    13e0:	89 5d dc             	mov    %ebx,-0x24(%ebp)
    13e3:	89 5e 44             	mov    %ebx,0x44(%esi)
    13e6:	89 5e 48             	mov    %ebx,0x48(%esi)
    13e9:	8b 5e 0c             	mov    0xc(%esi),%ebx
    13ec:	8b 03                	mov    (%ebx),%eax
    13ee:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    13f2:	8b 75 e4             	mov    -0x1c(%ebp),%esi
    13f5:	83 c6 0c             	add    $0xc,%esi
    13f8:	39 f3                	cmp    %esi,%ebx
    13fa:	74 31                	je     142d <operation_target_load_callback+0xfd>
    13fc:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    13ff:	89 da                	mov    %ebx,%edx
    1401:	e8 4a ed ff ff       	call   150 <fix_payload>
    1406:	85 c0                	test   %eax,%eax
    1408:	75 17                	jne    1421 <operation_target_load_callback+0xf1>
    140a:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
    140d:	8d 53 08             	lea    0x8(%ebx),%edx
    1410:	8b 41 48             	mov    0x48(%ecx),%eax
    1413:	89 51 48             	mov    %edx,0x48(%ecx)
    1416:	8b 4d dc             	mov    -0x24(%ebp),%ecx
    1419:	89 43 0c             	mov    %eax,0xc(%ebx)
    141c:	89 4b 08             	mov    %ecx,0x8(%ebx)
    141f:	89 10                	mov    %edx,(%eax)
    1421:	8b 1b                	mov    (%ebx),%ebx
    1423:	8b 03                	mov    (%ebx),%eax
    1425:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1429:	39 f3                	cmp    %esi,%ebx
    142b:	75 cf                	jne    13fc <operation_target_load_callback+0xcc>
    142d:	89 f8                	mov    %edi,%eax
    142f:	e8 fc ff ff ff       	call   1430 <operation_target_load_callback+0x100>
			1430: R_386_PC32	mutex_unlock
    1434:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
    1437:	8b 75 e4             	mov    -0x1c(%ebp),%esi
    143a:	8b 5b 34             	mov    0x34(%ebx),%ebx
    143d:	89 5d e0             	mov    %ebx,-0x20(%ebp)
    1440:	8b 56 44             	mov    0x44(%esi),%edx
    1443:	8b 02                	mov    (%edx),%eax
    1445:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1449:	39 55 dc             	cmp    %edx,-0x24(%ebp)
    144c:	c7 45 ec 00 00 00 00 	movl   $0x0,-0x14(%ebp)
    1453:	74 1e                	je     1473 <operation_target_load_callback+0x143>
    1455:	31 c9                	xor    %ecx,%ecx
    1457:	eb 09                	jmp    1462 <operation_target_load_callback+0x132>
    1459:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    1460:	89 d0                	mov    %edx,%eax
    1462:	83 c1 01             	add    $0x1,%ecx
    1465:	8b 10                	mov    (%eax),%edx
    1467:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    146b:	39 45 dc             	cmp    %eax,-0x24(%ebp)
    146e:	75 f0                	jne    1460 <operation_target_load_callback+0x130>
    1470:	89 4d ec             	mov    %ecx,-0x14(%ebp)
    1473:	ba d0 00 00 00       	mov    $0xd0,%edx
    1478:	b8 90 02 00 00       	mov    $0x290,%eax
			1479: R_386_32	kmalloc_caches
    147d:	e8 fc ff ff ff       	call   147e <operation_target_load_callback+0x14e>
			147e: R_386_PC32	kmem_cache_alloc
    1482:	83 3d 04 00 00 00 00 	cmpl   $0x0,0x4
			1484: R_386_32	__tracepoint_kmalloc
    1489:	89 c3                	mov    %eax,%ebx
    148b:	a1 94 02 00 00       	mov    0x294,%eax
			148c: R_386_32	kmalloc_caches
    1490:	89 45 f0             	mov    %eax,-0x10(%ebp)
    1493:	0f 85 3e 04 00 00    	jne    18d7 <operation_target_load_callback+0x5a7>
    1499:	85 db                	test   %ebx,%ebx
    149b:	0f 84 26 02 00 00    	je     16c7 <operation_target_load_callback+0x397>
    14a1:	8b 4d ec             	mov    -0x14(%ebp),%ecx
    14a4:	ba d0 00 00 00       	mov    $0xd0,%edx
    14a9:	89 0b                	mov    %ecx,(%ebx)
    14ab:	8b 45 e0             	mov    -0x20(%ebp),%eax
    14ae:	e8 fc ff ff ff       	call   14af <operation_target_load_callback+0x17f>
			14af: R_386_PC32	__kmalloc
    14b3:	85 c0                	test   %eax,%eax
    14b5:	89 43 04             	mov    %eax,0x4(%ebx)
    14b8:	0f 84 7e 04 00 00    	je     193c <operation_target_load_callback+0x60c>
    14be:	8b 45 e0             	mov    -0x20(%ebp),%eax
    14c1:	ba d0 00 00 00       	mov    $0xd0,%edx
    14c6:	e8 fc ff ff ff       	call   14c7 <operation_target_load_callback+0x197>
			14c7: R_386_PC32	__kmalloc
    14cb:	85 c0                	test   %eax,%eax
    14cd:	89 43 08             	mov    %eax,0x8(%ebx)
    14d0:	0f 84 46 04 00 00    	je     191c <operation_target_load_callback+0x5ec>
    14d6:	6b 45 ec 0c          	imul   $0xc,-0x14(%ebp),%eax
    14da:	ba d0 00 00 00       	mov    $0xd0,%edx
    14df:	e8 fc ff ff ff       	call   14e0 <operation_target_load_callback+0x1b0>
			14e0: R_386_PC32	__kmalloc
    14e4:	85 c0                	test   %eax,%eax
    14e6:	89 43 0c             	mov    %eax,0xc(%ebx)
    14e9:	0f 84 5b 04 00 00    	je     194a <operation_target_load_callback+0x61a>
    14ef:	83 7d ec 00          	cmpl   $0x0,-0x14(%ebp)
    14f3:	74 4d                	je     1542 <operation_target_load_callback+0x212>
    14f5:	31 ff                	xor    %edi,%edi
    14f7:	c7 45 f0 00 00 00 00 	movl   $0x0,-0x10(%ebp)
    14fe:	8b 45 e0             	mov    -0x20(%ebp),%eax
    1501:	ba d0 00 00 00       	mov    $0xd0,%edx
    1506:	8b 73 0c             	mov    0xc(%ebx),%esi
    1509:	e8 fc ff ff ff       	call   150a <operation_target_load_callback+0x1da>
			150a: R_386_PC32	__kmalloc
    150e:	01 fe                	add    %edi,%esi
    1510:	85 c0                	test   %eax,%eax
    1512:	89 46 04             	mov    %eax,0x4(%esi)
    1515:	0f 84 ae 03 00 00    	je     18c9 <operation_target_load_callback+0x599>
    151b:	8b 45 e0             	mov    -0x20(%ebp),%eax
    151e:	ba d0 00 00 00       	mov    $0xd0,%edx
    1523:	e8 fc ff ff ff       	call   1524 <operation_target_load_callback+0x1f4>
			1524: R_386_PC32	__kmalloc
    1528:	85 c0                	test   %eax,%eax
    152a:	89 46 08             	mov    %eax,0x8(%esi)
    152d:	0f 84 3d 03 00 00    	je     1870 <operation_target_load_callback+0x540>
    1533:	83 45 f0 01          	addl   $0x1,-0x10(%ebp)
    1537:	83 c7 0c             	add    $0xc,%edi
    153a:	8b 75 f0             	mov    -0x10(%ebp),%esi
    153d:	39 75 ec             	cmp    %esi,-0x14(%ebp)
    1540:	7f bc                	jg     14fe <operation_target_load_callback+0x1ce>
    1542:	8b 45 e0             	mov    -0x20(%ebp),%eax
    1545:	8b 55 e0             	mov    -0x20(%ebp),%edx
    1548:	c1 e8 02             	shr    $0x2,%eax
    154b:	89 45 d0             	mov    %eax,-0x30(%ebp)
    154e:	31 c0                	xor    %eax,%eax
    1550:	8b 7b 04             	mov    0x4(%ebx),%edi
    1553:	8b 4d d0             	mov    -0x30(%ebp),%ecx
    1556:	f3 ab                	rep stos %eax,%es:(%edi)
    1558:	f6 c2 02             	test   $0x2,%dl
    155b:	74 02                	je     155f <operation_target_load_callback+0x22f>
    155d:	66 ab                	stos   %ax,%es:(%edi)
    155f:	f6 c2 01             	test   $0x1,%dl
    1562:	74 01                	je     1565 <operation_target_load_callback+0x235>
    1564:	aa                   	stos   %al,%es:(%edi)
    1565:	8b 7b 08             	mov    0x8(%ebx),%edi
    1568:	8b 4d d0             	mov    -0x30(%ebp),%ecx
    156b:	f3 ab                	rep stos %eax,%es:(%edi)
    156d:	f6 c2 02             	test   $0x2,%dl
    1570:	74 02                	je     1574 <operation_target_load_callback+0x244>
    1572:	66 ab                	stos   %ax,%es:(%edi)
    1574:	f6 c2 01             	test   $0x1,%dl
    1577:	74 01                	je     157a <operation_target_load_callback+0x24a>
    1579:	aa                   	stos   %al,%es:(%edi)
    157a:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
    157d:	8b 41 3c             	mov    0x3c(%ecx),%eax
    1580:	8d 70 f8             	lea    -0x8(%eax),%esi
    1583:	89 75 e8             	mov    %esi,-0x18(%ebp)
    1586:	8b 4e 08             	mov    0x8(%esi),%ecx
    1589:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    158d:	39 45 d8             	cmp    %eax,-0x28(%ebp)
    1590:	74 52                	je     15e4 <operation_target_load_callback+0x2b4>
    1592:	8b 75 e8             	mov    -0x18(%ebp),%esi
    1595:	8b 46 14             	mov    0x14(%esi),%eax
    1598:	8b 50 08             	mov    0x8(%eax),%edx
    159b:	89 55 ec             	mov    %edx,-0x14(%ebp)
    159e:	8b 78 04             	mov    0x4(%eax),%edi
    15a1:	89 d0                	mov    %edx,%eax
    15a3:	03 55 e0             	add    -0x20(%ebp),%edx
    15a6:	39 c2                	cmp    %eax,%edx
    15a8:	76 26                	jbe    15d0 <operation_target_load_callback+0x2a0>
    15aa:	8b 30                	mov    (%eax),%esi
    15ac:	85 f6                	test   %esi,%esi
    15ae:	74 12                	je     15c2 <operation_target_load_callback+0x292>
    15b0:	e9 63 01 00 00       	jmp    1718 <operation_target_load_callback+0x3e8>
    15b5:	8d 76 00             	lea    0x0(%esi),%esi
    15b8:	8b 30                	mov    (%eax),%esi
    15ba:	85 f6                	test   %esi,%esi
    15bc:	0f 85 56 01 00 00    	jne    1718 <operation_target_load_callback+0x3e8>
    15c2:	83 c0 04             	add    $0x4,%eax
    15c5:	39 d0                	cmp    %edx,%eax
    15c7:	72 ef                	jb     15b8 <operation_target_load_callback+0x288>
    15c9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    15d0:	89 c8                	mov    %ecx,%eax
    15d2:	8d 49 f8             	lea    -0x8(%ecx),%ecx
    15d5:	89 4d e8             	mov    %ecx,-0x18(%ebp)
    15d8:	8b 49 08             	mov    0x8(%ecx),%ecx
    15db:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    15df:	39 45 d8             	cmp    %eax,-0x28(%ebp)
    15e2:	75 ae                	jne    1592 <operation_target_load_callback+0x262>
    15e4:	83 3b 00             	cmpl   $0x0,(%ebx)
    15e7:	0f 84 fc 00 00 00    	je     16e9 <operation_target_load_callback+0x3b9>
    15ed:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    15f0:	8b 50 44             	mov    0x44(%eax),%edx
    15f3:	8d 4a f8             	lea    -0x8(%edx),%ecx
    15f6:	89 4d d4             	mov    %ecx,-0x2c(%ebp)
    15f9:	8b 41 08             	mov    0x8(%ecx),%eax
    15fc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1600:	39 55 dc             	cmp    %edx,-0x24(%ebp)
    1603:	0f 84 5c 02 00 00    	je     1865 <operation_target_load_callback+0x535>
    1609:	c7 45 cc 00 00 00 00 	movl   $0x0,-0x34(%ebp)
    1610:	c7 45 e8 00 00 00 00 	movl   $0x0,-0x18(%ebp)
    1617:	89 5d c4             	mov    %ebx,-0x3c(%ebp)
    161a:	8b 5d d4             	mov    -0x2c(%ebp),%ebx
    161d:	8b 55 cc             	mov    -0x34(%ebp),%edx
    1620:	8b 43 14             	mov    0x14(%ebx),%eax
    1623:	8b 70 08             	mov    0x8(%eax),%esi
    1626:	89 75 ec             	mov    %esi,-0x14(%ebp)
    1629:	8b 40 04             	mov    0x4(%eax),%eax
    162c:	8b 75 e8             	mov    -0x18(%ebp),%esi
    162f:	89 45 f0             	mov    %eax,-0x10(%ebp)
    1632:	8b 45 c4             	mov    -0x3c(%ebp),%eax
    1635:	03 50 0c             	add    0xc(%eax),%edx
    1638:	85 f6                	test   %esi,%esi
    163a:	0f 84 ed 01 00 00    	je     182d <operation_target_load_callback+0x4fd>
    1640:	8d 42 f4             	lea    -0xc(%edx),%eax
    1643:	8b 7a 08             	mov    0x8(%edx),%edi
    1646:	8b 70 08             	mov    0x8(%eax),%esi
    1649:	8b 4d d0             	mov    -0x30(%ebp),%ecx
    164c:	f3 a5                	rep movsl %ds:(%esi),%es:(%edi)
    164e:	8b 4d e0             	mov    -0x20(%ebp),%ecx
    1651:	83 e1 03             	and    $0x3,%ecx
    1654:	74 02                	je     1658 <operation_target_load_callback+0x328>
    1656:	f3 a4                	rep movsb %ds:(%esi),%es:(%edi)
    1658:	8b 7a 04             	mov    0x4(%edx),%edi
    165b:	8b 70 04             	mov    0x4(%eax),%esi
    165e:	8b 4d d0             	mov    -0x30(%ebp),%ecx
    1661:	f3 a5                	rep movsl %ds:(%esi),%es:(%edi)
    1663:	8b 4d e0             	mov    -0x20(%ebp),%ecx
    1666:	83 e1 03             	and    $0x3,%ecx
    1669:	74 02                	je     166d <operation_target_load_callback+0x33d>
    166b:	f3 a4                	rep movsb %ds:(%esi),%es:(%edi)
    166d:	8b 45 ec             	mov    -0x14(%ebp),%eax
    1670:	8b 4d e0             	mov    -0x20(%ebp),%ecx
    1673:	01 c1                	add    %eax,%ecx
    1675:	39 c1                	cmp    %eax,%ecx
    1677:	76 20                	jbe    1699 <operation_target_load_callback+0x369>
    1679:	8b 18                	mov    (%eax),%ebx
    167b:	85 db                	test   %ebx,%ebx
    167d:	74 13                	je     1692 <operation_target_load_callback+0x362>
    167f:	e9 46 01 00 00       	jmp    17ca <operation_target_load_callback+0x49a>
    1684:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1688:	8b 38                	mov    (%eax),%edi
    168a:	85 ff                	test   %edi,%edi
    168c:	0f 85 38 01 00 00    	jne    17ca <operation_target_load_callback+0x49a>
    1692:	83 c0 04             	add    $0x4,%eax
    1695:	39 c8                	cmp    %ecx,%eax
    1697:	72 ef                	jb     1688 <operation_target_load_callback+0x358>
    1699:	8b 4d d4             	mov    -0x2c(%ebp),%ecx
    169c:	8b 41 14             	mov    0x14(%ecx),%eax
    169f:	89 02                	mov    %eax,(%edx)
    16a1:	8b 41 08             	mov    0x8(%ecx),%eax
    16a4:	8d 58 f8             	lea    -0x8(%eax),%ebx
    16a7:	89 5d d4             	mov    %ebx,-0x2c(%ebp)
    16aa:	8b 53 08             	mov    0x8(%ebx),%edx
    16ad:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    16b1:	83 45 cc 0c          	addl   $0xc,-0x34(%ebp)
    16b5:	39 45 dc             	cmp    %eax,-0x24(%ebp)
    16b8:	0f 84 a4 01 00 00    	je     1862 <operation_target_load_callback+0x532>
    16be:	83 45 e8 01          	addl   $0x1,-0x18(%ebp)
    16c2:	e9 53 ff ff ff       	jmp    161a <operation_target_load_callback+0x2ea>
    16c7:	c7 04 24 8c 07 00 00 	movl   $0x78c,(%esp)
			16ca: R_386_32	.rodata.str1.4
    16ce:	e8 fc ff ff ff       	call   16cf <operation_target_load_callback+0x39f>
			16cf: R_386_PC32	printk
    16d3:	c7 04 24 ec 08 00 00 	movl   $0x8ec,(%esp)
			16d6: R_386_32	.rodata.str1.4
    16da:	e8 fc ff ff ff       	call   16db <operation_target_load_callback+0x3ab>
			16db: R_386_PC32	printk
    16df:	8b 5d e4             	mov    -0x1c(%ebp),%ebx
    16e2:	c7 43 4c 00 00 00 00 	movl   $0x0,0x4c(%ebx)
    16e9:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    16ec:	8b 50 3c             	mov    0x3c(%eax),%edx
    16ef:	8d 5a f8             	lea    -0x8(%edx),%ebx
    16f2:	8b 43 08             	mov    0x8(%ebx),%eax
    16f5:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    16f9:	39 55 d8             	cmp    %edx,-0x28(%ebp)
    16fc:	0f 84 99 00 00 00    	je     179b <operation_target_load_callback+0x46b>
    1702:	8b 53 14             	mov    0x14(%ebx),%edx
    1705:	8b 52 0c             	mov    0xc(%edx),%edx
    1708:	85 d2                	test   %edx,%edx
    170a:	74 08                	je     1714 <operation_target_load_callback+0x3e4>
    170c:	8b 45 c8             	mov    -0x38(%ebp),%eax
    170f:	ff d2                	call   *%edx
    1711:	8b 43 08             	mov    0x8(%ebx),%eax
    1714:	89 c2                	mov    %eax,%edx
    1716:	eb d7                	jmp    16ef <operation_target_load_callback+0x3bf>
    1718:	85 c0                	test   %eax,%eax
    171a:	0f 84 b0 fe ff ff    	je     15d0 <operation_target_load_callback+0x2a0>
    1720:	8b 73 08             	mov    0x8(%ebx),%esi
    1723:	89 c1                	mov    %eax,%ecx
    1725:	2b 4d ec             	sub    -0x14(%ebp),%ecx
    1728:	01 ce                	add    %ecx,%esi
    172a:	83 3e 00             	cmpl   $0x0,(%esi)
    172d:	75 5c                	jne    178b <operation_target_load_callback+0x45b>
    172f:	89 55 f0             	mov    %edx,-0x10(%ebp)
    1732:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1738:	8b 10                	mov    (%eax),%edx
    173a:	83 c0 04             	add    $0x4,%eax
    173d:	89 16                	mov    %edx,(%esi)
    173f:	8b 34 0f             	mov    (%edi,%ecx,1),%esi
    1742:	8b 53 04             	mov    0x4(%ebx),%edx
    1745:	89 34 0a             	mov    %esi,(%edx,%ecx,1)
    1748:	3b 45 f0             	cmp    -0x10(%ebp),%eax
    174b:	73 43                	jae    1790 <operation_target_load_callback+0x460>
    174d:	8b 30                	mov    (%eax),%esi
    174f:	85 f6                	test   %esi,%esi
    1751:	75 20                	jne    1773 <operation_target_load_callback+0x443>
    1753:	8b 55 f0             	mov    -0x10(%ebp),%edx
    1756:	eb 06                	jmp    175e <operation_target_load_callback+0x42e>
    1758:	8b 08                	mov    (%eax),%ecx
    175a:	85 c9                	test   %ecx,%ecx
    175c:	75 12                	jne    1770 <operation_target_load_callback+0x440>
    175e:	83 c0 04             	add    $0x4,%eax
    1761:	39 d0                	cmp    %edx,%eax
    1763:	72 f3                	jb     1758 <operation_target_load_callback+0x428>
    1765:	8b 45 e8             	mov    -0x18(%ebp),%eax
    1768:	8b 48 08             	mov    0x8(%eax),%ecx
    176b:	e9 60 fe ff ff       	jmp    15d0 <operation_target_load_callback+0x2a0>
    1770:	89 55 f0             	mov    %edx,-0x10(%ebp)
    1773:	85 c0                	test   %eax,%eax
    1775:	0f 84 96 01 00 00    	je     1911 <operation_target_load_callback+0x5e1>
    177b:	8b 73 08             	mov    0x8(%ebx),%esi
    177e:	89 c1                	mov    %eax,%ecx
    1780:	2b 4d ec             	sub    -0x14(%ebp),%ecx
    1783:	01 ce                	add    %ecx,%esi
    1785:	8b 16                	mov    (%esi),%edx
    1787:	85 d2                	test   %edx,%edx
    1789:	74 ad                	je     1738 <operation_target_load_callback+0x408>
    178b:	0f 0b                	ud2a   
    178d:	eb fe                	jmp    178d <operation_target_load_callback+0x45d>
    178f:	90                   	nop
    1790:	8b 75 e8             	mov    -0x18(%ebp),%esi
    1793:	8b 4e 08             	mov    0x8(%esi),%ecx
    1796:	e9 35 fe ff ff       	jmp    15d0 <operation_target_load_callback+0x2a0>
    179b:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
    179e:	8b 51 44             	mov    0x44(%ecx),%edx
    17a1:	8d 5a f8             	lea    -0x8(%edx),%ebx
    17a4:	8b 43 08             	mov    0x8(%ebx),%eax
    17a7:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    17ab:	39 55 dc             	cmp    %edx,-0x24(%ebp)
    17ae:	0f 84 af fb ff ff    	je     1363 <operation_target_load_callback+0x33>
    17b4:	8b 53 14             	mov    0x14(%ebx),%edx
    17b7:	8b 52 0c             	mov    0xc(%edx),%edx
    17ba:	85 d2                	test   %edx,%edx
    17bc:	74 08                	je     17c6 <operation_target_load_callback+0x496>
    17be:	8b 45 c8             	mov    -0x38(%ebp),%eax
    17c1:	ff d2                	call   *%edx
    17c3:	8b 43 08             	mov    0x8(%ebx),%eax
    17c6:	89 c2                	mov    %eax,%edx
    17c8:	eb d7                	jmp    17a1 <operation_target_load_callback+0x471>
    17ca:	85 c0                	test   %eax,%eax
    17cc:	0f 84 c7 fe ff ff    	je     1699 <operation_target_load_callback+0x369>
    17d2:	8b 7d f0             	mov    -0x10(%ebp),%edi
    17d5:	89 4d f0             	mov    %ecx,-0x10(%ebp)
    17d8:	8b 30                	mov    (%eax),%esi
    17da:	89 c3                	mov    %eax,%ebx
    17dc:	83 c0 04             	add    $0x4,%eax
    17df:	8b 4a 04             	mov    0x4(%edx),%ecx
    17e2:	2b 5d ec             	sub    -0x14(%ebp),%ebx
    17e5:	89 34 19             	mov    %esi,(%ecx,%ebx,1)
    17e8:	8b 34 1f             	mov    (%edi,%ebx,1),%esi
    17eb:	8b 4a 08             	mov    0x8(%edx),%ecx
    17ee:	89 34 19             	mov    %esi,(%ecx,%ebx,1)
    17f1:	39 45 f0             	cmp    %eax,-0x10(%ebp)
    17f4:	0f 86 9f fe ff ff    	jbe    1699 <operation_target_load_callback+0x369>
    17fa:	8b 30                	mov    (%eax),%esi
    17fc:	85 f6                	test   %esi,%esi
    17fe:	75 1d                	jne    181d <operation_target_load_callback+0x4ed>
    1800:	8b 4d f0             	mov    -0x10(%ebp),%ecx
    1803:	eb 09                	jmp    180e <operation_target_load_callback+0x4de>
    1805:	8d 76 00             	lea    0x0(%esi),%esi
    1808:	8b 18                	mov    (%eax),%ebx
    180a:	85 db                	test   %ebx,%ebx
    180c:	75 0c                	jne    181a <operation_target_load_callback+0x4ea>
    180e:	83 c0 04             	add    $0x4,%eax
    1811:	39 c1                	cmp    %eax,%ecx
    1813:	77 f3                	ja     1808 <operation_target_load_callback+0x4d8>
    1815:	e9 7f fe ff ff       	jmp    1699 <operation_target_load_callback+0x369>
    181a:	89 4d f0             	mov    %ecx,-0x10(%ebp)
    181d:	85 c0                	test   %eax,%eax
    181f:	90                   	nop
    1820:	75 b6                	jne    17d8 <operation_target_load_callback+0x4a8>
    1822:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1828:	e9 6c fe ff ff       	jmp    1699 <operation_target_load_callback+0x369>
    182d:	8b 7a 08             	mov    0x8(%edx),%edi
    1830:	8b 4d d0             	mov    -0x30(%ebp),%ecx
    1833:	8b 45 e8             	mov    -0x18(%ebp),%eax
    1836:	8b 5d e0             	mov    -0x20(%ebp),%ebx
    1839:	f3 ab                	rep stos %eax,%es:(%edi)
    183b:	f6 c3 02             	test   $0x2,%bl
    183e:	74 02                	je     1842 <operation_target_load_callback+0x512>
    1840:	66 ab                	stos   %ax,%es:(%edi)
    1842:	f6 c3 01             	test   $0x1,%bl
    1845:	74 01                	je     1848 <operation_target_load_callback+0x518>
    1847:	aa                   	stos   %al,%es:(%edi)
    1848:	8b 7a 04             	mov    0x4(%edx),%edi
    184b:	8b 4d d0             	mov    -0x30(%ebp),%ecx
    184e:	f3 ab                	rep stos %eax,%es:(%edi)
    1850:	f6 c3 02             	test   $0x2,%bl
    1853:	74 02                	je     1857 <operation_target_load_callback+0x527>
    1855:	66 ab                	stos   %ax,%es:(%edi)
    1857:	f6 c3 01             	test   $0x1,%bl
    185a:	74 01                	je     185d <operation_target_load_callback+0x52d>
    185c:	aa                   	stos   %al,%es:(%edi)
    185d:	e9 0b fe ff ff       	jmp    166d <operation_target_load_callback+0x33d>
    1862:	8b 5d c4             	mov    -0x3c(%ebp),%ebx
    1865:	8b 75 e4             	mov    -0x1c(%ebp),%esi
    1868:	89 5e 4c             	mov    %ebx,0x4c(%esi)
    186b:	e9 79 fe ff ff       	jmp    16e9 <operation_target_load_callback+0x3b9>
    1870:	c7 04 24 8c 08 00 00 	movl   $0x88c,(%esp)
			1873: R_386_32	.rodata.str1.4
    1877:	e8 fc ff ff ff       	call   1878 <operation_target_load_callback+0x548>
			1878: R_386_PC32	printk
    187c:	8b 46 04             	mov    0x4(%esi),%eax
    187f:	e8 fc ff ff ff       	call   1880 <operation_target_load_callback+0x550>
			1880: R_386_PC32	kfree
    1884:	c7 04 24 bc 08 00 00 	movl   $0x8bc,(%esp)
			1887: R_386_32	.rodata.str1.4
    188b:	e8 fc ff ff ff       	call   188c <operation_target_load_callback+0x55c>
			188c: R_386_PC32	printk
    1890:	8b 75 f0             	mov    -0x10(%ebp),%esi
    1893:	83 ee 01             	sub    $0x1,%esi
    1896:	6b d6 0c             	imul   $0xc,%esi,%edx
    1899:	89 d7                	mov    %edx,%edi
    189b:	89 da                	mov    %ebx,%edx
    189d:	83 fe ff             	cmp    $0xffffffff,%esi
    18a0:	0f 84 ba 00 00 00    	je     1960 <operation_target_load_callback+0x630>
    18a6:	8b 5a 0c             	mov    0xc(%edx),%ebx
    18a9:	83 ee 01             	sub    $0x1,%esi
    18ac:	01 fb                	add    %edi,%ebx
    18ae:	83 ef 0c             	sub    $0xc,%edi
    18b1:	8b 43 08             	mov    0x8(%ebx),%eax
    18b4:	89 55 c0             	mov    %edx,-0x40(%ebp)
    18b7:	e8 fc ff ff ff       	call   18b8 <operation_target_load_callback+0x588>
			18b8: R_386_PC32	kfree
    18bc:	8b 43 04             	mov    0x4(%ebx),%eax
    18bf:	e8 fc ff ff ff       	call   18c0 <operation_target_load_callback+0x590>
			18c0: R_386_PC32	kfree
    18c4:	8b 55 c0             	mov    -0x40(%ebp),%edx
    18c7:	eb d4                	jmp    189d <operation_target_load_callback+0x56d>
    18c9:	c7 04 24 5c 08 00 00 	movl   $0x85c,(%esp)
			18cc: R_386_32	.rodata.str1.4
    18d0:	e8 fc ff ff ff       	call   18d1 <operation_target_load_callback+0x5a1>
			18d1: R_386_PC32	printk
    18d5:	eb ad                	jmp    1884 <operation_target_load_callback+0x554>
    18d7:	8b 35 10 00 00 00    	mov    0x10,%esi
			18d9: R_386_32	__tracepoint_kmalloc
    18dd:	85 f6                	test   %esi,%esi
    18df:	0f 84 b4 fb ff ff    	je     1499 <operation_target_load_callback+0x169>
    18e5:	8b 3e                	mov    (%esi),%edi
    18e7:	8b 55 f0             	mov    -0x10(%ebp),%edx
    18ea:	83 c6 04             	add    $0x4,%esi
    18ed:	b9 10 00 00 00       	mov    $0x10,%ecx
    18f2:	c7 44 24 04 d0 00 00 	movl   $0xd0,0x4(%esp)
    18f9:	00 
    18fa:	b8 73 14 00 00       	mov    $0x1473,%eax
			18fb: R_386_32	.text
    18ff:	89 14 24             	mov    %edx,(%esp)
    1902:	89 da                	mov    %ebx,%edx
    1904:	ff d7                	call   *%edi
    1906:	8b 3e                	mov    (%esi),%edi
    1908:	85 ff                	test   %edi,%edi
    190a:	75 db                	jne    18e7 <operation_target_load_callback+0x5b7>
    190c:	e9 88 fb ff ff       	jmp    1499 <operation_target_load_callback+0x169>
    1911:	8b 55 e8             	mov    -0x18(%ebp),%edx
    1914:	8b 4a 08             	mov    0x8(%edx),%ecx
    1917:	e9 b4 fc ff ff       	jmp    15d0 <operation_target_load_callback+0x2a0>
    191c:	c7 04 24 ec 07 00 00 	movl   $0x7ec,(%esp)
			191f: R_386_32	.rodata.str1.4
    1923:	e8 fc ff ff ff       	call   1924 <operation_target_load_callback+0x5f4>
			1924: R_386_PC32	printk
    1928:	8b 43 04             	mov    0x4(%ebx),%eax
    192b:	e8 fc ff ff ff       	call   192c <operation_target_load_callback+0x5fc>
			192c: R_386_PC32	kfree
    1930:	89 d8                	mov    %ebx,%eax
    1932:	e8 fc ff ff ff       	call   1933 <operation_target_load_callback+0x603>
			1933: R_386_PC32	kfree
    1937:	e9 97 fd ff ff       	jmp    16d3 <operation_target_load_callback+0x3a3>
    193c:	c7 04 24 b4 07 00 00 	movl   $0x7b4,(%esp)
			193f: R_386_32	.rodata.str1.4
    1943:	e8 fc ff ff ff       	call   1944 <operation_target_load_callback+0x614>
			1944: R_386_PC32	printk
    1948:	eb e6                	jmp    1930 <operation_target_load_callback+0x600>
    194a:	c7 04 24 24 08 00 00 	movl   $0x824,(%esp)
			194d: R_386_32	.rodata.str1.4
    1951:	e8 fc ff ff ff       	call   1952 <operation_target_load_callback+0x622>
			1952: R_386_PC32	printk
    1956:	8b 43 08             	mov    0x8(%ebx),%eax
    1959:	e8 fc ff ff ff       	call   195a <operation_target_load_callback+0x62a>
			195a: R_386_PC32	kfree
    195e:	eb c8                	jmp    1928 <operation_target_load_callback+0x5f8>
    1960:	8b 42 0c             	mov    0xc(%edx),%eax
    1963:	89 d3                	mov    %edx,%ebx
    1965:	e8 fc ff ff ff       	call   1966 <operation_target_load_callback+0x636>
			1966: R_386_PC32	kfree
    196a:	eb ea                	jmp    1956 <operation_target_load_callback+0x626>
    196c:	90                   	nop
    196d:	90                   	nop
    196e:	90                   	nop
    196f:	90                   	nop

00001970 <data_map_delete_all>:
data_map_delete_all():
    1970:	55                   	push   %ebp
    1971:	89 e5                	mov    %esp,%ebp
    1973:	57                   	push   %edi
    1974:	56                   	push   %esi
    1975:	53                   	push   %ebx
    1976:	83 ec 0c             	sub    $0xc,%esp
    1979:	e8 fc ff ff ff       	call   197a <data_map_delete_all+0xa>
			197a: R_386_PC32	mcount
    197e:	89 45 e8             	mov    %eax,-0x18(%ebp)
    1981:	89 d6                	mov    %edx,%esi
    1983:	89 4d f0             	mov    %ecx,-0x10(%ebp)
    1986:	8b 48 04             	mov    0x4(%eax),%ecx
    1989:	b8 01 00 00 00       	mov    $0x1,%eax
    198e:	d3 e0                	shl    %cl,%eax
    1990:	85 c0                	test   %eax,%eax
    1992:	7e 71                	jle    1a05 <data_map_delete_all+0x95>
    1994:	c7 45 ec 00 00 00 00 	movl   $0x0,-0x14(%ebp)
    199b:	90                   	nop
    199c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    19a0:	8b 7d ec             	mov    -0x14(%ebp),%edi
    19a3:	8b 45 e8             	mov    -0x18(%ebp),%eax
    19a6:	c1 e7 02             	shl    $0x2,%edi
    19a9:	03 38                	add    (%eax),%edi
    19ab:	8b 1f                	mov    (%edi),%ebx
    19ad:	85 db                	test   %ebx,%ebx
    19af:	74 44                	je     19f5 <data_map_delete_all+0x85>
    19b1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    19b8:	8b 03                	mov    (%ebx),%eax
    19ba:	8b 53 04             	mov    0x4(%ebx),%edx
    19bd:	85 c0                	test   %eax,%eax
    19bf:	89 02                	mov    %eax,(%edx)
    19c1:	74 03                	je     19c6 <data_map_delete_all+0x56>
    19c3:	89 50 04             	mov    %edx,0x4(%eax)
    19c6:	85 f6                	test   %esi,%esi
    19c8:	c7 03 00 01 10 00    	movl   $0x100100,(%ebx)
    19ce:	c7 43 04 00 02 20 00 	movl   $0x200200,0x4(%ebx)
    19d5:	74 0b                	je     19e2 <data_map_delete_all+0x72>
    19d7:	8b 53 08             	mov    0x8(%ebx),%edx
    19da:	8b 43 0c             	mov    0xc(%ebx),%eax
    19dd:	8b 4d f0             	mov    -0x10(%ebp),%ecx
    19e0:	ff d6                	call   *%esi
    19e2:	89 d8                	mov    %ebx,%eax
    19e4:	e8 fc ff ff ff       	call   19e5 <data_map_delete_all+0x75>
			19e5: R_386_PC32	kfree
    19e9:	8b 1f                	mov    (%edi),%ebx
    19eb:	85 db                	test   %ebx,%ebx
    19ed:	75 c9                	jne    19b8 <data_map_delete_all+0x48>
    19ef:	8b 45 e8             	mov    -0x18(%ebp),%eax
    19f2:	8b 48 04             	mov    0x4(%eax),%ecx
    19f5:	83 45 ec 01          	addl   $0x1,-0x14(%ebp)
    19f9:	b8 01 00 00 00       	mov    $0x1,%eax
    19fe:	d3 e0                	shl    %cl,%eax
    1a00:	3b 45 ec             	cmp    -0x14(%ebp),%eax
    1a03:	7f 9b                	jg     19a0 <data_map_delete_all+0x30>
    1a05:	83 c4 0c             	add    $0xc,%esp
    1a08:	5b                   	pop    %ebx
    1a09:	5e                   	pop    %esi
    1a0a:	5f                   	pop    %edi
    1a0b:	5d                   	pop    %ebp
    1a0c:	c3                   	ret    
    1a0d:	8d 76 00             	lea    0x0(%esi),%esi

00001a10 <data_map_destroy>:
data_map_destroy():
    1a10:	55                   	push   %ebp
    1a11:	89 e5                	mov    %esp,%ebp
    1a13:	57                   	push   %edi
    1a14:	56                   	push   %esi
    1a15:	53                   	push   %ebx
    1a16:	83 ec 04             	sub    $0x4,%esp
    1a19:	e8 fc ff ff ff       	call   1a1a <data_map_destroy+0xa>
			1a1a: R_386_PC32	mcount
    1a1e:	8b 48 04             	mov    0x4(%eax),%ecx
    1a21:	89 c6                	mov    %eax,%esi
    1a23:	b8 01 00 00 00       	mov    $0x1,%eax
    1a28:	d3 e0                	shl    %cl,%eax
    1a2a:	85 c0                	test   %eax,%eax
    1a2c:	7e 5a                	jle    1a88 <data_map_destroy+0x78>
    1a2e:	31 d2                	xor    %edx,%edx
    1a30:	31 db                	xor    %ebx,%ebx
    1a32:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1a38:	8d 3c 9d 00 00 00 00 	lea    0x0(,%ebx,4),%edi
    1a3f:	03 3e                	add    (%esi),%edi
    1a41:	8b 07                	mov    (%edi),%eax
    1a43:	85 c0                	test   %eax,%eax
    1a45:	74 2f                	je     1a76 <data_map_destroy+0x66>
    1a47:	90                   	nop
    1a48:	8b 10                	mov    (%eax),%edx
    1a4a:	8b 48 04             	mov    0x4(%eax),%ecx
    1a4d:	85 d2                	test   %edx,%edx
    1a4f:	89 11                	mov    %edx,(%ecx)
    1a51:	74 03                	je     1a56 <data_map_destroy+0x46>
    1a53:	89 4a 04             	mov    %ecx,0x4(%edx)
    1a56:	c7 00 00 01 10 00    	movl   $0x100100,(%eax)
    1a5c:	c7 40 04 00 02 20 00 	movl   $0x200200,0x4(%eax)
    1a63:	e8 fc ff ff ff       	call   1a64 <data_map_destroy+0x54>
			1a64: R_386_PC32	kfree
    1a68:	8b 07                	mov    (%edi),%eax
    1a6a:	85 c0                	test   %eax,%eax
    1a6c:	75 da                	jne    1a48 <data_map_destroy+0x38>
    1a6e:	8b 4e 04             	mov    0x4(%esi),%ecx
    1a71:	ba 01 00 00 00       	mov    $0x1,%edx
    1a76:	b8 01 00 00 00       	mov    $0x1,%eax
    1a7b:	83 c3 01             	add    $0x1,%ebx
    1a7e:	d3 e0                	shl    %cl,%eax
    1a80:	39 d8                	cmp    %ebx,%eax
    1a82:	7f b4                	jg     1a38 <data_map_destroy+0x28>
    1a84:	85 d2                	test   %edx,%edx
    1a86:	75 16                	jne    1a9e <data_map_destroy+0x8e>
    1a88:	8b 06                	mov    (%esi),%eax
    1a8a:	e8 fc ff ff ff       	call   1a8b <data_map_destroy+0x7b>
			1a8b: R_386_PC32	kfree
    1a8f:	89 f0                	mov    %esi,%eax
    1a91:	e8 fc ff ff ff       	call   1a92 <data_map_destroy+0x82>
			1a92: R_386_PC32	kfree
    1a96:	83 c4 04             	add    $0x4,%esp
    1a99:	5b                   	pop    %ebx
    1a9a:	5e                   	pop    %esi
    1a9b:	5f                   	pop    %edi
    1a9c:	5d                   	pop    %ebp
    1a9d:	c3                   	ret    
    1a9e:	c7 04 24 28 09 00 00 	movl   $0x928,(%esp)
			1aa1: R_386_32	.rodata.str1.4
    1aa5:	e8 fc ff ff ff       	call   1aa6 <data_map_destroy+0x96>
			1aa6: R_386_PC32	printk
    1aaa:	eb dc                	jmp    1a88 <data_map_destroy+0x78>
    1aac:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00001ab0 <data_map_delete>:
data_map_delete():
    1ab0:	55                   	push   %ebp
    1ab1:	89 e5                	mov    %esp,%ebp
    1ab3:	57                   	push   %edi
    1ab4:	56                   	push   %esi
    1ab5:	53                   	push   %ebx
    1ab6:	83 ec 08             	sub    $0x8,%esp
    1ab9:	e8 fc ff ff ff       	call   1aba <data_map_delete+0xa>
			1aba: R_386_PC32	mcount
    1abe:	b9 20 00 00 00       	mov    $0x20,%ecx
    1ac3:	2b 48 04             	sub    0x4(%eax),%ecx
    1ac6:	69 f2 01 00 37 9e    	imul   $0x9e370001,%edx,%esi
    1acc:	89 c7                	mov    %eax,%edi
    1ace:	8d 40 08             	lea    0x8(%eax),%eax
    1ad1:	89 d3                	mov    %edx,%ebx
    1ad3:	89 45 f0             	mov    %eax,-0x10(%ebp)
    1ad6:	d3 ee                	shr    %cl,%esi
    1ad8:	e8 fc ff ff ff       	call   1ad9 <data_map_delete+0x29>
			1ad9: R_386_PC32	_spin_lock_irqsave
    1add:	89 c2                	mov    %eax,%edx
    1adf:	8b 07                	mov    (%edi),%eax
    1ae1:	8b 04 b0             	mov    (%eax,%esi,4),%eax
    1ae4:	85 c0                	test   %eax,%eax
    1ae6:	74 23                	je     1b0b <data_map_delete+0x5b>
    1ae8:	8b 08                	mov    (%eax),%ecx
    1aea:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1aee:	39 58 08             	cmp    %ebx,0x8(%eax)
    1af1:	75 14                	jne    1b07 <data_map_delete+0x57>
    1af3:	eb 33                	jmp    1b28 <data_map_delete+0x78>
    1af5:	8d 76 00             	lea    0x0(%esi),%esi
    1af8:	8b 31                	mov    (%ecx),%esi
    1afa:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1afe:	39 59 08             	cmp    %ebx,0x8(%ecx)
    1b01:	89 c8                	mov    %ecx,%eax
    1b03:	74 23                	je     1b28 <data_map_delete+0x78>
    1b05:	89 f1                	mov    %esi,%ecx
    1b07:	85 c9                	test   %ecx,%ecx
    1b09:	75 ed                	jne    1af8 <data_map_delete+0x48>
    1b0b:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
    1b10:	8b 45 f0             	mov    -0x10(%ebp),%eax
    1b13:	e8 fc ff ff ff       	call   1b14 <data_map_delete+0x64>
			1b14: R_386_PC32	_spin_unlock_irqrestore
    1b18:	89 d8                	mov    %ebx,%eax
    1b1a:	83 c4 08             	add    $0x8,%esp
    1b1d:	5b                   	pop    %ebx
    1b1e:	5e                   	pop    %esi
    1b1f:	5f                   	pop    %edi
    1b20:	5d                   	pop    %ebp
    1b21:	c3                   	ret    
    1b22:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1b28:	8b 08                	mov    (%eax),%ecx
    1b2a:	8b 70 04             	mov    0x4(%eax),%esi
    1b2d:	8b 58 0c             	mov    0xc(%eax),%ebx
    1b30:	85 c9                	test   %ecx,%ecx
    1b32:	89 0e                	mov    %ecx,(%esi)
    1b34:	74 03                	je     1b39 <data_map_delete+0x89>
    1b36:	89 71 04             	mov    %esi,0x4(%ecx)
    1b39:	c7 00 00 01 10 00    	movl   $0x100100,(%eax)
    1b3f:	c7 40 04 00 02 20 00 	movl   $0x200200,0x4(%eax)
    1b46:	89 55 ec             	mov    %edx,-0x14(%ebp)
    1b49:	e8 fc ff ff ff       	call   1b4a <data_map_delete+0x9a>
			1b4a: R_386_PC32	kfree
    1b4e:	8b 55 ec             	mov    -0x14(%ebp),%edx
    1b51:	eb bd                	jmp    1b10 <data_map_delete+0x60>
    1b53:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1b59:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001b60 <data_map_get>:
data_map_get():
    1b60:	55                   	push   %ebp
    1b61:	89 e5                	mov    %esp,%ebp
    1b63:	57                   	push   %edi
    1b64:	56                   	push   %esi
    1b65:	53                   	push   %ebx
    1b66:	83 ec 04             	sub    $0x4,%esp
    1b69:	e8 fc ff ff ff       	call   1b6a <data_map_get+0xa>
			1b6a: R_386_PC32	mcount
    1b6e:	b9 20 00 00 00       	mov    $0x20,%ecx
    1b73:	2b 48 04             	sub    0x4(%eax),%ecx
    1b76:	69 f2 01 00 37 9e    	imul   $0x9e370001,%edx,%esi
    1b7c:	89 c7                	mov    %eax,%edi
    1b7e:	89 d3                	mov    %edx,%ebx
    1b80:	d3 ee                	shr    %cl,%esi
    1b82:	8d 48 08             	lea    0x8(%eax),%ecx
    1b85:	89 c8                	mov    %ecx,%eax
    1b87:	89 4d f0             	mov    %ecx,-0x10(%ebp)
    1b8a:	e8 fc ff ff ff       	call   1b8b <data_map_get+0x2b>
			1b8b: R_386_PC32	_spin_lock_irqsave
    1b8f:	8b 17                	mov    (%edi),%edx
    1b91:	8b 4d f0             	mov    -0x10(%ebp),%ecx
    1b94:	8b 3c b2             	mov    (%edx,%esi,4),%edi
    1b97:	85 ff                	test   %edi,%edi
    1b99:	74 20                	je     1bbb <data_map_get+0x5b>
    1b9b:	8b 17                	mov    (%edi),%edx
    1b9d:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1ba1:	39 5f 08             	cmp    %ebx,0x8(%edi)
    1ba4:	75 11                	jne    1bb7 <data_map_get+0x57>
    1ba6:	eb 30                	jmp    1bd8 <data_map_get+0x78>
    1ba8:	8b 32                	mov    (%edx),%esi
    1baa:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1bae:	39 5a 08             	cmp    %ebx,0x8(%edx)
    1bb1:	89 d7                	mov    %edx,%edi
    1bb3:	74 23                	je     1bd8 <data_map_get+0x78>
    1bb5:	89 f2                	mov    %esi,%edx
    1bb7:	85 d2                	test   %edx,%edx
    1bb9:	75 ed                	jne    1ba8 <data_map_get+0x48>
    1bbb:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
    1bc0:	89 c2                	mov    %eax,%edx
    1bc2:	89 c8                	mov    %ecx,%eax
    1bc4:	e8 fc ff ff ff       	call   1bc5 <data_map_get+0x65>
			1bc5: R_386_PC32	_spin_unlock_irqrestore
    1bc9:	89 d8                	mov    %ebx,%eax
    1bcb:	83 c4 04             	add    $0x4,%esp
    1bce:	5b                   	pop    %ebx
    1bcf:	5e                   	pop    %esi
    1bd0:	5f                   	pop    %edi
    1bd1:	5d                   	pop    %ebp
    1bd2:	c3                   	ret    
    1bd3:	90                   	nop
    1bd4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1bd8:	8b 5f 0c             	mov    0xc(%edi),%ebx
    1bdb:	eb e3                	jmp    1bc0 <data_map_get+0x60>
    1bdd:	8d 76 00             	lea    0x0(%esi),%esi

00001be0 <data_map_add>:
data_map_add():
    1be0:	55                   	push   %ebp
    1be1:	89 e5                	mov    %esp,%ebp
    1be3:	57                   	push   %edi
    1be4:	56                   	push   %esi
    1be5:	53                   	push   %ebx
    1be6:	83 ec 1c             	sub    $0x1c,%esp
    1be9:	e8 fc ff ff ff       	call   1bea <data_map_add+0xa>
			1bea: R_386_PC32	mcount
    1bee:	89 4d f0             	mov    %ecx,-0x10(%ebp)
    1bf1:	89 c7                	mov    %eax,%edi
    1bf3:	8b 40 04             	mov    0x4(%eax),%eax
    1bf6:	89 d3                	mov    %edx,%ebx
    1bf8:	ba d0 00 00 00       	mov    $0xd0,%edx
    1bfd:	89 45 e8             	mov    %eax,-0x18(%ebp)
    1c00:	b8 90 02 00 00       	mov    $0x290,%eax
			1c01: R_386_32	kmalloc_caches
    1c05:	e8 fc ff ff ff       	call   1c06 <data_map_add+0x26>
			1c06: R_386_PC32	kmem_cache_alloc
    1c0a:	89 c6                	mov    %eax,%esi
    1c0c:	a1 94 02 00 00       	mov    0x294,%eax
			1c0d: R_386_32	kmalloc_caches
    1c11:	89 45 ec             	mov    %eax,-0x14(%ebp)
    1c14:	a1 04 00 00 00       	mov    0x4,%eax
			1c15: R_386_32	__tracepoint_kmalloc
    1c19:	85 c0                	test   %eax,%eax
    1c1b:	0f 85 be 00 00 00    	jne    1cdf <data_map_add+0xff>
    1c21:	85 f6                	test   %esi,%esi
    1c23:	0f 84 a3 00 00 00    	je     1ccc <data_map_add+0xec>
    1c29:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
    1c2f:	c7 46 04 00 00 00 00 	movl   $0x0,0x4(%esi)
    1c36:	89 5e 08             	mov    %ebx,0x8(%esi)
    1c39:	8b 45 f0             	mov    -0x10(%ebp),%eax
    1c3c:	89 46 0c             	mov    %eax,0xc(%esi)
    1c3f:	8d 47 08             	lea    0x8(%edi),%eax
    1c42:	89 45 f0             	mov    %eax,-0x10(%ebp)
    1c45:	e8 fc ff ff ff       	call   1c46 <data_map_add+0x66>
			1c46: R_386_PC32	_spin_lock_irqsave
    1c4a:	b9 20 00 00 00       	mov    $0x20,%ecx
    1c4f:	2b 4d e8             	sub    -0x18(%ebp),%ecx
    1c52:	89 45 ec             	mov    %eax,-0x14(%ebp)
    1c55:	69 c3 01 00 37 9e    	imul   $0x9e370001,%ebx,%eax
    1c5b:	d3 e8                	shr    %cl,%eax
    1c5d:	c1 e0 02             	shl    $0x2,%eax
    1c60:	03 07                	add    (%edi),%eax
    1c62:	8b 38                	mov    (%eax),%edi
    1c64:	85 ff                	test   %edi,%edi
    1c66:	74 42                	je     1caa <data_map_add+0xca>
    1c68:	8b 17                	mov    (%edi),%edx
    1c6a:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1c6e:	39 5f 08             	cmp    %ebx,0x8(%edi)
    1c71:	75 12                	jne    1c85 <data_map_add+0xa5>
    1c73:	eb 3d                	jmp    1cb2 <data_map_add+0xd2>
    1c75:	8d 76 00             	lea    0x0(%esi),%esi
    1c78:	8b 0a                	mov    (%edx),%ecx
    1c7a:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1c7e:	39 5a 08             	cmp    %ebx,0x8(%edx)
    1c81:	74 2f                	je     1cb2 <data_map_add+0xd2>
    1c83:	89 ca                	mov    %ecx,%edx
    1c85:	85 d2                	test   %edx,%edx
    1c87:	75 ef                	jne    1c78 <data_map_add+0x98>
    1c89:	89 3e                	mov    %edi,(%esi)
    1c8b:	89 77 04             	mov    %esi,0x4(%edi)
    1c8e:	89 30                	mov    %esi,(%eax)
    1c90:	31 db                	xor    %ebx,%ebx
    1c92:	89 46 04             	mov    %eax,0x4(%esi)
    1c95:	8b 55 ec             	mov    -0x14(%ebp),%edx
    1c98:	8b 45 f0             	mov    -0x10(%ebp),%eax
    1c9b:	e8 fc ff ff ff       	call   1c9c <data_map_add+0xbc>
			1c9c: R_386_PC32	_spin_unlock_irqrestore
    1ca0:	83 c4 1c             	add    $0x1c,%esp
    1ca3:	89 d8                	mov    %ebx,%eax
    1ca5:	5b                   	pop    %ebx
    1ca6:	5e                   	pop    %esi
    1ca7:	5f                   	pop    %edi
    1ca8:	5d                   	pop    %ebp
    1ca9:	c3                   	ret    
    1caa:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
    1cb0:	eb dc                	jmp    1c8e <data_map_add+0xae>
    1cb2:	c7 04 24 90 09 00 00 	movl   $0x990,(%esp)
			1cb5: R_386_32	.rodata.str1.4
    1cb9:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
    1cbe:	e8 fc ff ff ff       	call   1cbf <data_map_add+0xdf>
			1cbf: R_386_PC32	printk
    1cc3:	89 f0                	mov    %esi,%eax
    1cc5:	e8 fc ff ff ff       	call   1cc6 <data_map_add+0xe6>
			1cc6: R_386_PC32	kfree
    1cca:	eb c9                	jmp    1c95 <data_map_add+0xb5>
    1ccc:	c7 04 24 58 09 00 00 	movl   $0x958,(%esp)
			1ccf: R_386_32	.rodata.str1.4
    1cd3:	bb f4 ff ff ff       	mov    $0xfffffff4,%ebx
    1cd8:	e8 fc ff ff ff       	call   1cd9 <data_map_add+0xf9>
			1cd9: R_386_PC32	printk
    1cdd:	eb c1                	jmp    1ca0 <data_map_add+0xc0>
    1cdf:	a1 10 00 00 00       	mov    0x10,%eax
			1ce0: R_386_32	__tracepoint_kmalloc
    1ce4:	85 c0                	test   %eax,%eax
    1ce6:	89 45 e4             	mov    %eax,-0x1c(%ebp)
    1ce9:	0f 84 32 ff ff ff    	je     1c21 <data_map_add+0x41>
    1cef:	8b 00                	mov    (%eax),%eax
    1cf1:	89 45 e0             	mov    %eax,-0x20(%ebp)
    1cf4:	8b 45 ec             	mov    -0x14(%ebp),%eax
    1cf7:	b9 10 00 00 00       	mov    $0x10,%ecx
    1cfc:	89 f2                	mov    %esi,%edx
    1cfe:	c7 44 24 04 d0 00 00 	movl   $0xd0,0x4(%esp)
    1d05:	00 
    1d06:	89 04 24             	mov    %eax,(%esp)
    1d09:	b8 e0 1b 00 00       	mov    $0x1be0,%eax
			1d0a: R_386_32	.text
    1d0e:	ff 55 e0             	call   *-0x20(%ebp)
    1d11:	83 45 e4 04          	addl   $0x4,-0x1c(%ebp)
    1d15:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    1d18:	8b 00                	mov    (%eax),%eax
    1d1a:	85 c0                	test   %eax,%eax
    1d1c:	89 45 e0             	mov    %eax,-0x20(%ebp)
    1d1f:	75 d3                	jne    1cf4 <data_map_add+0x114>
    1d21:	e9 fb fe ff ff       	jmp    1c21 <data_map_add+0x41>
    1d26:	8d 76 00             	lea    0x0(%esi),%esi
    1d29:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001d30 <data_map_create>:
data_map_create():
    1d30:	55                   	push   %ebp
    1d31:	89 e5                	mov    %esp,%ebp
    1d33:	57                   	push   %edi
    1d34:	56                   	push   %esi
    1d35:	53                   	push   %ebx
    1d36:	83 ec 10             	sub    $0x10,%esp
    1d39:	e8 fc ff ff ff       	call   1d3a <data_map_create+0xa>
			1d3a: R_386_PC32	mcount
    1d3e:	ba d0 00 00 00       	mov    $0xd0,%edx
    1d43:	89 c6                	mov    %eax,%esi
    1d45:	b8 90 02 00 00       	mov    $0x290,%eax
			1d46: R_386_32	kmalloc_caches
    1d4a:	e8 fc ff ff ff       	call   1d4b <data_map_create+0x1b>
			1d4b: R_386_PC32	kmem_cache_alloc
    1d4f:	8b 15 04 00 00 00    	mov    0x4,%edx
			1d51: R_386_32	__tracepoint_kmalloc
    1d55:	8b 3d 94 02 00 00    	mov    0x294,%edi
			1d57: R_386_32	kmalloc_caches
    1d5b:	85 d2                	test   %edx,%edx
    1d5d:	89 c3                	mov    %eax,%ebx
    1d5f:	0f 85 a5 00 00 00    	jne    1e0a <data_map_create+0xda>
    1d65:	85 db                	test   %ebx,%ebx
    1d67:	89 df                	mov    %ebx,%edi
    1d69:	0f 84 8d 00 00 00    	je     1dfc <data_map_create+0xcc>
    1d6f:	8d 04 b6             	lea    (%esi,%esi,4),%eax
    1d72:	ba 25 49 92 24       	mov    $0x24924925,%edx
    1d77:	8d 4c 00 ff          	lea    -0x1(%eax,%eax,1),%ecx
    1d7b:	31 f6                	xor    %esi,%esi
    1d7d:	89 c8                	mov    %ecx,%eax
    1d7f:	f7 e2                	mul    %edx
    1d81:	29 d1                	sub    %edx,%ecx
    1d83:	d1 e9                	shr    %ecx
    1d85:	01 ca                	add    %ecx,%edx
    1d87:	c1 ea 02             	shr    $0x2,%edx
    1d8a:	85 d2                	test   %edx,%edx
    1d8c:	74 62                	je     1df0 <data_map_create+0xc0>
    1d8e:	66 90                	xchg   %ax,%ax
    1d90:	83 c6 01             	add    $0x1,%esi
    1d93:	d1 ea                	shr    %edx
    1d95:	75 f9                	jne    1d90 <data_map_create+0x60>
    1d97:	85 f6                	test   %esi,%esi
    1d99:	74 55                	je     1df0 <data_map_create+0xc0>
    1d9b:	b8 04 00 00 00       	mov    $0x4,%eax
    1da0:	89 f1                	mov    %esi,%ecx
    1da2:	d3 e0                	shl    %cl,%eax
    1da4:	ba d0 00 00 00       	mov    $0xd0,%edx
    1da9:	e8 fc ff ff ff       	call   1daa <data_map_create+0x7a>
			1daa: R_386_PC32	__kmalloc
    1dae:	85 c0                	test   %eax,%eax
    1db0:	0f 84 98 00 00 00    	je     1e4e <data_map_create+0x11e>
    1db6:	89 f1                	mov    %esi,%ecx
    1db8:	ba 01 00 00 00       	mov    $0x1,%edx
    1dbd:	d3 e2                	shl    %cl,%edx
    1dbf:	31 c9                	xor    %ecx,%ecx
    1dc1:	85 d2                	test   %edx,%edx
    1dc3:	7e 11                	jle    1dd6 <data_map_create+0xa6>
    1dc5:	8d 76 00             	lea    0x0(%esi),%esi
    1dc8:	c7 04 88 00 00 00 00 	movl   $0x0,(%eax,%ecx,4)
    1dcf:	83 c1 01             	add    $0x1,%ecx
    1dd2:	39 d1                	cmp    %edx,%ecx
    1dd4:	75 f2                	jne    1dc8 <data_map_create+0x98>
    1dd6:	89 03                	mov    %eax,(%ebx)
    1dd8:	89 73 04             	mov    %esi,0x4(%ebx)
    1ddb:	c7 43 08 00 00 00 00 	movl   $0x0,0x8(%ebx)
    1de2:	83 c4 10             	add    $0x10,%esp
    1de5:	89 f8                	mov    %edi,%eax
    1de7:	5b                   	pop    %ebx
    1de8:	5e                   	pop    %esi
    1de9:	5f                   	pop    %edi
    1dea:	5d                   	pop    %ebp
    1deb:	c3                   	ret    
    1dec:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1df0:	b8 08 00 00 00       	mov    $0x8,%eax
    1df5:	be 01 00 00 00       	mov    $0x1,%esi
    1dfa:	eb a8                	jmp    1da4 <data_map_create+0x74>
    1dfc:	c7 04 24 d0 09 00 00 	movl   $0x9d0,(%esp)
			1dff: R_386_32	.rodata.str1.4
    1e03:	e8 fc ff ff ff       	call   1e04 <data_map_create+0xd4>
			1e04: R_386_PC32	printk
    1e08:	eb d8                	jmp    1de2 <data_map_create+0xb2>
    1e0a:	a1 10 00 00 00       	mov    0x10,%eax
			1e0b: R_386_32	__tracepoint_kmalloc
    1e0f:	85 c0                	test   %eax,%eax
    1e11:	89 45 f0             	mov    %eax,-0x10(%ebp)
    1e14:	0f 84 4b ff ff ff    	je     1d65 <data_map_create+0x35>
    1e1a:	8b 08                	mov    (%eax),%ecx
    1e1c:	89 4d ec             	mov    %ecx,-0x14(%ebp)
    1e1f:	b8 30 1d 00 00       	mov    $0x1d30,%eax
			1e20: R_386_32	.text
    1e24:	b9 0c 00 00 00       	mov    $0xc,%ecx
    1e29:	c7 44 24 04 d0 00 00 	movl   $0xd0,0x4(%esp)
    1e30:	00 
    1e31:	89 da                	mov    %ebx,%edx
    1e33:	89 3c 24             	mov    %edi,(%esp)
    1e36:	ff 55 ec             	call   *-0x14(%ebp)
    1e39:	83 45 f0 04          	addl   $0x4,-0x10(%ebp)
    1e3d:	8b 45 f0             	mov    -0x10(%ebp),%eax
    1e40:	8b 00                	mov    (%eax),%eax
    1e42:	85 c0                	test   %eax,%eax
    1e44:	89 45 ec             	mov    %eax,-0x14(%ebp)
    1e47:	75 d6                	jne    1e1f <data_map_create+0xef>
    1e49:	e9 17 ff ff ff       	jmp    1d65 <data_map_create+0x35>
    1e4e:	c7 04 24 00 0a 00 00 	movl   $0xa00,(%esp)
			1e51: R_386_32	.rodata.str1.4
    1e55:	31 ff                	xor    %edi,%edi
    1e57:	e8 fc ff ff ff       	call   1e58 <data_map_create+0x128>
			1e58: R_386_PC32	printk
    1e5c:	89 d8                	mov    %ebx,%eax
    1e5e:	e8 fc ff ff ff       	call   1e5f <data_map_create+0x12f>
			1e5f: R_386_PC32	kfree
    1e63:	e9 7a ff ff ff       	jmp    1de2 <data_map_create+0xb2>

Disassembly of section .exit.text:

00000000 <cleanup_module>:
cleanup_module():
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	e8 fc ff ff ff       	call   4 <cleanup_module+0x4>
			4: R_386_PC32	mcount
   8:	a1 00 00 00 00       	mov    0x0,%eax
			9: R_386_32	.bss
   d:	e8 fc ff ff ff       	call   e <cleanup_module+0xe>
			e: R_386_PC32	operation_replacer_destroy
  12:	5d                   	pop    %ebp
  13:	c3                   	ret    

Disassembly of section .init.text:

00000000 <init_module>:
init_module():
   0:	55                   	push   %ebp
   1:	ba 04 00 00 00       	mov    $0x4,%edx
   6:	89 e5                	mov    %esp,%ebp
   8:	b8 0a 00 00 00       	mov    $0xa,%eax
   d:	83 ec 04             	sub    $0x4,%esp
  10:	e8 fc ff ff ff       	call   11 <init_module+0x11>
			11: R_386_PC32	operation_replacer_create
  15:	31 d2                	xor    %edx,%edx
  17:	85 c0                	test   %eax,%eax
  19:	a3 00 00 00 00       	mov    %eax,0x0
			1a: R_386_32	.bss
  1e:	75 11                	jne    31 <init_module+0x31>
  20:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
			23: R_386_32	.rodata.str1.4
  27:	e8 fc ff ff ff       	call   28 <init_module+0x28>
			28: R_386_PC32	printk
  2c:	ba f4 ff ff ff       	mov    $0xfffffff4,%edx
  31:	89 d0                	mov    %edx,%eax
  33:	c9                   	leave  
  34:	c3                   	ret    

Disassembly of section .altinstr_replacement:

00000000 <.altinstr_replacement>:
   0:	0f 18 02             	prefetchnta (%edx)
   3:	0f 18 02             	prefetchnta (%edx)
   6:	0f 18 02             	prefetchnta (%edx)
   9:	0f 18 02             	prefetchnta (%edx)
   c:	0f 18 00             	prefetchnta (%eax)
   f:	0f 18 00             	prefetchnta (%eax)
  12:	0f 18 00             	prefetchnta (%eax)
  15:	0f 18 00             	prefetchnta (%eax)
  18:	0f 18 02             	prefetchnta (%edx)
  1b:	0f 18 02             	prefetchnta (%edx)
  1e:	0f 18 02             	prefetchnta (%edx)
  21:	0f 18 02             	prefetchnta (%edx)
  24:	0f 18 00             	prefetchnta (%eax)
  27:	0f 18 00             	prefetchnta (%eax)
  2a:	0f 18 00             	prefetchnta (%eax)
  2d:	0f 18 00             	prefetchnta (%eax)
  30:	0f 18 00             	prefetchnta (%eax)
  33:	0f 18 02             	prefetchnta (%edx)
  36:	0f 18 01             	prefetchnta (%ecx)
  39:	0f 18 01             	prefetchnta (%ecx)
  3c:	0f 18 00             	prefetchnta (%eax)
  3f:	0f 18 02             	prefetchnta (%edx)
  42:	0f 18 00             	prefetchnta (%eax)
  45:	0f 18 00             	prefetchnta (%eax)
  48:	0f 18 01             	prefetchnta (%ecx)
  4b:	0f 18 06             	prefetchnta (%esi)
  4e:	0f 18 02             	prefetchnta (%edx)
  51:	0f 18 06             	prefetchnta (%esi)
  54:	0f 18 02             	prefetchnta (%edx)
  57:	0f 18 01             	prefetchnta (%ecx)
