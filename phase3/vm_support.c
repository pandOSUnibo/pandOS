#include <umps3/umps/libumps.h>

#include "vm_support.h"

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)

pteEntry_t swapTable[FRAMENUMBER];
semaphore semSwapPool;