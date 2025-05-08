#include "SpriteSortingCommon.hlsli"

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
RWStructuredBuffer<RadixHistogram> localHistogram : register(u1);
RWStructuredBuffer<uint> globalOffset : register(u2);

groupshared uint localOffset[LocalThreadCount];
groupshared uint localMaskedDepth[LocalThreadCount];

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid: SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
    uint threadID = DTid.x;
    
    localOffset[groupThreadID] = 0;
    localMaskedDepth[groupThreadID] = 0;
    
    SpriteAliveIndex spriteAliveIndex = aliveIndexSet[threadID];
    uint maskedDepth = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);
    
    if (threadID < emitterTotalParticleCount)
    {
        localMaskedDepth[groupThreadID] = maskedDepth;
    }
    GroupMemoryBarrierWithGroupSync();
    
    if (threadID < emitterTotalParticleCount)
    {
        for (uint groupThreadIdx = 0; groupThreadIdx < groupThreadID; ++groupThreadIdx)
        {
            if (localMaskedDepth[groupThreadIdx] == maskedDepth)
            {
                localOffset[groupThreadID]++;
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    // 병목 구간 ===================
    if (threadID < emitterTotalParticleCount)
    {
        uint offset = 0;
        for (uint groupIdx = 0; groupIdx < groupID; ++groupIdx)
        {
            offset += localHistogram[groupIdx].histogram[maskedDepth];
        }
        globalOffset[threadID] = offset + localOffset[groupThreadID];
    }
}