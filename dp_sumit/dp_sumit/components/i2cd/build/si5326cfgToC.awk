BEGIN {
# print standard header
print "//#################################################################################################";
print "// Icron Technology Corporation - Copyright 2015";
print "//";
print "// This source file and the information contained in it are confidential and proprietary to Icron";
print "// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside";
print "// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to";
print "// any employee of Icron who has not previously obtained written authorization for access from the";
print "// individual responsible for the source code, will have a significant detrimental effect on Icron";
print "// and is expressly prohibited.";
print "//#################################################################################################";
print "";
print "//#################################################################################################";
print "// Module Description";
print "//#################################################################################################";
print "// Configuration of jitter chip";
print "//#################################################################################################";
print "";
print "//#################################################################################################";
print "// Design Notes";
print "//#################################################################################################";
print "// TODO";
print "//#################################################################################################";
print "";
print "";
print "// Includes #######################################################################################";
print "#include <ibase.h>";
print "#include \"i2cd_si5326cfg.h\"";
print "";
print "// Constants and Macros ###########################################################################";
print "";
print "// Data Types #####################################################################################";
print "";
print "// Global Variables ###############################################################################";
print "";
print "// Static Variables ###############################################################################";
print "const struct I2cWrite deJitterInit[] =";
print "{";
}

# remove spaces
# ignore commented section
# ignore h
# insert 0x before each number
# encapsulate in curly brackets
# add comma after each line read
!/#/ {
    line=$0;
    gsub(" ","",line);
    split(line,lineArr,",")
    addr=sprintf("0x%02X",lineArr[1]);
    split(lineArr[2],val,"h")
    print "    {" addr ", 0x" val[1] "},";
}

END {
print "};";
print "";
print "// Static Function Declarations ###################################################################";
print "";
print "// Exported Function Definitions ##################################################################";
print "";
print "// Component Scope Function Definitions ###########################################################";
print "uint8_t getDeJitterInitLength(void)";
print "{";
print "    return ARRAYSIZE(deJitterInit);";
print "}";
print "";
};
