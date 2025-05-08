#include "SpriteSortingCommon.hlsli"

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint sortBitOffset;
    uint2 emitterPropertyDummy;
};

cbuffer indirectStagingBuffer : register(b3)
{
    uint emitterTotalParticleCount;
    uint3 indirectStagingDummy;
};

RWStructuredBuffer<SpriteAliveIndex> aliveIndexSet : register(u0);
RWStructuredBuffer<RadixHistogram> localHistogram : register(u1);
RWStructuredBuffer<PrefixSumStatus> localPrefixSumStatus : register(u2);

groupshared uint groupHistogram[LocalThreadCount];

void LocalUpSweep(uint groupScanID, uint statusID, uint groupThreadID)
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

        localPrefixSumStatus[statusID].aggregate = aggregate;

        if (groupScanID == 0)
        {
            localPrefixSumStatus[statusID].inclusivePrefix = aggregate;
        }
        localPrefixSumStatus[statusID].statusFlag = (groupScanID == 0) ?  2 : 1;
    }
}

void DecoupledLookBack(uint groupScanID, uint radixID, uint statusID, uint groupThreadID)
{
    if (groupThreadID == 0 && groupScanID > 0)
    {
        uint exclusivePrefix = 0;
        bool loopExit = false;

        for (int lookbackID = (groupScanID - 1); lookbackID >= 0 && !loopExit; --lookbackID)
        {
            uint lookBackStatusID = lookbackID * RadixBinCount + radixID;
            uint currentStatus = 0;

            do
            {
                InterlockedCompareExchange(localPrefixSumStatus[lookBackStatusID].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
            } while (currentStatus == 0);

            uint aggregate  = localPrefixSumStatus[lookBackStatusID].aggregate;
            uint inclusive = localPrefixSumStatus[lookBackStatusID].inclusivePrefix;

           bool isPartial = (currentStatus == 1);
            exclusivePrefix += isPartial ? aggregate : inclusive;

            loopExit = !isPartial;
        }

        localPrefixSumStatus[statusID].exclusivePrefix = exclusivePrefix;
        localPrefixSumStatus[statusID].inclusivePrefix = localPrefixSumStatus[statusID].aggregate + exclusivePrefix;
        InterlockedCompareStore(localPrefixSumStatus[statusID].statusFlag, 1, 2);
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
void main(uint3 Gid: SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
    uint groupScanID = Gid.x;
    uint radixID = Gid.y;
    uint groupThreadID = GTid.x;

    uint groupID = groupScanID * LocalThreadCount + groupThreadID;
    uint statusID = groupScanID * RadixBinCount + radixID;

    groupHistogram[groupThreadID] = 0;
    groupHistogram[groupThreadID] = localHistogram[groupID].histogram[radixID];

    GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(groupScanID, statusID, groupThreadID);
    DecoupledLookBack(groupScanID, radixID, statusID, groupThreadID);
    LocalDownSweep(groupThreadID);

    localHistogram[groupID].histogram[radixID] = groupHistogram[groupThreadID] + localPrefixSumStatus[statusID].exclusivePrefix;
}