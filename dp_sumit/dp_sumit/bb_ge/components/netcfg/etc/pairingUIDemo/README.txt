The build system for this GUI has changed from make to Netbeans' project
system. This program has been successfully compiled on Windows 7 with Netbeans
8.0.2 and Oracle JDK 1.8. It should be possible to build this on Linux via make
without too much effort, but given the low rate of expected churn for this GUI,
it currently doesn't seem necessary.

Notes:
    - The GUI's executable expects xusbnetcfg.exe to be in its directory.
