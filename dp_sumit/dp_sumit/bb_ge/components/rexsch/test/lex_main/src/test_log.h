///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
//!   @file  -  test_log.h
//          
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  - 
//    
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_LOG_H
#define TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

// Sample:
//ILOG_CREATE(ILOG_COMPONENT)
//    ILOG_ENTRY(INVALID_COMPONENT, "ILOG Received an invalid component\n")
//    ILOG_ENTRY(INVALID_CODE, "ILOG Received an invalid code\n")
//ILOG_END(ILOG_COMPONENT, ILOG_FATAL_ERROR)

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    // enum.c
    ILOG_ENTRY(PCP, "pcp\n")
    ILOG_ENTRY(DNS_SETUP, "dns setup\n")
    ILOG_ENTRY(DNS_OUT  , "dns out\n")
    ILOG_ENTRY(DNS_PING , "dns ping\n")
    ILOG_ENTRY(UPS_ACK  , "ups ack\n")
    ILOG_ENTRY(UPS_DATA , "ups data\n")
    ILOG_ENTRY(CLR_BCI  , "clr bci\n")
    ILOG_ENTRY(CLR_BCO  , "clr bco\n")
    ILOG_ENTRY(TAG      , "tag 0x%x ac 0x%x\n")
    ILOG_ENTRY(BREQ     , "breq 0x%x\n")
    // lex_main.c
    ILOG_ENTRY(LEX_START         , "Lex Start\n")
    ILOG_ENTRY(INIT_CACHE        , "Init Cache\n")
    ILOG_ENTRY(ALLOCATE_QUEUES   , "Allocate queues\n")
    ILOG_ENTRY(LINK_MGR_INIT     , "link mgr init\n")
    ILOG_ENTRY(XUSB_ENABLE       , "xusb enable\n")
    ILOG_ENTRY(LEX_INIT_COMPLETE , "Lex Init Complete\n")
    ILOG_ENTRY(SEND_CTRL_LINK_PKT, "send ctrl link pkt\n")
    ILOG_ENTRY(GOT_LEX_CTRL_INT  , "got lex ctrl int\n")
    ILOG_ENTRY(CPU_RX_STATUS, "cpu rx, status is %d\n")
    ILOG_ENTRY(LEN, "len 0x%x\n")
    ILOG_ENTRY(DATA, "data : 0x%x\n")
    ILOG_ENTRY(T1, "t1 0x%x\n")
    ILOG_ENTRY(T2, "t2 0x%x\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef TEST_LOG_H

