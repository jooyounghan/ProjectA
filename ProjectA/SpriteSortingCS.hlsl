#include "SpriteSortingCommon.hlsli"

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint aliveParticleCount;
    uint sortBitOffset;
    uint emitterPropertyDummy;
};

cbuffer indirectStagingBuffer : register(b3)
{
    uint emitterTotalParticleCount;
    uint3 indirectStagingDummy;
};

RWStructuredBuffer<SpriteAliveIndex> aliveIndexSet : register(u0);
RWStructuredBuffer<SpriteAliveIndex> sortedAliveIndexSet : register(u1);
RWStructuredBuffer<RadixHistogram> localHistogram : register(u2);
RWStructuredBuffer<uint> globalHistogram : register(u3);

groupshared uint localOffset[LocalThreadCount];
groupshared uint localMaskedDepth[LocalThreadCount];

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 Gid: SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
    uint threadID = DTid.x;
    bool isValid = threadID < emitterTotalParticleCount;

    localOffset[groupThreadID] = 0;
    SpriteAliveIndex spriteAliveIndex = aliveIndexSet[threadID];
    uint maskedDepth = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);
    
    localMaskedDepth[groupThreadID] = isValid ? maskedDepth : 0;
    GroupMemoryBarrierWithGroupSync();

    if (isValid)
    {
        uint offset = 0;
        for (uint groupThreadIdx = 0; groupThreadIdx < groupThreadID; ++groupThreadIdx)
        {
            offset += (localMaskedDepth[groupThreadIdx] == maskedDepth) ? 1 : 0;
        }
        localOffset[groupThreadID] = offset;

        uint scatterIdx = globalHistogram[maskedDepth] + localHistogram[groupID].histogram[maskedDepth] + localOffset[groupThreadID];
        sortedAliveIndexSet[scatterIdx] = spriteAliveIndex;
    }
}