
using DHPJ2534Sharp.Protocols;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;

namespace DHPJ2534Sharp
{
    public class PeriodicMessage
    {
        public IProtocol Protocol { get; private set; }
        public KeplerCommand Message { get; set; }

        private Timer PeriodicMessageTimer;
        /// <summary>
        /// Creates a new periodic message
        /// </summary>
        /// <param name="Message">Message to send</param>
        /// <param name="Protocol">Protocol to use</param>
        /// <param name="TimeInterval">Time between messages</param>
        public PeriodicMessage(KeplerCommand Message, IProtocol Protocol, double TimeInterval)
        {
            //Set the protocol
            this.Protocol = Protocol;
            //Set the single message (J2534 Compatability)
            this.Message = Message;
            //Create the TX timer
            PeriodicMessageTimer = new Timer(TimeInterval);
            //Set the event handler
            PeriodicMessageTimer.Elapsed += PeriodicMessageTimer_Elapsed;
            //Write the message to the TX queue
            Protocol.WriteMessages(new List<KeplerCommand>() { Message });
            //Start the timer
            PeriodicMessageTimer.Start();

        }
        /// <summary>
        /// Stops the internal periodic timer
        /// </summary>
        internal void Stop()
        {
            PeriodicMessageTimer.Stop();
        }
        /// <summary>
        /// Called when the periodic timer timesout. We should send the message now
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void PeriodicMessageTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            //Write the message to the TX queue
            Protocol.WriteMessages( new List<KeplerCommand>() { Message });
            //Start the timer again
            PeriodicMessageTimer.Start();
        }
    }
}
