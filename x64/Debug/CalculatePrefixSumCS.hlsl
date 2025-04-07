#include "ScanCommon.hlsli"

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	uint groupThreadID = GTid.x;
	uint threadID = DTid.x;
	LocalUpSweep(groupThreadID, threadID);
	LocalDownSweep(groupThreadID, threadID);
}


