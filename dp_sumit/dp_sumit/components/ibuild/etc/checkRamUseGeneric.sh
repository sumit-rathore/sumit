# This script is intended to be sourced, which is why it isn't executable and there is no #!/bin/sh

# The following functions are useful for examining the sizes of binaries

# Calculate how much Lex IRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the IRAM
LexAhbRamUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/\.stext/              {stext = "0x"$5}
/\.lexstext/           {lexstext = "0x"$5}
/\.srodata/             {srodata = "0x"$5}
/\.lexsrodata/          {lexsrodata = "0x"$5}
END {
srodataDec=sprintf("%d",srodata);
lexsrodataDec=sprintf("%d",lexsrodata);
stextDec=sprintf("%d",stext);
lexstextDec=sprintf("%d",lexstext);
totalUsedAHBram=sprintf("%d", stext + lexstext + srodata + lexsrodata );
freeDec=sprintf("%d", ('"$2"' * 1024) - stext - srodata - lexstext - lexsrodata);
print "AHBRAM "totalUsedAHBram" Free: "freeDec;
print "     stext "stextDec", srodata "srodataDec", lexstext "lexstextDec", lexsrodata "lexsrodataDec;
print "";
}'
}

RexAhbRamUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/\.stext/              {stext = "0x"$5}
/\.rexstext/           {rexstext = "0x"$5}
/\.srodata/             {srodata = "0x"$5}
/\.rexsrodata/          {rexsrodata = "0x"$5}
END {
srodataDec=sprintf("%d",srodata);
rexsrodataDec=sprintf("%d",rexsrodata);
stextDec=sprintf("%d",stext);
rexstextDec=sprintf("%d",rexstext);
totalUsedAHBram=sprintf("%d", stext + rexstext + srodata + rexsrodata );
freeDec=sprintf("%d", ('"$2"' * 1024) - stext - srodata - rexstext - rexsrodata);
print "AHBRAM  "totalUsedAHBram" Free: "freeDec;
print "     stext "stextDec", srodata "srodataDec", rexstext "rexstextDec" rexsrodata "rexsrodataDec;
print "";
}'
}

# Calculate how much Lex IRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the IRAM
LexIramUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/\.ftext/              {ftext = "0x"$5}
/\.lexftext/           {lexftext = "0x"$5}
/\.atext/              {atext = "0x"$5}
/\.lexatext/           {lexatext = "0x"$5}
/\.rodata/             {rodata = "0x"$5}
/\.lexrodata/          {lexrodata = "0x"$5}
END {
ftextDec=sprintf("%d",ftext);
lexftextDec=sprintf("%d",lexftext);
rodataDec=sprintf("%d",rodata);
lexrodataDec=sprintf("%d",lexrodata);
atextDec=sprintf("%d",atext);
lexatextDec=sprintf("%d",lexatext);
totalUsedIram=sprintf("%d", ftext + lexftext + atext + lexatext + rodata + lexrodata);
freeDec=sprintf("%d", ('"$2"' * 1024) - ftext - lexftext - atext - rodata - lexatext - lexrodata);
print "IRAM  "totalUsedIram", Free: "freeDec;
print "     ftext "ftextDec", lexftext "lexftextDec;
print "     atext "atextDec", rodata "rodataDec", lexatext "lexatextDec", lexrodata "lexrodataDec;
print ""}'
}


