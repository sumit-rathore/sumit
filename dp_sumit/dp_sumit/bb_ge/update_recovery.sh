#!/bin/sh

# update the Recovery Image files
# These are for manufacturing to program at the start of flash

# argument is the directory of the recovery release

if [ $# != 1 ]
then
    echo "Please use as \"$0 <path to recovery release\>" >&2
    exit 1
fi

cp "$1"/recoveryImage.bin etc/

