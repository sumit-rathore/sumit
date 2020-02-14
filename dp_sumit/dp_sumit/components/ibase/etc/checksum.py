#!/usr/bin/env python

import sys
import array
import multiprocessing


def chunkify(seq, chunkSize):
    for i in range(0, len(seq), chunkSize):
        yield seq[i:i + chunkSize]


if __name__ == '__main__':
    # initialize globals
    checksum = 0
    data = array.array('B')

    # determine flash size from args
    if (len(sys.argv) != 2):
        print "Please supply flash size as only argument"
        sys.exit(1)
    try:
        flashSize = int(sys.argv[1])
    except ValueError:
        print "Unable to intpret flash size as a valid integer"
        sys.exit(2)

    # read the whole input file into the array
    f = sys.stdin
    try:
        while True:
            data.fromfile(f, 1024)
    except EOFError:
        pass
    f.close()

    # Split the input data array into chunks and then use a multiprocess map implementation to
    # calculate the checksum of each of the chunks.  Then add the checksum of the chunks together
    # and additional 0xFF's for blank space in the image.  We use half of the CPUs in the system so
    # we don't starve out other processes too badly.
    pool = multiprocessing.Pool(max(1, multiprocessing.cpu_count() / 4))
    chunkedSums = pool.map(sum, chunkify(data, (2 ** 16)), 1)
    checksum = sum(chunkedSums) + (0xFF * (flashSize - len(data)))
    pool.close()
    print hex(checksum)
