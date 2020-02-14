#!/usr/bin/env python3

import sys


if __name__ == '__main__':
    # initialize globals
    checksum = 0
    bytes_read = 0

    # determine flash size from args
    if (len(sys.argv) != 2):
        sys.stderr.write("Please supply flash size as only argument\n")
        sys.exit(1)
    try:
        flashSize = int(sys.argv[1])
    except ValueError:
        sys.stderr.write("Unable to interpret flash size as a valid integer\n")
        sys.exit(2)

    f = sys.stdin.buffer
    l = None
    while l != 0:
        bs = f.read(1024)
        l = len(bs)
        checksum += sum(bs)
        bytes_read += l

    checksum += (0xFF * (flashSize - bytes_read))
    print(hex(checksum))
