#include "SpriteSortingCommon.hlsli"

StructuredBuffer<PrefixSumDescriptor> localPrefixSumDescriptors : register(t0);
RWStructuredBuffer<uint> globalHistogram : register(u0);

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

static void LocalDownSweep(uint groupThreadIdx)
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

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 GTid : SV_GroupThreadID )
{
    uint groupThreadIdx = GTid.x;    
    float invLocalThreadCount = 1 / FLocalThreadCount;
    uint groupCount = uint(ceil(aliveParticleCount * invLocalThreadCount * invLocalThreadCount));
    uint prefixDescriptorIndex = groupCount * groupThreadIdx + (groupCount - 1);
    groupHistogram[groupThreadIdx] = localPrefixSumDescriptors[prefixDescriptorIndex].inclusivePrefix;
    GroupMemoryBarrierWithGroupSync();

    LocalUpSweep(groupThreadIdx);
    LocalDownSweep(groupThreadIdx);
    
    globalHistogram[groupThreadIdx] = groupHistogram[groupThreadIdx];
}