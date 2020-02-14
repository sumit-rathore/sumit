///////////////////////////////////////////////////////////////////////////////
//
//   Icron Technology Corporation - Copyright 2007, 2008
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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
namespace ExpressLink
{
    /// <summary>
    /// A stewiefile sender controls the sending of a stewie file to a device.
    /// </summary>
    /// <remarks> Do not catch generic exceptions, because on abort thread abort exception is thrown and it will be caught and cause
    /// a program freeze if generic exceptions are caught</remarks>
    public class StewieFileSender : IProgrammer
    {
        //private members
        private StewieFile stewFile;
        private char syncChar;
        //private const int Timeout = 5000;
        private int nRecordIndex = 0;
        enum stewieState { waitForStewieAck, waitForHeaderAck, waitForRecordAck, waitForFooterAck }
        private stewieState curStewieState;

        //private byte[] StewieBytes = { (byte)'@', (byte)'#', (byte)'~', (byte)'!', (byte)'}', (byte)'{' };
        /// <summary>
        /// Initializes the stewiefile to be sent.
        /// </summary>
        /// <param name="fileName">the name of the .icr file</param>
        /// <param name="image">the .icr file's bytes</param>
        /// <param name="syncChar">the character recieved in synchronization</param>
        public StewieFileSender(string fileName, byte[] image, char syncChar)
        {
            this.stewFile = new StewieFile(fileName,image);
            this.syncChar = syncChar;
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

        /*
         * Methods
         */
        /// <summary>
        /// Writes stewiefile to device
        /// </summary>
        public void SendFile()
        {
            // Sync with receiver
            UpdateMessage("SendFile ->  Sending \'?\' to the target!");
            SendByte((byte)'?');
            curStewieState = stewieState.waitForStewieAck;

            // Then when the receiver responds, we will send the file
        }

        /// <summary>
        /// gets the name of the current file being sent
        /// </summary>
        /// <returns>the name of the current file being sent</returns>
        public string GetFileName()
        {
            return this.stewFile.FileName;
        }

        /// <summary>
        /// </summary>
        /// <param name="c">the byte received</param>
        public void SignalRecievedChar(byte c)
        {
            // TODO: DAVIDM: we don't really have timeout support in here anymore
            if ((curStewieState == stewieState.waitForStewieAck) && (c == syncChar))
            {
                // Send the header
                UpdateMessage("SendStewieFile -> Downloading " + stewFile.FileName + " (" + stewFile.FileSize + " bytes)");
                this.SendBytes(StewieFile.Header());
                curStewieState = stewieState.waitForHeaderAck;
            }
            else if ((curStewieState == stewieState.waitForHeaderAck) && (c == '{'))
            {
                // Send the first record
                this.nRecordIndex = 0;
                this.SendBytes(stewFile.GetRecord(nRecordIndex));
                curStewieState = stewieState.waitForRecordAck;
            }
            else if ((curStewieState == stewieState.waitForRecordAck) && (c == '~'))
            {
                this.nRecordIndex++;
                if (this.nRecordIndex < stewFile.RecordCount)
                {
                    this.SendBytes(stewFile.GetRecord(nRecordIndex));
                }
                else
                {
                    this.SendBytes(StewieFile.Footer());
                    curStewieState = stewieState.waitForFooterAck;
                }
            }
            else if ((curStewieState == stewieState.waitForFooterAck) && (c == '}'))
            {
                // We are done
                this.UpdateMessage("SendStewieFile -> Download complete!!!");
                this.eTransferComplete();
            }
            else
            {
                UpdateMessage("SignalReceivedChar in state " + curStewieState + " unexpectedly received " + c);
            }
        }

        /// <summary>
        /// Returns the percentage of the file bytes that has been sent as a decimal.
        /// </summary>
        /// <returns>the percent of the file bytes that have been sent</returns>
        public float GetProgress()
        {
                return ((float)this.nRecordIndex / (float)stewFile.RecordCount);
        }
    }
}
