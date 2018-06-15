using DHPJ2534Sharp.Protocols;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
    public unsafe class J2534
    {
        public Kepler kepler;
        public J2534()
        {
            Debug.WriteLine("J2534 Created");
        }
        public int PassThruOpen()
        {
            //string portNumber = "COM3";
            //Debug.WriteLine("PassThruOpen - " + portNumber);
            //  kepler = new Kepler(portNmber);
            kepler = new Kepler("03EB","2404");
            return 0x00;
        }

        public int PassThruClose(ulong DeviceID)
        {
            Debug.WriteLine("PassThruClose - " + DeviceID);

            return 0x00;
        }

        public int PassThruConnect(ulong DeviceID, ulong ProtocolID, ulong Flags, ulong Baudrate)
        {
            Debug.WriteLine("PassThruConnect - Device:" + DeviceID + "Protocol: "+ProtocolID+" Flags: "+Flags+ " Bitrate: Baudrate");

            return kepler.OpenChannel((int)ProtocolID, (int) Flags, (int)Baudrate);
            //return 0;
        }

        public int PassThruDisconnect(ulong ChannelID)
        {
            Debug.WriteLine("PassThruDisconnect - Channel: " + ChannelID);

           kepler.OpenChannels[(int)ChannelID].DeleteAllFilters();
           kepler.OpenChannels[(int)ChannelID].ClearRxBuffer();
           kepler.OpenChannels[(int)ChannelID].StopAllPeriodicMessages();
           kepler.OpenChannels.Remove((int)ChannelID);
           kepler.ClearFilterCounter();
            return 0x00;
        }
      
        public List<KeplerCommand> PassThruReadMsgs(ulong ChannelID, ulong pNumMsgs, ulong Timeout)
        {
            Debug.WriteLine("J2534::PassThruReadMsgs -- User application requesting " + pNumMsgs+" with a timeout of " + Timeout);
            return kepler.OpenChannels[(int)ChannelID].ReadMessages((int)pNumMsgs,Timeout);
        }


        public int PassThruWriteMsg(ulong ChannelID, byte[] data, int DataSize, ulong TxFlags)
        {
            Debug.WriteLine("PassThruWriteMsg - Channel: " + ChannelID);

            kepler.OpenChannels[(int)ChannelID].WriteMessages(new List<KeplerCommand> { new KeplerCommand { ProtocolID =  ChannelID, DataSize = DataSize, Message = data, TxFlags = TxFlags} });
            return 0x00;
        }


        public int PassThruStartPeriodicMsg(ulong ChannelID, byte[] Data, ulong TxFlags, ulong TimeInterval)
        {
            Debug.WriteLine("PassThruStartPeriodicMsg - Channel: " + ChannelID);

            // return kepler.StartPeriodicMessage(new KeplerCommand() { DataSize = Data.Length, Message = Data, TxFlags = TxFlags, ProtocolID = ChannelID }, (int)ChannelID, TimeInterval);
            return kepler.OpenChannels[(int)(ChannelID)].StartPeriodicMessage(new KeplerCommand() { DataSize = Data.Length, Message = Data, TxFlags = TxFlags, ProtocolID = ChannelID }, kepler.OpenChannels[(int)ChannelID], TimeInterval);
        }


        public int PassThruStopPeriodicMsg(ulong MessageID, ulong ChannelID)
        {
            Debug.WriteLine("PassThruStopPeriodicMsg - Channel: " + ChannelID + " Message: "+MessageID);

            return kepler.OpenChannels[(int)(ChannelID)].StopPeriodicMessage((int)MessageID);
        }


        public int PassThruStartMsgFilter(ulong ChannelID, ulong FilterType, byte[] MaskMsg, byte[] PatternMsg, byte[] FlowControlMsg)
        {
            Debug.WriteLine("PassThruStartMsgFilter - Channel: " + ChannelID);

            return kepler.CreateMessageFilter(ChannelID, FilterType, MaskMsg, PatternMsg, FlowControlMsg);
        }


        public int PassThruStopMsgFilter(ulong ChannelID, ulong FilterID)
        {
            Debug.WriteLine("PassThruStopMsgFilter - Channel: " + ChannelID);

            return kepler.StopMessageFilter((int) ChannelID, (int)FilterID);
        }


        public int PassThruSetProgrammingVoltage(ulong DeviceID, ulong PinNumber, ulong Voltage)
        {
            return 0x00;
        }

        public KeplerVersion PassThruReadVersion(ulong DeviceID)
        {
          return kepler.GetVersion();
           // return null;
        }

        public int GetAvailableMessages(ulong ChannelID)
        {
           return kepler.OpenChannels[(int)ChannelID].ReceiverQueue.Count();
        }

        public int ClearTxBuffer()
        {
            kepler.KeplerSerialCommunication.ClearTxBuffer();
            return 0x00;
        }

        public int ClearRxBuffer(ulong Channel)
        {
            kepler.OpenChannels[(int)Channel].ClearRxBuffer();
            return 0x00;
        }

        public int ClearPeriodicMessages(ulong Channel)
        {
            kepler.OpenChannels[(int)Channel].StopAllPeriodicMessages();
            return 0x00;
        }
        public void Shutdown()
        {
            kepler.Shutdown();
        }

       public void ClearMessageFilters(ulong ChannelID)
        {
            kepler.OpenChannels[(int)ChannelID].DeleteAllFilters();
        }



    }
}
