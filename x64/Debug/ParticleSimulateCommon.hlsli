#include "ParticleCommon.hlsli"

#define Pcurrent particleCounts[0]

RWStructuredBuffer<uint> particleCounts : register(u0);

float rand(float2 seed)
{
	return ((frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453)) - 0.5f);
}