#!/usr/bin/env python


# This program is intended to show how the size of functions change in the map
# file between builds.

import sys
import re

def processFile(filename):
    data = []
    with open(filename, 'r') as f:
        for line in f:
            parts = line.split()
            l = len(parts)
            if l == 6 or l == 7:
                section = parts[3]
                size = parts[4]
                function = parts[5 if l == 6 else 6]

                # cleanup the data
                size = int(size, 16)
                function = re.sub(r'(\w+)(\.\d+)+', r'\g<1>', function)

                data.append((section, function, size))
        data.sort()
    return data


def dumpData(d):
    print 'Map File Data Dump'
    print 'SIZE  SECTION FUNCTION'
    print '----------------------'
    for (section, function, size) in d:
        print '%4d %8s %s' % (size, section, function)


def summarizeSizeBySection(d):
    temp = dict()
    for (section, function, size) in d:
        if temp.has_key(section):
            temp[section] = temp[section] + size
        else:
            temp[section] = size

    print 'Map File Summary By Section'
    print 'SECTION      SIZE'
    print '-----------------'
    for k in temp:
        print '%-8s %8d' % (k, temp[k])


def compareMapDataBySection(d1, d2):
    print 'Map File Comparison'
    print 'SECTION   FUNCTION                          F1_SIZE  F2_SIZE    DELTA'
    print '---------------------------------------------------------------------'

    def ellipsis(s, l):
        if(len(s) > l):
            s = s[:(l - 3)] + '...'
        return s

    sectionalDelta = dict()
    def pLine(section, function, f1sz, f2sz):
        delta = f2sz - f1sz
        if sectionalDelta.has_key(section):
            sectionalDelta[section] = sectionalDelta[section] + delta
        else:
            sectionalDelta[section] = delta
        # We don't want to show lines that haven't changed
        if(delta != 0):
            print '%-9s %-32s %8d %8d %8d' % (section, ellipsis(function, 32), f1sz, f2sz, delta)

    i = 0
    j = 0
    while i < len(d1) and j < len(d2):
        (section1, function1, size1) = d1[i]
        (section2, function2, size2) = d2[j]
        c1 = (section1, function1)
        c2 = (section2, function2)
        if c1 == c2:
            pLine(section1, function1, size1, size2)
            i += 1
            j += 1
        elif c1 < c2:
            # This element is in D1 only
            pLine(section1, function1, size1, 0)
            i += 1
        else:
            # This element is in D2 only
            pLine(section2, function2, 0, size2)
            j += 1
    for x in range(i, len(d1)):
        (section1, function1, size1) = d1[x]
        pLine(section1, function1, size1, 0)
    for x in range(j, len(d2)):
        (section2, function2, size2) = d2[x]
        pLine(section1, function1, 0, size2)

    print '---------------------------------------------------------------------'
    print 'SUMMARY'
    totalDelta = 0
    for k in sectionalDelta:
        totalDelta += sectionalDelta[k]
        if sectionalDelta[k] != 0:
            print '%-9s %8d' % (k, sectionalDelta[k])
    print 'TOTAL     %8d' % (totalDelta)



def usage():
    print 'To analyze one file: mapFileAnalyzer.py file.map'
    print 'To compare two files: mapFileAnalyzer.py file1.map file2.map'


if __name__ == '__main__':
    if len(sys.argv) == 2:
        data = processFile(sys.argv[1])
        dumpData(data)
        print
        print
        summarizeSizeBySection(data)
    elif len(sys.argv) == 3:
        data1 = processFile(sys.argv[1])
        data2 = processFile(sys.argv[2])
        compareMapDataBySection(data1, data2)
        pass
    else:
        usage()
