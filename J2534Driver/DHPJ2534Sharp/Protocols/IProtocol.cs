using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp.Protocols
{
    public interface IProtocol
    {
        ConcurrentQueue<KeplerCommand> ReceiverQueue { get;  set; }
        string ProtocolName { get;  set; }
        int ProtocolID { get;  set; }
        List<KeplerCommand> ReadMessages(int MessageCount, double Timeout);
        KeplerCommand ReadMessage(double Timeout);

        int WriteMessages(List<KeplerCommand> Messages);
        int CreateFilter(ulong FilterType, byte[] MaskMsg, byte[] PatternMsg, byte[] FlowControlMsg);
        int DeleteFilter(int FilterID);
        int DeleteAllFilters();
        void WriteKeplerMessage(byte[] Message);
        void StopAllPeriodicMessages();
        void ClearRxBuffer();
        int StartPeriodicMessage(KeplerCommand Messages, IProtocol Channel, double TimeInterval);
        int StopPeriodicMessage(int MessageID);


    }
}
