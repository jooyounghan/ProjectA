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
RWStructuredBuffer<uint> globalHistogram : register(u1);

groupshared uint localHistogram[RadixBinCount];

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
    uint threadID = DTid.x;    
    
    [unroll]
    for (uint localIdx = 0; localIdx < countPerGroupThread; ++localIdx)
    {
        uint currentRadixIdx = groupThreadID + LocalThreadCount * localIdx;
        if (currentRadixIdx < RadixBinCount)
        {
            localHistogram[currentRadixIdx] = 0;
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    if (threadID < emitterTotalParticleCount)
    {
        SpriteAliveIndex spriteAliveIndex = aliveIndexSet[threadID];
        uint depthRadix = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);
        InterlockedAdd(localHistogram[depthRadix], 1);
    }
    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for (uint globalIdx = 0; globalIdx < countPerGroupThread; ++globalIdx)
    {
        uint currentRadixIdx = groupThreadID + LocalThreadCount * globalIdx;
        if (currentRadixIdx < RadixBinCount)
        {
            InterlockedAdd(globalHistogram[currentRadixIdx], localHistogram[currentRadixIdx]);
        }
    }
    GroupMemoryBarrierWithGroupSync();
}
