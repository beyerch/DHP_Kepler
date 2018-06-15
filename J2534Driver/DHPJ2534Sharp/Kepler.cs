using DHPJ2534Sharp.Handlers;
using DHPJ2534Sharp.Protocols;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
    public class Kepler
    {
        internal DateTime startTime;
        public Dictionary<int, IProtocol> OpenChannels { get; private set; }

        private SerialCommunicationHandler keplerComm;


        public SerialCommunicationHandler KeplerSerialCommunication
        {
            get { return keplerComm; }
            set { keplerComm = value; }
        }

        public Kepler(string PortName)
        {
            Connect(PortName);
            OpenChannels.Add(0, new SystemProtocol(this));
        }
        public Kepler(string PID, string VID)
        {
            var ports = GetVirtualSerialPortName(PID, VID);
            foreach (var port in ports)
            {
                try
                {
                    Debug.WriteLine("Attemping to connect to " + port);
                    Connect(port);
                    Debug.WriteLine("Success!");
                    return;
                }catch (Exception e)
                {
                    Debug.WriteLine("Connection Failed!");

                }
            }
        }

        private List<string> GetVirtualSerialPortName(String VID, String PID)
        {
            String pattern = String.Format("VID_{0}&PID_{1}", VID, PID);
            Regex _rx = new Regex(pattern, RegexOptions.IgnoreCase);
            List<string> comports = new List<string>();

            RegistryKey rk1 = Registry.LocalMachine;
            RegistryKey rk2 = rk1.OpenSubKey("SYSTEM\\CurrentControlSet\\Enum\\USB\\" + pattern);

            foreach (String s in rk2.GetSubKeyNames())
            {
                RegistryKey rk3 = rk2.OpenSubKey(s+"\\Device Parameters");
                comports.Add((string)rk3.GetValue("PortName"));
                //if (_rx.Match(s).Success)
                //{
                //    RegistryKey rk4 = rk3.OpenSubKey(s);
                //    foreach (String s2 in rk4.GetSubKeyNames())
                //    {
                //        RegistryKey rk5 = rk4.OpenSubKey(s2);
                //        string location = (string)rk5.GetValue("LocationInformation");
                //        if (!String.IsNullOrEmpty(location))
                //        {
                //            string port = location.Substring(location.IndexOf('#') + 1, 4).TrimStart('0');
                //            if (!String.IsNullOrEmpty(port)) comports.Add(String.Format("COM{0:####}", port));
                //        }
                //        //RegistryKey rk6 = rk5.OpenSubKey("Device Parameters");
                //        //comports.Add((string)rk6.GetValue("PortName"));
            }
            return comports;
        }

        public void Shutdown()
        {
            KeplerSerialCommunication.Shutdown();

        }
        private void Connect(string PortName)
        {
            startTime = DateTime.UtcNow;
            OpenChannels = new Dictionary<int, IProtocol>();
            KeplerSerialCommunication = new SerialCommunicationHandler(PortName, this);

        }
        public int OpenChannel(int ProtocolID, int Flags, int Baudrate)
        {
            var ChannelID = OpenChannels.Count + 1;

            switch (ProtocolID)
            {
                case 1:
                    OpenChannels.Add(ProtocolID, new J1850VPWProtocol(this));
                    break;
                case 5:
                    OpenChannels.Add(ProtocolID, new CANProtocol(this));
                    break;
                case 6:
                    OpenChannels.Add(ProtocolID, new ISO15765Protocol(this));
                    break;
                default:
                    return 1;
            }
            return ProtocolID;
        }
        public KeplerVersion GetVersion()
        {
            KeplerSerialCommunication.SendMessage(new KeplerCommand { Message = new byte[] { 0x02, 0x00, 0x01, 0xE0 } });
            return new KeplerVersion { BuildDate = DateTime.Now, MajorVersion = 0x01, MinorVersion = 0x02 };
        }
        public double GetTimestamp()
        {
            return (DateTime.UtcNow - startTime).TotalMilliseconds;
        }


        int filterCounter = 0;
        public int CreateMessageFilter(ulong ChannelID, ulong FilterType, byte[] MaskMsg, byte[] PatternMsg, byte[] FlowControlMsg)
        {
            OpenChannels[0].ClearRxBuffer();
            OpenChannels[(int)ChannelID].CreateFilter(FilterType, MaskMsg, PatternMsg, FlowControlMsg);
            OpenChannels[(int)ChannelID].ClearRxBuffer();
            Thread.Sleep(500);
            filterCounter++;
            //TODO: GET THIS FROM THE FIRMWARE RESPONSE
            // return filterCounter++;
            //TODO: WRITE A FUNCTION WHICH TAKES IN A MESSAGE AND WAITS FOR THAT MESSAGE OR TIMES OUT. SIMULAR TO BELOW
            //try
            //{
            //    Thread.Sleep(300);
            //    var filterMessage = OpenChannels[0].ReceiverQueue.Where(msg => msg.Message.Length == 3 && msg.Message[0] == 0xc0).ToArray()[0];
            //    return filterMessage.Message[2];
            //}
            //catch (Exception e)
            //{
            //    Debug.WriteLine("We broke");
            //    return filterCounter;
            //}
            return filterCounter;
            //return -1;
        }

        public void ClearFilterCounter()
        {
            filterCounter = 0;
        }

        public int StopMessageFilter(int ChannelID, int FilterID)
        {
            try
            {
                OpenChannels[(int)ChannelID].DeleteFilter(FilterID);
                filterCounter--;
                return 0x00;
            }
            catch (Exception e)
            {
                return 0x02;
            }

        }

        public void SetLoopback(ulong Mode, ulong ChannelID)
        {
            if (Mode == 1)
            {
                (OpenChannels[(int)ChannelID] as BaseProtocol).isLoopbackEnabled = true;
            }
            else
            {
                (OpenChannels[(int)ChannelID] as BaseProtocol).isLoopbackEnabled = false;
            }
        }

        public int GetLoopback(ulong ChannelID)
        {
            if ((OpenChannels[(int)ChannelID] as BaseProtocol).isLoopbackEnabled == true)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    public class KeplerVersion
    {
        public DateTime BuildDate { get; set; }
        public int MajorVersion { get; set; }
        public int MinorVersion { get; set; }

    }
}
