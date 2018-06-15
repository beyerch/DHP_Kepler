using DHPJ2534Sharp.Handlers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Concurrent;
using System.Threading;

namespace DHPJ2534Sharp.Protocols
{
    /// <summary>
    /// Handles the functionality that is common across all protocols
    /// </summary>
    public class BaseProtocol : IProtocol
    {
        public int AvailableMessages { get { return ReceiverQueue.Count; } private set { } }
        /// <summary>
        /// Should always be overridden
        /// </summary>
        public string ProtocolName { get { return "Base"; }  set { } }
        /// <summary>
        /// Should always be overridden
        /// </summary>
        public int ProtocolID { get { return -1; }  set { } }
        /// <summary>
        /// Tracks the state of loopback
        /// </summary>
        public bool isLoopbackEnabled { get; set; }
        /// <summary>
        /// Queue to hold the messages from the network
        /// </summary>
        public ConcurrentQueue<KeplerCommand> ReceiverQueue { get;  set; }

        /// <summary>
        /// Reference to the base kepler interface object.
        /// </summary>
        private Kepler keplerInterface;
        /// <summary>
        /// Creates a base protocol and stores the reference to kepler
        /// </summary>
        /// <param name="KeplerInterface">Reference to active kepler interface</param>
        /// 

        internal PeriodicMessageHandler PeriodicHandler;

        public BaseProtocol(Kepler KeplerInterface)
        {
           this.keplerInterface = KeplerInterface;
           this.ReceiverQueue = new ConcurrentQueue<KeplerCommand>();
           PeriodicHandler = new PeriodicMessageHandler();


        }
        public void SetLoopback(bool mode)
        {
            isLoopbackEnabled = mode;
        }
        public virtual int CreateFilter(ulong FilterType, byte[] pMaskMsg, byte[] pPatternMsg, byte[] pFlowControlMsg)
        {
            return 0x01;
        }
        /// <summary>
        /// Deletes all filters currently active on the kepler interface
        /// </summary>
        /// <returns>Success code</returns>
        public int DeleteAllFilters()
        {
            keplerInterface.KeplerSerialCommunication.SendMessage(new KeplerCommand() { Message = new byte[] { 0x02, 0x00, 0x01, 0xC3 }});
            
            return 0;
        }
        /// <summary>
        /// Deletes a specific filter on kepler
        /// </summary>
        /// <param name="FilterID">ID of filter to be removed</param>
        /// <returns></returns>
        public int DeleteFilter(int FilterID)
        {
            keplerInterface.KeplerSerialCommunication.SendMessage(new KeplerCommand() { Message = new byte[] { 0x02, 0x00, 0x02, 0xC4, (byte)((FilterID&0x00FF)-1) } });
            return 0x00;
        }
        /// <summary>
        /// Dequeus a specific message count from the internal Rx buffers
        /// </summary>
        /// <param name="MessageCount">Number of messages to collect</param>
        /// <returns>List of messages collected</returns>
        public virtual List<KeplerCommand> ReadMessages(int MessageCount, double Timeout)
        {
            List<KeplerCommand> Messages = new List<KeplerCommand>(MessageCount);

            var start = DateTime.UtcNow;
            var NumToRead = WaitForMessages(Timeout, MessageCount);
            var end = DateTime.UtcNow;

            Debug.WriteLine("We watied for " + (end - start).TotalMilliseconds + "ms");
            Debug.WriteLine("We are going to read " + NumToRead + " messages");

            for (int MessageCounter = 0; MessageCounter < NumToRead; MessageCounter++)
            {
                Messages.Add(DequeueMessage());
            }
            return Messages;
        }

