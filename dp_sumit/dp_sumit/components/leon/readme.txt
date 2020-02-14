Program Startup 
1) As this is buried in a linker file a linker script needs to have
EXTERN(start)
and if imain is also in a library it should be in an EXTERN(imain) as well to force it link in

2) define USE_OPTIONS_H to have the Leon component include <options.h>, otherwise defines should be on the compiler command line

3) The following options are available as optional defines to be set.  Each
one of them differs significantly from standard C, so only use very carefully.
This in intended for the BOOT ROM to save on ROM space.

  LEON_NO_TRAP_INIT             -- no support for interrupts (& traps) will be available
  LEON_NO_MOVE_FTEXT            -- no support for an .ftext memory section
  LEON_NO_MOVE_DATA             -- .data won't be copied from flash to dram at startup
  LEON_NO_CLEAR_BSS             -- .bss will be left uninitialized, instead of zeroed at startup
  LEON_NO_IMAIN_RETURN_SUPPORT  -- the icron void * imain(void); function won't support return from imain with a new function pointer to call

4) The following options are also available, and are intended for standard use
(IE not a non C conforming bootloader startup)

  LEON_DEFAULT_UART_RX_HANDLER  -- set to name of uart handler function



