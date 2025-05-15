#include "ParticleCommon.hlsli"

struct PrefixSumStatus
{
    uint aggregate;
    uint statusFlag; /* X : 0, A : 1, P : 2*/
    uint exclusivePrefix;
    uint inclusivePrefix;
};

struct Histogram
{
    uint bin[LocalThreadCount];
};

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint aliveParticleCount;
    uint emitterPropertyDummy1;
    uint emitterPropertyDummy2;
};

cbuffer RadixSortProperties : register(b3)
{
    uint sortBitOffset;
    uint3 radixSortPropertyDummy;
};

cbuffer indirectStagingBuffer : register(b4)
{
    uint emitterTotalParticleCount;
    uint3 indirectStagingDummy;
};

