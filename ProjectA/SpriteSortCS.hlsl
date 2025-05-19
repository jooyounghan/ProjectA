#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
StructuredBuffer<uint> localHistogram : register(t1);
StructuredBuffer<uint> globalHistogram : register(t2);

RWStructuredBuffer<SpriteAliveIndex> sortedAliveIndices : register(u0);

groupshared uint localOffset[LocalThreadCount];
groupshared int localDepth[LocalThreadCount];

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupIdx = Gid.x;
    uint groupThreadIdx = GTid.x;
    uint threadIdx = DTid.x;
    
    float invLocalThreadCount = 1 / FLocalThreadCount;
    uint groupCount = uint(ceil(aliveParticleCount * invLocalThreadCount));
    
    SpriteAliveIndex spriteAliveIndex = aliveIndices[threadIdx];
    int radix = int((spriteAliveIndex.depth >> sortBitOffset) & (LocalThreadCount - 1));
    uint groupRadixIdx = radix * groupCount + groupIdx;

    bool isValid = threadIdx < aliveParticleCount;

    localOffset[groupThreadIdx] = 0;
    localDepth[groupThreadIdx] = isValid ? radix : -1;
    GroupMemoryBarrierWithGroupSync();

    uint offset = 0;
    for (uint index = 0; index < groupThreadIdx; ++index)
    {
        offset += (localDepth[index] == radix) ? 1 : 0;
    }
    localOffset[groupThreadIdx] = offset;

    uint scatterIdx = globalHistogram[radix] + localHistogram[groupRadixIdx] + localOffset[groupThreadIdx];
    sortedAliveIndices[scatterIdx] = spriteAliveIndex;
}