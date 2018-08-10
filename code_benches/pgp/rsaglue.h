/* rsaglue.h - Glue routines for RSA encryption and decryption */

extern char signon_legalese[];

/* Declarations */
int
rsa_public_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr N);
int
rsa_private_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N);
int
rsa_public_decrypt(byteptr outbuf, unitptr inbuf,
	unitptr E, unitptr N);
int
rsa_private_decrypt(byteptr outbuf, unitptr inbuf,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N);
