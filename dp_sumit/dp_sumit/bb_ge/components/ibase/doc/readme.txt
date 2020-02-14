ibase
-----

ibase is the lowest level component in icron.  It defines
* the bare types
* common C macros
* Icron's icomponent system
* C functions that the compiler depends on (memset/memcpy) for basic optimization
* common C structures and routines, which currently include ififo

ibase also includes unit tests and test harnesses to verify the various blocks
of ibase



itypes.h
--------
itypes.h defines the basic following signed and unsigned types, as well as the
fundamental definitions for NULL, TRUE, and FALSE
uint8
sint8
uint16
sint16
uint32
sint32
boolT

itypes.h should be included by all icron C files.



ibase.h
-------
ibase.h contains the common C macros to Icron projects, as well as the
fundamental definitions for memset and memcpy.  ibase assumes that no C
library is available, but the C compiler assumes memset/memcpy are available
for the compiler to use in an optimize for code space optimization.


icomponent.h
------------
This header includes the macros for creating components for a project.  To use
this copy the project_components.h.example file, and follow the sample to
include each component.  Every part of the project that needs to know the
component IDs, only needs to include project_components.h.

icomponent also provides the ability for custom parsers of the
project_components.h file. To do this define the following 3 macros

COMPONENT_PARSER_PREFIX
COMPONENT_PARSER(_name_)
COMPONENT_PARSER_POSTFIX

Everytime a source file wishes to parse the project_components.h, it may
redefine the COMPONENT_PARSER macros, for its use.

Example from icmd:
    // Define all icmd entries for each component as a weak variable
    #define COMPONENT_PARSER_PREFIX 
    #define COMPONENT_PARSER(x) extern const void * icmd_ ## x __attribute__ ((weak));
    #define COMPONENT_PARSER_POSTFIX
    #include <project_components.h>
    #undef COMPONENT_PARSER_PREFIX
    #undef COMPONENT_PARSER
    #undef COMPONENT_PARSER_POSTFIX
    
    // Create an array of the icmd entries for each component
    #define COMPONENT_PARSER_PREFIX static const void * icmd_callbacks[] = {
    #define COMPONENT_PARSER(x) &icmd_ ## x,
    #define COMPONENT_PARSER_POSTFIX };
    #include <project_components.h>
    #undef COMPONENT_PARSER_PREFIX
    #undef COMPONENT_PARSER
    #undef COMPONENT_PARSER_POSTFIX

This results in the following C code, after pre-processing, and reducing the
list to 3 items for readability for this readme file

    extern const void * icmd_TOPLEVEL_COMPONENT __attribute__ ((weak));
    extern const void * icmd_TEST_HARNESS_COMPONENT __attribute__ ((weak));
    extern const void * icmd_ILOG_COMPONENT __attribute__ ((weak));
    
    static const void * icmd_callbacks[] = {
        &icmd_TOPLEVEL_COMPONENT,
        &icmd_TEST_HARNESS_COMPONENT,
        &icmd_ILOG_COMPONENT,
    };


ififo.h
-------
ififo.h defines a single macro that expands to memory allocations for a
buffer and read/write pointers, and a set of static inline functions for
operating on the data.  The API for using the functions to operate on the data
is documented in the header file.  Please note the following 2 design
decisions:
 1) No ififo function uses any locks of any kind.  It is up to the caller to
ensure that no other operation that uses this ififo will occur.
 2) There is no protection from under/over flows in the read and write
functions.

This is done because ififo is intended for a very low level operation, so an
ISR would already know that there is no need for locking.  Since the caller is
responsible for locking, the caller is also responsible for checking bufFull,
bufEmpty, and bufAvail, before calling read and write, as the caller also has
knowledge of when locks were taken and released, and when it is safe to read
or write without under/over flows.

ipool.h
-------
ipool.h defines a single macro that expands to a single memory pool with
unique function handlers for initialization, alloc, and free.  Please note:
1) There are no locks taken of any kind, so the user is responsible for
locking and/or disable interrupts if needed
2) There is no protection from free-ing an non-existant item, or free-ing an
item twice.

Once again these decisions were made to implement a low level quick API


