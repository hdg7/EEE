// ====================================
// REMOVE BUBBLES INI
// ====================================

section BUBBLES_1_V1
	push edi
	push esi
	push ebp
	mov ebp,esp
        sub esp, 72
        mov dword ptr [ebp-8],0
        mov dword ptr [ebp-16],0
        mov dword ptr [ebp-24],0
        mov dword ptr [ebp-32],1
	mov eax, offset delimiterSize
        mov dword ptr [ebp-56],eax
	mov eax, offset delimiter0
	mov dword ptr [ebp-64],eax
	mov eax, offset delimiter1
        mov dword ptr [ebp-60],eax
        mov eax, offset delay
	mov dword ptr [ebp-72],eax

//=====================================
//DELAY
//=====================================
	mov eax, dword ptr [ebp-72]
	cmp eax,0
	jna end_delay_v1
loop2_delay_V1:
	mov ecx, 0x20000000
loop1_delay_V1:
	sub ecx,1
	jnz loop1_delay_V1
	sub eax,1
	jnz loop2_delay_V1
end_delay_v1:
//END DELAY
//====================================

	mov edi,esi
        mov esi, offset compression_size
	add esi,edi


section BUBBLES_1_V2
        push edi
        push esi
        push ebp
        mov ebp,esp
        sub esp, 72
        mov dword ptr [ebp-8],0
        mov ebx, offset delimiter0
        mov dword ptr [ebp-64],ebx
        mov ebx, offset delimiter1
        mov dword ptr [ebp-16],0
        mov dword ptr [ebp-60],ebx
        mov ebx, offset delimiterSize
        mov dword ptr [ebp-56],ebx
        mov ebx, offset delay
        mov dword ptr [ebp-72],ebx
        mov dword ptr [ebp-24],0
        mov dword ptr [ebp-32],1

//=====================================
//DELAY
//=====================================
        mov ebx, dword ptr [ebp-72]
	cmp ebx,0
	jna end_delay_v2
loop2_delay_V2:
        mov edx, 0x20000000
loop1_delay_V2:
        sub edx,1
        jnz loop1_delay_V2
        sub ebx,1
        jnz loop2_delay_V2
end_delay_v2:

//END DELAY
//====================================

        mov edi,esi
        mov esi, offset compression_size
        add esi,edi

section BUBBLES_1_V3
        push edi
        push esi
        push ebp
        mov ebp,esp
        sub esp, 72
        mov ecx, offset delimiterSize
        mov dword ptr [ebp-56],ecx
        mov dword ptr [ebp-16],0
        mov ecx, offset delimiter0
        mov dword ptr [ebp-24],0
        mov dword ptr [ebp-32],1
        mov dword ptr [ebp-64],ecx
        mov ecx, offset delimiter1
        mov dword ptr [ebp-8],0
        mov dword ptr [ebp-60],ecx
        mov ecx, offset delay
        mov dword ptr [ebp-72],ecx

//=====================================
//DELAY
//=====================================
        mov edx, dword ptr [ebp-72]
        cmp edx,0
        jna end_delay_v3
loop2_delay_V3:
        mov ebx, 0x20000000
loop1_delay_V3:
        sub ebx,1
        jnz loop1_delay_V3
        sub edx,1
        jnz loop2_delay_V3
end_delay_v3:

//END DELAY
//====================================

        mov edi,esi
        mov esi, offset compression_size
        add esi,edi

section BUBBLES_1_V4
        push edi
        push esi
        push ebp
        mov ebp,esp
        sub esp, 72
        mov ecx, offset delimiterSize
        mov dword ptr [ebp-56],ecx
        mov dword ptr [ebp-16],0
        mov ecx, offset delimiter0
        mov dword ptr [ebp-24],0
        mov dword ptr [ebp-16],0
        mov dword ptr [ebp-32],1
        mov dword ptr [ebp-64],ecx
        mov ecx, offset delimiter1
        mov dword ptr [ebp-8],0
        mov dword ptr [ebp-60],ecx
        mov ecx, offset delay
        mov dword ptr [ebp-72],ecx

//=====================================
//DELAY
//=====================================
        mov edx, dword ptr [ebp-72]
        cmp edx,0
        jna end_delay_v4
