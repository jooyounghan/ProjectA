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

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint totalPixelCount = groupSize * groupSize;
	float weightedSum = 0.0;

	int averageCount = int(totalPixelCount * 0.05);
	int remaining = averageCount;
	
// 상위 5%에 대한 평균 구하기
// 1, 2, 3, 2, 1, 2, 2, 3, 2, 3 중 상위 60%(6개)에 대한 평균 구하기 예제 생각

	for (int i = groupSize - 1; i >= 0 && remaining > 0; --i)
	{
		uint binCount = logLuminanceHistogram[i];
		uint used = min(remaining, binCount);
		weightedSum += float(used) * float(i);
		remaining -= used;
	}

	float weightedAverage = weightedSum / float(averageCount);
	float logLumRange = maxLogLum - minLogLum;
	float weightedAvgLum = exp2(((weightedAverage / (float(groupSize) - 1.0)) * logLumRange) + minLogLum);

	float lumLastFrame = adaptedLuminance[0];
	float timeCoeff = 1.0 - exp(-dt);
	float adaptedLum = lumLastFrame + (weightedAvgLum - lumLastFrame) * timeCoeff;
	adaptedLuminance[0] = adaptedLum;
}