ExpressLink readme.txt
======================

makefile build proccess
-----------------------
When this readme.txt was written, expressLink had just migrated to a makefile based build.  To build a release a few steps currently need to be taken

1) place AsicFirmware.bin & flashwriter.icr or flashwriter.bin into ./<project>_obj/
2) define <project> in build/makefile
3) in build/makefile update EXPRESS_LINK_RELEASE_NAME to R03 or whatever release name is required
4) set BOOT_TRANSFER_MODE to stewie for LG1
4) go into build/ and run make 

ExpressLink Structure
---------------------
There are 7 threads used in ExpressLink.
1) The Main GUI thread
2) bgWorkerUpdateProgressBar: to update the progress bar on 5 second intervals
3) bgWorkerUpdateSW: to write bytes to the uart
4) DownloadFlashBoot: runs Stewie to send an image to the board that jump to
   flash.  Used for reading the Software version, created when the
   readSoftwareVersion button is clicked
5) ProgramBoard: runs Stewie to transfer flashwriter & then Xmodem to transfer
   firmware.  Created when the programBoard button is clicked
6) ReadCOMByte: dot Net thread calls this function everytime there are uart
   bytes to be read
7) CloseCOMPort: Created on the file when the user tries to close the uart
   This is to work around a dot net bug