loop2_delay_V4:
        mov ecx, 0x10000000
	add ecx, 0x10000000
loop1_delay_V4:
        sub ebx,1
        jnz loop1_delay_V4
        sub edx,1
        jnz loop2_delay_V4
end_delay_v4:

//END DELAY
//====================================

        mov edi,esi
        mov esi, offset compression_size
        add esi,edi

section BUBBLES_1_8_V1
        jmp check_loop1_V1
section BUBBLES_1_8_V2
        jmp check_loop1_V2
section BUBBLES_1_8_V3
        jmp check_loop1_V3
section BUBBLES_1_8_V4
        jmp check_loop1_V4


section BUBBLES_2_V1

loop1_ini_V1:
	mov eax, dword ptr [ebp-8]
        movzx edx, byte ptr [edi+eax]
        mov eax, dword ptr [ebp-16]
        movzx eax, byte ptr [ebp-64 + eax]

        cmp dl,al

section BUBBLES_2_V2

loop1_ini_V2:
	mov ebx, dword ptr [ebp-8]
        movzx edx, byte ptr [edi+ebx]
        mov ebx, dword ptr [ebp-16]
        movzx ebx, byte ptr [ebp-64 + ebx]

        cmp dl,bl

section BUBBLES_2_V3

loop1_ini_V3:
	mov eax, dword ptr [ebp-8]
        movzx ebx, byte ptr [edi+eax]
        mov eax, dword ptr [ebp-16]
        movzx eax, byte ptr [ebp-64 + eax]

        cmp bl,al

section BUBBLES_2_V4

loop1_ini_V4:
	mov ecx, dword ptr [ebp-8]
        movzx ebx, byte ptr [edi+ecx]
        mov ecx, dword ptr [ebp-16]
        movzx ecx, byte ptr [ebp-64 + ecx]

        cmp bl,cl


section BUBBLES_2_7_V1
        jne found_check_V1
section BUBBLES_2_7_V2
        jne found_check_V2
section BUBBLES_2_7_V3
        jne found_check_V3
section BUBBLES_2_7_V4
        jne found_check_V4


section BUBBLES_3_V1

        mov eax, [ebp-8]
        add eax, 1
        mov [ebp-40],eax
        mov eax, [ebp-16]
        add eax,1
        mov [ebp-48],eax

section BUBBLES_3_V2

        mov ebx, [ebp-16]
        add ebx,1
        mov [ebp-48],ebx
        mov ebx, [ebp-8]
        add ebx, 1
        mov [ebp-40],ebx

section BUBBLES_3_V3

        mov ecx, [ebp-16]
        add ecx,1
        mov [ebp-48],ecx
        mov ecx, [ebp-8]
        add ecx, 1
        mov [ebp-40],ecx

section BUBBLES_3_V4

        mov edx, [ebp-8]
        mov ecx, [ebp-16]
        add ecx,1
        add edx, 1
        mov [ebp-40],edx
        mov [ebp-48],ecx

section BUBBLES_3_4_V1
        jmp check_loop2_V1
section BUBBLES_3_4_V2
        jmp check_loop2_V2
section BUBBLES_3_4_V3
        jmp check_loop2_V3
section BUBBLES_3_4_V4
        jmp check_loop2_V4


section BUBBLES_4_V1

start_loop2_V1:
	add dword ptr [ebp-40],1
        add dword ptr [ebp-48],1

check_loop2_V1:
       	mov eax, [ebp-40]
       	movzx edx, byte ptr [edi+eax]
       	mov eax, [ebp-48]
       	movzx eax, byte ptr [ebp-64+eax]
        cmp dl,al

section BUBBLES_4_V2

start_loop2_V2:
	add dword ptr [ebp-40],1
        add dword ptr [ebp-48],1

check_loop2_V2:
	mov eax, [ebp-40]
        mov ebx, [ebp-48]
        movzx edx, byte ptr [edi+eax]
        movzx ebx, byte ptr [ebp-64+ebx]
        cmp dl,bl

section BUBBLES_4_V3

start_loop2_V3:
        add dword ptr [ebp-48],1
	add dword ptr [ebp-40],1

