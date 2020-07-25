/* Needed by RSAREF */

#ifdef __alpha
typedef unsigned UINT4;
#else
typedef unsigned long UINT4;
#endif

typedef unsigned short UINT2;

typedef void *POINTER;
#define NULL_PTR ((POINTER)0)

#define PROTOTYPES 1
#define PROTO_LIST(x) x
