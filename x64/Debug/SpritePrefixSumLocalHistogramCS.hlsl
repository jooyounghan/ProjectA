#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
RWStructuredBuffer<Histogram> localHistogram : register(u0);
RWStructuredBuffer<PrefixSumDescriptor> localPrefixSumDescriptors : register(u1);

groupshared uint groupHistogram[LocalThreadCount];

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

void LocalDownSweep(uint groupThreadID)
{
   if (groupThreadID == 0)
    {
        groupHistogram[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = LocalThreadCount >> 1; stride > 0; stride >>= 1)
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

static void DecoupledLookBack(uint workGroupIdx, uint workRadixIdx, uint groupRadixIdx, uint groupThreadIdx)
{

if (groupThreadIdx == 0)
    {
        uint aggregate = groupHistogram[LocalThreadCount - 1];
        bool isFirstGroup = (workGroupIdx == 0);

        PrefixSumDescriptor startDescriptor;
        startDescriptor.aggregate = aggregate;
        startDescriptor.exclusivePrefix = 0;        
        startDescriptor.inclusivePrefix = isFirstGroup ? aggregate : 0;
        startDescriptor.statusFlag = isFirstGroup ? 2 : 1;
        localPrefixSumDescriptors[groupRadixIdx] = startDescriptor;
        DeviceMemoryBarrier();

        if (workGroupIdx > 0)
        {
            uint exclusivePrefix = 0;
            bool isInclusiveChecked = false;
    
            for (int loopBackID = (workGroupIdx - 1); loopBackID >= 0 && !isInclusiveChecked; --loopBackID)
            {
                uint loopBackRadixIdx = workRadixIdx + LocalThreadCount * loopBackID;
                uint currentStatus = 0;

                do
                {
                    InterlockedCompareExchange(localPrefixSumDescriptors[loopBackRadixIdx].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
                }  
                while (currentStatus == 0);

                uint lookBackAggregate  = localPrefixSumDescriptors[loopBackRadixIdx].aggregate;
                uint lookBackInclusive  = localPrefixSumDescriptors[loopBackRadixIdx].inclusivePrefix;
                isInclusiveChecked = (currentStatus == 2);
                exclusivePrefix += isInclusiveChecked ? lookBackInclusive : lookBackAggregate;
            }

            PrefixSumDescriptor finishDescriptor;
            finishDescriptor.aggregate = localPrefixSumDescriptors[groupRadixIdx].aggregate;
            finishDescriptor.exclusivePrefix = exclusivePrefix;        
            finishDescriptor.inclusivePrefix = finishDescriptor.aggregate + exclusivePrefix;
            finishDescriptor.statusFlag = 2;
            localPrefixSumDescriptors[groupRadixIdx] = finishDescriptor;
            DeviceMemoryBarrier();
        }
    }
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

    LocalUpSweep(groupThreadIdx);
    DecoupledLookBack(workGroupIdx, workRadixIdx, groupRadixIdx, groupThreadIdx);
    LocalDownSweep(groupThreadIdx);
    localHistogram[groupIdx].bin[workRadixIdx] = groupHistogram[groupThreadIdx];
}
