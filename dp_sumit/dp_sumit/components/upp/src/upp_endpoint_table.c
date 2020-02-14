///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  upp_endpoint_table.c
//
//!   @brief -  manages the endpoint table
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include "upp_log.h"
#include "upp_loc.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
struct EndpointTableEntry
{
    uint8_t deviceAddress;              // the address of this device
    uint8_t endpoint;                   // it's endpoint
    enum EndpointTransferType type;     // the endpoint type

    uint8_t nextBuffer;               // next in this list
};

// Static Function Declarations ###################################################################

static uint8_t UppAllocateIsoBuffer(void);

// Static Variables ###############################################################################

static struct
{
    // the table is only as big as the buffers we have
    struct EndpointTableEntry table[UPP_MAX_ISO_QUEUE_BUFFERS];

    uint8_t headFreeBuffer; // the first free buffer
    uint8_t headInUseBuffer; // the first buffer that is in use (newest)

} uppTableContext;;

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the endpoint table management
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppEndpointTableInit(void)
{
    UppEndpointTableReinit();
}

//#################################################################################################
// Re-initialize the endpoint allocator - frees all in-use buffers
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppEndpointTableReinit(void)
{
    memset(uppTableContext.table, 0, sizeof(uppTableContext.table));

    for (int index = 0; index < UPP_MAX_ISO_QUEUE_BUFFERS; index++)
    {
        // make sure each transaction is
        // set up the free list
        uppTableContext.table[index].nextBuffer = index+1;
    }

    uppTableContext.headFreeBuffer = 0;  // point to the first free buffer one on the list

    // no buffers/endpoints in use to start
    uppTableContext.headInUseBuffer = UPP_MAX_ISO_QUEUE_BUFFERS;
}

//#################################################################################################
// Allocates and sets an endpoint tracking entry, if required
//
// Parameters:
// Return:
//    UPP_SET_ENDPOINT_NO_ACTION,             // endpoint not set because not the right type, or already set
//    UPP_SET_ENDPOINT_NO_BUFFERS,            // endpoint not set because no free buffers available
//    UPP_SET_ENDPOINT_BUFFER_ALLOCATED       // endpoint tracking buffer set; endpoint still needs to be set in table
//                                            // if UPP_SET_ENDPOINT_BUFFER_ALLOCATED is returned, the lower bits
//                                            // return the buffer to use (1 - UPP_MAX_ISO_QUEUE_BUFFERS)
// Assumptions:
//#################################################################################################
uint8_t UppSetEndpointTableEntry(struct Endpoint *entryContents )
{
    enum UppSetEndpointEntryResult result = UPP_SET_ENDPOINT_NO_ACTION;

    // for now, we only set ISO endpoints with data going from the device to the host
    if (entryContents->info.type == EP_TYPE_ISO)
    {
        result = UppEndpointExists(entryContents->route.deviceAddress, entryContents->info.number);

        if (result == false)
        {
            // endpoint does not exist - try to allocate a buffer for it
            uint8_t freeBuffer = UppAllocateIsoBuffer();

            if (freeBuffer != UPP_MAX_ISO_QUEUE_BUFFERS)
            {
                struct EndpointTableEntry *availableBuffer = &uppTableContext.table[freeBuffer];

                availableBuffer->deviceAddress = entryContents->route.deviceAddress;
                availableBuffer->endpoint      = entryContents->info.number;
                availableBuffer->type          = entryContents->info.type;

                // +1 so buffers go from 1-3, rather then starting at 0 (queue 0 is for Bulk and Interrupt IN endpoints)
                result = UPP_SET_ENDPOINT_BUFFER_ALLOCATED + freeBuffer +1;

                ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_ENDPOINT_ENTRY_SET, entryContents->route.deviceAddress, entryContents->info.number, freeBuffer+1 );
            }
            else
            {
                result = UPP_SET_ENDPOINT_NO_BUFFERS;
            }
        }
        else
        {
            result |= UPP_SET_ENDPOINT_NO_ACTION; // buffer already set!
        }
    }

    return (result);
}


