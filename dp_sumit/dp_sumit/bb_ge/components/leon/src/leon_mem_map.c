#include <ibase.h>
#include "leon_mem_map.h"


void LEON_performMemMapCompileTimeAsserts(void)
{
    // The DRAM packed pointer bits should be exactly sufficient to cover the DRAM length
    COMPILE_TIME_ASSERT(LEON_DRAM_LEN  == (2 << (LEON_PACKED_DRAM_POINTER_BITS - 1)));

    // The "word" value should be 2 bits less than the byte value assuming 4-byte words
    COMPILE_TIME_ASSERT(LEON_PACKED_DRAM_POINTER_BITS - LEON_PACKED_WORD_DRAM_POINTER_BITS == 2);
}
