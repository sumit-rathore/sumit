using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Threading;
using System.IO;

namespace ExpressLink
{
    public class XmodemSender : IProgrammer
    {
        //private const int SyncTimeout = 10000;
        private const byte ACK = (byte)6;
        private const byte NAK = (byte)21;
        private const byte EOT = (byte)4;
        //private const int AckNackTimeout = 10000;
        //private const int MaxTries = 10;

        private XmodemFile xmodemFile;
        private int packetIndex = 0;
        private int retryCnt = 0;


        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="xmodemFile">the name of the file for flashing</param>
        /// <param name="image">Bytes of the file to be sent</param>
        public XmodemSender(string fileName, byte[] image)
        {
            this.xmodemFile = new XmodemFile(fileName,image);
        }
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="xmodemFile">the name of the file for flashing</param>
        public XmodemSender(string fileName)
        {
            this.xmodemFile = new XmodemFile(fileName);
        }

        //event handlers
        private UpdateMessageHandler UpdateMessage;
        private SendByteCallback SendByte;
        private SendBytesCallback SendBytes;
        private FlushBufferCallback Flush;
        private TransferCompleteCallback TransferComplete;
        /// <summary>
        /// Signals when there is Data To Be Logged/OutPut Must be externally handled 
        /// </summary>
        public UpdateMessageHandler eUpdateMessage { get { return UpdateMessage; } set { UpdateMessage = value; } }

        /// <summary>
        /// Signaled when we have byte(s) to send. Must be externally handled.
        /// </summary>
        public SendByteCallback eSendByte { get { return SendByte; } set { SendByte = value; } }

        /// <summary>
        /// Signaled when we have byte(s) to send. Must be externally handled.
        /// </summary>
        public SendBytesCallback eSendBytes { get { return SendBytes; } set { SendBytes = value; } }

        /// <summary>
        /// Flushes input and output buffers.
        /// </summary>
        public FlushBufferCallback eFlush { get { return Flush; } set { Flush = value; } }
     
        /// <summary>
        /// Called when the transfer is complete
        /// </summary>
        public TransferCompleteCallback eTransferComplete { get { return TransferComplete; } set { TransferComplete = value; } }

        /// <summary>
        /// Sends a file using Xmodem to a target
        /// </summary>
        public void SendFile()
        {
            this.retryCnt = 0;
            this.packetIndex = 0;

            //Clear any characters that may already be in the buffer
            //Dump any extra ACKs or NAKs
            Flush();
        }

        /// <summary>
        /// gets the name of the current file being sent
        /// </summary>
        /// <returns>the name of the current file being sent</returns>
        public string GetFileName()
        {
            return this.xmodemFile.FileName;
        }

        /// <summary>
        /// Signaled when a character that needs to be read by the XmodemSender is recieved
        /// </summary>
        /// <param name="b">the character recieved</param>
        public void SignalRecievedChar(byte b)
        {
        //TODO:DAVIDM: This doesn't support timeouts anymore
            if (b == NAK)
            {
                retryCnt++;
                if (this.xmodemFile.HasPacket(this.packetIndex))
                {
                    this.SendBytes(this.xmodemFile.GetPacket(this.packetIndex));
                }
                else
                {
                    this.SendByte(EOT);
                }
            }
            else if (b == ACK)
            {
                if (this.xmodemFile.HasPacket(this.packetIndex))
                {
                    this.packetIndex++;
                    if (this.xmodemFile.HasPacket(this.packetIndex))
                    {
                        this.SendBytes(this.xmodemFile.GetPacket(this.packetIndex));
                    }
                    else
                    {
                        this.SendByte(EOT);
                    }
                }
                else
                {
                    // we are done
                    this.UpdateMessage("File transfer complete num retries: " + retryCnt);
                    this.eTransferComplete();
                }
            }
            else
            {
                this.UpdateMessage("Bogus byte " + b + " received");
            }
        }

        /// <summary>
        /// Returns the percentage of the file bytes that has been sent as a decimal
        /// </summary>
        /// <returns>the percent of the file bytes that have been sent</returns>
        public float GetProgress()
        {
            return ((float)packetIndex /(float)xmodemFile.packetCount);
        }
    }
}
