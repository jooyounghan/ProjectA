#include "ParticleSimulateCommon.hlsli"

StructuredBuffer<uint> aliveFlags : register(t0);
RWStructuredBuffer<uint> prefixSums : register(u1);

#define LocalThreadCount 64
groupshared uint localPrefixSums[LocalThreadCount];

void LocalUpSweep(uint groupThreadID, uint threadID)
{
    localPrefixSums[groupThreadID] = aliveFlags[threadID];
    GroupMemoryBarrierWithGroupSync();

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
}

void LocalDownSweep(uint groupThreadID, uint threadID)
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

    prefixSums[threadID] = localPrefixSums[groupThreadID];
}
