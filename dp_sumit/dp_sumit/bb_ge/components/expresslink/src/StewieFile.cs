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
using System.Text;
using System.IO;
using System.Reflection;
using System.Globalization;

namespace ExpressLink
{
    /// <summary>
    /// A Stewiie stores a stewie file to be sent to a device
    /// </summary>
    class StewieFile
    {
        private string fileName = "";
        private byte[] StewieFileImage;
        private int recordCount = 0;
        private int nextRecordIndex = -1;
        private int nextByteIndex = -1;

        private static byte[] StewieHeader = { 0x53, 0x30, 0x30, 0x33 };
        private static byte[] StewieFooter = { 0x53, 0x38 };


        /// <summary>
        /// Create a StewieFile from an array of bytes.
        /// </summary>
        /// <param name="fileName">The name of the image file to load.</param>
        public StewieFile(string fileName, byte[] image)
        {
            this.fileName = fileName;
            if (VerifyStewieImage(image))
            {
                StewieFileImage = image;
            }
        }        
        

        /// <summary>
        /// Returns the number of stewie records in the file.
        /// </summary>
        public int RecordCount
        {
            get
            {
                return recordCount;
            }
        }


        /// <summary>
        /// Returns the header of the Stewie image.
        /// </summary>
        public static byte[] Header()
        {
            // Stewie files always have a fixed header
            return StewieHeader;
        }


        /// <summary>
        /// Returns the footer of the Stewie image.
        /// </summary>
        public static byte[] Footer()
        {
            // Stewie files always have a fixed footer
            return StewieFooter;
        }


        /// <summary>
        /// Returns the size of the file in bytes.
        /// </summary>
        public int FileSize
        {
            get
            {
                return StewieFileImage.Length;
            }
        }


        /// <summary>
        /// Returns the filename associated with this stewie file.
        /// </summary>
        public string FileName
        {
            get
            {
                return this.fileName;
            }
        }


        /// <summary>
        /// Gets the binary Stewie record from the file.
        /// </summary>
        /// <param name="recordIndex">Index of the record to retrieve. Must be less than the RecordCount property.</param>
        /// <returns>A byte array containing the specified record.</returns>
        /// <exception cref="IndexOutOfRangeException">if the recordIndex is invalid</exception>
        public byte[] GetRecord(int recordIndex)
        {
            if ((recordIndex < 0)
             || (recordIndex > recordCount))
            {
                // Invalid index
                throw new StewieInvalidIndexException(String.Format("Stewie record index={0} is out of range. RecordCount={1}", recordIndex, recordCount));
            }

            if (recordIndex != nextRecordIndex)
            {
                // We are not positioned at the correct record, so search from the beginning
                int nRecordIndex = 0;
                int nByteIndex = StewieHeader.Length;
                for (nRecordIndex = 0; nRecordIndex < recordIndex; ++nRecordIndex)
                {
                    // Skip over the 'S' and Type bytes
                    nByteIndex += 2;

                    // Skip over the rest of the record
                    nByteIndex += (1 + StewieFileImage[nByteIndex]);
                }

                // Now we are positioned at the correct record
                nextRecordIndex = nRecordIndex;
                nextByteIndex = nByteIndex;
            }

            // Get the length of the record
            int nRecordLength = StewieFileImage[nextByteIndex + 2];

            // Return a copy of the record
            byte[] recordBytes = new byte[3 + nRecordLength];
            Array.ConstrainedCopy(StewieFileImage, nextByteIndex, recordBytes, 0, recordBytes.Length);

            // Update the next record for fast sequential access
            nextByteIndex += recordBytes.Length;
            ++nextRecordIndex;

            // We are done
            return (recordBytes);
        }


        /// <summary>
        /// Verify the image file against the stewie file format.
        /// Stewie format is described here:
        /// http://linux.die.net/man/5/srec_stewie
        /// </summary>
        /// <param name="image">the stewie image to be flashed in bytes</param>
        private bool VerifyStewieImage(byte[] image)
        {
            bool result = false;

            if (image.Length < 6)
            {
                // File is too small to be valid
                throw new StewieWrongFormatException(String.Format("File {0} is invalid. FileSize={1} bytes.", fileName, image.Length));
            }

            if ((0x53 != image[0])
             || (0x30 != image[1])
             || (0x30 != image[2])
             || (0x33 != image[3]))
            {
                // Invalid header
                throw new StewieWrongFormatException(String.Format("File {0} is invalid. Missing S003 header.", fileName));
            }

            if ((0x53 != image[image.Length - 2])
              || (0x38 != image[image.Length - 1]))
            {
                // Invalid footer
                throw new StewieWrongFormatException(String.Format("File {0} is invalid. Missing S8 footer.", fileName));
            }

            // Now validate each record inside
            for (int nIndex = 4; nIndex < image.Length - 2; )
            {
                if (0x53 != image[nIndex])
                {
                    // Invalid record
                    throw new StewieWrongFormatException(String.Format("File {0} is invalid. Missing S marker at offset=0x{1:X}", fileName, nIndex));
                }
                ++nIndex;

                int nAddrLen = 0;
                switch (image[nIndex])
                {
                    case 0x31:
                        {
                            // 2-byte address
                            nAddrLen = 2;
                            break;
                        }

                    case 0x32:
                        {
                            // 3-byte address
                            nAddrLen = 3;
                            break;
                        }

                    case 0x33:
                        {
                            // 4-byte address
                            nAddrLen = 4;
                            break;
                        }

                    default:
                        {
                            // Invalid type
                            throw new StewieWrongFormatException(String.Format("File {0} is invalid. Unexpected record type=0x{1:X2} offset=0x{2:X}", fileName, image[nIndex], nIndex));
                        }
                }

                // Reset the checksum
                ++nIndex;
                byte nChecksum = 0;

                // Read the record length
                nChecksum += image[nIndex];
                byte nRecordLen = image[nIndex];
                ++nIndex;

                if ((nRecordLen < nAddrLen)
                 || (nIndex + nRecordLen > image.Length))
                {
                    // Invalid record length
                    throw new StewieWrongFormatException(String.Format("File {0} is invalid. Invalid recordLength=0x{1:X2} offset=0x{2:X}", fileName, nRecordLen, nIndex));
                }

                // Read the address
                int nAddr = 0;
                for (; nAddrLen > 0; --nAddrLen, --nRecordLen)
                {
                    nChecksum += image[nIndex];
                    nAddr = (nAddr << 8) + image[nIndex];
                    ++nIndex;
                }

                // Read the data
                for (--nRecordLen; nRecordLen > 0; --nRecordLen)
                {
                    nChecksum += image[nIndex];
                    ++nIndex;
                }

                // Read the checksum
                byte nExpectedChecksum = image[nIndex];
                ++nIndex;
                nChecksum = (byte)~nChecksum;

                if (nExpectedChecksum != nChecksum)
                {
                    // Invalid checksum
                    throw new StewieWrongFormatException(String.Format("File {0} is invalid. Invalid checksum @offset=0x{1:X} calcCsum={2:X2} expCsum={3:X2}", fileName, nIndex, nChecksum, nExpectedChecksum));
                }

                // Increment the record count
                ++recordCount;
            }

            result = true;

            return result;
        }
    }
}
