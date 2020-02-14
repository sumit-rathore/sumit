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

namespace ExpressLink
{
    public delegate void UpdateMessageHandler(string param);
    public delegate void SendByteCallback(byte byteToSend);
    public delegate void SendBytesCallback(byte[] bytesToSend);
    public delegate void FlushBufferCallback();
    public delegate void TransferCompleteCallback();
    ///<summary>An IProgrammer is an interface that is inherited by classes which 
    ///implement the sending of a file to a device over the uart
    ///</summary>
    public interface IProgrammer
    {
        //Signals the interface that a char has been recieved,
        void SignalRecievedChar(byte b);

        void SendFile();

        float GetProgress();

        string GetFileName();

        //event handlers
        /// <summary>
        /// Signals when there is Data To Be Logged/OutPut Must be externally handled 
        /// </summary>
        UpdateMessageHandler eUpdateMessage { get; set; }

        /// <summary>
        /// Signaled when we have byte(s) to send. Must be externally handled.
        /// </summary>
        SendByteCallback eSendByte { get; set; }

        /// <summary>
        /// Signaled when we have byte(s) to send. Must be externally handled.
        /// </summary>
        SendBytesCallback eSendBytes { get; set; }

        /// <summary>
        /// Flushes input and output buffers.
        /// </summary>
        FlushBufferCallback eFlush { get; set; }

        /// <summary>
        /// Called when the transfer is complete
        /// </summary>
        TransferCompleteCallback eTransferComplete { get; set; }
    }
}
