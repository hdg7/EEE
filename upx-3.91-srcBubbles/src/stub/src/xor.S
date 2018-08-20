////////BEGIN XOR

section DECRYTP_1_V1
		lea eax, section_nb
		cmp eax,0 
		je end_patch
		mov ebx, 5
		lea ecx, end_decrypt0
		add ebx,77
		lea edi, start_decrypt0
		sub ebx,32
		lea ebx, key_val0
section DECRYTP_1_V2
                lea eax, section_nb
                cmp eax,0
                je end_patch
		mov ebx,10
               	lea ecx, end_decrypt0
		sub ebx,2
               	lea edi, start_decrypt0
		sub ebx,7
               	lea ebx, key_val0
section DECRYTP_1_V3
                lea eax, section_nb
                cmp eax,0
                je end_patch
		mov ecx,5
               	lea ecx, end_decrypt0
		add edi,10
               	lea edi, start_decrypt0
		sub ebx,11
               	lea ebx, key_val0
section DECRYTP_1_V4
                lea eax, section_nb
                cmp eax,0
                je end_patch
		mov ecx,10
               	lea ecx, end_decrypt0
		mov edi,10
               	lea edi, start_decrypt0
		mov ebx,11
               	lea ebx, key_val0
section DECRYTP_1_5_V1
		call decrypt_xor_V1
section DECRYTP_1_5_V2
		call decrypt_xor_V2
section DECRYTP_1_5_V3
		call decrypt_xor_V3
section DECRYTP_1_5_V4
		call decrypt_xor_V4


section	DECRYTP_2_V1
		lea eax, section_nb
                cmp eax,1
                je end_patch
		mov ebx,4
                lea ecx, end_decrypt1
		add ebx,32
                lea edi, start_decrypt1
		add ebx,33
                lea ebx, key_val1
section DECRYTP_2_V2
                lea eax, section_nb
                cmp eax,1
                je end_patch
		mov ecx,3
                lea ecx, end_decrypt1
		mov edi,3
                lea edi, start_decrypt1
		mov ebx,3
                lea ebx, key_val1
section DECRYTP_2_V3
                lea eax, section_nb
                cmp eax,1
                je end_patch
		mov edi,3
		mov ebx,ecx
                lea ecx, end_decrypt1
		add edi,4
                lea edi, start_decrypt1
                lea ebx, key_val1
section DECRYTP_2_V4
                lea eax, section_nb
                cmp eax,1
                je end_patch
		mov ecx,ebx
                lea ecx, end_decrypt1
		add ecx,2
                lea edi, start_decrypt1
		sub ecx,2
                lea ebx, key_val1
section DECRYTP_2_5_V1
		call decrypt_xor_V1
section DECRYTP_2_5_V2
		call decrypt_xor_V2
section DECRYTP_2_5_V3
		call decrypt_xor_V3
section DECRYTP_2_5_V4
		call decrypt_xor_V4

section DECRYTP_3_V1
		lea eax, section_nb
                cmp eax,2
                je end_patch
		mov ebx,7
                lea ecx, end_decrypt2
		inc ebx
                lea edi, start_decrypt2
		add ebx,21
                lea ebx, key_val2
section DECRYTP_3_V2
                lea eax, section_nb
                cmp eax,2
                je end_patch
                mov ebx,17
                lea ecx, end_decrypt2
		mov edx,3
                lea edi, start_decrypt2
		inc edx
                lea ebx, key_val2
section DECRYTP_3_V3
                lea eax, section_nb
                cmp eax,2
                je end_patch
                mov ebx,2
                lea ecx, end_decrypt2
                lea edi, start_decrypt2
		inc ebx
                lea ebx, key_val2
section DECRYTP_3_V4
                lea eax, section_nb
                cmp eax,2
                je end_patch
                mov ebx,3
                lea ecx, end_decrypt2
		nop
                lea edi, start_decrypt2
		nop
                lea ebx, key_val2
section DECRYTP_3_5_V1
		call decrypt_xor_V1
section DECRYTP_3_5_V2
		call decrypt_xor_V2
section DECRYTP_3_5_V3
		call decrypt_xor_V3
section DECRYTP_3_5_V4
		call decrypt_xor_V4

section	DECRYTP_4_V1
                lea eax, section_nb
                cmp eax,3
                je end_patch
		mov ebx,2
                lea ecx, end_decrypt3
		inc ebx
                lea edi, start_decrypt3
                lea ebx, key_val3
section DECRYTP_4_V2
                lea eax, section_nb
                cmp eax,3
                je end_patch
                mov ebx,ecx
		mov edx,1
                lea ecx, end_decrypt3
                lea edi, start_decrypt3
		inc edx
                lea ebx, key_val3
section DECRYTP_4_V3
                lea eax, section_nb
                cmp eax,3
                je end_patch
                mov ecx,2
                lea ecx, end_decrypt3
		mov edx,50
		lea edi, start_decrypt3
                lea ebx, key_val3
section DECRYTP_4_V4
                lea edx, section_nb
                cmp edx,3
                je end_patch
                mov ecx,20
                lea ecx, end_decrypt3
                lea edi, start_decrypt3
                lea ebx, key_val3
section DECRYTP_4_5_V1
		call decrypt_xor_V1
                jmp end_patch
section DECRYTP_4_5_V2
		call decrypt_xor_V2
                jmp end_patch
section DECRYTP_4_5_V3
		call decrypt_xor_V3
                jmp end_patch
section DECRYTP_4_5_V4
		call decrypt_xor_V4
                jmp end_patch

section	DECRYTP_5_V1
decrypt_xor_V1:
	label1_V1:
		xor eax,eax
		mov al, [edi]
		xor al,bl
		add al,1
		sub al,1
		mov [edi],al
		inc edi
		cmp edi, ecx
		jne label1_V1
		ret	
section	DECRYTP_5_V2
decrypt_xor_V2:
	label1_V2:
		xor al,al
		nop
		mov al, [edi]
		xor al,bl
		nop
		mov [edi],al
		add edi,1
		cmp edi, ecx
		jne label1_V2
		ret	
section	DECRYTP_5_V3
decrypt_xor_V3:
	label1_V3:
		xor dl,dl
		mov dl, [edi]
		xor dl,bl
		mov [edi],dl
		inc edi
		cmp edi, ecx
		jne label1_V3
		ret	
section	DECRYTP_5_V4
decrypt_xor_V4:
	label1_V4:
		mov al,0
		mov al, [edi]
		xor al,bl
		nop
		mov [edi],al
		inc edi
		nop
		cmp edi, ecx
		jne label1_V4
		ret	

section	DECRYTP_6
end_patch:
///////END XOR
