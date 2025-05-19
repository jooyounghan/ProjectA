#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
RWStructuredBuffer<uint> localHistogram : register(u0);
RWStructuredBuffer<PrefixSumDescriptor> localPrefixSumDescriptors : register(u1);

groupshared uint groupHistogram[LocalThreadCount];
groupshared uint exclusivePrefix;

static void LocalUpSweep(uint groupThreadIdx)
{
    [unroll]
    for (uint uIdx = 1; uIdx < LocalThreadCount; uIdx <<= 1)
    {
        uint index = (groupThreadIdx + 1) * uIdx * 2 - 1;
        if (index < LocalThreadCount)
        {
            groupHistogram[index] += groupHistogram[index - uIdx];
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

void LocalDownSweep(uint groupThreadIdx)
{
    if (groupThreadIdx == 0)
    {
        groupHistogram[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = LocalThreadCount >> 1; stride > 0; stride >>= 1)
    {
        uint index = (groupThreadIdx + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            uint t = groupHistogram[index - stride];
            groupHistogram[index - stride] = groupHistogram[index];
            groupHistogram[index] += t;
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

static void DecoupledLookBack(uint groupIdx, uint radixIdx, uint prefixDescriptorIdx, uint groupCount, uint groupThreadIdx)
{
    if (groupThreadIdx == 0)
    {
        exclusivePrefix = 0;
        uint aggregate = groupHistogram[LocalThreadCount - 1];
        bool isFirstGroup = (groupIdx == 0);

        PrefixSumDescriptor startDescriptor;
        startDescriptor.aggregate = aggregate;
        startDescriptor.exclusivePrefix = 0;
        startDescriptor.inclusivePrefix = isFirstGroup ? aggregate : 0;
        startDescriptor.statusFlag = isFirstGroup ? 2 : 1;
        localPrefixSumDescriptors[prefixDescriptorIdx] = startDescriptor;
        DeviceMemoryBarrier();

        if (groupIdx > 0)
        {
            bool isInclusiveChecked = false;
            uint spinLockCount = 0;
            for (int loopBackID = (groupIdx - 1); loopBackID >= 0 && !isInclusiveChecked; --loopBackID)
            {
                uint loopBackRadixIdx = groupCount * radixIdx + loopBackID;
                uint currentStatus = 0;

                do
                {
                    InterlockedCompareExchange(localPrefixSumDescriptors[loopBackRadixIdx].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
                }
                while (currentStatus == 0 && ++spinLockCount < 1000);

                uint lookBackAggregate = localPrefixSumDescriptors[loopBackRadixIdx].aggregate;
                uint lookBackInclusive = localPrefixSumDescriptors[loopBackRadixIdx].inclusivePrefix;
                isInclusiveChecked = (currentStatus == 2);
                exclusivePrefix += isInclusiveChecked ? lookBackInclusive : lookBackAggregate;
            }

            PrefixSumDescriptor finishDescriptor;
            finishDescriptor.aggregate = aggregate;
            finishDescriptor.exclusivePrefix = exclusivePrefix;
            finishDescriptor.inclusivePrefix = aggregate + exclusivePrefix;
            finishDescriptor.statusFlag = 2;
            localPrefixSumDescriptors[prefixDescriptorIdx] = finishDescriptor;
            DeviceMemoryBarrier();
        }
    }
    GroupMemoryBarrierWithGroupSync();
}


[numthreads(LocalThreadCount, 1, 1)]
void main(
	uint3 Gid : SV_GroupID,
	uint3 GTid : SV_GroupThreadID,
    uint3 DTid : SV_DispatchThreadID
)
{
    uint groupIdx = Gid.x;
    uint radixIdx = Gid.y;
    uint groupThreadIdx = GTid.x;
    uint threadXIdx = DTid.x;
    
    float invLocalThreadCount = 1 / FLocalThreadCount;
    uint groupCount = uint(ceil(aliveParticleCount * invLocalThreadCount * invLocalThreadCount));
    uint groupElementIdx = groupIdx * LocalThreadCount + groupThreadIdx;
    uint prefixDescriptorIdx = groupCount * radixIdx + groupIdx;
    
    uint workIdx = radixIdx * (groupCount * LocalThreadCount) + threadXIdx;
    groupHistogram[groupThreadIdx] = localHistogram[workIdx];
    GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(groupThreadIdx);
    DecoupledLookBack(groupIdx, radixIdx, prefixDescriptorIdx, groupCount, groupThreadIdx);
    LocalDownSweep(groupThreadIdx);

    localHistogram[workIdx] = groupHistogram[groupThreadIdx] + exclusivePrefix;
}
