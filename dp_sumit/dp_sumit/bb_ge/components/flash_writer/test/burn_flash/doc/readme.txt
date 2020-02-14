This test harness performs various tests on the flash chip of a board. There
are four tests, all of which write to 128 KB of flash:

1. Pattern test - writes a 32-bit integer
2. Address test - writes x to address x, e.g. the value 0x30000000 to address
0x30000000, value 0x30000004 to address 0x30000004, etc.
3. Walking zeroes test - writes a 32-bit integer containing one 0, then
shifts the position of the zero, i.e. first bit 0, then second bit 0, etc.
4. Walking ones test - see 3., except the integer contains one 1

Each test contains stat counters for tests that were run/successful/failures
and the number of erase/verification failures. These are retrieved by
getStats() and reset by clearStats().

The startTests() command begins a cycle of tests:
-walking zeroes test
-walking ones test
-0x00000000 pattern test
-0xFFFFFFFF pattern test
-0xAAAAAAAA pattern test
-0x55555555 pattern test
-address test

The stopTests() command stops the cycle begun by startTests(). Note that if
the cycle is running through a walking zeroes or walking ones test, the cycle
will cease after the erase/write/verify loop in progress ends.

