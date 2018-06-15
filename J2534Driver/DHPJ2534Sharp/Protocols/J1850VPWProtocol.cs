using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp.Protocols
{
    public class J1850VPWProtocol : BaseProtocol
    {
        public new string ProtocolName { get { return "J1850VPW"; } set { } }
        public new int ProtocolID { get { return 1; } set { } }

        public J1850VPWProtocol(Kepler KeplerInterface) : base(KeplerInterface)
        {
            //Enter VPW Mode
            KeplerInterface.KeplerSerialCommunication.SendMessage(new KeplerCommand { Message = new byte[] { 0x02, 0x00, 0x02, 0xA0, 0x00 } });
        }

        public override int WriteMessages(List<KeplerCommand> Messages)
        {
            List<KeplerCommand> FormattedMessages = new List<KeplerCommand>();

            //Loop through Messages and add kepler header for VPW to each
            foreach (var Message in Messages)
            {
                var NewMessage = new byte[Message.Message.Length + 4];
                NewMessage[0] = 0x02;
                NewMessage[1] = (byte)(((Message.Message.Length + 1) & 0xFF00) >> 8);
                NewMessage[2] = (byte)((Message.Message.Length + 1) & 0x00FF);
                NewMessage[3] = 0xA1;
                Message.Message.CopyTo(NewMessage, 4);
                var FormattedCommand = new KeplerCommand() { Message = NewMessage };
                FormattedMessages.Add(FormattedCommand);

                if (base.isLoopbackEnabled)
                {
                    Message.ProtocolID = (ulong)ProtocolID;
                    Message.RxStatus |= 0x01;
                    base.Loopback(Message);
                }
            }
            return base.WriteMessages(FormattedMessages);
        }

        public override int CreateFilter(ulong FilterType, byte[] MaskMsg, byte[] PatternMsg, byte[] FlowControlMsg)
        {

            //All length calculations are based off the mask message length because the lengths cannot vary. Checking that now.
            if (MaskMsg.Length != PatternMsg.Length)
            {
                return -1;
            }

            //The length of the kepler filter message is: 4 byte header + 2 byte filter length + 1 byte filter type + FilterLength * 3 = 7 + (FilterLength*3)
            var FilterMessageLength = 7 + (MaskMsg.Length * 3);
            var KeplerCreateFilterMessage = new byte[FilterMessageLength];

            //In CAN mode we can't have a flow control message so we are just going to zero it out
            FlowControlMsg = new byte[MaskMsg.Length];

            //Build the message header with Create filter command byte
            KeplerCreateFilterMessage[0] = 0x02;
            KeplerCreateFilterMessage[1] = (byte)(((FilterMessageLength - 3) & 0xFF00) >> 8);
            KeplerCreateFilterMessage[2] = (byte)((FilterMessageLength - 3) & 0x00FF);
            KeplerCreateFilterMessage[3] = 0xC0;
            //Filter length bytes
            KeplerCreateFilterMessage[4] = (byte)(((MaskMsg.Length) & 0xFF00) >> 8);
            KeplerCreateFilterMessage[5] = (byte)((MaskMsg.Length) & 0x00FF);
            //Filter type byte
            KeplerCreateFilterMessage[6] = (byte)FilterType;
            //Copy mask
            MaskMsg.CopyTo(KeplerCreateFilterMessage, 7);
            //Copy pattern after mask
            PatternMsg.CopyTo(KeplerCreateFilterMessage, MaskMsg.Length + 7);
            //Copy flow control after mask after pattern
            FlowControlMsg.CopyTo(KeplerCreateFilterMessage, MaskMsg.Length + MaskMsg.Length + 7);

            //The length of the kepler CAN mailbox message is 4 + 2 + (Length*2)
            //FilterMessageLength = 4 + 2 + (MaskMsg.Length * 2);
            //var KeplerCreateMailboxMessage = new byte[FilterMessageLength];
            ////Build the message header with Create filter command byte
            //KeplerCreateMailboxMessage[0] = 0x02;
            //KeplerCreateMailboxMessage[1] = (byte)(((FilterMessageLength - 3) & 0xFF00) >> 8);
            //KeplerCreateMailboxMessage[2] = (byte)((FilterMessageLength - 3) & 0x00FF);
            //KeplerCreateMailboxMessage[3] = 0xB6;
            ////Filter length bytes
            //KeplerCreateMailboxMessage[4] = (byte)(((MaskMsg.Length) & 0xFF00) >> 8);
            //KeplerCreateMailboxMessage[5] = (byte)((MaskMsg.Length) & 0x00FF);
            ////Copy mask
            //MaskMsg.CopyTo(KeplerCreateMailboxMessage, 6);
            ////Copy pattern after mask
            //PatternMsg.CopyTo(KeplerCreateMailboxMessage, MaskMsg.Length + 6);



            //write the messages to kepler 
            base.WriteKeplerMessage(KeplerCreateFilterMessage);
            //  base.WriteKeplerMessage(KeplerCreateMailboxMessage);


            //TODO: Firmware needs to return a filter ID which can be used to remove that filter from the system.  
            //For now SPS only uses delete all filters which is implemented. Though this feature needs to be added 
            //for full J2534 compatibility.
            return 0x00;
        }
    }
}
