icmd
====

icmd is a method of calling functions on a target from a development tool.  It allows a developer to export a list of functions with their argument descriptions for each component.  A development tool like Tigger can parse a description of these lists of functions.  Tigger can then present the human debugger with a list of exported functions, and human debugger can call any function and provide the arguments.  On the target side icmd will process the binary command, determine which function to call, setup the arguments, and call the function.

icmd is intended to be used for calling functions such as
* setting the runtime logging level
* viewing the USB topology
* viewing the endpoint configuration of any USB device
* reading registers
* querying the states of various components
* etc

icmd doesn't provide any mechanism for the calling functions to provide feedback.  A system like ilog is intended for this

To provide a deterministic, low overhead implementation, as much work as possible is pushed back to the development tool Tigger.  icmd then only needs to read component numbers, and function indexes from its internal function pointer arrays.  This will provide a deterministic and low overhead implementation.

icmdresp
--------
icmdresp is a mechanism to correlate icmds to ilog responses.  The idea is that when the 2 are correlated a script can be written to send icmds, and wait for the specified ilog message to be returned.  The ilog message will contain the return value for the script to use.  The <component>_cmdresp.h contains all the macros to define the assoication between the ilogs and the icmds.


Important notes about losing uart bytes
---------------------------------------
icmd uses timers to keep track of commands that have timed out.  This requires the user of icmd to ensure that the timer ticks are running, and wouldn't work in a polling mode program.

This is done because the Lionsgate1 chip tends to get a lot of ghost characters show up in the uart buffer from time to time.  icmd will just flush these out if there are not an expected header, or if no following character is received in a reasonable amount of time.

icmd exposes the define ICMD_USE_NON_ACTIVITY_TIMER.  If this is set (C default value for defines is 1), then icmd use its value as the timeout between uart bytes in milliseconds.  If it is not set, then icmd will not check for command timeouts.


Interrupt Initialization & Polling Debug Mode
---------------------------------------------
icmd can be used in 2 methods: as an interrupt driven system, or a polling driven system.  Normally icmd would run off of interrupts and to kick this process off ICMD_Init() must be called at startup.  Intended as debug, a polling mode also exists.  This can be started by calling ICMD_PollingLoop() which will never return, and just continue to poll the uart and process icmd's for ever.  As a debug system it is intended to be called from an assert handler, but could be called anywhere in a program to stop processing and only accept developer icmds.  Please note that it is intended as the final call in a debug process, or iassert, so interrupts are expected to be disabled when called.


Wire protocol
-------------
 
//Note: args are sent in big endian, this makes shifting as they arrive simpler 
// 1byte icmd header
    Upper 5 bits: icmd signature in binary 10011
    Lower 3 bits: # of args (which are all 32 bit) // NOTE: we don't support a 7th arg
// 2nd byte component number
// 3rd byte function number
// Optional: 1st byte of first arg (big endian order)
// Optional: 2nd byte of first arg (big endian order)
// etc. 


Custom Parsing of Component icmd Files and ibuild Processing
------------------------------------------------------------
ibuild creates a customer parser to run through the command file, and produces an output suitable for Tigger/Hobbes to parse.  The output file is broken up into a function name, prefixed by F:, a help string prefixed by H: and an argument list prefixed by A:

For example the following cmd file

ICMD_FUNCTIONS_CREATE(TOPOLOGY_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(ShowDevice, "Show the XSST entry for a single device: Argument is the logical address", uint8)
    ICMD_FUNCTIONS_ENTRY(ShowDeviceAll, "Show the XSST entry for all devices in-sys", void)
    ICMD_FUNCTIONS_ENTRY(ShowTopologyByUsb, "Show the topology table listed by usb address", void)
    ICMD_FUNCTIONS_ENTRY(ShowTopologyByLogical, "Show the topology table listed by logical address", void)
    ICMD_FUNCTIONS_ENTRY(WriteXSSTCmd, "Do a rmw of a word to the XSST: Args USB Address, end point, XSST value, mask", uint8, uint8, uint32, uint32)
ICMD_FUNCTIONS_END(TOPOLOGY_COMPONENT)

will get parsed into the following icmd file for Tigger/Hobbes

component:TOPOLOGY_COMPONENT
F:ShowDevice H:"Show the XSST entry for a single device: Argument is the logical address" A:uint8
F:ShowDeviceAll H:"Show the XSST entry for all devices in-sys" A:void
F:ShowTopologyByUsb H:"Show the topology table listed by usb address" A:void
F:ShowTopologyByLogical H:"Show the topology table listed by logical address" A:void
F:WriteXSSTCmd H:"Do a rmw of a word to the XSST: Args USB Address, end point, XSST value, mask" A:uint8, uint8, uint32, uint32

And the same cmd file will get parsed into C declarations when compiled for the target processor


Best Practices !!!!
-------------------
If an image loaded onto a board, is unknown it is always ideal to be able to find out what that image is, and how to upgrade to a new image.  The following 3 rules should help to ensure that this goal is always obtainable

1) Toplevel component should always be zero
2) Toplevel Ilog for displaying version should should always be message 0
3) Toplevel icmd for upgrading version should always be command 0

<component>_cmd.h to <component>_cmd.o
--------------------------------------
ibuild works with icmd to create an object file for each command file.  These object files are loosely defined as an object file with the single read only array inside that is equivalent to the following C code.

const void * icmd_<component_name>_component_code[] = { &fcn1, &fcn2, &fcn3 }

where fcn1, fcn2 and fcn3 are the icmd functions for this component.

This is done in mostly portable assembly to avoid the C requirements of having each function be declared before referencing it.


