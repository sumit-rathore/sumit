This test harness can run all of the USB test modes without needing to enumerate the bus.

There are a couple of known issues
1) When switching between tests (with icmd) the ULM and XUSB components are reset to put them back to their default states, however this doesn't reset the USB Phy.  Which can result in residual J or K states.  For a clean test do a power cycle between each test
2) Everytime the test packet test is run, the XCSR_Init function is run to configure the cache.  This function also allocates a timer, which normally isn't an issue as timers are only allocated at init time.  If the icmd is repeatedly run, the system will run out of free timers and assert.  A power cycle will resolve this

