#include "ParticleSimulateCommon.hlsli"

StructuredBuffer<uint> aliveFlags : register(t0);
StructuredBuffer<uint> prefixSums : register(t1);
RWStructuredBuffer<uint> currnetIndices : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint groupID = Gid.x;
	uint groupThreadID = GTid.x;
	uint index = DTid.x;

	if (groupID == 0 && groupThreadID == 0)
	{
		Pcurrent = 0;
	}
	DeviceMemoryBarrier();

	if (index < particleMaxCount)
	{
		if (aliveFlags[index])
		{
			currnetIndices[prefixSums[index] - 1] = index;
			uint Plast;
			InterlockedAdd(Pcurrent, 1, Plast);
		}
	}
}