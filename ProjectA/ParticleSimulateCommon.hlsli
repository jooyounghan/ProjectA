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