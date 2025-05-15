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
    indirectDispatchCountStager[0] = uint(ceil(emitterTotalParticleCount / localThreadCount));
    indirectDispatchCountStager[1] = 1;
    indirectDispatchCountStager[2] = 1;
    indirectDispatchCountStager[3] = 0;

}