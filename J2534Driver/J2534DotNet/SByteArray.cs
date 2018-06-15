using System;
using System.Runtime.InteropServices;

namespace J2534DotNet
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SByteArray
    {
        public int NumOfBytes;
        public IntPtr BytePtr;

        public override string ToString()
        {
            var byteList = BytePtr.AsList<byte>(NumOfBytes);
            return BitConverter.ToString(byteList.ToArray()).Replace("-", " ");
        }
    }
}
