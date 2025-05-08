#include "SpriteSortingCommon.hlsli"

RWStructuredBuffer<uint> globalHistogram : register(u0);
RWStructuredBuffer<PrefixSumStatus> globalPrefixSumStatus : register(u1);

groupshared uint groupHistogram[LocalThreadCount];

void LocalUpSweep(uint groupID, uint groupThreadID)
{
    [unroll]
    for (uint stride = 1; stride < LocalThreadCount; stride *= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            groupHistogram[index] += groupHistogram[index - stride];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupThreadID == 0)
    {
        int aggregate = groupHistogram[LocalThreadCount - 1];

        globalPrefixSumStatus[groupID].aggregate = aggregate;

        if (groupID == 0)
        {
            globalPrefixSumStatus[groupID].inclusivePrefix = globalPrefixSumStatus[groupID].aggregate;
        }
        globalPrefixSumStatus[groupID].statusFlag = (groupID == 0) ?  2 : 1;
    }
}

void DecoupledLookBack(uint groupID, uint groupThreadID)
{
    if (groupThreadID == 0 && groupID > 0)
    {
        uint exclusivePrefix = 0;
        bool loopExit = false;

        for (int lookbackID = (groupID - 1); lookbackID >= 0 && !loopExit; --lookbackID)
        {
            uint currentStatus = 0;

            do
            {
                InterlockedCompareExchange(globalPrefixSumStatus[lookbackID].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
            } while (currentStatus == 0);

            uint aggregate  = globalPrefixSumStatus[lookbackID].aggregate;
            uint inclusive = globalPrefixSumStatus[lookbackID].inclusivePrefix;

           bool isPartial = (currentStatus == 1);
            exclusivePrefix += isPartial ? aggregate : inclusive;

            loopExit = !isPartial;
        }

        globalPrefixSumStatus[groupID].exclusivePrefix = exclusivePrefix;
        globalPrefixSumStatus[groupID].inclusivePrefix = globalPrefixSumStatus[groupID].aggregate + exclusivePrefix;
        InterlockedCompareStore(globalPrefixSumStatus[groupID].statusFlag, 1, 2);
    }
}

void LocalDownSweep(uint groupThreadID)
{
   if (groupThreadID == 0)
    {
        groupHistogram[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = LocalThreadCount / 2; stride > 0; stride /= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            uint t = groupHistogram[index - stride];
            groupHistogram[index - stride] = groupHistogram[index];
            groupHistogram[index] += t;
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID,  uint3 DTid : SV_DispatchThreadID )
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
    uint threadID = DTid.x;    

    bool isValid = threadID < RadixBinCount;
    
     groupHistogram[groupThreadID] = isValid ? globalHistogram[threadID] : 0;

    GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(groupID, groupThreadID);
    DecoupledLookBack(groupID, groupThreadID);
    LocalDownSweep(groupThreadID);
    
    if (isValid)
    {
        globalHistogram[threadID] = groupHistogram[groupThreadID] + globalPrefixSumStatus[groupID].exclusivePrefix;
    }
}