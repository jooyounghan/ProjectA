#include "ParticleCommon.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]
#define IndexedParticleThreadGroupXCount indexedParticleDispatchIndirectArgs[0]

RWStructuredBuffer<uint> particleDrawIndirectArgs: register(u0);
RWStructuredBuffer<uint> indexedParticleDispatchIndirectArgs: register(u1);