#include "ParticleCommon.hlsli"
#include "FilterToneMappingCommon.hlsli"

cbuffer luminanceFilterProperty : register(b2)
{
	float minLogLum;
	float maxLogLum;
	uint2 lumDummy;
};

RWStructuredBuffer<uint> logLuminanceHistogram : register(u0);
RWStructuredBuffer<float> adaptedLuminance : register(u1);

const static uint groupSize = GroupTexWidth * GroupTexHeight;

groupshared uint luminanceHistogramShared[groupSize];

[numthreads(groupSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint threadID = DTid.x;
	uint logLuminanceCount = logLuminanceHistogram[threadID];
	luminanceHistogramShared[threadID] = logLuminanceCount * threadID;
	GroupMemoryBarrierWithGroupSync();

	logLuminanceHistogram[threadID] = 0;
	GroupMemoryBarrierWithGroupSync();

	[unroll]
	for (uint cutoff = (groupSize >> 1); cutoff > 0; cutoff >>= 1)
	{
		if (uint(threadID) < cutoff) 
		{
			luminanceHistogramShared[threadID] += luminanceHistogramShared[threadID + cutoff];
		}
		GroupMemoryBarrierWithGroupSync();
	}

	if (threadID == 0) 
	{
		float weightedLogAverage = (luminanceHistogramShared[0] / max(groupSize * groupSize - float(logLuminanceCount), 1.0)) - 1.0;
		float logLumRange = maxLogLum - minLogLum;
		float weightedAvgLum = exp2(((weightedLogAverage / (float(groupSize) - 1.f)) * logLumRange) + minLogLum);

		float lumLastFrame = adaptedLuminance[0];
		float timeCoeff = 1.0 - exp(-dt);
		float adaptedLum = lumLastFrame + (weightedAvgLum - lumLastFrame) * timeCoeff;

		adaptedLuminance[0] = adaptedLum;
	}
}