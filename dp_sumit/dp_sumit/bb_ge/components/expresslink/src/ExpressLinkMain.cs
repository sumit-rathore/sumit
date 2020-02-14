///////////////////////////////////////////////////////////////////////////////
//
//   Icron Technology Corporation - Copyright 2009 - 2014
//
//
//   This source file and the information contained in it are confidential and
//   proprietary to Icron Technology Corporation. The reproduction or disclosure,
//   in whole or in part, to anyone outside of Icron without the written approval
//   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
//   Icron who has not previously obtained written authorization for access from
//   the individual responsible for the source code, will have a significant
//   detrimental effect on Icron and is expressly prohibited.
//
///////////////////////////////////////////////////////////////////////////////

//#define DEBUG
//#define CONSOLE_DEBUG

using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Management;
using System.Text.RegularExpressions;



[assembly: CLSCompliant(true)]
namespace ExpressLink
{
    // An exception that is thrown when checking for a platform identifier and an unexpected or
    // unsupported value is found.
    public class UnsupportedPlatformException: Exception
    {
        public UnsupportedPlatformException(ExpressLink.PlatformId platformId, string exceptionSource)
            : base(String.Format("Unsupported Platform ({0}): {1}", platformId, exceptionSource))
        {
        }
    }


    public partial class ExpressLink : Form
    {
        /*
         * Message values needed to intercept when a device is added
         * or removed from the system.
         */
        private const int WM_DEVICECHANGE = 0x0219;
        private const int DBT_DEVICEARRIVAL = 0x8000;
        private const int DBT_DEVICEREMOVECOMPLETE = 0x8004;

        private Boolean COMPortConnected = false;

