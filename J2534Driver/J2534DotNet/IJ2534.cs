#region License
/* 
 * Copyright (c) 2010, Michael Kelly
 * michael.e.kelly@gmail.com
 * http://michael-kelly.com/
 * 
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

using System;

namespace J2534DotNet
{
    public interface IJ2534
    {
        bool LoadLibrary(J2534Device device);
        bool FreeLibrary();

        J2534Err PassThruOpen(IntPtr name, ref int deviceId);
        J2534Err PassThruClose(int deviceId);

        J2534Err PassThruConnect(int deviceId, ProtocolID protocolId, ConnectFlag flags, BaudRate baudRate,
            ref int channelId);

        J2534Err PassThruDisconnect(int channelId);
        J2534Err PassThruReadMsgs(int channelId, IntPtr msgs, ref int numMsgs, int timeout);
        J2534Err PassThruWriteMsgs(int channelId, IntPtr msg, ref int numMsgs, int timeout);
        J2534Err PassThruStartPeriodicMsg(int channelId, IntPtr msg, ref int msgId, int timeInterval);
        J2534Err PassThruStopPeriodicMsg(int channelId, int msgId);

        J2534Err PassThruStartMsgFilter(int channelid, FilterType filterType, IntPtr maskMsg,
            IntPtr patternMsg, IntPtr flowControlMsg, ref int filterId);

        J2534Err PassThruStopMsgFilter(int channelId, int filterId);
        J2534Err PassThruSetProgrammingVoltage(int deviceId, PinNumber pinNumber, int voltage);
        J2534Err PassThruReadVersion(int deviceId, IntPtr firmwareVersion, IntPtr dllVersion, IntPtr apiVersion);
        J2534Err PassThruGetLastError(IntPtr errorDescription);
        J2534Err PassThruIoctl(int channelId, int ioctlID, IntPtr input, IntPtr output);
    }
}