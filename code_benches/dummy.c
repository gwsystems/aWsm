// !!! HACK ALERT !!!
// We need the __init_libc symbol in the output executable (so the runtime can initialize libc)
// We can't directly export it since it's in a linked library
// Thus we export a dummy function that uses it, forcing it to be included


#define IMPORT __attribute__ ((visibility ("default")))
#define EXPORT __attribute__ ((visibility ("default")))

IMPORT void __init_libc(char **, char *);

void dummy() {
	__init_libc(0, 0);
}

