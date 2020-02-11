
#if !defined(USE_WIN32_ASSEMBLER)
#error "You shouldn't be including this file because you're not using assembler routines"
#endif

#define mp_addc	P_ADDC
#define mp_add	P_ADD
#define mp_rotate_left P_ROTATE_LEFT
#define mp_subb P_SUBB
#define mp_smula P_SMULA

#if !defined(_MSC_VER)
#error "This code needs a Microsoft compiler"
#endif

#if defined(_M_IX86)


#pragma warning(disable:4035)  // this stops the compiler complaining about no return values from the routines

extern unsigned int global_precision;


__inline boolean P_ROTATE_LEFT(unitptr r1, boolean carry)
{
	__asm{
		mov edi,DWORD PTR [global_precision]
		mov	ecx,DWORD PTR [r1]
		xor	esi,esi								// clear esi, which is the offset into the digit arrays
		// cetup carry
		// note that the instruction above clears the carry
		mov	eax,DWORD PTR [carry]
		rcr	eax,1
loop_t3:
		mov	eax,DWORD PTR [ecx + esi * 4]
		rcl	eax,1
		mov DWORD PTR [ecx + esi * 4],eax
		inc	esi
		dec	edi
		jnz	loop_t3
		// compute carry
		rcl	eax,1
		and	eax,1
	}
}

__inline boolean P_SUBB(unitptr r1, unitptr r2, boolean borrow)
{
	__asm{
		mov edi,DWORD PTR [global_precision]
		mov	ecx,DWORD PTR [r1]
		mov edx,DWORD PTR [r2]
		xor	esi,esi								// clear esi, which is the offset into the digit arrays
		// cetup carry
		// note that the instruction above clears the carry
		mov	eax,DWORD PTR [borrow]
		rcr	eax,1
loop_t3:
		mov	eax,DWORD PTR [ecx + esi * 4]
		mov	ebx,DWORD PTR [edx + esi * 4]
		sbb	eax,ebx
		mov DWORD PTR [ecx + esi * 4],eax
		inc	esi
		dec	edi
		jnz	loop_t3
		// compute carry
		rcl	eax,1
		and	eax,1
	}
}

__inline void P_SMULA(unitptr prod,unitptr multiplicand, unit multiplier)
{
	__asm{
		mov ecx,DWORD PTR [global_precision]
		mov	edi,DWORD PTR [prod]
		mov esi,DWORD PTR [multiplicand]
		push	ebp
		mov ebp,DWORD PTR [multiplier]

		xor ebx,ebx
loop_t3:
		mov	eax,DWORD PTR [esi]
		mul	ebp
		add eax,ebx
		adc	edx,0
		add eax,DWORD PTR [edi]
		adc	edx,0
		mov DWORD PTR [edi],eax
		mov	ebx,edx
		add	esi,4
		add	edi,4
		dec	ecx
		jnz	loop_t3
		add	DWORD PTR [edi],ebx
		pop	ebp
	}
}

__inline boolean P_ADDC(unitptr r1, unitptr r2, boolean carry)
{
	__asm{
		mov edi,DWORD PTR [global_precision]
		mov	ecx,DWORD PTR [r1]
		mov edx,DWORD PTR [r2]
		xor	esi,esi								// clear esi, which is the offset into the digit arrays
		// cetup carry
		// note that the instruction above clears the carry
		mov	eax,DWORD PTR [carry]
		rcr	eax,1
loop_t3:
		mov	eax,DWORD PTR [ecx + esi * 4]
		mov	ebx,DWORD PTR [edx + esi * 4]
		adc	eax,ebx
		mov DWORD PTR [ecx + esi * 4],eax
		inc	esi
		dec	edi
		jnz	loop_t3
		// compute carry
		rcl	eax,1
		and	eax,1
	}
}


// special version which doesn't read or write a carry.
// we actually call this more often than the full version !
__inline P_ADD(unitptr r1, unitptr r2)
{
	__asm{
		mov edi,DWORD PTR [global_precision]
		mov	ecx,DWORD PTR [r1]
		mov edx,DWORD PTR [r2]
		xor	esi,esi								// clear esi, which is the offset into the digit arrays
loop_t3:
		mov	eax,DWORD PTR [ecx + esi * 4]
		mov	ebx,DWORD PTR [edx + esi * 4]
		adc	eax,ebx
		mov DWORD PTR [ecx + esi * 4],eax
		inc	esi
		dec	edi
		jnz	loop_t3
	}
}

#if defined(SMITH)

#define  mp_quo_digit  P_QUO_DIGIT

extern unit reciph,recipl;
extern int  mshift;

#endif /*#defined(SMITH) */

#endif /* X86 */

#if defined(_M_PPC)
#error "We've not written the PowerPC Code Yet!"
#endif /* _M_PPC */

#if defined(_M_ALPHA)
#error "We've not written the Alpha Code Yet!"
#endif /* _M_ALPHA */

#if defined(_M_MRX000)
#error "We've not written the MIPS Code Yet!"
#endif /* _M_MRX000 */


#pragma warning(default:4035)