//#################################################################################################
// Checks to see if the given device and endpoint already has a buffer allocated to it
//
// Parameters:
// Return:          buffer number (1 - UPP_MAX_ISO_QUEUE_BUFFERS)  if already allocated, false if not
// Assumptions:
//#################################################################################################
uint8_t UppEndpointExists(uint8_t deviceAddress, uint8_t endpoint)
{
    uint8_t bufferUsed = 0;

    uint8_t nextBuffer = uppTableContext.headInUseBuffer;

    // (index is just here in case the list gets corrupt - sanity check)
    for (int index = 0; (nextBuffer != UPP_MAX_ISO_QUEUE_BUFFERS) && (index < UPP_MAX_ISO_QUEUE_BUFFERS); index++)
    {
        if ( (uppTableContext.table[nextBuffer].deviceAddress == deviceAddress) &&
             (uppTableContext.table[nextBuffer].endpoint == endpoint) )
        {
            bufferUsed = nextBuffer+1;   // this is already allocated!
            break;
        }

        nextBuffer = uppTableContext.table[nextBuffer].nextBuffer;
    }

    return (bufferUsed);
}


//#################################################################################################
// Frees the specified endpoint, returns the buffer number used
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
uint8_t UppFreeEndpointTableEntry(uint8_t deviceAddress, uint8_t endpointNumber )
{
    uint8_t nextBuffer = uppTableContext.headInUseBuffer;
    uint8_t prevBuffer = nextBuffer;

    uint8_t queueId = 0;

    // (index is just here in case the list gets corrupt - sanity check)
    for (int index = 0; (nextBuffer != UPP_MAX_ISO_QUEUE_BUFFERS) && (index < UPP_MAX_ISO_QUEUE_BUFFERS); index++)
    {
        if ( (uppTableContext.table[nextBuffer].deviceAddress == deviceAddress) &&
             (uppTableContext.table[nextBuffer].endpoint == endpointNumber) )
        {
            // take it off the open queue first
            if (nextBuffer == uppTableContext.headInUseBuffer)
            {
                uppTableContext.headInUseBuffer = uppTableContext.table[nextBuffer].nextBuffer;
            }
            else
            {
                uppTableContext.table[prevBuffer].nextBuffer = uppTableContext.table[nextBuffer].nextBuffer;
            }

            queueId = nextBuffer +1;   // save the buffer queue ID, +1 so we go from 1-3, rather then start at 0

            uppTableContext.table[nextBuffer].nextBuffer = uppTableContext.headFreeBuffer;
            uppTableContext.headFreeBuffer = nextBuffer;

            break;
        }

        prevBuffer = nextBuffer;
        nextBuffer = uppTableContext.table[nextBuffer].nextBuffer;
    }

    return(queueId);
}


// Static Function Definitions ####################################################################


//#################################################################################################
// Checks to see if the given device and endpoint already has a buffer allocated to it
//
// Parameters:
// Return:          true if already allocated, false if not
// Assumptions:
//#################################################################################################
static uint8_t UppAllocateIsoBuffer(void)
{
    uint8_t freeBuffer = uppTableContext.headFreeBuffer;
    struct EndpointTableEntry *availableBuffer = NULL;

    // make sure the free list is not empty
    if (freeBuffer != UPP_MAX_ISO_QUEUE_BUFFERS)
    {
        availableBuffer = &uppTableContext.table[freeBuffer];

        // remove it from the free list
        uppTableContext.headFreeBuffer = availableBuffer->nextBuffer;

        memset(availableBuffer, 0, sizeof(struct EndpointTableEntry)); // make sure it is cleared before use!

        // now put it on the open list
        availableBuffer->nextBuffer = uppTableContext.headInUseBuffer;
        uppTableContext.headInUseBuffer = freeBuffer;
    }
    else
    {
        ilog_UPP_COMPONENT_0(ILOG_MINOR_EVENT, UPP_NO_FREE_ISO_BUFFERS);
    }

    return (freeBuffer);
}


