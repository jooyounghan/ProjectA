#include "ParticleCommon.hlsli"

cbuffer indirectStagingBuffer : register(b2)
{
    uint emitterTotalParticleCount;
    uint3 indirectStagingDummy;
};

RWStructuredBuffer<uint> indirectDispatchCountStager : register(u0);


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{  
    indirectDispatchCountStager[0] = uint(ceil(emitterTotalParticleCount / float(LocalThreadCount)));
    indirectDispatchCountStager[1] = 1;
    indirectDispatchCountStager[2] = 1;
    indirectDispatchCountStager[3] = 0;

}