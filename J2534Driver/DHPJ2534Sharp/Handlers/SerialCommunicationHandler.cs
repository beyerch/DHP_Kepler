using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
    /// <summary>
    /// This class handles all serial communication to Kepler. It will create a TX and RX queue to 
    /// handle communication. 
    /// </summary>
    public class SerialCommunicationHandler
    {
        private SerialPort KeplerSerialPort;
        private DateTime StartTime;
        private ConcurrentQueue<KeplerCommand> TransmitterQueue = new ConcurrentQueue<KeplerCommand>();
        private CancellationToken ReceiverCancellationToken = new CancellationToken();
        private CancellationToken TransmitterCancellationToken = new CancellationToken();
        private Task ReceiverTask;
        private Task TransmitterTask;
        private string portName;
        private Kepler kepler;

        public event EventHandler OnMessageReceived;

        /// <summary>
        /// Returns the number of messages in the RX queue
        /// </summary>
        /// <summary>
        /// Constructs a new serial communication handler object
        /// </summary>
        /// <param name="KeplerSerialPortName">COM Port to use for communication</param>
        /// <param name="StartTime">Date time object used to calculate the timestamp on messages</param>
        public SerialCommunicationHandler(string KeplerSerialPortName, Kepler Interface)
        {
            //Create Serial Port for communications
            KeplerSerialPort = new SerialPort(KeplerSerialPortName);
            KeplerSerialPort.Open();

            //Start multithreaded communications
            ReceiverTask = new Task(KeplerReceiver, ReceiverCancellationToken);
            TransmitterTask = new Task(KeplerTransmitter, TransmitterCancellationToken);
            ReceiverTask.Start();
            TransmitterTask.Start();
            //Attach a blank event handler to prevernt a null point exception if attaches to this event
            OnMessageReceived += Kepler_OnMessageReceived;
            this.kepler = Interface;

            //Set the StartTime
            this.StartTime = Interface.startTime;
        }

        /// <summary>
        /// Closes down the handler
        /// </summary>
        public void Shutdown()
        {
            KeplerSerialPort.Close();
        }
       
        /// <summary>
        /// Removes all messages form the Tx queue
        /// </summary>
        public void ClearTxBuffer()
        {
            TransmitterQueue = new ConcurrentQueue<KeplerCommand>();
        }

        private void Kepler_OnMessageReceived(object sender, EventArgs e)
        {
            //Blank to prevent null pointer exceptions
        }
        /// <summary>
        /// Sens a messsage to kepler
        /// </summary>
        /// <param name="Command">KeplerCommand object containing the message to be sent</param>
        public void SendMessage(KeplerCommand Command)
        {
#if DEBUG        
            Debug.WriteLine(DateTime.UtcNow + " -- Sent: " + Helpers.ByteArrayToString(Command.Message));
#endif
            //Add the command to the tx queue
            TransmitterQueue.Enqueue(Command);
        }
       
        /// <summary>
        /// This is the thread which handles receiving messages from kepler
        /// </summary>
        private void KeplerReceiver()
        {
            while (KeplerSerialPort.IsOpen)
            {
                try
                {
                    //Check for task canellation
                    if (TransmitterCancellationToken.IsCancellationRequested)
                    {
                        return;
                    }
                    //Get a byte

                    //TODO: CHECK THIS! THREAD SAFE?? HAMMERING SERIAL??
                    var CurrByte = KeplerSerialPort.ReadByte(); 
                    //Check if start byet
                    if (CurrByte == Properties.StartByte)
                    {
                        //Build Kepler haeder
                        var IncomingCommand = new KeplerCommand();
                        var LengthHigh = KeplerSerialPort.ReadByte();
                        var LengthLow = KeplerSerialPort.ReadByte();
                        var Length = (short)((LengthHigh << 8) | LengthLow);
                        IncomingCommand.Message = new byte[Length];
                        //Get the rest of the message

                        //TODO: TIME OUT?
                        KeplerSerialPort.Read(IncomingCommand.Message, 0, Length);
                         
                        //Add timestamp
                        IncomingCommand.Timestamp = (DateTime.UtcNow - StartTime).TotalMilliseconds * 1000;
                        if(IncomingCommand.Message[0] == 0xAA)
                        {
                        var Protocol = IncomingCommand.Message[1];
                        byte[] ActualMessage = new byte[IncomingCommand.Message.Length - 2];
                        Buffer.BlockCopy(IncomingCommand.Message, 2, ActualMessage, 0, ActualMessage.Length);
                        IncomingCommand.Message = ActualMessage;
                        IncomingCommand.DataSize = ActualMessage.Length;
                        IncomingCommand.ExtraDataIndex = ActualMessage.Length;
                        //Add it to the RX queue
                       
                            switch (Protocol & 0x03)
                            {
                                case 1:
                                    IncomingCommand.ProtocolID = 1;
                                    try
                                    {
                                        kepler.OpenChannels[(int)IncomingCommand.ProtocolID].ReceiverQueue.Enqueue(IncomingCommand);
                                    }
                                    catch (Exception e)
                                    {
                                        Debug.WriteLine("Channel 1 Not found");
                                        //Channel not found error
                                        //TODO: HANDLE CHANNEL NOT OPEN
                                    }
                                    break;
                                case 2:
                                    IncomingCommand.ProtocolID = 5;
                                    try
                                    {
                                        kepler.OpenChannels[(int)IncomingCommand.ProtocolID].ReceiverQueue.Enqueue(IncomingCommand);
                                    }
                                    catch (Exception e)
                                    {
                                        Debug.WriteLine("Channel 5 Not found");

                                        //Channel not found error
                                        //TODO: HANDLE CHANNEL NOT OPEN
                                    }
                                    break;
                                case 3:
                                    IncomingCommand.ProtocolID = 6;
                                    try
                                    {
                                        kepler.OpenChannels[(int)IncomingCommand.ProtocolID].ReceiverQueue.Enqueue(IncomingCommand);
                                    }
                                    catch (Exception e)
                                    {
                                        Debug.WriteLine("Channel 6 Not found");

                                        //Channel not found error
                                        //TODO: HANDLE CHANNEL NOT OPEN
                                    }
                                    break;
                            }
                        }
                        else
                        {
                            IncomingCommand.ProtocolID = 0;
                            try
                            {
                                if (IncomingCommand.DataSize >= 4 && IncomingCommand.Message[3] == 0xEF)
                                {
                                    (kepler.OpenChannels[(int)IncomingCommand.ProtocolID] as Protocols.SystemProtocol).ErrorRecieved(IncomingCommand);
                                }
                                else
                                {
                                    kepler.OpenChannels[(int)IncomingCommand.ProtocolID].ReceiverQueue.Enqueue(IncomingCommand);
                                }
                            }
                            catch (Exception e)
                            {
                                //Channel not found error
                            }
                        }
                          //  ReceiverQueue.Enqueue(IncomingCommand);

#if DEBUG
                            Debug.WriteLine(DateTime.UtcNow + " Queued Message from "+ kepler.OpenChannels[(int)IncomingCommand.ProtocolID].ProtocolName +" " + Helpers.ByteArrayToString(IncomingCommand.Message));
                           // Debug.WriteLine(ReceiverQueue.Count + " messages in RX queue");
#endif
                            //Notify we got a message (Message not attached to event as it is in the queue)
                            OnMessageReceived(this, new KeplerEventArgs {ProtocolID = (int)IncomingCommand.ProtocolID });
                        }
                    }
                catch (Exception e)
                {
                    Debug.WriteLine("Serial Port Error");
                    //Serial Port Error
                    //TODO: SERIAL PORT HANDLE ERROR 
                }
            }
        }
        /// <summary>
        /// This is the thread which handles sending messages to kepler
        /// </summary>
        private void KeplerTransmitter()
        {
            KeplerCommand CurrentCommand = null;
            while (KeplerSerialPort.IsOpen)
            {
                //Check for task canellation
                if (ReceiverCancellationToken.IsCancellationRequested)
                {
                    return;
                }
                //If the tx queue isn't empty
                if (!TransmitterQueue.IsEmpty)
                {
                    //Try and get a message from the non-empty queue
                    var success = TransmitterQueue.TryDequeue(out CurrentCommand);
                    
                    if (success && CurrentCommand != null)
                    {
                        //Write the message to the serial port. .NET handes the rest. 
                        try
                        {
                            KeplerSerialPort.Write(CurrentCommand.Message, 0, CurrentCommand.Message.Length);
                        }catch (Exception e)
                        {

                        }
                    }
                }
            }
        }
    }
    public class KeplerEventArgs : EventArgs
    {
        public int ProtocolID { get; set; }
    }

    public class KeplerCommand
    {
        public KeplerCommand()
        {
            
        }
        public KeplerCommand(int bytes)
        {
            Message = new byte[bytes];
        }
        /// <summary>
        /// Protocol type
        /// </summary>
        public ulong ProtocolID { get; set; }
        /// <summary>
        /// Receive message status – See RxStatus in "Message Flags and Status Definition" section
        /// </summary>
        public long RxStatus { get; set; }
        /// <summary>
        /// Transmit message flags
        /// </summary>
        public ulong TxFlags { get; set; }
        /// <summary>
        /// Received message timestamp (microseconds): For the START_OF_FRAME
        /// indication, the timestamp is for the start of the first bit of the message. For all other
        /// indications and transmit and receive messages, the timestamp is the end of the last
        /// bit of the message. For all other error indications, the timestamp is the time the error
        /// is detected.
        /// </summary>
        public double Timestamp { get; set; }
        /// <summary>
        /// Data size in bytes, including header bytes, ID bytes, message data bytes, and extra
        /// data, if any.
        /// </summary>
        public long DataSize { get; set; }
        /// <summary>
        ///  Start position of extra data in received message (for example, IFR). The extra data
        /// bytes follow the body bytes in the Data array. The index is zero-based. When no
        /// extra data bytes are present in the message, ExtraDataIndex shall be set equal to
        /// DataSize. Therefore, if DataSize equals ExtraDataIndex, there are no extra data
        /// bytes. If ExtraDataIndex=0, then all bytes in the data array are extra bytes.
        /// </summary>
        public long ExtraDataIndex { get; set; }
        /// <summary>
        ///  Message Data
        /// </summary>
        public byte[] Message { get; set; }
    }
}
