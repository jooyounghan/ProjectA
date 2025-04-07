#include "ParticleCommon.hlsli"

#define Pmax particleCounts[0]
#define Pcurrent particleCounts[1]

RWStructuredBuffer<uint> particleCounts : register(u0);
/*
	uint Pmax
	uint Pcurrent
	uint
	uint
*/

float rand(float2 seed)
{
	return ((frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453)) - 0.5f);
}