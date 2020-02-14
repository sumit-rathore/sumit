Installation and build instructions taken from http://www.codeproject.com/KB/IP/custom_dissector.aspx and http://www.wireshark.org/docs/wsdg_html_chunked/ChSetupWin32.html.

Using Plugin:
1. Install WireShark
2. Copy icron.dll to WireShark/plugins/1.6.x directory
3. Open Wireshark and load plugin. 
   - If there is an error when opening WireShark saying couldn't load module:
     Copy vcredist_x86.exe from src dir and run on your computer.

Needed programs:
1. Visual C++ 2010 Express Edition (If you use a different version you will need uncomment the correct one in the config.nmake file).  

2. Cygwin with modules:     Archive/unzip
                            Devel/bison
                            Devel/flex
                            Interpreters/perl
                            Utils/patch
                            Web/wget
3. Python 2.x
4. Windows platform SDK found here: http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=22668
5. Wireshark source code from: http://www.wireshark.org/download.html (used to build plugin)
6. Installed WireShark to test plugin.

The seperate installed instance of WireShark is not needed if you can compile WireShark from the source code, but that didn't work for me.

Editing dissector:
Create a directory in wireshark-1.6.x/plugins called icron, and into that directory copy:
Makefile.am, Makefile.common, Makefile.nmake, moduleinfo.h, moduleinfo.nmake, packet-icron.c, and plugin.rc.in.
Into the main wireshark-1.6.x directory copy and replace config.nmake.
All actual protocol functionality changes take place in packet-icron.c file.  See comments there for structure and dissector programming tips.

Setup (do once):
    Open a command line and navigate to the main WireShark folder.
    Execute: nmake -f Makefile.nmake verify_tools
             nmake -f Makefile.nmake setup
             nmake -f Makefile.nmake distclean
             nmake -f Makefile.nmake all

    These should work without any errors if you have everything installed, but they will take a while to complete(15+ min for last command).
    If you get "fatal error U1077" when running the last command try adding /I to the end of it, to ignore that error and continue building.
    After building there is supposed to be a wireshark.exe in C:/wireshark/wireshark-gtk2/ but that did not occur for me because of the error.
    If this happens to you you will need a seperate install of WireShark to test the plugin.

Building dissector:

1. Open command line and call C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/vcvars32.bat file (location may differ depending on VS version, 
    this has to be done every time a new command line is used).
2. Navigate to the icron plugin directory.
3. "nmake -f Makefile.nmake distclean" and "nmake -f Makefile.nmake all" clean and build the plugin.
4. After running the make command "icron.dll" will appear in the folder, copy this to the plugins directory of WireShark.
5. WireShark should now automatically use this dissector for icron packets (if building from source worked).
6. If using a seperate install of WireShark, copy icron.dll to the plugins/1.6.x directory of that install.

Useful dissector writing links:
http://www.codeproject.com/KB/IP/custom_dissector.aspx
http://www.wireshark.org/docs/wsdg_html_chunked/ChapterDissection.html
http://anonsvn.wireshark.org/wireshark/trunk/doc/README.developer
Also look at other dissectors' source code in wireshark/epan/dissectors directory
