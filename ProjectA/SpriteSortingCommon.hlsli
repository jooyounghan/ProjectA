#include "ParticleCommon.hlsli"

#define RadixBinCount (1 << RadixBitCount)

static const uint countPerGroupThread = (RadixBinCount + LocalThreadCount - 1) / LocalThreadCount;

struct PrefixSumStatus
{
    uint aggregate;
    uint statusFlag; /* X : 0, A : 1, P : 2*/
    uint exclusivePrefix;
    uint inclusivePrefix;
};