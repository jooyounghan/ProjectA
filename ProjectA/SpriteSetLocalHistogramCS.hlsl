#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
RWStructuredBuffer<uint> localHistogram : register(u0);

groupshared uint groupHistogram[LocalThreadCount];

[numthreads(LocalThreadCount, 1, 1)]
void main(
	uint3 Gid : SV_GroupID, 
	uint3 GTid : SV_GroupThreadID, 
	uint3 DTid : SV_DispatchThreadID
)
{
    uint groupIdx = Gid.x;
    uint radixIdx = GTid.x;
    uint threadIdx = DTid.x;    

    float invLocalThreadCount = 1 / FLocalThreadCount;
    uint groupCount = uint(ceil(aliveParticleCount * invLocalThreadCount));
    uint groupRadixIdx = radixIdx * groupCount + groupIdx;

    groupHistogram[radixIdx] = 0;
    GroupMemoryBarrierWithGroupSync();

    SpriteAliveIndex spriteAliveIndex = aliveIndices[threadIdx];
    uint radixDepth = (spriteAliveIndex.depth >> sortBitOffset) & (LocalThreadCount - 1);
    uint isAlive = (threadIdx < aliveParticleCount);
    InterlockedAdd(groupHistogram[radixDepth], isAlive);
    GroupMemoryBarrierWithGroupSync();

    localHistogram[groupRadixIdx] = groupHistogram[radixIdx];
}
