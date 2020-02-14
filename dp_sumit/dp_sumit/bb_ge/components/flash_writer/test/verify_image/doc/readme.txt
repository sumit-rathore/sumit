This test harness checks for differences between an image transferred by XMODEM
and flash memory. Once the harness is loaded, it will send NAKs, waiting for an
image to be transferred. If there are any differences between the image and
flash memory, ilogs will be sent detailing each error; if there are more than
eight, the harness will stop.


In Hobbes 0, one would probably do something like the following:

1. Open a Device with the .icron file for this test harness

2. Ensure the board is in bootloading mode, and you see "Device {SERIAL PORT}
is in bootloading mode" in the console

3. Click "Bootload"

4. Wait until you see "NAK" messages in the DeviceWindow

5. Execute the following code. Note {} denotes areas that must be filled in by
the user. Note {PATH OF .ICRON FILE WITH IMAGE} should be enclosed by r"", e.g.
r"C:\path\to\icron\file\icronfile.icron", and {IMAGE FILE NAME} should be
enclosed by "", e.g. "imagefile.bin".


psi = ProcessStartInfo(programPath + "\\bsdtar.exe",
                       'xjf "{0}" -O {1}'.format(
                       {PATH OF .ICRON FILE WITH IMAGE}, {IMAGE FILE NAME}))
psi.UseShellExecute = False
psi.RedirectStandardOutput = True
psi.CreateNoWindow = True
proc = Process.Start(psi)

b = proc.StandardOutput.CurrentEncoding.GetBytes(
    proc.StandardOutput.ReadToEnd())
send_XMODEM_raw_binary(hobbes.devices[{SERIAL PORT NAME}], b,
                       {IMAGE FILE NAME})

