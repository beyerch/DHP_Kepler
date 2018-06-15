using System;

namespace J2534DotNet
{
    public unsafe struct PassThruMsg
    {
        public ProtocolID ProtocolID;
        public RxStatus RxStatus;
        public TxFlag TxFlags;
        public uint Timestamp;
        public uint DataSize;
        public uint ExtraDataIndex;
        public fixed byte Data[4128];

        public PassThruMsg(ProtocolID myProtocolId, TxFlag myTxFlag, byte[] myByteArray) : this()
        {
            ProtocolID = myProtocolId;
            TxFlags = myTxFlag;
            SetBytes(myByteArray);
        }

        public void SetBytes(byte[] myByteArray)
        {
            DataSize = (uint)myByteArray.Length;
            fixed (byte* data = Data)
            {
                for (int i = 0; i < myByteArray.Length; i++)
                {
                    *(data + i) = myByteArray[i];
                }
            }
        }

        public byte[] GetBytes()
        {
            var bytes = new byte[DataSize];
            fixed (byte* data = Data)
            {
                for (int i = 0; i < DataSize; i++)
                {
                    bytes[i] = *(data + i);
                }
            }

            return bytes;
        }

        //private const string tab = "    ";

        public string ToString(string tab)
        {
            return
                string.Format(
                    "{6}{5}Protocol: {0}{6}{5}RxStatus: {1}{6}{5}Timestamp: {2}{6}{5}ExtraDataIndex: {3}{6}{5}Data: {4}",
                    ProtocolID,
                    RxStatus,
                    Timestamp,
                    ExtraDataIndex,
                    BitConverter.ToString(GetBytes()),
                    tab,
                    Environment.NewLine);
        }
    }
}
