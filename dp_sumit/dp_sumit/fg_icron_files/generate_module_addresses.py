import sys

outfile = open("../inc/module_addresses_regs.h", "w")
infile = open("../inc/bb_chip_a7_regs.h", "r")
outfile.write("/*Icron Technologies ***** Copyright 2018 All Rights Reserved. ******//* \n\
This file includes module starting addresses from file bb_chip_a7_regs.h \n\
This is an automatically generated file please do not delete/modify it*/ \n\
\n\
\n\
\n\
\n\
\n\
#ifndef _MODULE_ADDRESSES_REGS_H_ \n\
#define _MODULE_ADDRESSES_REGS_H_\n\
\n\
\n")
for line in infile:
    if "_s_ADDRESS" in line:
        outfile.write(line)
outfile.write("\n\
#endif /* _MODULE_ADDRESSES_REGS_H_ */ ")
infile.close()
outfile.close()
