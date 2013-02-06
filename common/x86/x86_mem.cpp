
/*
	Copyright (C) 2010-2013 Dimitris V. DimitrisV22@gmail.com

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
	documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, 
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
	DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "x86_mem.hpp"

void sse_memcpy_div22(void* dst,const void* src,unsigned int sz) {
	__asm {
		mov esi,src
		prefetchnta [esi + 0]
		prefetchnta [esi + 32]
		mov edi,dst
		mov ecx,sz
		cmp ecx,128
		jge sse_memcpy_div22_bulk
		rep movsb
		jmp sse_memcpy_div22_aligned_exit2
sse_memcpy_div22_bulk:
		mov eax,edi
		and eax,15
		jz sse_memcpy_div22_aligned_dst
		push ecx
		mov ecx,16
		sub ecx,eax
		push ecx
		rep movsb
		pop ecx
		mov esi,src
		add esi,ecx
		mov edi,dst
		add edi,ecx
		mov eax,ecx
		pop ecx
		sub ecx,eax
		cmp ecx,128
		jge sse_memcpy_div22_aligned_dst
		rep movsb
		jmp sse_memcpy_div22_aligned_exit2
sse_memcpy_div22_aligned_dst:
		prefetchnta [esi + 0]
		prefetchnta [esi + 32]
		prefetchnta [esi + 64]
		prefetchnta [esi + 96]
//sse_memcpy_div22_aligned_cpy:
		mov eax,esi
		and eax,15
		jnz sse_memcpy_div22_aligned_cpy_unaligned_src
		//int 3
		mov eax,ecx
		shr eax,7
		and ecx,127
sse_memcpy_div22_aligned_cpy_body:
		movdqa xmm0,0[esi]
		movdqa xmm1,16[esi]
		movdqa xmm2,32[esi]
		movdqa xmm3,48[esi]
		movdqa xmm4,64[esi]
		movdqa xmm5,80[esi]
		movdqa xmm6,96[esi]
		movdqa xmm7,112[esi]
		movntdq 0[edi],xmm0
		movntdq 16[edi],xmm1
		movntdq 32[edi],xmm2
		movntdq 48[edi],xmm3
		movntdq 64[edi],xmm4
		movntdq 80[edi],xmm5
		movntdq 96[edi],xmm6
		movntdq 112[edi],xmm7
		add edi,128
		add esi,128
		prefetchnta [esi + 512]
		prefetchnta [esi + 512 + 32]
		prefetchnta [esi + 512 + 64]
		prefetchnta [esi + 512 + 96]
		dec eax
		jnz sse_memcpy_div22_aligned_cpy_body
		jmp sse_memcpy_div22_aligned_small
sse_memcpy_div22_aligned_cpy_unaligned_src:
		//int 3
		mov eax,ecx
		shr eax,7
		and ecx,127
sse_memcpy_div22_aligned_cpy_unaligned_src_body:
		movdqu xmm0,0[esi]
		movdqu xmm1,16[esi]
		movdqu xmm2,32[esi]
		movdqu xmm3,48[esi]
		movdqu xmm4,64[esi]
		movdqu xmm5,80[esi]
		movdqu xmm6,96[esi]
		movdqu xmm7,112[esi]
		movntdq 0[edi],xmm0
		movntdq 16[edi],xmm1
		movntdq 32[edi],xmm2
		movntdq 48[edi],xmm3
		movntdq 64[edi],xmm4
		movntdq 80[edi],xmm5
		movntdq 96[edi],xmm6
		movntdq 112[edi],xmm7
		add edi,128
		add esi,128
		prefetchnta [esi + 512]
		prefetchnta [esi + 512 + 32]
		prefetchnta [esi + 512 + 64]
		prefetchnta [esi + 512 + 96]
		dec eax
		jnz sse_memcpy_div22_aligned_cpy_unaligned_src_body
sse_memcpy_div22_aligned_small:
		//int 3
		cmp ecx,0
		je sse_memcpy_div22_aligned_exit
		rep movsb
sse_memcpy_div22_aligned_exit:
		sfence
		emms
sse_memcpy_div22_aligned_exit2:
	}
}