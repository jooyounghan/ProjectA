#include "ParticleCommon.hlsli"

cbuffer indirectStagingBuffer : register(b2)
{
    uint emitterTotalParticleCount;
    uint3 indirectStagingDummy;
};

RWStructuredBuffer<uint> indirectDispatchCountStager : register(u0);

static float localThreadCount = float(LocalThreadCount);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{  
#ifdef RADIX_INDIRECT

    indirectDispatchCountStager[0] = uint(ceil(ceil(emitterTotalParticleCount / localThreadCount) / localThreadCount));
    indirectDispatchCountStager[1] = RadixBinCount;
#else
    indirectDispatchCountStager[0] = uint(ceil(emitterTotalParticleCount / localThreadCount));
    indirectDispatchCountStager[1] = 1;
#endif
    indirectDispatchCountStager[2] = 1;
    indirectDispatchCountStager[3] = 0;

}