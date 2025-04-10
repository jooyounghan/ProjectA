#include "ParticleCommon.hlsli"

#define Pcurrent particleCounts[0]
#define InstanceCount particleCounts[1]

RWStructuredBuffer<uint> particleCounts : register(u0);