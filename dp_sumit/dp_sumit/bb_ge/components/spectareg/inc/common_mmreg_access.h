/******************************************************************************
 * 
 * SpectaReg Hardware Abstraction Layer in C (MACRO) API
 *
 * Confidential & Proprietary Property of Productivity Design Tools Inc. (PDTi)
 * Copyright (c) Productivity Design Tools Inc. (PDTi) 2006, 2007. 
 * All rights reserved.
 * $Revision: 1048 $
 *****************************************************************************/

#ifndef _COMMON_MMREG_ACCESS_H_
#define _COMMON_MMREG_ACCESS_H_

#include <itypes.h>
#include <leon_mem_map.h>

#define UINT32 uint32                           //DAVIDM: Changed to match Icron definitions
#define UINT8 uint8                             //DAVIDM: Changed to match Icron definitions
#define COMP_BASE_TYPE UINT32
#define COMP_OFFSET_TYPE UINT32
#define ARRAY_OFFSET_TYPE UINT32
#define REG_VALUE_TYPE UINT32
#define BF_SHIFT_TYPE UINT8
#define REG_TYPE UINT32
#define VOL_REG_VALUE_TYPE REG_VALUE_TYPE       //DAVIDM: No one uses, but odd it isn't a volatile type
#define REG_ADDR_TYPE volatile UINT32 *         //DAVIDM: Added the volatile type, this needs to be here
#define BYTES_PER_REG sizeof(REG_VALUE_TYPE)

/**
 * Reads an on-chip register.
 * 
 * @param compBaseAddr The base address of the component
 * @param compOffset   The register offset relative to compBaseAddr
 * @param arrayOffset   The register array offset relative to compOffset
 * @return             The register value
 */
static inline REG_TYPE READ_REG( 
																COMP_BASE_TYPE   compBaseAddr,
																COMP_OFFSET_TYPE compOffset,
																ARRAY_OFFSET_TYPE arrayOffset
																){
    return LEON_AHB_read(compBaseAddr + compOffset + arrayOffset); // DAVIDM: changed to use LEON driver
}

/**
 * Writes an on-chip register.
 * 
 * @param compBaseAddr The base address of the component
 * @param compOffset   The register offset relative to compBaseAddr
 * @param arrayOffset   The register array offset relative to compOffset
 */
static inline void WRITE_REG( 
														 COMP_BASE_TYPE   compBaseAddr,
														 COMP_OFFSET_TYPE compOffset,
														 ARRAY_OFFSET_TYPE arrayOffset,
														 REG_VALUE_TYPE wrValue  
														 ){
    LEON_AHB_write(compBaseAddr + compOffset + arrayOffset, wrValue); // DAVIDM: changed to use LEON driver
}

static inline REG_TYPE READMASK_REG( 
														 COMP_BASE_TYPE   compBaseAddr,
														 COMP_OFFSET_TYPE compOffset,
														 ARRAY_OFFSET_TYPE arrayOffset,
														 REG_VALUE_TYPE   mask
														 ){
	return  READ_REG(compBaseAddr,compOffset,arrayOffset) & mask;
}

static inline void READMODWRITE_REG(
														COMP_BASE_TYPE   compBaseAddr,
														COMP_OFFSET_TYPE compOffset,
														ARRAY_OFFSET_TYPE arrayOffset,
														REG_VALUE_TYPE   mask,
														REG_VALUE_TYPE   wrValue
														) __attribute__((always_inline));
static inline void READMODWRITE_REG(
														COMP_BASE_TYPE   compBaseAddr,
														COMP_OFFSET_TYPE compOffset,
														ARRAY_OFFSET_TYPE arrayOffset,
														REG_VALUE_TYPE   mask,
														REG_VALUE_TYPE   wrValue
														){
	REG_VALUE_TYPE regValue = READMASK_REG(compBaseAddr,compOffset,arrayOffset,~mask);
	regValue |= wrValue & mask;
	WRITE_REG(compBaseAddr,compOffset,arrayOffset,regValue); 
}

static inline REG_TYPE GET_BF(
														 REG_VALUE_TYPE   mask,
														 BF_SHIFT_TYPE    shift,
														 REG_VALUE_TYPE   regValue
														 ){
	return (regValue & mask) >> shift;
}

static inline REG_VALUE_TYPE SET_BF(
														 REG_VALUE_TYPE   mask,
														 BF_SHIFT_TYPE    shift,
														 REG_VALUE_TYPE   regValue,
														 REG_VALUE_TYPE   bfValue
														 ){
	return (regValue & ~mask) | ((bfValue << shift) & mask);
}

static inline REG_TYPE READ_BF( 
														 COMP_BASE_TYPE   compBaseAddr,
														 COMP_OFFSET_TYPE compOffset,
														 ARRAY_OFFSET_TYPE arrayOffset,
														 REG_VALUE_TYPE   mask,
														 BF_SHIFT_TYPE    shift
															 ){
	return  READMASK_REG(compBaseAddr,compOffset,arrayOffset,mask) >> shift;
}

static inline void WRITE_BF( 
														COMP_BASE_TYPE   compBaseAddr,
														COMP_OFFSET_TYPE compOffset,
														ARRAY_OFFSET_TYPE arrayOffset,
														REG_VALUE_TYPE   mask,
														BF_SHIFT_TYPE    shift,
														REG_VALUE_TYPE   unshiftedValue
														) __attribute__((always_inline)); 
static inline void WRITE_BF( 
														COMP_BASE_TYPE   compBaseAddr,
														COMP_OFFSET_TYPE compOffset,
														ARRAY_OFFSET_TYPE arrayOffset,
														REG_VALUE_TYPE   mask,
														BF_SHIFT_TYPE    shift,
														REG_VALUE_TYPE   unshiftedValue
														){
	READMODWRITE_REG(compBaseAddr,compOffset,arrayOffset,mask,unshiftedValue << shift);
}

static inline void TOGGLE_BF(
														COMP_BASE_TYPE   compBaseAddr,
														COMP_OFFSET_TYPE compOffset,
														ARRAY_OFFSET_TYPE arrayOffset,
														REG_VALUE_TYPE   mask
														){
	WRITE_REG(compBaseAddr,compOffset,arrayOffset,
	    READ_REG(compBaseAddr,compOffset,arrayOffset) ^ mask); // DAVIDM: changed to use LEON driver
}

#endif // _COMMON_MMREG_ACCESS_H_
