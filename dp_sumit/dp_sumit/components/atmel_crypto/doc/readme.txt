This component is a driver for the Atmel CryptoAuthentication family of chips.  The initial targeted chip is the ATSHA204

Operation
---------
All operations on the Atmel chip call _ATMEL_submitI2cOperation().  Every public API, and icommand just wraps this function

_ATMEL_submitI2cOperation() is in atmel_packets_i2c.c.  This file contains all functions to operate on the Atmel chip. See this file for internal documentation

