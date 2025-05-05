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
RWStructuredBuffer<SpriteAliveIndex> sortedAliveIndexSet : register(u1);
RWStructuredBuffer<uint> globalHistogram : register(u2);
RWStructuredBuffer<uint> globalOffset : register(u3);

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint threadID = DTid.x;    

    if (threadID < emitterTotalParticleCount)
    {
        SpriteAliveIndex spriteAliveIndex = aliveIndexSet[threadID];
        uint maskedDepth = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);

        uint scatterIdx = globalHistogram[maskedDepth] + globalOffset[threadID];
        sortedAliveIndexSet[scatterIdx] = spriteAliveIndex;
    }
}