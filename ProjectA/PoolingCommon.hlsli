#include "ParticleCommon.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]
#define SimulateThreadGroupXCount particleSimulateDispatchIndirectArgs[0]

RWStructuredBuffer<uint> particleDrawIndirectArgs: register(u0);
RWStructuredBuffer<uint> particleSimulateDispatchIndirectArgs: register(u1);