# Calculate how much Rex IRAM is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the IRAM
RexIramUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/\.ftext/              {ftext = "0x"$5}
/\.rexftext/           {rexftext = "0x"$5}
/\.atext/              {atext = "0x"$5}
/\.rexatext/           {rexatext = "0x"$5}
/\.rodata/             {rodata = "0x"$5}
/\.rexrodata/          {rexrodata = "0x"$5}
END {
ftextDec=sprintf("%d",ftext);
rexftextDec=sprintf("%d",rexftext);
rodataDec=sprintf("%d",rodata);
rexrodataDec=sprintf("%d",rexrodata);
atextDec=sprintf("%d",atext);
rexatextDec=sprintf("%d",rexatext);
totalUsedIram=sprintf("%d", ftext + rexftext +atext + rexatext + rodata + rexrodata );
freeDec=sprintf("%d", ('"$2"' * 1024) - ftext - rexftext- atext - rodata - rexatext - rexrodata);
print "IRAM  "totalUsedIram", Free: "freeDec;
print "     ftext "ftextDec", rexftext "rexftextDec;
print "     atext "atextDec", rodata "rodataDec", rexatext "rexatextDec" rexrodata "rexrodataDec;
print ""}'
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
/\.lexdata/    {lexdata = "0x"$5}
/\.bss/     {bss = "0x"$5}
/\.lexbss/    {lexbss = "0x"$5}
END {
fstackDec=sprintf("%d",stack);
dataDec=sprintf("%d",data);
lexdataDec=sprintf("%d",lexdata);
bssDec=sprintf("%d",bss);
lexbssDec=sprintf("%d",lexbss);
totalUsedDram=sprintf("%d", stack + data + bss + lexbss + lexdata );
FreeDram= sprintf("%d", ('"$2"' * 1024) - (stack + data + bss + lexbss + lexdata));

print "DRAM  "totalUsedDram" Free: "FreeDram;
print "     stack "fstackDec", data "dataDec", bss "bssDec", lexdata "lexdataDec", lexbss "lexbssDec;
print ""}'
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
/\.rexdata/    {rexdata = "0x"$5}
/\.bss/     {bss = "0x"$5}
/\.rexbss/    {rexbss = "0x"$5}
END {
fstackDec=sprintf("%d",stack);
dataDec=sprintf("%d",data);
rexdataDec=sprintf("%d",rexdata);
bssDec=sprintf("%d",bss);
rexbssDec=sprintf("%d",rexbss);
totalUsedDram=sprintf("%d", stack + data + bss + rexbss);
FreeDram= sprintf("%d", ('"$2"' * 1024) - (stack + data + bss + rexbss));

print "DRAM  "totalUsedDram" Free: "FreeDram;
print "     stack "fstackDec", data "dataDec", bss "bssDec", rexdata "rexdataDec", rexbss "rexbssDec;
print ""}'
}

# Calculate how much Flash is used minus what is available
# Args:
#   $1: elf file
#   $2: Size in KBytes of the Flash
FlashUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/\.atext/              {atext = "0x"$5}
/\.rexatext/           {rexatext = "0x"$5}
/\.lexatext/           {lexatext = "0x"$5}
/\.data/    {data = "0x"$5}
/\.ftext/              {ftext = "0x"$5}
/\.lexftext/           {lexftext = "0x"$5}
/\.lexdata/    {lexdata = "0x"$5}
/\.rodata/             {rodata = "0x"$5}
/\.rexrodata/          {rexrodata = "0x"$5}
/\.lexrodata/          {lexrodata = "0x"$5}
/\.start_text/    {start_text = "0x"$5}
/\.flashcode/    {flashcode = "0x"$5}
/\.flashrodata/     {flashrodata = "0x"$5}
/\.pgmbb/    {pgmbb = "0x"$5}
/\.ge_flshwtr/     {ge_flshwtr = "0x"$5}
/\.ge_fw/    {ge_fw = "0x"$5}
END {
start_textDec=sprintf("%d",start_text);
flashcodeDec=sprintf("%d",flashcode);
flashrodataDec=sprintf("%d",flashrodata);
pgmbbDec=sprintf("%d",pgmbb);
ge_flshwtrDec=sprintf("%d",ge_flshwtr);
ge_fwDec=sprintf("%d",ge_fw);
blackbirdDec=sprintf("%d", ftext + lexftext + atext + rexatext + lexatext + data + lexdata + rodata + stack + rexrodata + lexrodata);
totalUsedFlash=sprintf("%d", start_text + flashcode + flashrodata + pgmbb + ge_flshwtr + ge_fw + ftext + lexftext + atext + rexatext \
                         + lexatext + data + lexdata + rodata + stack + rexrodata + lexrodata);
FreeFlash= sprintf("%d", ('"$2"' * 1024) - (start_text + flashcode + flashrodata + pgmbb + ge_flshwtr + ge_fw+ ftext + lexftext + atext \
                                            + rexatext + lexatext + data + lexdata + rodata + stack + rexrodata + lexrodata));
print "FLASH  "totalUsedFlash" Free: "FreeFlash;
print "     start_text "start_textDec", flashcode "flashcodeDec", flashrodata "flashrodataDec;
print "     blackbird_image "blackbirdDec", pgmbb "pgmbbDec", ge_flshwtr "ge_flshwtrDec", ge_fw "ge_fwDec;
print ""}'
}

