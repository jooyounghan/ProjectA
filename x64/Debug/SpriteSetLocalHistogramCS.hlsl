#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);

RWStructuredBuffer<Histogram> localHistogram : register(u0);
RWStructuredBuffer<uint> localOffset : register(u1);

struct BitonicSortKey
{
    uint depth;
    uint index;
};

groupshared Histogram groupHistogram;
groupshared BitonicSortKey groupBitonicKeys[LocalThreadCount];

static void GetGroupHistogramPrefix(uint groupThreadIdx)
{
    [unroll]
    for (uint upSweepIdx = 1; upSweepIdx < LocalThreadCount; upSweepIdx <<= 1)
    {
        uint index = (groupThreadIdx + 1) * upSweepIdx * 2 - 1;
        if (index < LocalThreadCount)
        {
            groupHistogram.bin[index] += groupHistogram.bin[index - upSweepIdx];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupThreadIdx == 0)
    {
        groupHistogram.bin[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for (uint downSweep = LocalThreadCount >> 1; downSweep > 0; downSweep >>= 1)
    {
        uint index = (groupThreadIdx + 1) * downSweep * 2 - 1;
        if (index < LocalThreadCount)
        {
            uint t = groupHistogram.bin[index - downSweep];
            groupHistogram.bin[index - downSweep] = groupHistogram.bin[index];
            groupHistogram.bin[index] += t;
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

void SortGroupIndices(uint groupThreadIdx)
{
    [unroll]
    for (uint k = 2; k <= LocalThreadCount; k <<= 1)
    {
        [unroll]
        for (uint j = k >> 1; j > 0; j >>= 1)
        {
            uint ixj = groupThreadIdx ^ j;

            if (ixj > groupThreadIdx)
            {
                bool ascending = ((groupThreadIdx & k) == 0);

                BitonicSortKey bitnoicKeyA = groupBitonicKeys[groupThreadIdx];
                BitonicSortKey bitnoicKeyB = groupBitonicKeys[ixj];

                bool doSwap = ((bitnoicKeyA.depth > bitnoicKeyB.depth) == ascending);
                if (doSwap)
                {
                    groupBitonicKeys[groupThreadIdx] = bitnoicKeyB;
                    groupBitonicKeys[ixj] = bitnoicKeyA;
                }
            }
            GroupMemoryBarrierWithGroupSync();
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
	uint groupIdx = Gid.x;
    uint groupThreadIdx = GTid.x;
    uint threadIdx = DTid.x;    

    SpriteAliveIndex spriteAliveIndex = aliveIndices[threadIdx];
    uint radixDepth = (spriteAliveIndex.depth >> sortBitOffset) & (LocalThreadCount - 1);

    BitonicSortKey bitonicSortKey;
    bitonicSortKey.depth = spriteAliveIndex.depth << (24 - sortBitOffset);
    bitonicSortKey.index = groupThreadIdx;
	groupHistogram.bin[groupThreadIdx] = 0;
    groupBitonicKeys[groupThreadIdx] = bitonicSortKey;
    GroupMemoryBarrierWithGroupSync();

    uint isAlive = (threadIdx < aliveParticleCount);
    InterlockedAdd(groupHistogram.bin[radixDepth], isAlive);

    GroupMemoryBarrierWithGroupSync();

    localHistogram[groupIdx].bin[groupThreadIdx] = groupHistogram.bin[groupThreadIdx];
    GroupMemoryBarrierWithGroupSync();

    GetGroupHistogramPrefix(groupThreadIdx);
    SortGroupIndices(groupThreadIdx);

    if (threadIdx < aliveParticleCount)
    {   
        BitonicSortKey sortedBitonicSortKey = groupBitonicKeys[groupThreadIdx];
        uint sortedIndex = sortedBitonicSortKey.index;
        uint sortedThreadIdx = sortedIndex + LocalThreadCount * groupIdx;

        SpriteAliveIndex sortedAliveIndex = aliveIndices[sortedThreadIdx];
        uint sortedDepth = (sortedAliveIndex.depth >> sortBitOffset) & (LocalThreadCount - 1);

        localOffset[sortedThreadIdx] = groupThreadIdx - groupHistogram.bin[sortedDepth];
    }
}