        /// <summary>
        /// Waits for a specified amount of time for a specified number of messages to be available. 
        /// </summary>
        /// <param name="Timeout">
        /// Timeout interval(in milliseconds) to wait for read completion. A value of zero instructs the function to
        /// return immediately with the number of messages that should be read. A nonzero timeout value instructs the function
        /// to return after the timeout interval has expired. The function will not wait the entire timeout
        /// interval if an error occurs or the specified number of messages have been read.
        /// </param>
        /// <param name="MessageCount">Messages to wait for</param>
        /// <returns>The number of messages that can be read out</returns>
        private int WaitForMessages(double Timeout, int MessageCount)
        {
          
            var MaxWaitingTime = TimeSpan.FromMilliseconds(Timeout);
            var sw = Stopwatch.StartNew();

            while (sw.Elapsed < MaxWaitingTime)
            {
                if (this.ReceiverQueue.Count >= MessageCount)
                {
                    return MessageCount;
                }
            }

            return this.ReceiverQueue.Count;
        }
        /// <summary>
        /// Reads a single message from the queue. If one is not present
        /// it will wait the timeout period. If a message still does not arrive
        /// it will return null. Waiting will be blocking.
        /// </summary>
        /// <param name="Timeout">Time to wait if no messages available</param>
        /// <returns>A message or null</returns>
        public virtual KeplerCommand ReadMessage(double Timeout)
        {
            var MaxWaitingTime = TimeSpan.FromMilliseconds(Timeout);
            var sw = Stopwatch.StartNew();

            while(sw.Elapsed < MaxWaitingTime)
            {
                if(this.ReceiverQueue.Count > 1)
                {
                    break;
                }
            }

           return DequeueMessage();
        }

        /// <summary>
        /// Writes a list of messages to kepler
        /// </summary>
        /// <param name="Messages">Messages to write</param>
        /// <returns>number of messages written</returns>
        public virtual int WriteMessages(List<KeplerCommand> Messages)
        {
            int i = 0;
            for(i = 0; i < Messages.Count; i++)
            {
                keplerInterface.KeplerSerialCommunication.SendMessage(Messages[i]);
            }
            return i;
        }

        /// <summary>
        /// Writes a single message. This message will not be formatted, so it
        /// must already have the required kepler header information appended to it. 
        /// </summary>
        /// <param name="Message">Kepler formatted message to send</param>
        public void WriteKeplerMessage(byte[] Message)
        {
            keplerInterface.KeplerSerialCommunication.SendMessage(new KeplerCommand() { Message = Message });
        }

       
        /// <summary>
        /// Loopsback a message to the RX queue.
        /// </summary>
        /// <param name="LoopbackMessage">Message to loopback</param>
        public void Loopback(KeplerCommand LoopbackMessage)
        {
            ReceiverQueue.Enqueue(LoopbackMessage);
        }
        /// <summary>
        /// Shuts down all periodic messages
        /// </summary>
   
        public int StartPeriodicMessage(KeplerCommand Messages, IProtocol Channel, double TimeInterval)
        {
            return PeriodicHandler.StartPeriodicMessage(Messages,Channel , TimeInterval);
        }

        public int StopPeriodicMessage(int MessageID)
        {
            return PeriodicHandler.StopPeriodicMessage(MessageID);
        }

        public void StopAllPeriodicMessages()
        {
            PeriodicHandler.StopAll();
        }
        /// <summary>
        /// Removes all messages from the Rx queue
        /// </summary>
        public void ClearRxBuffer()
        {
            ReceiverQueue = new ConcurrentQueue<KeplerCommand>();
        }

        /// <summary>
        /// Gets a message from the Rx queue if one is available. 
        /// </summary>
        /// <returns>A KeplerCommand object containing a message. Can return null if no messages are available</returns>
        public KeplerCommand DequeueMessage()
        {
            KeplerCommand cmd;
            //Try to get a command from the rx queue
            ReceiverQueue.TryDequeue(out cmd);
#if DEBUG
            if (cmd != null)
            {
                Debug.WriteLine(DateTime.UtcNow + " Dequeued: " + Helpers.ByteArrayToString(cmd.Message));
            }
            else
            {
                Debug.WriteLine("Dequeued NULL message");
            }
#endif
            return cmd;
        }

      
    }
}
