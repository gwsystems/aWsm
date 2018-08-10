/*	rsagen.h - C include file for RSA public-key key generation routines.

	(c) Copyright 1987 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use 
	of this software, even if the damage results from defects in this 
	software.  No warranty is expressed or implied.  

	RSA-specific routines follow.  These are the only functions that 
	are specific to the RSA public key cryptosystem.  The other
	multiprecision integer math functions may be used for non-RSA
	applications.  Without these functions that follow, the rest of 
	the software cannot perform the RSA public key algorithm.  

	NOTE:  This assumes previous inclusion of "mpilib.h" and "genprime.h"
*/

int rsa_keygen(unitptr n,unitptr e,unitptr d,
	unitptr p,unitptr q,unitptr u,short keybits,short ebits);
	/* Generate RSA key components p, q, n, e, d, and u. */


