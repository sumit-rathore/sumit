/*
 * eeprom_cmd.h
 *
 *  Created on: Sep 15, 2014
 *      Author: kevinb
 */

#ifndef EEPROM_CMD_H_
#define EEPROM_CMD_H_

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(EEPROM_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(EEPROM_icmdReadPage, "Reads the specified page. Args: Page to read, display as words as boolean", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(EEPROM_icmdWritePage, "Writes the specified page. Args: page to write, msw0, lsw0, msw1, lsw1", uint8_t, uint32_t, uint32_t, uint32_t, uint32_t)
ICMD_FUNCTIONS_END(EEPROM_COMPONENT)



#endif /* EEPROM_CMD_H_ */
