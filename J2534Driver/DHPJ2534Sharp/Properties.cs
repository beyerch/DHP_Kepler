using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{

    internal static class Properties
    {
        private static int _VID = 0x03EB;
        private static int _PID = 0xDFDF;
        private static string _VIDString = "03EB";
        private static string _PIDString = "DFDF";
        private static byte _StartByte = 0x02;

        public static byte StartByte
        {
            get { return _StartByte; }
            private set { _StartByte = value; }
        }


        public static int VID
        {
            get { return _VID; }
            private set { _VID = value; }
        }

        public static int PID
        {
            get { return _PID; }
            private set { _PID = value; }
        }


        public static string VIDString
        {
            get { return _VIDString; }
            private set { _VIDString = value; }
        }

        public static string PIDString
        {
            get { return _PIDString; }
            private set { _PIDString = value; }
        }


    }

}
