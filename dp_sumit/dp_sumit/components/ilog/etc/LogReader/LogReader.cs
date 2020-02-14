using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Collections;
using System.Runtime.InteropServices;
using System.IO.Compression;

namespace LogReader
{
    public partial class LogReader : Form
    {        
        /*
         * Message values needed to intercept when a device is added
         * or removed from the system.
         */
        private const int WM_DEVICECHANGE = 0x0219;
        private const int DBT_DEVICEARRIVAL = 0x8000;
        private const int DBT_DEVICEREMOVECOMPLETE = 0x8004;
        private SerialPort comPort = new SerialPort();
        private Stream comPortStream;
        private Boolean COMPortConnected = false;
        private GZipStream LogFile;
      
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
                    //toolStripStatusLabel.Text = "Waiting to enter boot loader mode ...";
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
                    COMPortConnected = false;
                    buttonConnect.BackColor = System.Drawing.SystemColors.Window;
                    buttonConnect.Text = "Connect";
                    comboBoxCOMPort.Enabled = true;
                }
                catch (UnauthorizedAccessException e)
                {
                    MessageBox.Show(comPort.PortName + " is already in use. " + e.Message, "Port Already in Use", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1, MessageBoxOptions.RtlReading);
                }
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
            String CurrentSelectedPort = comboBoxCOMPort.SelectedItem.ToString();

            /*
             * Default the selected index to the first item in the list
             */
            Int32 PortIndex = 0;

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
                    PortIndex = comboBoxCOMPort.Items.IndexOf(portName);
                }
            }

            comboBoxCOMPort.SelectedIndex = PortIndex;

            /*
             * If currently opened port isn't on the list,
             * disconnect the port immediately.
             */
            if (!portList.Contains(comPort.PortName) && buttonConnect.Text == "Disconnect")
            {
                OpenCloseCOMPort();
            }
        }

        /// <summary>
        /// SerialPort.DataReceived event handler. If ExpressLink is in programming state, pass
        /// the character received to the serial data parser, else just print the line to the
        /// log window.
        /// </summary>
        private void ReadCOMByte(object sender, SerialDataReceivedEventArgs e)
        {
            // Create timestamp for the logFile
            while (comPort != null && comPort.IsOpen && comPort.BytesToRead > 0)
            {
                try
                {
                    // The timestamp we want is in microseconds from the unix epoch
                    DateTime unixEpoch = new DateTime(1970, 1, 1);
                    ulong timestamp = (ulong)((DateTime.UtcNow - unixEpoch).TotalMilliseconds * 1000);

                    // Pack time as 64 bit Network (Big) Endian
                    byte[] timeArray = BitConverter.GetBytes(timestamp);
                    if (BitConverter.IsLittleEndian)
                        Array.Reverse(timeArray);

                    // int byteSize = comPort.BytesToRead;
                    byte[] byteBuffer = new byte[1024];
                    int bytesRead = comPort.Read(byteBuffer, 0, 1024);

                    // Pack number of data as 32 bit Network (Big) Endian
                    byte[] numDataBytesArray = BitConverter.GetBytes(bytesRead);
                    if (BitConverter.IsLittleEndian)
                        Array.Reverse(numDataBytesArray);

                    LogFile.Write(timeArray, 0, timeArray.Length);
                    LogFile.Write(numDataBytesArray, 0, numDataBytesArray.Length);
                    LogFile.Write(byteBuffer, 0, bytesRead);
                }
                catch (InvalidOperationException)
                {
                    // The port may not be open or disconnceted while data is being read
                    // Can be ignored since we are no longer recording
                }
            }
        }

        public LogReader()
        {
            InitializeComponent();

            comPort.BaudRate = 115200;
            comPort.Parity = Parity.None;
            comPort.DataBits = 8;
            comPort.StopBits = StopBits.One;
            comPort.Handshake = Handshake.None;
            comPort.DataReceived += new SerialDataReceivedEventHandler(ReadCOMByte);
        }

        /// <summary>
        /// Populate the list of available serial ports combo box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void LogReader_Load(object sender, EventArgs e)
        {
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
            TooltipConnect.SetToolTip(this.buttonConnect, "Once the COM port is selected use this buttom to connect \r\n" +
                                                          "to the Icron device.");
            TooltipConnect.AutoPopDelay = 15000;
            TooltipConnect.IsBalloon = true;

        }

        /// <summary>
        /// Open the selected COM port in the combo box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonConnect_Click(object sender, EventArgs e)
        {
            if (buttonConnect.Text == "Connect")
            {
                //string Saved_file = "";
                saveFileDialog1.Title = "Select file to save as";
                saveFileDialog1.FileName = "";
                saveFileDialog1.Filter = "GZip Files|*.gz|All Files|*.*";

                if (saveFileDialog1.ShowDialog() == DialogResult.OK)
                {
                    //LogFile = saveFileDialog1.OpenFile();
                    //LogFile = new StreamWriter(saveFileDialog1.OpenFile());
                    //try
                    //{
                    //    File.Delete(saveFileDialog1.FileName);
                    //}
                    //catch(Exception Exception)
                    //{

                    //}
                    LogFile = new GZipStream(File.Open(saveFileDialog1.FileName, FileMode.Create), CompressionMode.Compress, true);
                    //LogFile.AutoFlush = true;
                    if (comboBoxCOMPort.SelectedIndex < 0)
                    {
                        MessageBox.Show("Please select a COM port first.", "ERROR!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                    {
                        OpenCloseCOMPort();
                    }
                }
            }
            else
            {
                LogFile.Flush();
                LogFile.Close();
                OpenCloseCOMPort();
            }
        }
    }
}
