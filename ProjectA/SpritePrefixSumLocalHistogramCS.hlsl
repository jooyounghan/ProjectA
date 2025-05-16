#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
RWStructuredBuffer<Histogram> localHistogram : register(u0);
RWStructuredBuffer<PrefixSumDescriptor> localPrefixSumDescriptors : register(u1);

groupshared uint groupHistogram[LocalThreadCount];

static void LocalUpSweep(uint workGroupIdx, uint groupRadixIdx, uint groupThreadIdx)
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

    if (groupThreadIdx == 0)
    {
        int aggregate = groupHistogram[LocalThreadCount - 1];
        bool isFirstGroup = (workGroupIdx == 0);

        localPrefixSumDescriptors[groupRadixIdx].aggregate = aggregate;
        localPrefixSumDescriptors[groupRadixIdx].exclusivePrefix = 0;        
        localPrefixSumDescriptors[groupRadixIdx].inclusivePrefix = isFirstGroup ? aggregate : 0;
        AllMemoryBarrier();
        InterlockedCompareStore(localPrefixSumDescriptors[groupRadixIdx].statusFlag, 0, isFirstGroup ? 2 : 1);
    }
}

static void DecoupledLookBack(uint workGroupIdx, uint workRadixIdx, uint groupRadixIdx, uint groupThreadIdx)
{
    uint exclusivePrefix = 0;
    
    for (int loopBackID = (workGroupIdx - 1); loopBackID >= 0; --loopBackID)
    {
        uint loopBackRadixIdx = workRadixIdx + LocalThreadCount * loopBackID;
        uint currentStatus = 0;
        uint spinCount = 0;

        while (currentStatus == 0 && spinCount++ < 10000);
        {
            InterlockedCompareExchange(localPrefixSumDescriptors[loopBackRadixIdx].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
        }  
        AllMemoryBarrier();
        uint aggregate  = localPrefixSumDescriptors[loopBackRadixIdx].aggregate;
        uint inclusive  = localPrefixSumDescriptors[loopBackRadixIdx].inclusivePrefix;

        bool isInclusiveChecked = (currentStatus == 2);
        exclusivePrefix += isInclusiveChecked ? inclusive : aggregate;

        if (isInclusiveChecked) break;
    }

    localPrefixSumDescriptors[groupRadixIdx].exclusivePrefix = exclusivePrefix;
    localPrefixSumDescriptors[groupRadixIdx].inclusivePrefix = localPrefixSumDescriptors[groupRadixIdx].aggregate + exclusivePrefix;
    AllMemoryBarrier();
    InterlockedCompareStore(localPrefixSumDescriptors[groupRadixIdx].statusFlag, 1, 2);
}

[numthreads(LocalThreadCount, 1, 1)]
void main(
	uint3 Gid : SV_GroupID,
	uint3 GTid : SV_GroupThreadID,
	uint3 DTid : SV_DispatchThreadID
)
{
	uint workGroupIdx = Gid.x;
    uint workRadixIdx = Gid.y;

	uint groupThreadIdx = GTid.x;
    uint groupIdx = groupThreadIdx + LocalThreadCount * workGroupIdx;
    uint groupRadixIdx = workRadixIdx + LocalThreadCount * workGroupIdx;

	uint threadIdx = DTid.x;

	groupHistogram[groupThreadIdx] = localHistogram[groupIdx].bin[workRadixIdx];
	GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(workGroupIdx, groupRadixIdx, groupThreadIdx);
    if (groupThreadIdx == 0 && workGroupIdx > 0)
    {
        DecoupledLookBack(workGroupIdx, workRadixIdx, groupRadixIdx, groupThreadIdx);
    }
}
