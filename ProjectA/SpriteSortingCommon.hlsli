#include "ParticleCommon.hlsli"

struct PrefixSumDescriptor
{
    uint aggregate;
    uint exclusivePrefix;
    uint inclusivePrefix;
    uint statusFlag; /* X : 0, A : 1, P : 2*/
};

struct Histogram
{
    uint bin[1 << RadixBitCount];
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
