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
    uint8_t *src1AsByte = (uint8_t*)src1;
    uint8_t *src2AsByte = (uint8_t*)src2;

    for (; n > 0; n--)
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
