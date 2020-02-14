The flashwriter component is responsible for taking a software build, received through the uart using interrupt driven xmodem, and using the leon serial flash interface drivers to program the build into serial flash.  Once the build has been programmed into the serial flash, the flashwriter begins executing the code at the start of flash.  The flashwriter begins by initializing the uart at a baud rate of 115200 and initializing rx interrupts. It then erases the serial flash, and verifies that it has been successfully erased.  It then initializes the xmodem component for interrupt driven mode.  The handler it passes to xmodem takes the packets recieved by xmodem and uses the leon component flash drivers to write them to serial flash.  Once all bytes have been received and written to flash, the flashwriter reads back the serial flash to verify that they have been written correctly.  Once the serial flash has been verified, the flashwriter begins executing the code that it has written to serial flash. 

The flashwriter component is compiled into a standalone binary which can be loaded into and run from IRAM.  The flashwriter component is mainly composed of two source files: flash_writer.c and flash_cmds.h.  Flash_writer.c contains an imain function and is responsible for the program flow described above.  Flash_cmds.h defines instructions that are passed to the leon flash drivers in order to write/verify the flash.

The flashwriter component is also responsible for directly calling all Serial Flash Interface commands.  For the details on the various commands, please read the SFI documentation from any SFI chip.

A project must define FLASHWRITER_ERASE_CHECK_SIZE to the value of desired size of area on the flash to check for a complete erasure.  This could be the size of the flash chip, or a smaller size to save time.

A project makefile must export FLASH_WRITER_LD_FLAGS for the flashwriter

