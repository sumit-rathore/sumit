This test harness is for DA testing. Ideally, with this harness, a Rex can be
used as a host. There are various icmds to send packets and read/write to two
64-byte buffers.

Note that some packets require a CRC-16 as the last two bytes. The CRC-16 must
be manually calculated; there should eventually be a Hobbes function to do it.
