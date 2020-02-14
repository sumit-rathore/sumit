///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012, 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - random.h
//
//!   @brief - exposed header file for component
//
//!   @note  - Exposes API for random number use
//
///////////////////////////////////////////////////////////////////////////////
#ifndef RANDOM_H
#define RANDOM_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

void getShaRandom(uint8_t *input, uint8_t length) __attribute__((section(".atext")));
// void printShaRandom() __attribute__((section(".atext")));

#endif // RANDOM_H

