namespace DHPJ2534Sharp
{
    public class PassThruMessageWrapper
    {
        /// <summary>
        /// Protocol type
        /// </summary>
        public long ProtocolID { get; set; }
        /// <summary>
        /// Receive message status – See RxStatus in "Message Flags and Status Definition" section
        /// </summary>
        public long RxStatus { get; set; }
        /// <summary>
        /// Transmit message flags
        /// </summary>
        public long TxFlags { get; set; }
        /// <summary>
        /// Received message timestamp (microseconds): For the START_OF_FRAME
        /// indication, the timestamp is for the start of the first bit of the message. For all other
        /// indications and transmit and receive messages, the timestamp is the end of the last
        /// bit of the message. For all other error indications, the timestamp is the time the error
        /// is detected.
        /// </summary>
        public long Timestamp { get; set; }
        /// <summary>
        /// Data size in bytes, including header bytes, ID bytes, message data bytes, and extra
        /// data, if any.
        /// </summary>
        public long DataSize { get; set; }
        /// <summary>
        ///  Start position of extra data in received message (for example, IFR). The extra data
        /// bytes follow the body bytes in the Data array. The index is zero-based. When no
        /// extra data bytes are present in the message, ExtraDataIndex shall be set equal to
        /// DataSize. Therefore, if DataSize equals ExtraDataIndex, there are no extra data
        /// bytes. If ExtraDataIndex=0, then all bytes in the data array are extra bytes.
        /// </summary>
        public long ExtraDataIndex { get; set; }
        /// <summary>
        ///  Message Data
        /// </summary>
        public char[] Data { get; set; }

    }
}