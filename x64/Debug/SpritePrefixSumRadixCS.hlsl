#include "SpriteSortingCommon.hlsli"

RWStructuredBuffer<PrefixSumStatus> prefixSumStatus : register(u0);
RWStructuredBuffer<uint> globalHistogram : register(u1);

groupshared uint localHistogram[LocalThreadCount];

void LocalUpSweep(uint groupID, uint groupThreadID)
{
    [unroll]
    for (uint stride = 1; stride < LocalThreadCount; stride *= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            localHistogram[index] += localHistogram[index - stride];
        }
        GroupMemoryBarrierWithGroupSync();
    }


    if (groupThreadID == 0)
    {
        int aggregate = localHistogram[LocalThreadCount - 1];

        prefixSumStatus[groupID].aggregate = aggregate;

        if (groupID == 0)
        {
            prefixSumStatus[groupID].inclusivePrefix = prefixSumStatus[groupID].aggregate;
            prefixSumStatus[groupID].statusFlag = 2;
        }
        else
        {
            prefixSumStatus[groupID].statusFlag = 1;
        }
    }

}

void DecoupledLookBack(uint groupID, uint groupThreadID)
{
    if (groupThreadID == 0 && groupID > 0)
    {
        uint exclusivePrefix = 0;

        for (int lookbackID = (groupID - 1); lookbackID >= 0; --lookbackID)
        {
            uint currentStatus = 0;

            do
            {
                InterlockedCompareExchange(prefixSumStatus[lookbackID].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
            } while (currentStatus == 0);


            if (currentStatus == 1)
            {
                exclusivePrefix += prefixSumStatus[lookbackID].aggregate;
                continue;
            }
            else if (currentStatus == 2)
            {
                exclusivePrefix += prefixSumStatus[lookbackID].inclusivePrefix;
                break;
            }
        }

        prefixSumStatus[groupID].exclusivePrefix = exclusivePrefix;
        prefixSumStatus[groupID].inclusivePrefix = prefixSumStatus[groupID].aggregate + exclusivePrefix;
        InterlockedCompareStore(prefixSumStatus[groupID].statusFlag, 1, 2);
    }
}

void LocalDownSweep(uint groupID, uint groupThreadID, uint threadID)
{
   if (groupThreadID == 0)
    {
        localHistogram[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = LocalThreadCount / 2; stride > 0; stride /= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            uint t = localHistogram[index - stride];
            localHistogram[index - stride] = localHistogram[index];
            localHistogram[index] += t;
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
    if (isValid)
    {
        localHistogram[groupThreadID] = globalHistogram[threadID];
    }
    else
    {
        localHistogram[groupThreadID] = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(groupID, groupThreadID);
    DecoupledLookBack(groupID, groupThreadID);
    LocalDownSweep(groupID, groupThreadID, threadID);
    
// 여기 수정 필요
    if (isValid)
    {
        globalHistogram[threadID] = localHistogram[groupThreadID] + prefixSumStatus[groupID].exclusivePrefix;
    }
}