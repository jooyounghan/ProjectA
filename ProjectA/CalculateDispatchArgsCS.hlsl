#include "ParticleCommon.hlsli"

cbuffer EmitterManagerProperties : register(b2)
{
	uint particleMaxCount;
	uint aliveParticleCount;
	uint2 emitterPropertyDummy;
};

struct DispatchIndirectArgs
{
	uint threadGroupCountX;
	uint threadGroupCountY;
	uint threadGroupCountZ;
	uint dummy;
};

RWStructuredBuffer<DispatchIndirectArgs> dispatchArgsStager : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{  
    DispatchIndirectArgs dispatchArgs;

#ifdef DISPATCH_RADIX_SORT
    dispatchArgs.threadGroupCountX = uint(ceil(aliveParticleCount * invLocalThreadCount * invLocalThreadCount ));
	dispatchArgs.threadGroupCountY = (1 << RadixBitCount);
#else
    dispatchArgs.threadGroupCountX = uint(ceil(aliveParticleCount * invLocalThreadCount));
    dispatchArgs.threadGroupCountY = 1;
#endif
    dispatchArgs.threadGroupCountZ = 1;
    dispatchArgs.dummy = 0;

    dispatchArgsStager[0] = dispatchArgs;
}