PgmBBAhbRamUse ()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
END {
totalUsedAHBram== "0" ;
freeDec=sprintf("%d", ('"$2"' * 1024));
print "AHBRAM  "0" Free: "freeDec;
print "";
}'
}

PgmbbLexIramUse()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/\.start_text/     {start_text = "0x"$5}
/\.ftext/    {ftext = "0x"$5}
/\.data/     {data = "0x"$5}
/\.lexftext/  {lexftext = "0x"$5}
/\.lexatext/  {lexatext = "0x"$5}
/\.lexrodata/ {lexrodata = "0x"$5}
/\.rodata/    {rodata+="0x"$5}
END {
start_textDec=sprintf("%d",start_text);
ftextDec=sprintf("%d",ftext);
rodataDec=sprintf("%d",rodata);
dataDec=sprintf("%d",data);
lexftextDec=sprintf("%d",lexftext);
lexatextDec=sprintf("%d",lexatext);
lexrodataDec=sprintf("%d",lexrodata);

totalUsedIram=sprintf("%d", start_text + data + rodata + ftext + lexftext + lexatext + lexrodata );
FreeIram= sprintf("%d", ('"$2"' * 1024) - (start_text + data + rodata + ftext + lexftext + lexatext + lexrodata ));

print "IRAM  "totalUsedIram" Free: "FreeIram;
print "     start_text "start_textDec", data "dataDec", rodata "rodataDec", ftext "ftextDec;
print "     lexftext "lexftextDec", lexatext "lexatextDec", lexrodata "lexrodataDec;
print ""}'
}

PgmbbRexIramUse()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/.start_text/     {start_text = "0x"$5}
/\.ftext/    {ftext = "0x"$5}
/\.data/     {data = "0x"$5}
/\.rexftext/  {rexftext = "0x"$5}
/\.rexatext/  {rexatext = "0x"$5}
/\.rexrodata/ {rexrodata = "0x"$5}
/\.rodata/    {rodata+= "0x"$5}
END {
start_textDec=sprintf("%d",start_text);
ftextDec=sprintf("%d",ftext);
rodataDec=sprintf("%d",rodata);
dataDec=sprintf("%d",data);
rexrtextDec=sprintf("%d",rexrtext);
rexatextDec=sprintf("%d",rexatext);
rexrodataDec=sprintf("%d",rexrodata);
totalUsedIram=sprintf("%d", start_text + data + rodata + ftext + rexftext + rexatext + rexrodata);
FreeIram= sprintf("%d", ('"$2"' * 1024) - (start_text + data + rodata + ftext + rexftext + rexatext + rexrodata ));

print "IRAM  "totalUsedIram" Free: "FreeIram;
print "     start_text "start_textDec", data "dataDec", rodata "rodataDec", ftext "ftextDec;
print "     rexftext "rexftextDec", rexatext "rexatextDec", rexrodata "rexrodataDec;
print ""}'
}

PgmbbLexDramUse()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/.bss/     {bss = "0x"$5}
/\.stack/    {stack = "0x"$5}
/\.lexbss/   {lexbss = "0x"$5}
END {
bssDec=sprintf("%d",bss);
stackDec=sprintf("%d",stack);
lexbssDec=sprintf("%d",lexbss);
totalUsedDram=sprintf("%d", bss + stack + lexbss);
FreeDram= sprintf("%d", ('"$2"' * 1024) - (bss + stack + lexbss));

print "DRAM  "totalUsedDram" Free: "FreeDram;
print "     bss "bssDec", stack "stackDec", lexbss "lexbssDec;
print ""}'
}

PgmbbRexDramUse()
{
    readelf -e "$1"  | awk '/\[/ { sub("^.*]", ""); print }' | awk --non-decimal-data 'BEGIN { }
/.bss/     {bss = "0x"$5}
/\.stack/    {stack = "0x"$5}
/\.rexbss/   {rexbss = "0x"$5}
END {
bssDec=sprintf("%d",bss);
stackDec=sprintf("%d",stack);
lexbssDec=spritnf("%d",rexbss);
totalUsedDram=sprintf("%d", bss + stack + rexbss);
FreeDram= sprintf("%d", ('"$2"' * 1024) - (bss + stack + rexbss));

print "DRAM  "totalUsedDram" Free: "FreeDram;
print "     bss "bssDec", stack "stackDec", rexbss "rexbssDec;
print ""}'
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

