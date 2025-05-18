#include "SpriteSortingCommon.hlsli"

RWStructuredBuffer<PrefixSumDescriptor> localPrefixSumDescriptors : register(u0);

groupshared uint groupHistogram[LocalThreadCount];

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 GTid : SV_GroupThreadID )
{

}