#include "PoolingCommon.hlsli"

StructuredBuffer<uint> aliveFlags : register(t0);

RWStructuredBuffer<uint> prefixSums : register(u2);
RWStructuredBuffer<PrefixDesciptor> prefixDescriptor : register(u3);

groupshared int localPrefixSums[LocalThreadCount];

void LocalUpSweep(uint groupID, uint groupThreadID, uint threadID)
{
    [unroll]
    for (uint stride = 1; stride < LocalThreadCount; stride *= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            localPrefixSums[index] += localPrefixSums[index - stride];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupThreadID == 0)
    {
        int aggregate = localPrefixSums[LocalThreadCount - 1];

        prefixDescriptor[groupID].aggregate = aggregate;
        uint Plast;
        InterlockedAdd(Pcurrent, aggregate, Plast);

        if (groupID == 0)
        {
            prefixDescriptor[groupID].inclusivePrefix = prefixDescriptor[groupID].aggregate;
            prefixDescriptor[groupID].statusFlag = 2;
        }
        else
        {
            prefixDescriptor[groupID].statusFlag = 1;
        }
    }
}

void DecoupledLookback(uint groupID, uint groupThreadID)
{
    if (groupThreadID == 0 && groupID > 0)
    {
        uint exclusivePrefix = 0;

        for (int lookbackID = (groupID - 1); lookbackID >= 0; --lookbackID)
        {
            uint currentStatus = 0;

            do
            {
                InterlockedCompareExchange(prefixDescriptor[lookbackID].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
            } while (currentStatus == 0);


            if (currentStatus == 1)
            {
                exclusivePrefix += prefixDescriptor[lookbackID].aggregate;
                continue;
            }
            else if (currentStatus == 2)
            {
                exclusivePrefix += prefixDescriptor[lookbackID].inclusivePrefix;
                break;
            }
        }

        prefixDescriptor[groupID].exclusivePrefix = exclusivePrefix;
        prefixDescriptor[groupID].inclusivePrefix = prefixDescriptor[groupID].aggregate + exclusivePrefix;
        InterlockedCompareStore(prefixDescriptor[groupID].statusFlag, 1, 2);
    }
}

void LocalDownSweep(uint groupID, uint groupThreadID, uint threadID)
{
    if (groupThreadID == (LocalThreadCount - 1))
    {
        localPrefixSums[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = LocalThreadCount / 2; stride > 0; stride /= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            uint t = localPrefixSums[index - stride];
            localPrefixSums[index - stride] = localPrefixSums[index];
            localPrefixSums[index] += t;
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupThreadID < (LocalThreadCount - 1))
    {
        prefixSums[threadID] = localPrefixSums[groupThreadID + 1] + prefixDescriptor[groupID].exclusivePrefix;
    }
    else
    {
        prefixSums[threadID] = prefixDescriptor[groupID].inclusivePrefix;
    }
}


[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupID = Gid.x;
	uint groupThreadID = GTid.x;
	uint threadID = DTid.x;

    localPrefixSums[groupThreadID] = aliveFlags[threadID];
    GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(groupID, groupThreadID, threadID);
    GroupMemoryBarrierWithGroupSync();

    DecoupledLookback(groupID, groupThreadID);
    GroupMemoryBarrierWithGroupSync();

    LocalDownSweep(groupID, groupThreadID, threadID);
}