check_loop2_V3:
        mov eax, [ebp-48]
        movzx eax, byte ptr [ebp-64+eax]
	mov edx, [ebp-40]
        movzx ebx, byte ptr [edi+edx]
        cmp bl,al

section BUBBLES_4_V4

start_loop2_V4:
	add dword ptr [ebp-40],1
	nop
        add dword ptr [ebp-48],1

check_loop2_V4:
        mov ecx, [ebp-48]
	mov ebx, [ebp-40]
        movzx edx, byte ptr [edi+ebx]
        movzx ecx, byte ptr [ebp-64+ecx]
        cmp dl,cl


section BUBBLES_4_6_V1
     	jne if_found_V1
section BUBBLES_4_6_V2
        jne if_found_V2
section BUBBLES_4_6_V3
        jne if_found_V3
section BUBBLES_4_6_V4
        jne if_found_V4


section BUBBLES_5_V1
        mov eax, [ebp-8]
        mov edx, [ebp-40]
        mov ecx, edx
        sub ecx,eax
       	mov eax,ecx
        add eax,1
       	mov ebx,eax
       	mov eax, [ebp-56]
	cmp ebx,eax
section BUBBLES_5_V2
        mov edx, [ebp-40]
        mov ecx, [ebp-8]
        mov eax, edx
        sub eax,ecx
        mov ecx,eax
        add ecx,1
        mov ebx,ecx
        mov ecx, [ebp-56]
        cmp ebx,ecx
section BUBBLES_5_V3
        mov edx, [ebp-40]
        mov ecx, edx
        mov ebx, [ebp-8]
        sub ecx,ebx
        mov ebx,ecx
        add ebx,1
        mov eax,ebx
        mov ebx, [ebp-56]
        cmp eax,ebx
section BUBBLES_5_V4
        mov edx, [ebp-8]
        mov eax, [ebp-40]
        mov ecx, eax
        sub ecx,edx
        mov edx,ecx
        add edx,1
        mov ebx,edx
        mov edx, [ebp-56]
        cmp ebx,edx


section BUBBLES_5_4_V1
        jb start_loop2_V1
section BUBBLES_5_4_V2
        jb start_loop2_V2
section BUBBLES_5_4_V3
        jb start_loop2_V3
section BUBBLES_5_4_V4
        jb start_loop2_V4

section BUBBLES_6_V1
if_found_V1:
        mov eax, [ebp-8]
        mov edx, [ebp-40]
        mov ecx,edx
        sub ecx,eax
        mov eax,ecx
       	add eax,1
        mov ebx,eax
        mov eax, [ebp-56]
        cmp ebx,eax

section BUBBLES_6_V2
if_found_V2:
       	mov eax, [ebp-8]
	mov edx, [ebp-40]
        mov ebx,edx
       	sub ebx,eax
       	mov eax,ebx
	add eax,1
        mov ecx,eax
        mov eax, [ebp-56]
        cmp ecx,eax

section BUBBLES_6_V3
if_found_V3:
       	mov eax, [ebp-8]
	mov ebx, [ebp-40]
        mov ecx,ebx
       	sub ecx,eax
       	mov eax,ecx
	add eax,1
        mov edx,eax
        mov eax, [ebp-56]
        cmp edx,eax

section BUBBLES_6_V4
if_found_V4:
       	mov eax, [ebp-8]
	mov ecx, [ebp-40]
        mov edx,ecx
       	sub edx,eax
       	mov eax,edx
	add eax,1
        mov ebx,eax
        mov eax, [ebp-56]
        cmp ebx,eax

section BUBBLES_6_7_V1
       	jne not_if_V1
section BUBBLES_6_7_V2
        jne not_if_V2
section BUBBLES_6_7_V3
        jne not_if_V3
section BUBBLES_6_7_V4
        jne not_if_V4


section BUBBLES_7_V1
        mov ebx, [ebp-40]
        add ebx,1
        mov dword ptr [ebp-8],ebx
        cmp dword ptr [ebp-32],0
       	sete al
	movzx eax,al
        mov dword ptr [ebp-32],eax

not_if_V1:
       	mov dword ptr [ebp-16],0
found_check_V1:
	cmp dword ptr [ebp-32],0

