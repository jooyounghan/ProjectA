#include "SpriteSortingCommon.hlsli"

StructuredBuffer<SpriteAliveIndex> aliveIndices : register(t0);
RWStructuredBuffer<Histogram> localHistogram : register(u0);

groupshared Histogram groupHistogram;

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

    groupHistogram.bin[groupThreadIdx] = 0;
    GroupMemoryBarrierWithGroupSync();

    SpriteAliveIndex spriteAliveIndex = aliveIndices[threadIdx];
    uint radixDepth = (spriteAliveIndex.depth >> sortBitOffset) & (LocalThreadCount - 1);
    uint isAlive = (threadIdx < aliveParticleCount);
    InterlockedAdd(groupHistogram.bin[radixDepth], isAlive);
    GroupMemoryBarrierWithGroupSync();

    if (groupThreadIdx == 0) localHistogram[groupIdx] = groupHistogram;
}
