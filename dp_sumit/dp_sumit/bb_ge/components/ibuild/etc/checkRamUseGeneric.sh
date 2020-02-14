# This script is intended to be sourced, which is why it isn't executable and there is no #!/bin/sh

# The following functions are useful for examining the sizes of binaries

# Calculate how much Lex IRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the IRAM
LexIramUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/ftext/              {ftext = "0x"$5}
/lextext/            {lextext = "0x"$5}
/icron_xusbcfg/      {icron_xusbcfg = "0x"$5}
/crestron_xusbcfg/   {crestron_xusbcfg = "0x"$5}
/generic_phy/        {generic_phy = "0x"$5}
/broadcom_phy/       {broadcom_phy = "0x"$5}
END { xusbcfg = (icron_xusbcfg > crestron_xusbcfg) ? icron_xusbcfg : crestron_xusbcfg; phytext = (generic_phy > broadcom_phy) ? generic_phy : broadcom_phy; print lextext + ftext + xusbcfg + phytext - ('"$2"' * 1024)}'
}


# Calculate how much Rex IRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the IRAM
RexIramUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/ftext/             {ftext = "0x"$5}
/rextext/           {rextext = "0x"$5}
/icron_xusbcfg/     {icron_xusbcfg = "0x"$5}
/crestron_xusbcfg/  {crestron_xusbcfg = "0x"$5}
/generic_phy/       {generic_phy = "0x"$5}
/broadcom_phy/      {broadcom_phy = "0x"$5}
END { xusbcfg = (icron_xusbcfg > crestron_xusbcfg) ? icron_xusbcfg : crestron_xusbcfg; phytext = (generic_phy > broadcom_phy) ? generic_phy : broadcom_phy; print rextext + ftext + xusbcfg + phytext - ('"$2"' * 1024)}'
}


# Calculate how much Lex DRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the DRAM
LexDramUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/stack/     {stack = "0x"$5}
/\.data/    {data = "0x"$5}
/\.bss/     {bss = "0x"$5}
/lexbss/    {lexbss = "0x"$5}
END {print stack + data + bss + lexbss - ('"$2"' * 1024)}'
}


# Calculate how much Rex DRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the DRAM
RexDramUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/stack/     {stack = "0x"$5}
/\.data/    {data = "0x"$5}
/\.bss/     {bss = "0x"$5}
/rexbss/    {rexbss = "0x"$5}
END {print stack + data + bss + rexbss - ('"$2"' * 1024)}'
}


# Calculate the usage of stack for each function
# Args:
#   $1: elf file
stackSizes ()
{
    sparc-elf-objdump -d "$1" |
        awk '/<.*>:$/ { funcName=$2 }
        /save/ { printf("%.04d, %s\n", -($8), funcName); }
        /add.*\<sp\>.*\<sp\>/ { printf("Grow: %.04d, %s\n", -($8), funcName); }' |
        sort
}

