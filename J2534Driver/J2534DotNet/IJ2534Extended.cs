using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace J2534DotNet
{
    public interface IJ2534Extended: IJ2534
    {
        J2534Err GetConfig(int channelId, ref List<SConfig> config);
        J2534Err SetConfig(int channelId, ref List<SConfig> config);
        J2534Err ReadBatteryVoltage(int deviceId, ref int voltage);
        J2534Err FiveBaudInit(int channelId, byte targetAddress, ref byte keyword1, ref byte keyword2);
        J2534Err FastInit(int channelId, PassThruMsg txMsg, ref PassThruMsg rxMsg);
        J2534Err ClearTxBuffer(int channelId);
        J2534Err ClearRxBuffer(int channelId);
        J2534Err ClearPeriodicMsgs(int channelId);
        J2534Err ClearMsgFilters(int channelId);
        J2534Err ClearFunctMsgLookupTable(int channelId);
        J2534Err AddToFunctMsgLookupTable(int channelId);
        J2534Err DeleteFromFunctMsgLookupTable(int channelId);
    }
}
