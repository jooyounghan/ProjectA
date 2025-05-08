#include "ParticleCommon.hlsli"

static const uint RadixCountPerGroupThread = (RadixBinCount + LocalThreadCount - 1) / LocalThreadCount;

struct PrefixSumStatus
{
    uint aggregate;
    uint statusFlag; /* X : 0, A : 1, P : 2*/
    uint exclusivePrefix;
    uint inclusivePrefix;
};

struct RadixHistogram
{
    uint histogram[RadixBinCount];
};