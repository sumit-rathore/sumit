//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
// Global Variables ###############################################################################
// Static Variables ###############################################################################
// Static Function Declarations ###################################################################
// Exported Function Definitions ##################################################################

//#################################################################################################
// Determine whether the first n bytes of two memory objects are equal.
//
// Parameters:
//      * src1              - A memory object to compare
//      * src2              - Another memory object to compare
//      * n                 - The number of bytes to compare
// Return:
//      true if the first n bytes of src1 and src2 are equal; false otherwise.
// Assumptions:
//      * src1 and src2 are valid pointers
//#################################################################################################
bool memeq(const void *src1, const void *src2, size_t n)
{
    if (src1 == src2)
    {
        return true;
    }

    uint32_t *src1AsWord = (uint32_t*)src1;
    uint32_t *src2AsWord = (uint32_t*)src2;

    // major passes at word granularity -- if either src isn't word-aligned,
    // fall back to byte-by-byte comparison since SPARC traps on unaligned word accesses
    uint32_t majorPasses = IS_ALIGNED(src1) && IS_ALIGNED(src2) ? n/sizeof(uint32_t) : 0;
    uint32_t minorPasses = n - sizeof(uint32_t) * majorPasses;

    for (; majorPasses > 0; majorPasses--)
    {
        if (*src1AsWord++ != *src2AsWord++)
        {
            return false;
        }
    }

    // handle last few bytes at byte granularity
    uint8_t *src1AsByte = (uint8_t*)src1AsWord;
    uint8_t *src2AsByte = (uint8_t*)src2AsWord;
    for (; minorPasses > 0; minorPasses--)
    {
        if (*src1AsByte++ != *src2AsByte++)
        {
            return false;
        }
    }

    return true;
}


// Component Scope Function Definitions ###########################################################
// Static Function Definitions ####################################################################
