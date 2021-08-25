#include <stdint.h>

/* In order to avoid making the wasi_backing a gaint macro, the header requires us to:
 * 1. Declare externs to the symbols we want to map. This could be thread-local or global
 * 2. Declare macros CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, and CURRENT_WASI_CONTEXT
 *    with the value of the externs you declared
 * 3. Include the wasi_backing.h header
 */

extern void*    memory;
extern uint32_t memory_size;
extern void*    current_wasi_context;

#define CURRENT_MEMORY_BASE  memory
#define CURRENT_MEMORY_SIZE  memory_size
#define CURRENT_WASI_CONTEXT current_wasi_context

#include "wasi_backing.h"
