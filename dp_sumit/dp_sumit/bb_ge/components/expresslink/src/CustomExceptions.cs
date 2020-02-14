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
using System.Runtime.Serialization;

namespace ExpressLink
{
    /// <summary>
    /// A Custom Exception to be thrown on target error
    /// </summary>
    [SerializableAttribute]
    public class TargetErrorException : SystemException
    {
        public TargetErrorException()
        {
        }

        public TargetErrorException(String message)
            : base(message)
        {
        }

        public TargetErrorException(String message, Exception innerException)
            : base (message, innerException)
        {
        }

        protected TargetErrorException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    ///<summary>
    ///a custom exception to be thrown when a stewie file has the wrong format
    ///</summary>
    [SerializableAttribute]
    public class StewieWrongFormatException : SystemException
    {
        public StewieWrongFormatException()
        {
        }

        public StewieWrongFormatException(String message)
            : base(message)
        {
        }

        public StewieWrongFormatException(String message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected StewieWrongFormatException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    [SerializableAttribute]
    public class StewieInvalidIndexException : SystemException
    {
        public StewieInvalidIndexException()
        {
        }

        public StewieInvalidIndexException(String message)
            : base(message)
        {
        }

        public StewieInvalidIndexException(String message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected StewieInvalidIndexException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }
}