        private string currentUartInput;

#if (DEBUG)
#if (CONSOLE_DEBUG)
        [DllImport("kernel32.dll", EntryPoint = "AllocConsole", SetLastError = true, CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        private static extern int AllocConsole();
#endif
#endif

        StringBuilder loadFlashBootCaptureString;
        MemoryStream loadFlashBootCaptureBytes;
        //private bool iLogVersionHeaderDetected = false;

        private const int R01_or_R02_BootMsgPreamble = 64; // old ascii output
        private const int R03_or_greater_BootMsgPreamble = 32; // ilog binary message output

        private const int GE_iLogLen = 17;

        // The minimum length of the text version string + the length of one ilog
        // e.g. "Goldenears SW 1.2.3\r\n" followed by an ilog
        private const int GE_BootMsgPreambleMinLen = 21 + GE_iLogLen;

        // The maximum length of the text version string + the length of one ilog
        // e.g. "Goldenears SW 255.255.255\r\n" followed by an ilog
        private const int GE_BootMsgPreambleMaxLen = 27 + GE_iLogLen;
        //private int bytesReceived = 0;

        // private String LogFileName = Path.GetTempFileName().Replace(".tmp", ".txt");
        // private StreamWriter LogFile;

        private SerialPort comPort = new SerialPort();

        Dictionary<string, string> lgVersions = new Dictionary<string, string>()
        {
            {"2.0.9",  "R03"},
            {"3.0.4",  "R04"},
            {"3.0.8",  "R05/C05"},
            {"3.1.9",  "R06/C06"},
            {"3.1.14", "R07"},
            {"2.0.12", "C01"},
            {"3.0.6",  "C03"},
            {"1.0.6",  "R01"},
            {"1.1.9",  "R02"}
        };

        Dictionary<string, string> geVersions = new Dictionary<string, string>()
        {
            {"0.9.1",  "N01"},
            {"0.10.1", "N02"},
            {"1.0.0",  "N03"},
            {"1.0.1",  "N04"},
            {"1.2.2",  "N05"},
            {"1.3.2",  "N06"},
            {"1.3.8",  "N07"}
        };

        enum RunState
        {
            ReadBootLoader,             // Watching for bootloader startup sequence
            ReadBootLoaderGE,           // Second phase of bootloader startup (GE only)
            ReadBootLoaderComplete,     // The bootloader information has been parsed
            UpdateSoftwareFlashWriter,
            UpdateSoftwareFirmware,
            LoadFlashBoot,
            ReadSoftwareVersion,
            Done
        }
        private RunState CurrState = RunState.ReadBootLoader;

        enum ChipType
        {
            LionsGate,
            GoldenEars
        }
        private ChipType chipType;

        // Only used for GE
        enum UnitSide
        {
            Lex,
            Rex
        }
        private UnitSide unitSide;

        // Only used for GE
        public enum PlatformId
        {
            // Values need to be synchronized with GRG_PlatformID from /components/grg/inc/grg.h
            KintexDevBoard,
            SpartanProduct,
            AsicProduct
        }
        private PlatformId platformId;
        private string fpgaVersion;
        private string bootloaderVersion;


        enum Proto { Stewie, Xmodem_cs };
        private IProgrammer myFileSender;

        private Stream comPortStream;

        /*
         * NOTE: Before calling thread.start(), make sure the thread property thread.IsBackground is set to TRUE.
         *       Threads with this property set to true will exit gracefully when the main thread of the process
         *       is terminated.
         */

        public delegate void UpdateLogWindowCallback(String text);
        private delegate void UpdateProgressCallback(float in_nValue);
        private delegate void SetProgressVisibilityCallback(Boolean visible);
        private delegate void bgWorkerUpdateProgressBar_DoWork_Callback(object sender, DoWorkEventArgs e);
        private delegate void SetDownloadButtonEnabledCallback(Boolean ButtonEnabled, Boolean ProgressBarVisible);
        private delegate void SetGenerateDebugLogTextCallback(String text, Boolean buttonStatus);
        private delegate void SetStatusLabelTextCallback(String text);
        private delegate void SerialPortDataReceivedEventHandler();
        private delegate void SerialPortDataReceivedProcessCallback(byte b);

        static System.Windows.Forms.Timer uartTimer = new System.Windows.Forms.Timer();

        private void debugMsg(String str)
        {
#if (DEBUG)
#if (CONSOLE_DEBUG)
            Console.WriteLine(str);
#else
            UpdateLogWindowDelegate(str);
#endif
#endif
        }

        /// <summary>
        /// Overrides the WndProc function to intercept messages and handle it if it
        /// is a device add or removal message and the device added/removed is a COM
        /// port.
        /// </summary>
        /// <param name="m"></param>
        [System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Demand, Name = "FullTrust")]
        protected override void WndProc(ref Message m)
        {
            Int32 deviceType;
            String deviceName;

            /*
             * We only care about it if the message is about a device change
             * on the computer.
             */
            if (m.Msg == WM_DEVICECHANGE)
            {
                /*
                 * Filter out other device add/remove messages as we are only
                 * interested in the message that tells us the device is ready
                 * to be used or completely removed.
                 */

                if (m.WParam.ToInt32() == DBT_DEVICEARRIVAL || m.WParam.ToInt32() == DBT_DEVICEREMOVECOMPLETE)
                {
                    /*
                     * Message.LParam returns a pointer to a structure identifying the device inserted.
                     * The structure consists of an event-independent header, followed by event-dependent
                     * members that describe the device.
                     *
                     * We are interested in the DEV_BROADCAST_PORT Structure because this is the one that
                     * tells us a port device was inserted or removed.
                     *
                     * The structure of DEV_BROADCAST_PORT looks like this
                     *
                     * typedef struct _DEV_BROADCAST_PORT {
                     *      DWORD dbcp_size;
                     *      DWORD dbcp_devicetype;
                     *      DWORD dbcp_reserved;
                     *      TCHAR dbcp_name[1];
                     *  }DEV_BROADCAST_PORT *PDEV_BROADCAST_PORT;
                     *
                     * The rest is just pointer arithmetic.
                     */

                    deviceType = Marshal.ReadInt32((IntPtr)(m.LParam.ToInt32() + sizeof(Int32)));
                    deviceName = Marshal.PtrToStringUni((IntPtr)(m.LParam.ToInt32() + 3 * sizeof(Int32)));

                    if (deviceType == 3 && deviceName.Contains("COM"))
                    {
                        RescanCOMPorts();
                    }
                }

            }

            base.WndProc(ref m);
        }

        /// <summary>
        /// Initializes the ExpressLink with default port settings.
        /// </summary>
        public ExpressLink()
        {
            this.loadFlashBootCaptureString = new StringBuilder("", 500);
            this.loadFlashBootCaptureBytes = new MemoryStream();

            InitializeComponent();


            /*
             * Removes the need of changing the window size and the text box visibility
             * between debugging build and release build.
             */
#if (DEBUG)
#if (CONSOLE_DEBUG)
            AllocConsole();
#else
            this.textBoxLogWindow.Visible = true;
            this.Height = 350;
            this.Text = "ExpressLink DEBUG VERSION";
#endif
#endif
            this.currentUartInput = "";

            // comPort.DiscardNull = true;
            comPort.BaudRate = 115200;
            comPort.Parity = Parity.None;
            comPort.DataBits = 8;
            comPort.StopBits = StopBits.One;
            comPort.Handshake = Handshake.None;
            comPort.DataReceived += new SerialDataReceivedEventHandler(ReadCOMByte); //TODO: not supported in Mono 2.6.7

            uartTimer.Tick += new EventHandler(UartTimerProcessor);
            // Sets the timer interval to 15 seconds.
            uartTimer.Interval = 15000;
            uartTimer.Start();

            // Override the default label for a more sensible name
            updateLabelWithReleaseInfo(config.ExpressLinkReleaseName);

            // Start up background worker to update progress bar
            this.bgWorkerUpdateProgressBar.RunWorkerAsync();

            // LogFile = new StreamWriter(LogFileName);
            // LogFile.AutoFlush = true;
            // WriteLogFileHeader();
        }


        /// <summary>
        /// Populate the version label and the list of available serial ports combo box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ExpressLink_Load(object sender, EventArgs e)
        {
            toolStripStatusLabel.Text = "";

            List<String> portList = SerialPort.GetPortNames().ToList();
            portList.Sort();

            foreach (String portName in portList)
            {
                comboBoxCOMPort.Items.Add(portName);
            }

            if (portList.Count > 0)
            {
                comboBoxCOMPort.SelectedIndex = 0;
            }

            ToolTip TooltipConnect = new ToolTip();
            TooltipConnect.SetToolTip(
                this.buttonConnect,
                "Once the COM port is selected use this buttom to connect \r\n" +
                "to the Icron ExtremeUSB device.");
            TooltipConnect.AutoPopDelay = 15000;
            TooltipConnect.IsBalloon = true;

            ToolTip TooltipUpdateSoftware = new ToolTip();
            TooltipUpdateSoftware.SetToolTip(
                this.buttonUpdateSoftware,
                "Once selected, the Icron ExtremeUSB device will be \r\n" +
                "programmed. A status bar shows the progress. Do not \r\n" +
                "select this option while the status bar is in progress.");
            TooltipUpdateSoftware.AutoPopDelay = 15000;
            TooltipUpdateSoftware.IsBalloon = true;
        }


        /// <summary>
        /// Open up the standard About box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonHelp_Click(object sender, EventArgs e)
        {
            Help help = new Help();
            help.ShowDialog();
        }


        /// <summary>
        /// Open the selected COM port in the combo box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonConnect_Click(object sender, EventArgs e)
        {
            if (comboBoxCOMPort.SelectedIndex < 0)
            {
                MessageBox.Show("Please select a COM port first.", "ERROR!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                OpenCloseCOMPort();
            }
        }


        /// <summary>
        /// Open or close the selected COM port.
        ///
        /// Check if the port is available before actually opening it.
        /// Catch all the exceptions and base on the result of the open
        /// action, change the button text and color accordingly.
        /// </summary>
        private void OpenCloseCOMPort()
        {
            if (!COMPortConnected)
            {
                comPort.PortName = comboBoxCOMPort.SelectedItem.ToString();

                try
                {
                    comPort.Open();
                    /*
                     * We need to save the BaseStream of this newly opened port. This is part of the workaround
                     * to fix the problem where an untrappable UnauthorizedAccessException would be thrown by
                     * .NET Framework when a disconnection on the USB COM port crashes the Serial Port.
                     *
                     * More information can be found at
                     * https://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=140018
                     */
                    comPortStream = comPort.BaseStream;
                    /*
                     * The line of code above this comment should be kept until the bug is fixed in .NET Framework
                     * and this exception handled properly
                     */
                    COMPortConnected = true;
                    buttonConnect.BackColor = System.Drawing.Color.LightGreen;
                    buttonConnect.Text = "Disconnect";
                    comboBoxCOMPort.Enabled = false;
                    toolStripStatusLabel.Text = "Waiting to enter boot loader mode ...";
                    CurrState = RunState.ReadBootLoader;
                }
                catch (UnauthorizedAccessException e)
                {
                    MessageBox.Show(comPort.PortName + " is already in use. " + e.Message, "Port Already in Use", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1, MessageBoxOptions.RtlReading);
                }
                catch (IOException e)
                {
                    MessageBox.Show(e.Message, "Port Does Not Exist", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                Thread threadCloseSerialPort = new Thread(new ThreadStart(CloseCOMPort));
                threadCloseSerialPort.IsBackground = true;
                threadCloseSerialPort.Start();

                COMPortConnected = false;
                buttonConnect.BackColor = System.Drawing.SystemColors.Window;
                buttonConnect.Text = "Connect";
                comboBoxCOMPort.Enabled = true;
                buttonUpdateSoftware.Enabled = false;
                buttonReadSoftware.Enabled = false;
                SetStatusLabelText("");
            }
        }

        // Work around for a bug by design in .NET serial port handling
        // See http://blogs.msdn.com/b/bclteam/archive/2006/10/10/top-5-serialport-tips-_5b00_kim-hamilton_5d00_.aspx tip #3
        // Summary: You can't close a serial port from the main thread, as that can cause a deadlock.  So the serial port is closed in a tmp thread
        private void CloseCOMPort()
        {
             /*
             * Part of the workaround for the bug stated above. We only catch UnauthorizedAccessException since
             * this is the only one that we want to surpress and prevent it from crashing the application.
             */
            try
            {
                comPortStream.Close();
            }
            catch (UnauthorizedAccessException)
            {
            }
            /*
             * Workaround code ends here.
             */

            try
            {
                comPort.Close();
            }
            catch (UnauthorizedAccessException e)
            {
                MessageBox.Show(comPort.PortName + " is already in use. " + e.Message, "Port Already in Use", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1, MessageBoxOptions.RtlReading);
            }
        }


        /// <summary>
        /// Reads the Software version
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonReadSoftware_Click(object sender, EventArgs e)
        {
            // SetStatusLabelText("Read Button Pressed");
            debugMsg("CurrState = " + CurrState);

            if (CurrState == RunState.ReadBootLoaderComplete)
            {
                if (comPort != null && comPort.IsOpen)
                {
                    // Reset all state related to tracking version #'s before kicking off a new thread
                    SetButtonsAvailability(false, true);
                    //this.iLogVersionHeaderDetected = false;
                    CurrState = RunState.LoadFlashBoot;
                    this.loadFlashBootCaptureString = new StringBuilder("", 500);
                    this.loadFlashBootCaptureBytes = new MemoryStream();
                    SetStatusLabelText("Loading Flash Boot");

                    // Start sending the file for LoadFlashBoot
                    switch (this.chipType)
                    {
                        case ChipType.LionsGate:
                            SendStewie("leon_boot_flash.icr", Properties.Resources.leon_boot_flash_icr, '@');
                            break;

                        case ChipType.GoldenEars:
                            if (this.platformId == PlatformId.SpartanProduct)
                            {
                                SendXmodem(
                                    "leon_boot_flash_w_recovery.bin",
                                    Properties.Resources.leon_boot_flash_w_recovery);
                            }
                            else if (this.platformId == PlatformId.AsicProduct ||
                                     this.platformId == PlatformId.KintexDevBoard)
                            {
                                SendXmodem("leon_boot_flash.bin", Properties.Resources.leon_boot_flash);
                            }
                            else
                            {
                                throw new UnsupportedPlatformException(this.platformId, "read software version");
                            }
                            break;
                    }
                }
                else
                {
                    MessageBox.Show("Please connect to a COM port first.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                debugMsg("ReadSoftware button clicked in the wrong state " + CurrState);
            }
        }



        /// <summary>
        /// Initializes the download process
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonUpdateSoftware_Click(object sender, EventArgs e)
        {
            /*
             * Every time Update Software is clicked, it is assumed that whatever
             * log information kept before this point is no longer relevant thus
             * can be safely discarded.
             */
            if (comPort != null && comPort.IsOpen)
            {
                SetButtonsAvailability(false, true);

                CurrState = RunState.UpdateSoftwareFlashWriter;
                if (this.chipType == ChipType.LionsGate)
                {
                    debugMsg(
                        "Calling SendStewie(), Properties.Resources.flash_writer_lg = " +
                        Properties.Resources.flash_writer_lg.Length);
                    SendStewie("flash_writer_lg", Properties.Resources.flash_writer_lg, '@');
                }
                else if (this.chipType == ChipType.GoldenEars)
                {
                    if (this.platformId == PlatformId.SpartanProduct)
                    {
                        SendXmodem("flash_writer_ge_spartan", Properties.Resources.flash_writer_ge_spartan);
                    }
                    else if (this.platformId == PlatformId.AsicProduct ||
                             this.platformId == PlatformId.KintexDevBoard)
                    {
                        SendXmodem("flash_writer_ge", Properties.Resources.flash_writer_ge);
                    }
                    else
                    {
                        throw new UnsupportedPlatformException(this.platformId, "update software");
                    }
                }
            }
            else
            {
                MessageBox.Show(
                    "Please connect to a COM port first.",
                    "Error",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
            }
        }


        /// <summary>
        /// SerialPort.DataReceived event handler. If ExpressLink is in programming state, pass
        /// the character received to the serial data parser, else just print the line to the
        /// log window.
        /// </summary>
        private void ReadCOMByte(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                while (comPort != null && comPort.IsOpen && comPort.BytesToRead > 0)
                {
                    Invoke(new SerialPortDataReceivedProcessCallback(processReceivedByte), (byte)comPort.ReadByte());
                }
            }
            catch (Exception ex)
            {
                debugMsg("Exception caught while reading serial port " + comPort.ToString() + ": " + ex.Message);
            }
        }

        private void processReceivedByte(byte ReceivedByte)
        {
            debugMsg("processReceivedByte(" + ReceivedByte + ")");

            uartTimer.Stop();
            uartTimer.Start();

            if (this.CurrState == RunState.ReadBootLoader ||
                this.CurrState == RunState.ReadBootLoaderComplete ||
                this.CurrState == RunState.Done)
            {
                if (ReceivedByte == (byte)'\n' || ReceivedByte > 127)
                {
                    // Reset the line because of a newline or non-ascii character
                    this.currentUartInput = "";
                }
                else
                {
                    this.currentUartInput = this.currentUartInput + (char)ReceivedByte;
                    // 1.3 is hardcoded because there is only one version of the LG bootloader
                    if (Regex.Match(this.currentUartInput, @"BtLdr: v1.3").Success)
                    {
                        this.chipType = ChipType.LionsGate;
                        this.currentUartInput = "";
                        SetStatusLabelText("LionsGate Bootloader Version 1.3 Detected");
                        buttonUpdateSoftware.Enabled = true;
                        buttonReadSoftware.Enabled = true;
                        this.CurrState = RunState.ReadBootLoaderComplete;
                    }
                    else
                    {
                        Match match = Regex.Match(
                            this.currentUartInput,
                            @"Goldenears (?<fpgaVersion>[0-9A-F]+\.[0-9A-F]+\.[0-9A-F]+) Bootloader version (?<btldrVersion>\d+\.\d+)");
                        if (match.Success)
                        {
                            this.chipType = ChipType.GoldenEars;
                            this.currentUartInput = "";
                            this.fpgaVersion = match.Groups["fpgaVersion"].Value;
                            this.bootloaderVersion = match.Groups["btldrVersion"].Value;
                            this.CurrState = RunState.ReadBootLoaderGE;
                        }
                    }
                }
            }
            else if(this.CurrState == RunState.ReadBootLoaderGE)
            {
                if (ReceivedByte > 127)
                {
                    this.currentUartInput = "";
                    this.CurrState = RunState.ReadBootLoaderComplete;
                }
                else
                {
                    this.currentUartInput = this.currentUartInput + (char)ReceivedByte;
                    Match match = Regex.Match(
                        this.currentUartInput,
                        @"(?<unitSide>[LR]ex) PlatformID 0x(?<platformId>[0-9a-fA-F][0-9a-fA-F])");
                    if (match.Success)
                    {
                        debugMsg("Success platformID is 0x" + match.Groups["platformId"].Value);
                        this.unitSide = match.Groups["unitSide"].Value == "Lex" ? UnitSide.Lex : UnitSide.Rex;
                        this.platformId = (PlatformId)Convert.ToUInt32(match.Groups["platformId"].Value, 16);
                        this.currentUartInput = "";
                        SetStatusLabelText(String.Format(
                            "GoldenEars Bootloader {0}, FPGA {1}, {2} PlatformID {3} Detected",
                            this.bootloaderVersion,
                            this.fpgaVersion,
                            this.unitSide,
                            this.platformId));
                        buttonUpdateSoftware.Enabled = true;
                        buttonReadSoftware.Enabled = true;
                        this.CurrState = RunState.ReadBootLoaderComplete;
                    }
                    else if(this.currentUartInput.Length > 25)
                    {
                        this.currentUartInput = "";
                        // The message we are searching for should come directly after the string
                        // that got us into this state, so if we haven't seen it by 25 characters,
                        // we know that something has gone wrong.  25 is somewhat arbitrary, but
                        // it's long enough.
                        this.CurrState = RunState.ReadBootLoaderComplete;
                    }
                }
            }
            else if (CurrState == RunState.UpdateSoftwareFlashWriter ||
                     CurrState == RunState.UpdateSoftwareFirmware ||
                     CurrState == RunState.LoadFlashBoot)
            {
                debugMsg("In readCOMByte(), CurrState = " + CurrState + " read byte: 0x" + ReceivedByte.ToString("x"));
                myFileSender.SignalRecievedChar(ReceivedByte);
            }
            else if (CurrState == RunState.ReadSoftwareVersion)
            {
                byte[] buf2 = new byte[1];
                buf2[0] = ReceivedByte;
                string asciiByte = System.Text.Encoding.UTF8.GetString(buf2);
                this.loadFlashBootCaptureString.Append(asciiByte);
                this.loadFlashBootCaptureBytes.WriteByte(ReceivedByte);
                string versionText = "";
                Dictionary<string, string> versionLookup = null;
                if (this.chipType == ChipType.LionsGate)
                {
                    versionLookup = this.lgVersions;
                    if (this.loadFlashBootCaptureBytes.Length > ExpressLink.R03_or_greater_BootMsgPreamble)
                    {
                        byte[] bytesReceived = this.loadFlashBootCaptureBytes.ToArray();

                        const byte lgLinkmgrComponentIndex = 0x0f;
                        const byte lgLocalSwVersionIndex = 0x2a;
                        const uint numParams = 3;
                        int matchIndex = this.searchForILog(
                            bytesReceived, lgLinkmgrComponentIndex, lgLocalSwVersionIndex, numParams);
                        if (matchIndex != -1)
                        {
                            versionText = String.Format(
                                "{0}.{1}.{2}",
                                bytesReceived[matchIndex + 7],
                                bytesReceived[matchIndex + 11],
                                bytesReceived[matchIndex + 15]);
                        }
                    }
                    // Haven't matched yet so start checking for old ascii based messages
                    if (versionText.Length == 0 &&
                        this.loadFlashBootCaptureString.Length > ExpressLink.R01_or_R02_BootMsgPreamble)
                    {
                        string bootString = this.loadFlashBootCaptureString.ToString();
                        Match match = Regex.Match(bootString, @"LGSW_v(?<major>\d{2})_(?<minor>\d{2})_(?<revision>\d{2})");
                        if (match.Success)
                        {
                            versionText = match.Groups["major"] + "." + match.Groups["minor"] + "." + match.Groups["revision"];
                        }
                    }
                }
                else if (this.chipType == ChipType.GoldenEars &&
                         this.loadFlashBootCaptureBytes.Length > ExpressLink.GE_BootMsgPreambleMinLen)
                {
                    versionLookup = this.geVersions;
                    byte[] bytesReceived = this.loadFlashBootCaptureBytes.ToArray();

                    const byte geToplevelComponentIndex = 0;
                    const byte geSoftwareVersionIndex = 0;
                    const byte numParams = 3;
                    int matchIndex = this.searchForILog(
                        bytesReceived, geToplevelComponentIndex, geSoftwareVersionIndex, numParams);
                    if (matchIndex != -1)
                    {
                        versionText = String.Format(
                            "{0}.{1}.{2}",
                            bytesReceived[matchIndex + 7],
                            bytesReceived[matchIndex + 11],
                            bytesReceived[matchIndex + 15]);
                    }
                }

                // Try to lookup the external release name
                if (versionText.Length != 0)
                {
                    versionText = versionLookup.ContainsKey(versionText) ?
                        versionLookup[versionText] : ("Engineering version " + versionText);
                }
                // If there is still no match and we should have had a release by now
                else if (
                    (this.chipType == ChipType.LionsGate &&
                     this.loadFlashBootCaptureBytes.Length > ExpressLink.R03_or_greater_BootMsgPreamble &&
                     this.loadFlashBootCaptureString.Length > ExpressLink.R01_or_R02_BootMsgPreamble) ||
                    (this.chipType == ChipType.GoldenEars &&
                     this.loadFlashBootCaptureBytes.Length > ExpressLink.GE_BootMsgPreambleMaxLen))
                {
                    versionText = "Could not detect version";
                }

                if (versionText.Length != 0)
                {
                    SetStatusLabelText("Software Version Detected: " + versionText);
                    CurrState = RunState.Done;
                    SetButtonsAvailability(true, false);
                }
            }
        }


        private static int searchForILog(
            byte[] toSearch, byte componentIndex, byte commandIndex, uint numParams)
        {
            // ilog: header, component, command, logLevel, paramWords
            uint ilogLength = 4 + (numParams * 4);
            for (int i = 0; i < toSearch.Length - ilogLength; i++)
            {
                const uint headerOffset    = 0;
                const uint componentOffset = 1;
                const uint commandOffset   = 2;
                //const uint logLevelOffset  = 3;
                //const uint paramsOffset    = 4;
                if (toSearch[i + headerOffset] != (0xFC | numParams))
                {
                    continue;
                }
                if (toSearch[i + componentOffset] != componentIndex)
                {
                    continue;
                }
                if (toSearch[i + commandOffset] != commandIndex)
                {
                    continue;
                }
                return i;
            }
            return -1;
        }


        /// <summary>
        /// Delegate for other functions to send text to the main thread
        /// for it to use and write out in the textBoxLogWindow.
        ///
        /// NOTE: This textBoxLogWindow should NEVER be visible to the customer.
        ///       It is only used for internal debug. Make sure to test the actual
        ///       release EXE file before sending it to the customer.
        /// </summary>
        /// <param name="str">String to be written to the text box</param>
        void UpdateLogWindowDelegate(String str)
        {
            /*
             * For thread safety, the use of InvokeRequired() is needed. It determines if
             * the thread trying to change the text of the control is the same thread that
             * creates the specific control.
             */
            if (this.InvokeRequired)
            {
                UpdateLogWindowCallback d = new UpdateLogWindowCallback(UpdateLogWindow);
                this.Invoke(d, new object[] { str });
            }
            else
            {
                UpdateLogWindow(str);
            }
        }


        /// <summary>
        /// WriteLine() will write the string PLUS the terminator but for the
        /// text box, we have to manually add the terminating characters to it.
        /// </summary>
        /// <param name="text">String to be written to the text box</param>
        private void UpdateLogWindow(String text)
        {
            text = text.Trim();
            textBoxLogWindow.AppendText(text);
            textBoxLogWindow.AppendText("\r\n");
        }

        private void debugStewie(String str)
        {
            debugMsg("Stewie: " + str);
        }

        /// <summary>
        /// Send the specified stewie file to the device through the serial port.
        /// </summary>
        /// <param name="fileName">Name of the file to be sent</param>
        /// <param name="image">Bytes of the file to be sent</param>
        /// <param name="c">Synchronization character</param>
        private void SendStewie(String fileName, byte[] image, char c)
        {

            myFileSender = new StewieFileSender(fileName, image, c);
            SetStatusLabelText("Downloading " + fileName);

            myFileSender.eUpdateMessage = debugStewie;
            myFileSender.eSendBytes = SendData;
            myFileSender.eSendByte = SendData;
            myFileSender.eFlush = Flush;
            myFileSender.eTransferComplete = fileTransferComplete;
            myFileSender.SendFile();
        }

        private void fileTransferComplete()
        {
            debugMsg("fileTransferComplete:Enter state is " + CurrState);
            switch (CurrState)
            {
                case RunState.ReadBootLoader: // WTF
                    break;

                case RunState.ReadBootLoaderComplete: // WTF
                    break;

                case RunState.UpdateSoftwareFlashWriter:
                    switch (this.chipType)
                    {
                        case ChipType.LionsGate:
                            SendXmodem("Firmware", Properties.Resources.firmware_lg);
                            break;

                        case ChipType.GoldenEars:
                            if (this.platformId == PlatformId.SpartanProduct)
                            {
                                SendXmodem(
                                    "Firmware",
                                    this.unitSide == UnitSide.Lex ?
                                        Properties.Resources.firmware_ge_spartan_lex :
                                        Properties.Resources.firmware_ge_spartan_rex);
                            }
                            else if (this.platformId == PlatformId.AsicProduct ||
                                     this.platformId == PlatformId.KintexDevBoard)
                            {
                                SendXmodem(
                                    "Firmware",
                                    Properties.Resources.firmware_ge);
                            }
                            else
                            {
                                throw new UnsupportedPlatformException(this.platformId, "update firmware");
                            }
                            break;
                    }
                    CurrState = RunState.UpdateSoftwareFirmware;
                    break;

                case RunState.UpdateSoftwareFirmware:
                    SetStatusLabelText("Download Complete");
                    SetButtonsAvailability(true, false);
                    CurrState = RunState.Done;
                    break;

                case RunState.LoadFlashBoot:
                    CurrState = RunState.ReadSoftwareVersion;
                    break;

                case RunState.ReadSoftwareVersion: // WTF
                    break;

                case RunState.Done: // WTF
                    break;

            }
            debugMsg("fileTransferComplete:Leave state is " + CurrState);
        }

        /// <summary>
        /// Send the specified binary file to the device through the serial port via xmodem (checksum).
        /// </summary>
        /// <param name="fileName">Name of the file to be sent</param>
        /// <param name="image">Bytes of the file to be sent</param>
        private void SendXmodem(String fileName, byte[] image)
        {

            myFileSender = new XmodemSender(fileName,image);
            SetStatusLabelText("Downloading " + fileName);

            myFileSender.eUpdateMessage = debugMsg;
            myFileSender.eSendBytes = SendData;
            myFileSender.eSendByte = SendData;
            myFileSender.eFlush = Flush;
            myFileSender.eTransferComplete = fileTransferComplete;
            myFileSender.SendFile();
        }


        /// <summary>
        /// Sends data through COM.
        /// </summary>
        /// <param name="cmd">the datta to be sent</param>
        private void SendData(byte cmd)
        {
            byte[] bytes = { cmd };
            this.SendData(bytes);
        }


        /// <summary>
        /// Flushes the com port buffers
        /// </summary>
        private void Flush()
        {
            debugMsg("Just entered Flush().");
            comPort.DiscardInBuffer();
            comPort.DiscardOutBuffer();
        }


        /// <summary>
        /// Sends data through COM.
        /// </summary>
        /// <param name="cmd">the data to be sent</param>
        private void SendData(byte[] cmd)
        {
            comPort.Write(cmd, 0, cmd.Length);
        }

        // This is the method to run when the timer is raised.
        private void UartTimerProcessor(object sender, EventArgs e)
        {
            // NOTE: this timer works on the GUI thread
           debugMsg("Enter UartTimerProcessor(), current state is " + this.CurrState);

            // clean up, the device isn't responding
            this.myFileSender = null;

            switch (this.CurrState)
            {
                case RunState.ReadBootLoader:
                case RunState.ReadBootLoaderComplete:
                case RunState.Done:
                    // Take no action.  There is no timeout in these states
                    break;

                case RunState.UpdateSoftwareFlashWriter:
                case RunState.UpdateSoftwareFirmware:
                case RunState.LoadFlashBoot:
                case RunState.ReadSoftwareVersion:
                    // Operation failed
                    SetStatusLabelText("Operation timed out");
                    CurrState = RunState.Done;
                    SetButtonsAvailability(true, false);
                    break;
            }
        }


        /// <summary>
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void bgWorkerUpdateProgressBar_DoWork(object sender, DoWorkEventArgs e)
        {
            Thread.Sleep(1000); // TODO: This works around bugs
                                // It seems Inoke can't be called until the GUI
                                // thread has done initialization, but there is
                                // no way to know when it has finished
                                // TODO: find some other mechanism
            while (true)
            {
                this.Invoke(
                    new bgWorkerUpdateProgressBar_DoWork_Callback(bgWorkerUpdateProgressBar_DoWork_Helper),
                    sender,
                    e);
                Thread.Sleep(500);
            }
        }

        private void bgWorkerUpdateProgressBar_DoWork_Helper(object sender, DoWorkEventArgs e)
        {
            const float one = 1;

            if ((this.myFileSender != null) && (this.myFileSender.GetProgress() != one))
            {
                SetProgressVisibility(true);
                this.UpdateProgress(this.myFileSender.GetProgress());
            }
            else
            {
                SetProgressVisibility(false);
            }
        }

        /// <summary>
        /// Set the visibility property of the progress bar
        /// </summary>
        /// <param name="visible">Progress bar is visible or invisible (true or false)</param>
        private void SetProgressVisibility(Boolean visible)
        {
            if (InvokeRequired)
            {
                Invoke(new SetProgressVisibilityCallback(SetProgressVisibility), visible);
                return;
            }
            toolStripProgressBar.Visible = visible;
        }


        /// <summary>
        /// Set the enabled property of the buttons.
        /// </summary>
        /// <param name="ButtonEnabled">Button is enabled or disabled (true or false)</param>
        /// <param name="ProgressBarVisible">Progress bar is enabled or disabled (true or false)</param>
        private void SetButtonsAvailability(Boolean ButtonEnabled, Boolean ProgressBarVisible)
        {
            if (InvokeRequired)
            {
                Invoke(new SetDownloadButtonEnabledCallback(SetButtonsAvailability), ButtonEnabled, ProgressBarVisible);
                return;
            }
            /*
             * Buttons not enabled means ExpressLink is updating, thus show the progress bar and label
             * When the buttons are enabled, then we should hide the progress bar and the label.
             */
            buttonUpdateSoftware.Enabled = false;
            buttonReadSoftware.Enabled = false;
            buttonConnect.Enabled = ButtonEnabled;

            // This is just to set the state before the
            // bgWorkerUpdateProgressBar_DoWork, wakes up. So the user doesn't
            // see a 5sec delay
            toolStripProgressBar.Visible = ProgressBarVisible;
        }


        /// <summary>
        /// Sets the text and visibility of the status strip label
        /// </summary>
        /// <param name="text">text</param>
        private void SetStatusLabelText(String text)
        {
            if (InvokeRequired)
            {
                Invoke(new SetStatusLabelTextCallback(SetStatusLabelText), text);
                return;
            }
            toolStripStatusLabel.Text = text;
        }


        /// <summary>
        /// Set the Generate Debug Log button text.
        /// </summary>
        /// <param name="text">The new button text</param>
        /// <param name="buttonStatus">Button is enabled or disabled (true or false)</param>
        private void SetGenerateDebugLogFileText(String text, Boolean buttonStatus)
        {
            if (InvokeRequired)
            {
                Invoke(new SetGenerateDebugLogTextCallback(SetGenerateDebugLogFileText), text, buttonStatus);
                return;
            }
        }


        ///<summary>
        /// progress bar status percentage and selection
        ///</summary>
        ///<param name="in_nValue">the amount to update the progressbar to</param>
        private void UpdateProgress(float in_nValue)
        {
            try
            {
                if (InvokeRequired)
                {
                    Invoke(new UpdateProgressCallback(this.UpdateProgress), new object[] { in_nValue });
                    return;
                }
                toolStripProgressBar.Value = (int)(100 * in_nValue);
            }
            catch (ObjectDisposedException e)
            {
                Thread.CurrentThread.Abort();
                MessageBox.Show(e.Message + "\r\n" + e.ObjectName, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }


        /// <summary>
        /// Get the list of available COM ports and populate the combo box
        /// with the items in the list.
        ///
        /// If there currently there is a COM port connected and the new list
        /// does not contain the current COM port, then close the port since
        /// the port is now lost.
        /// </summary>
        private void RescanCOMPorts()
        {
            List<String> portList = SerialPort.GetPortNames().ToList();
            portList.Sort();

            /*
             * Record the currently selected COM port in the combo box
             */
            String CurrentSelectedPort = "";
            Object tmp = comboBoxCOMPort.SelectedItem;
            if (tmp != null)
            {
                CurrentSelectedPort = tmp.ToString();
            }

            comboBoxCOMPort.Items.Clear();

            foreach (String portName in portList)
            {
                comboBoxCOMPort.Items.Add(portName);

                /*
                 * If currently selected port is in the updated list
                 * then update the index to the index of this port
                 */
                if (portName.Equals(CurrentSelectedPort))
                {
                    comboBoxCOMPort.SelectedIndex = comboBoxCOMPort.Items.IndexOf(portName);
                }
            }

            /*
             * If currently opened port isn't on the list,
             * disconnect the port immediately.
             */
            if ((COMPortConnected) && (!portList.Contains(comPort.PortName)))
            {
                OpenCloseCOMPort();
            }
        }

    }
}
