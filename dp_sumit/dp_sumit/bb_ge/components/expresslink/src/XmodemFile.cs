using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace ExpressLink
{
    /// <summary>
    /// An XmomdemFile parses file into Xmodem packets which can be sent to a target using the Xmodem1k protocol.
    /// </summary>
    class XmodemFile
    {
        private const int DataSize = 128;
        private const byte SOH = (byte)01;
        private const byte FF = (byte)255;
        private const int HeaderSize = 3;
        private const int ChecksumModulo = 256;

        private List<List<byte>> packets = new List<List<byte>>();
        private string file;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="fileName">the name of the binary file to load this from</param>
        /// <param name="image">Bytes of the file to be sent</param>
        public XmodemFile(string fileName, byte[] image)
        {
            file = fileName;
            this.GetPackets(image);
        }         
        
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="fileName">the name of the binary file to load this from</param>
        public XmodemFile(string fileName)
        {
            StreamReader sr = new StreamReader(File.Open(fileName, FileMode.Open));
            byte[] bytes = sr.CurrentEncoding.GetBytes(sr.ReadToEnd());
            file = fileName;
            this.GetPackets(bytes);
        }



        /// <summary>
        /// the number of packets to send
        /// </summary>
        public int packetCount
        {
            get
            {
                return this.packets.Count;
            }
        }

        /// <summary>
        ///the filename for this
        /// </summary>
        public string FileName
        {
            get
            {
                return this.file;
            }
        }

        /// <summary>
        /// Returns one packet with a certain index
        /// </summary>
        /// <param name="index">the index of the packet to return</param>
        /// <returns></returns>
        public byte[] GetPacket(int index)
        {
            return this.packets[index].ToArray();
        }

        /// <summary>
        /// verifies that a packet with a certain index exists
        /// </summary>
        /// <param name="index">the index of packet to verify the existance of</param>
        /// <returns>true if the packet exists else returns false</returns>
        public bool HasPacket(int index)
        {
            return (index < this.packets.Count);
        }

        /// <summary>
        /// Part of initialization, sorts bytes into packets folllowing Xmodem1k format
        /// </summary>
        /// <param name="bytes">the bytes to sort into packets</param>
        private void GetPackets(byte[] bytes)
        {
            for (int i = 0; i < bytes.Length; i += DataSize)
            {
                int checksum = 0;

                List<byte> temp = new List<byte>();
                temp.Add(SOH);
                temp.Add((byte)(this.packets.Count + 1));
                temp.Add((byte)(255 - (this.packets.Count + 1)));
                for (int j = i;
                    ((j < (i + DataSize)) && (j < bytes.Length));
                    j++)
                {
                    checksum += bytes[j];
                    temp.Add(bytes[j]);
                }
                while (temp.Count < (DataSize + HeaderSize))
                {
                    checksum += FF;
                    temp.Add(FF);
                }

                checksum = checksum % ChecksumModulo ;
                byte finalChecksum = (byte)checksum;
                temp.Add(finalChecksum);

                this.packets.Add(temp);
            }
        }

        
    }
}
