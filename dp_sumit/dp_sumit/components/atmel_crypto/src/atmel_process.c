///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - atmel_init.c
//
//!   @brief - Contains the main initialization function for this module
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <atmel_crypto.h>
#include "atmel_loc.h"
#include <random.h>
// #include <uart.h> //For TEST
/************************ Defined Constants and Macros ***********************/
/**************************** External Data Types ****************************/

extern const struct ATMEL_KeyStore ATMEL_secretKeyStore;
/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
uint8_t macChallenge[ATMEL_MAC_CHALLENGE_SIZE];
void (*checkValidationHandler)(enum ATMEL_processState atmelState, uint8_t *featureBuffer);

/************************ Local Function Prototypes **************************/
static void ATMEL_isChipLockedDone(enum ATMEL_processState atmelState, bool dataAndOtpZonesLocked, bool configZoneLocked);
static void ATMEL_Authenticate(void);
static void ATMEL_AuthenticateDone(enum ATMEL_processState atmelState);
static void ATMEL_EncryptReadDone(bool success, uint8_t *data);
/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ATMEL_init()
*
* @brief  - Initialization for this module
*
* @return - void
*
* @note   -
*
*/
void ATMEL_init(void)
{
    ilog_SetLevel(ILOG_MAJOR_EVENT, ATMEL_CRYPTO_COMPONENT);
    // ilog_SetLevel(ILOG_DEBUG, ATMEL_CRYPTO_COMPONENT);

    ATMEL_i2cInit();
    ATMEL_encryptInit();
}

/**
* FUNCTION NAME: ATMEL_checkValidation()
*
* @brief  - Atmel validation start
*
* @return - void
*
* @note   - Chip lock > Authentification > Read Featurebit
*
*/
void ATMEL_checkValidation(void (*completionHandler)(enum ATMEL_processState atmelState, uint8_t *featureBuffer))
{
    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_DEBUG, ATMEL_VALIDATION);
    checkValidationHandler = completionHandler;
    ATMEL_isChipLocked(&ATMEL_isChipLockedDone);
}

/**
* FUNCTION NAME: ATMEL_isChipLockedDone()
*
* @brief  - call back of ATMEL_checkValidation
*
* @return - void
*
* @note   -
*
*/
static void ATMEL_isChipLockedDone(enum ATMEL_processState atmelState, bool dataAndOtpZonesLocked, bool configZoneLocked)
{
    if(atmelState == ATMEL_LOCKED)        // atmel chipset locked, run authenticate
    {
        ATMEL_Authenticate();
    }
    else                                        // error state
    {
        (*checkValidationHandler)(atmelState, NULL);
    }
}


/**
* FUNCTION NAME: ATMEL_Authenticate()
*
* @brief  - start authenticate
*
* @return - void
*
* @note   -
*
*/
static void ATMEL_Authenticate(void)
{
    getShaRandom(macChallenge, ATMEL_MAC_CHALLENGE_SIZE);
    ATMEL_runMac(ATMEL_secretKeyStore.key[0], macChallenge, &ATMEL_AuthenticateDone);
}

/**
* FUNCTION NAME: ATMEL_AuthenticateDone()
*
* @brief  - completion function for when the ATMEL_Authenticate has completed
*
* @return - void
*
* @note   -
*
*/
static void ATMEL_AuthenticateDone(enum ATMEL_processState atmelState)
{
    if (atmelState == ATMEL_AUTH_PASS)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_DEBUG, MAC_PASSED);

        // Read Feature bits from Atmel
        ATMEL_encryptStart(
            ATMEL_FEATUREBITS_SLOT,
            true,
            NULL,
            &ATMEL_EncryptReadDone
        );
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_ERROR, MAC_FAILED);
        (*checkValidationHandler)(atmelState, NULL);
    }
}

/**
* FUNCTION NAME: ATMEL_EncryptReadDone()
*
* @brief  - completion function for when reading the ATMEL feature has completed
*
* @return - void
*
* @note   - This is the end of the ATMEL_checkValidation process
*
*/
static void ATMEL_EncryptReadDone(bool success, uint8_t *data)
{
    if (success)
    {
        (*checkValidationHandler)(ATMEL_FEATURE_READ, data);
    }
    else
    {
        (*checkValidationHandler)(ATMEL_FEATURE_FAIL, NULL);
    }
}
