/* This is a C++ wrapper around dummy.c. 
 * It is provided in order to enable use of -std and other C++ flags that trigger errors when
 * compiling mixed C and C++ translation units
 */

extern "C"
{
#include "dummy.c"
}
