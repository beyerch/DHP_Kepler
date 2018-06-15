using DHPJ2534Sharp.Protocols;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp.Handlers
{
    /// <summary>
    /// Stores a dictionary of periodic messages and provides methods to create
    /// and destroy periodic messages
    /// </summary>
   public class PeriodicMessageHandler
    {
        /// <summary>
        /// Containter for active periodic messages
        /// </summary>
        public Dictionary<int,PeriodicMessage> PeriodicMessages { get; set; }
     
        public PeriodicMessageHandler()
        {
            PeriodicMessages = new Dictionary<int, PeriodicMessage>();
        }
        /// <summary>
        /// Stops an  active periodic message and delete it.
        /// </summary>
        /// <param name="MessageID">ID of the periodic message to be delted</param>
        /// <returns>Success code</returns>
        public int StopPeriodicMessage(int MessageID)
        {
            PeriodicMessages[MessageID].Stop();
            PeriodicMessages.Remove(MessageID);
            return 0x00;
        }
        /// <summary>
        /// Creates a periodic message
        /// </summary>
        /// <param name="Messages">Message to send periodically</param>
        /// <param name="Protocol">Protocol to use to send message</param>
        /// <param name="TimeInterval">Time between messages</param>
        /// <returns>Message ID</returns>
        public int StartPeriodicMessage(KeplerCommand Messages, IProtocol Protocol, double TimeInterval )
        {
            var PeriodicMessageID = PeriodicMessages.Count + 1;
            var PeriodicMesssage = new PeriodicMessage(Messages, Protocol, TimeInterval);
            PeriodicMessages.Add(PeriodicMessageID, PeriodicMesssage);
            return PeriodicMessageID;
        }

        internal void StopAll()
        {
            foreach(var periodicMessage in PeriodicMessages)
            {
                periodicMessage.Value.Stop();
            }
            PeriodicMessages.Clear();

        }
    }
}
