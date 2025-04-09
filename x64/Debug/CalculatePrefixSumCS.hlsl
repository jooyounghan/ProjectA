#include "ScanCommon.hlsli"


[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupID = Gid.x;
	uint groupThreadID = GTid.x;
	uint threadID = DTid.x;
	
    InitializePartitionDescriptor(groupID, groupThreadID);
    DeviceMemoryBarrier();

    LocalUpSweep(groupID, groupThreadID, threadID);
    GroupMemoryBarrierWithGroupSync();

    DecoupledLookback(groupID, groupThreadID);
    GroupMemoryBarrierWithGroupSync();

    LocalDownSweep(groupID, groupThreadID, threadID);
    GroupMemoryBarrierWithGroupSync();
}


