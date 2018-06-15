using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
    public class DHPLogger
    {
        private FileStream LogFile;
        public DHPLogger(string LogPath)
        {
            LogFile = new FileStream(LogPath, FileMode.OpenOrCreate);
        }

        public void LogLine(string Message)
        {
            string TimeStampedMessage = DateTime.UtcNow.ToString() + " -- " + Message;

            using (var LogWriter = new StreamWriter(LogFile))
            {
                LogWriter.WriteLine(Message);
            }
        }
        public void LogMessage(string Message)
        {
            string TimeStampedMessage = DateTime.UtcNow.ToString() + " -- " + Message;

            using (var LogWriter = new StreamWriter(LogFile))
            {
                LogWriter.Write(Message);
            }
        }

        public void LogByteArray(byte[] ByteArray)
        {
            string Message = "\n \\_"+ ByteArrayToString(ByteArray);
            LogLine(Message);
        }
        private string ByteArrayToString(byte[] ba)
        {
            StringBuilder hex = new StringBuilder(ba.Length * 2);
            foreach (byte b in ba)
                hex.AppendFormat("{0:x2} ", b);
            return hex.ToString();
        }

    }
}
