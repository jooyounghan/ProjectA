#include "ParticleCommon.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]

StructuredBuffer<uint> particleDrawIndirectArgs: register(t0);
StructuredBuffer<uint> currentIndices : register(t1);

RWStructuredBuffer<Particle> particles : register(u0);