section BUBBLES_7_V2
        cmp dword ptr [ebp-32],0
        sete al
        movzx eax,al
       	mov dword ptr [ebp-32],eax
       	mov ecx, [ebp-40]
	add ecx,1
        mov dword ptr [ebp-8],ecx

not_if_V2:
       	mov dword ptr [ebp-16],0
found_check_V2:
	cmp dword ptr [ebp-32],0

section BUBBLES_7_V3
       	mov edx, [ebp-40]
	add edx,1
        mov dword ptr [ebp-8],edx
        cmp dword ptr [ebp-32],0
        sete al
        movzx eax,al
       	mov dword ptr [ebp-32],eax

not_if_V3:
       	mov dword ptr [ebp-16],0
found_check_V3:
	cmp dword ptr [ebp-32],0

section BUBBLES_7_V4
        cmp dword ptr [ebp-32],0
        sete al
        movzx eax,al
       	mov dword ptr [ebp-32],eax
       	mov eax, [ebp-40]
	add eax,1
        mov dword ptr [ebp-8],eax

not_if_V4:
       	mov dword ptr [ebp-16],0
found_check_V4:
	cmp dword ptr [ebp-32],0

section BUBBLES_7_8_V1
        je loop1_increment_V1
section BUBBLES_7_8_V2
        je loop1_increment_V2
section BUBBLES_7_8_V3
        je loop1_increment_V3
section BUBBLES_7_8_V4
        je loop1_increment_V4

section BUBBLES_8_V1

        mov edx, [ebp-8]
        mov eax, [ebp-24]
        movzx edx, byte ptr [edi+edx]
        mov byte ptr [edi+eax],dl
        add dword ptr [ebp-24],1
loop1_increment_V1:
        add dword ptr [ebp-8],1

check_loop1_V1:
	mov ebx, dword ptr [ebp-8]
	add ebx,edi
        cmp ebx,esi

section BUBBLES_8_V2

        mov edx, [ebp-8]
        movzx edx, byte ptr [edi+edx]
        mov ebx, [ebp-24]
        mov byte ptr [edi+ebx],dl
        add dword ptr [ebp-24],1
loop1_increment_V2:
        add dword ptr [ebp-8],1

check_loop1_V2:
	mov eax, dword ptr [ebp-8]
        add eax,edi
        cmp eax,esi

section BUBBLES_8_V3

        mov ecx, [ebp-24]
        mov edx, [ebp-8]
        movzx edx, byte ptr [edi+edx]
        mov byte ptr [edi+ecx],dl
        add dword ptr [ebp-24],1
loop1_increment_V3:
        add dword ptr [ebp-8],1

check_loop1_V3:
	mov ebx, dword ptr [ebp-8]
        add ebx,edi
        cmp ebx,esi
section BUBBLES_8_V4

        mov edx, [ebp-24]
        mov eax, [ebp-8]
        movzx eax, byte ptr [edi+eax]
        mov byte ptr [edi+edx],al
        add dword ptr [ebp-24],1
loop1_increment_V4:
        add dword ptr [ebp-8],1

check_loop1_V4:
	mov ebx, dword ptr [ebp-8]
        add ebx,edi
        cmp ebx,esi

section BUBBLES_8_2_V1
        jb loop1_ini_V1
section BUBBLES_8_2_V2
        jb loop1_ini_V2
section BUBBLES_8_2_V3
        jb loop1_ini_V3
section BUBBLES_8_2_V4
        jb loop1_ini_V4

section BUBBLES_9
fin:
	add esp,72
	mov eax,1
	pop ebp
	pop esi
	pop edi
	jmp decompress_aux

//section BUBBLES_1_8_V1
//	jmp check_loop1
//section BUBBLES_2_7_V1
//	jne found_check
//section BUBBLES_3_4_V1
//	jmp check_loop2
//section BUBBLES_4_6_V1
//	jne if_found
//section BUBBLES_5_4_V1
//	jb start_loop2
//section BUBBLES_6_7_V1
//	jne not_if
//section BUBBLES_7_8_V1
//	je loop1_increment
//section BUBBLES_8_2_V1
//	jb loop1_ini

// ====================================
// REMOVE BUBBLES END
// ====================================

