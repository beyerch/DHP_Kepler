#region License
/* 
 * (c) 2015 kuzkok
 * kuzkok@gmail.com
 * 
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the organization nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#endregion License

namespace J2534DotNet
{
    using System;

    public class J2534: IJ2534
    {

        protected J2534Device m_device;
        protected J2534DllWrapper m_wrapper;

        public bool LoadLibrary(J2534Device device)
        {
            m_device = device;
            m_wrapper = new J2534DllWrapper();
            return m_wrapper.LoadJ2534Library(m_device.FunctionLibrary);
        }

        public bool FreeLibrary()
        {
            return m_wrapper.FreeLibrary();
        }

        public J2534Err PassThruOpen(IntPtr name, ref int deviceId)
        {
            return (J2534Err)m_wrapper.Open(name, ref deviceId);
        }

        public J2534Err PassThruClose(int deviceId)
        {
            return (J2534Err)m_wrapper.Close(deviceId);
        }

        public J2534Err PassThruConnect(int deviceId, ProtocolID protocolId, ConnectFlag flags, BaudRate baudRate, ref int channelId)
        {
            return (J2534Err)m_wrapper.Connect(deviceId, (int)protocolId, (int)flags, (int)baudRate, ref channelId);
        }

        public J2534Err PassThruDisconnect(int channelId)
        {
            return (J2534Err)m_wrapper.Disconnect(channelId);
        }

        public J2534Err PassThruReadMsgs(int channelId, IntPtr msgs, ref int numMsgs, int timeout)
        {
            return (J2534Err) m_wrapper.ReadMsgs(channelId, msgs, ref numMsgs, timeout);
        }

        public J2534Err PassThruWriteMsgs(int channelId, IntPtr msgs, ref int numMsgs, int timeout)
        {
            return (J2534Err)m_wrapper.WriteMsgs(channelId, msgs, ref numMsgs, timeout);
        }

        public J2534Err PassThruStartPeriodicMsg(int channelId, IntPtr msg, ref int msgId, int timeInterval)
        {
            return (J2534Err)m_wrapper.StartPeriodicMsg(channelId, msg, ref msgId, timeInterval);
        }

        public J2534Err PassThruStopPeriodicMsg(int channelId, int msgId)
        {
            return (J2534Err)m_wrapper.StopPeriodicMsg(channelId, msgId);
        }

        public J2534Err PassThruStartMsgFilter(int channelid, FilterType filterType, IntPtr maskMsg,
            IntPtr patternMsg, IntPtr flowControlMsg, ref int filterId)
        {
            return
                (J2534Err)
                    m_wrapper.StartMsgFilter(channelid, (int) filterType, maskMsg, patternMsg,
                        flowControlMsg, ref filterId);
        }

        public J2534Err PassThruStopMsgFilter(int channelId, int filterId)
        {
            return (J2534Err)m_wrapper.StopMsgFilter(channelId, filterId);
        }

        public J2534Err PassThruSetProgrammingVoltage(int deviceId, PinNumber pinNumber, int voltage)
        {
            return (J2534Err)m_wrapper.SetProgrammingVoltage(deviceId, (int)pinNumber, voltage);
        }

        public J2534Err PassThruReadVersion(int deviceId, IntPtr firmwareVersion, IntPtr dllVersion, IntPtr apiVersion)
        {
            return (J2534Err)m_wrapper.ReadVersion(deviceId, firmwareVersion, dllVersion, apiVersion);
        }

        public J2534Err PassThruGetLastError(IntPtr errorDescription)
        {
            return (J2534Err)m_wrapper.GetLastError(errorDescription);
        }

        public J2534Err PassThruIoctl(int channelId, int ioctlID, IntPtr input, IntPtr output)
        {
            return (J2534Err)m_wrapper.Ioctl(channelId, ioctlID, input, output);
        }
    }
}
