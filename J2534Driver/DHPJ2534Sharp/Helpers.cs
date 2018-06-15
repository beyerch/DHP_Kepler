using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
    /// <summary>
    /// This static class provides access to helper functions 
    /// which can be used throughout the entire project
    /// </summary>
    internal static class Helpers
    {
        /// <summary>
        /// Converts a byte array to a hex string such as:
        /// "0x02 0x00 0x 05"
        /// </summary>
        /// <param name="ba"></param>
        /// <returns></returns>
        public static string ByteArrayToString(byte[] ba)
        {
            StringBuilder hex = new StringBuilder(ba.Length * 2);
            foreach (byte b in ba)
                hex.AppendFormat("{0:x2} ", b);
            return hex.ToString();
        }
    }
}
