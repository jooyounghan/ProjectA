#include "SortingCommon.hlsli"

cbuffer SortStatus : register(b2)
{
	uint passCount;
	uint bitOffset;
	uint2 sortStatusDummy;
};

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	uint groupID = Gid.x;
	uint groupThreadID = GTid.x;
	uint threadID = DTid.x;

	uint offset = passCount * bitOffset;
	
	if (threadID < Pcurrent)
	{
		uint countIndex = (indicesBuffer[threadID].depthInverseBit >> offset) & (CountArraySizePerThread - 1);

		LocalUpSweep1(groupID, groupThreadID, countIndex, threadID);
		GroupMemoryBarrierWithGroupSync();

		DecoupledLookback(groupID, groupThreadID);
		GroupMemoryBarrierWithGroupSync();

		LocalDownSweep(groupID, groupThreadID, threadID);
		GroupMemoryBarrierWithGroupSync();

		Reorder(groupID, groupThreadID, threadID);
		// LocalUpSweep
		// Decoupled-Lookback
		// LocalDownSweep
		// Reorder
	}
}