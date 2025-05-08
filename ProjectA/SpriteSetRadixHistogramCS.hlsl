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
RWStructuredBuffer<RadixHistogram> localHistogram : register(u1);
RWStructuredBuffer<uint> globalHistogram : register(u2);

groupshared RadixHistogram groupHistogram;

[numthreads(LocalThreadCount, RadixCountPerGroupThread, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
    uint radixIdx = GTid.y;
    uint threadID = DTid.x;    
    uint currentRadixIdx = groupThreadID + LocalThreadCount * radixIdx;
    
    groupHistogram.histogram[currentRadixIdx] = 0;
    GroupMemoryBarrierWithGroupSync();
    
    SpriteAliveIndex spriteAliveIndex = aliveIndexSet[threadID];
    uint maskedDepth = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);
    
    if (threadID < emitterTotalParticleCount)
    {   
        InterlockedAdd(groupHistogram.histogram[maskedDepth], 1);
    }
    GroupMemoryBarrierWithGroupSync();

    if (currentRadixIdx < RadixBinCount)
    {
        localHistogram[groupID].histogram[currentRadixIdx] = groupHistogram.histogram[currentRadixIdx];
        InterlockedAdd(globalHistogram[currentRadixIdx], groupHistogram.histogram[currentRadixIdx]);
    }
}
