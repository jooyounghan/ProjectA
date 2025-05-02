#include "ParticleCommon.hlsli"

#define RadixBinCount (1 << RadixBitCount)

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
RWStructuredBuffer<SpriteAliveIndex> sortedAliveIndexSet : register(u1);
RWStructuredBuffer<PrefixSumStatus> prefixSumStatus : register(u2);
RWStructuredBuffer<uint> globalHistogram : register(u3);

groupshared uint localHistogram[RadixBinCount];

static const uint countPerGroupThread = (RadixBinCount + LocalThreadCount - 1) / LocalThreadCount;

void LocalUpSweep(uint groupID, uint groupThreadID)
{
    for (uint stride = 1; stride < RadixBinCount; stride *= 2)
    {
        for (uint idx = 0; idx < countPerGroupThread; ++idx)
        {
            uint currentRadixIdx = groupThreadID + LocalThreadCount * idx;
            uint radixIdx = (currentRadixIdx + 1) * stride * 2 - 1;
            if (radixIdx < RadixBinCount)
            {
                localHistogram[radixIdx] += localHistogram[radixIdx - stride];
            }
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupThreadID == 0)
    {
        int aggregate = localHistogram[RadixBinCount - 1];

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

void LocalDownSweep(uint groupID, uint groupThreadID)
{
    if (groupThreadID == 0)
    {
        localHistogram[RadixBinCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = RadixBinCount / 2; stride > 0; stride /= 2)
    {
        for (uint idx = 0; idx < countPerGroupThread; ++idx)
        {
            uint currentRadixIdx = groupThreadID + LocalThreadCount * idx;
            uint radixIdx = (currentRadixIdx + 1) * stride * 2 - 1;

            if (radixIdx < RadixBinCount)
            {
                uint left = radixIdx - stride;
                if (left < RadixBinCount)
                {
                    uint t = localHistogram[left];
                    localHistogram[left] = localHistogram[radixIdx];
                    localHistogram[radixIdx] += t;
                }
            }
        }
        GroupMemoryBarrierWithGroupSync();
    }

    for (uint idx = 0; idx < countPerGroupThread; ++idx)
    {
        uint currentRadixIdx = groupThreadID + LocalThreadCount * idx;
        if (currentRadixIdx < (RadixBinCount - 1))
        {
            localHistogram[currentRadixIdx] = localHistogram[currentRadixIdx] + prefixSumStatus[groupID].exclusivePrefix;
        }
        else
        {
            localHistogram[currentRadixIdx] = prefixSumStatus[groupID].inclusivePrefix;
        }
    }
}

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
    uint threadID = DTid.x;    
    
    [unroll]
    for (uint idx = 0; idx < countPerGroupThread; ++idx)
    {
        uint currentRadixIdx = groupThreadID + LocalThreadCount * idx;
        if (currentRadixIdx < RadixBinCount)
        {
            localHistogram[currentRadixIdx] = 0;
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    // Set Histogram
    uint depthRadix = 0;
    SpriteAliveIndex spriteAliveIndex;
    bool isValid = (threadID < emitterTotalParticleCount);
    if (isValid)
    {
        spriteAliveIndex = aliveIndexSet[threadID];
        depthRadix = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);
        InterlockedAdd(localHistogram[depthRadix], 1);
    }
    GroupMemoryBarrierWithGroupSync();
    
    // Prefix-Sum With Decoupled Lookback
    LocalUpSweep(groupID, groupThreadID);
    DecoupledLookBack(groupID, groupThreadID);
    LocalDownSweep(groupID, groupThreadID);

    if (isValid)
    {
    for (uint idx = 0; idx < countPerGroupThread; ++idx)
    {
        uint currentRadixIdx = groupThreadID + LocalThreadCount * idx;
        if (currentRadixIdx < RadixBinCount)
        {
            globalHistogram[RadixBinCount * groupID + currentRadixIdx] = localHistogram[currentRadixIdx];
        }
    }

        uint scatterIdx;
        InterlockedAdd(localHistogram[depthRadix], 1, scatterIdx);
        sortedAliveIndexSet[scatterIdx] = spriteAliveIndex;
    }
}
