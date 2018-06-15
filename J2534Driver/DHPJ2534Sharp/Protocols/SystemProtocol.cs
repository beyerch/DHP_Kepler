using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp.Protocols
{
    public delegate void InterfaceErrorReceievedEventHandler(object sender, InterfaceErrorEventArgs e);

    public class SystemProtocol : BaseProtocol
    {
        public new string ProtocolName { get { return "System"; } set { } }
        public new int ProtocolID { get { return 0; } set { } }

        public event InterfaceErrorReceievedEventHandler OnInterfaceErrorReceieved;
        public SystemProtocol(Kepler KeplerInterface) : base(KeplerInterface)
        {

        }

        public override int WriteMessages(List<KeplerCommand> Messages)
        {
            List<KeplerCommand> FormattedMessages = new List<KeplerCommand>();
            foreach (var Message in Messages)
            {
                var NewMessage = new byte[Message.Message.Length + 3];
                var MessageIndex = 0;
                NewMessage[MessageIndex++] = 0x02;
                NewMessage[MessageIndex++] = (byte)((NewMessage.Length - 3 & 0xFF00) >> 8);
                NewMessage[MessageIndex++] = (byte)((NewMessage.Length - 3 & 0x00FF));
                var FormattedCommand = new KeplerCommand() { Message = NewMessage };
                FormattedMessages.Add(FormattedCommand);
            }
            return base.WriteMessages(FormattedMessages);
        }

        internal void ErrorRecieved(KeplerCommand Message)
        {
            OnInterfaceErrorReceieved(this, new InterfaceErrorEventArgs(Message));
        }
      

    }
}
