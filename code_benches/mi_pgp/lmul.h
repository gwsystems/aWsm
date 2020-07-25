/*
 * lmul.h  - Macros for doing a n x n -> 2N multiply.
 *
 * Included by R3000.C, for use in Philip Zimmermann's PGP program.
 * Implemented by Castor Fu, Sep 1992.
 *
 * Define PATCH_ASM if you want to patch the assembly code yourself. . .
 *
 * Currently has support for mips, i386 and 680[2+]0 under GCC.
 *
 * The general (and generally undesired ) case manually
 * performs the multiply by splitting the numbers up into two pieces. . .
 *
 * lmul(i1,i2,ol,oh) takes the product of i1, i2 and yields ol, oh
 * the lower and upper halves of the result.
 */

#ifdef PATCH_ASM
#define	lmul(i1,i2,ol, oh)	(ol=(i1)*(i2), oh=(i1)^(i2))
#else

#if	defined(__GNUC__) && defined(mips)  && defined(MUNIT32)
#define lmul(a,b,xl,xh) \
{ \
	asm("multu	%2, %3\n\tmflo	%0\n\tmfhi	%1" : \
		"=d" (xl) , "=d" (xh) : "d" (a), "d" (b)); \
}
#else 
#if	defined(__GNUC__) && defined(mc68020)  && defined(MUNIT32)
#define	lmul(a,b,xl,xh) \
{\
	xl = a; \
	asm("mulul	%3, %1:%0" : "=d" (xl) , "=d" (xh) : "0" (xl) , \
"d" (b)); \
}
#else
#if	defined(__GNUC__) && defined(i386)  && defined(MUNIT32)
#define	lmul(a,b,xl,xh)	asm("mull %3":"=a" (xl),"=d" (xh):"0" (a),"g" (b))
#else

#define MUNITHMASK	(((unsigned long)1 <<(MULTUNITSIZE/2)) - 1)

#ifdef MUNIT16 
#define lmul(a,b,xl,xh) \
{\
	unsigned long XX = (unsigned long) (a)*(b); \
	xl = XX & 0xffff; \
	xh = XX >> 16; \
}

#else 
#define	lmul(a,b,xl, xh)  \
{\
   MULTUNIT Xaa, Xbb, Xal, Xah, Xbl, Xbh, Xx1,Xx2,Xx3,Xx4,Xx5,Xx6; \
     Xaa= (a); Xbb = (b); Xal = Xaa&MUNITHMASK; Xah = Xaa>>(MULTUNITSIZE/2); \
     Xbl = Xbb&MUNITHMASK; Xbh = Xbb>>(MULTUNITSIZE/2); \
	Xx1 = Xal*Xbl; \
	Xx2 = Xah*Xbl; \
	Xx3 = Xal*Xbh; \
	Xx4 = Xah*Xbh; \
	Xx5 = Xx2+Xx3; \
	Xx4 += ((MULTUNIT) (Xx5 < Xx2)) << (MULTUNITSIZE/2); \
	Xx6 = Xx1 + (Xx5 << (MULTUNITSIZE/2)); \
	Xx4 += (Xx6 < Xx1) + (Xx5>>(MULTUNITSIZE/2)); \
 \
 \
	xl = Xx6; \
	xh = Xx4; \
}
#endif
#endif
#endif
#endif
#endif /* PATCH_ASM */
