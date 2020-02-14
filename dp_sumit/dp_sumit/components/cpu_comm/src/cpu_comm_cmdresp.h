//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef CPU_COMM_CMDRESP_H
#define CPU_COMM_CMDRESP_H

// Includes #######################################################################################
#include <icmdresp.h>

// Constants and Macros ###########################################################################

// macro use
//ICMDRESP_START( <component name from project_components.h> )
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//ICMDRESP_END( <component name from project_components.h> )

// Sample
//ICMDRESP_START(ICMD_COMPONENT)
//  ICMDRESP_ENTRY(read32, readMemory, BASE_READ_MEM, 1)
//ICMDRESP_END(ICMD_COMPONENT)

ICMDRESP_START(CPU_COMM_COMPONENT)
    ICMDRESP_ENTRY(readCpuMsg, CPU_COMM_ReadCpuMessageIcmd, CPU_COMM_ICMD_READ_MSG, (0, 1, 2))
ICMDRESP_END(CPU_COMM_COMPONENT)

#endif // CPU_COMM_CMDRESP_H
