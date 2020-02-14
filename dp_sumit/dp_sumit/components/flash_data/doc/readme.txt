

Programming flash
-----------------
new component that can be used for writing flash variables

flash writer isn't going to use this, because
1) it is already written and works just fine
2) it breaks our coding convention that all code is called with interrupts disabled, unless explicitly noted otherwise
So this component just needs to provide an API to use the data section(s) of flash


Internal
-----------
Always use 64KB flash blocks.  As this seems to be the most widely accepted

Use 2 flash data sectors, once a flash sector is full, populate the other sector, and mark this one as unused

Flash layout
------------
0KB   - 63KB:  Code
64KB  - 127KB: Data section 1
128KB - 191KB: Data section 2
192KB - 255KB: Unused
// In future could have this set with defines in the project options.h file


data flash sector header
------------------------
1st byte of flash
bit flags: (note set is 0 and unset is 1, as flash is written as 0 and erased to 1)
0: active
1: not-active (overrides active)
2-7: unused

data variables
--------------
are continually appended as new variables are added, starting right after the header.  Each one is a variable size

each variable should be stored as
byte 1) header with the following bitfields:
    0-6:    size:       Allows variables up to 127 bytes in size. Stored as bit inverse, so 0x7F indicates size 0, or unused var
    7:      active:     Written to 0 once this variable is no longer used
byte 2) Variable name:  enum of all the variables we support
                        ie enum { MAC_ADDR, STATIC_IP, GATEWAY_IP, NETMASK_IP, USE_DHCP, DST_IP, etc , UNUSED_FLASH=0xFF}
byte 3 - ?) Variable data.  Anywhere from 1 byte to 127 bytes in size


init
----
ping pongs between flash sectors, by doing erase if necessary
looks for duplicate variables, always erases later variable

writing
-------
append to the end of the flash block
if the variable already existed it is erased after the new variable is written
this is to allow atomic replacements, in case of power failures


External
--------
in a project level flash_vars.h (IE GE specific)
enum flash_varName
{
    MAC_ADDR,
    STATIC_IP,
    GATEWAY_IP,
    NETMASK_IP,
    USE_DHCP,
    DST_IP,
    etc.
    FLASH_NUMBER_OF_VARIABLE_NAMES
}

new flash_sfi component API:
struct flash_var { uint8 size; enum flash_varName; uint8 data[127]; };

//using flash data sections:
void  flash_init(void); //garbage collect flash.  Only place where a flash sector can be erased. Does bouncing between data section 1 and data section 2
boolT flash_writeVar(struct flash_var *);    // return TRUE on success; FALSE otherwise (perhaps flash is full, or there is already a write in progress)
void  flash_eraseVar(enum flash_varName);
boolT flash_readVar(struct flash_var *, enum flash_varName); // return TRUE if Variable found and read, FALSE otherwise


Debug
-----
//icmd:
void dumpAllVars(void);
void dumpAllVarsIncludingErased(void);
void writeVar(uint8 size, uint8 flash_varName, uint32 data0_BE, uint32 data1_BE, uint32 data2_BE, uint32 data3_BE); //support vars up to 16 bytes - Max # of icmd args
void eraseVar(uint8 flash_varName);
void readVar(uint8 flash_varName);


