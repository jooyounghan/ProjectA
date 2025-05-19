#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
StructuredBuffer<uint> localHistogram : register(t1);
StructuredBuffer<uint> globalHistogram : register(t2);

RWStructuredBuffer<SpriteAliveIndex> sortedAliveIndices : register(u0);

groupshared uint localDepth[LocalThreadCount];

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupIdx = Gid.x;
    uint groupThreadIdx = GTid.x;
    uint threadIdx = DTid.x;
    
    float invLocalThreadCount = 1 / FLocalThreadCount;
    uint maxGroupCount = uint(ceil(particleMaxCount * invLocalThreadCount));
    
    SpriteAliveIndex spriteAliveIndex = aliveIndices[threadIdx];
    uint radix = (spriteAliveIndex.depth >> sortBitOffset) & (RadixBinCount - 1);
    uint groupRadixIdx = radix * maxGroupCount + groupIdx;

    bool isValid = threadIdx < aliveParticleCount;
    localDepth[groupThreadIdx] = radix;
    GroupMemoryBarrierWithGroupSync();

    if (!isValid) return;
    
    uint offset = 0;
    for (uint index = 0; index < groupThreadIdx; ++index)
    {
        offset += (localDepth[index] == radix) ? 1 : 0;
    }

    uint scatterIdx = globalHistogram[radix] + localHistogram[groupRadixIdx] + offset;
    sortedAliveIndices[scatterIdx] = spriteAliveIndex;
}