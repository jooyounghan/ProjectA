#include "ParticleCommon.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]

StructuredBuffer<uint> particleDrawIndirectArgs : register(t0);
StructuredBuffer<uint> indicesBuffer : register(t1);

RWStructuredBuffer<uint> countsBuffer : register(u0);
RWStructuredBuffer<PrefixDesciptor> countsPrefixDescriptors : register(u1);
RWStructuredBuffer<uint> sortedIndicesBuffer : register(u2);

groupshared uint threadStatus[LocalThreadCount];
groupshared uint threadExclusive[LocalThreadCount];
groupshared uint threadInclusive[LocalThreadCount];
groupshared uint localCountsPrefixSums[LocalThreadCount * CountArraySizePerThread];

void LocalUpSweep(uint groupID, uint groupThreadID, uint threadID)
{
    //[unroll]
    //for (uint stride = 1; stride < LocalThreadCount; stride *= 2)
    //{
    //    uint index = (groupThreadID + 1) * stride * 2 - 1;
    //    if (index < LocalThreadCount)
    //    {
    //        localPrefixSums[index] += localPrefixSums[index - stride];
    //    }
    //    GroupMemoryBarrierWithGroupSync();
    //}

    //if (groupThreadID == 0)
    //{
    //    int aggregate = localPrefixSums[LocalThreadCount - 1];

    //    prefixDescriptor[groupID].aggregate = aggregate;
    //    uint Plast;
    //    InterlockedAdd(Pcurrent, aggregate, Plast);

    //    if (groupID == 0)
    //    {
    //        prefixDescriptor[groupID].inclusivePrefix = prefixDescriptor[groupID].aggregate;
    //        prefixDescriptor[groupID].statusFlag = 2;
    //    }
    //    else
    //    {
    //        prefixDescriptor[groupID].statusFlag = 1;
    //    }
    //}
}

// 영한 방법
void LocalUpSweep1(uint groupID, uint groupThreadID, uint countIndex, uint threadID)
{
    threadStatus[groupThreadID] = 0;
    threadExclusive[groupThreadID] = 0;
    threadInclusive[groupThreadID] = 0;

    [unroll]
    for (uint uncountedIndex = 0; uncountedIndex < countIndex; ++uncountedIndex)
    {
        localCountsPrefixSums[CountArraySizePerThread * groupThreadID + uncountedIndex] = 0;
    }
    [unroll]
    for (uint countedIndex = countIndex; countedIndex < CountArraySizePerThread; ++countedIndex)
    {
        localCountsPrefixSums[CountArraySizePerThread * groupThreadID + countedIndex] = 1;
    }

    if (groupThreadID == 0)
    {
        threadInclusive[groupThreadID] = 1;
        threadStatus[groupThreadID] = 2;
    }
    else
    {
        threadStatus[groupThreadID] = 1;
    }

    if (groupThreadID > 0)
    {
        for (int lookbackID = (groupThreadID - 1); lookbackID >= 0; --lookbackID)
        {
            uint currentStatus = 0;

            do
            {
                InterlockedCompareExchange(threadStatus[lookbackID], 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
            } while (currentStatus == 0);


            if (currentStatus == 1)
            {
                threadExclusive[groupThreadID] += 1;
                continue;
            }
            else if (currentStatus == 2)
            {
                threadExclusive[groupThreadID] += threadInclusive[lookbackID];
                break;
            }
        }
        threadInclusive[groupThreadID] = 1 + threadExclusive[groupThreadID];
        InterlockedCompareStore(threadStatus[groupThreadID], 1, 2);
    }

    [unroll]
    for (uint index = 0; index < CountArraySizePerThread; ++index)
    {
        localCountsPrefixSums[CountArraySizePerThread * groupThreadID + index] += (threadInclusive[groupThreadID] - 1);
        countsBuffer[CountArraySizePerThread * threadID + index] = localCountsPrefixSums[CountArraySizePerThread * groupThreadID + index];
    }

    if (groupThreadID == LocalThreadCount - 1)
   {
       int aggregate = localCountsPrefixSums[CountArraySizePerThread * LocalThreadCount - 1];

       countsPrefixDescriptors[groupID].aggregate = aggregate;
       if (groupID == 0)
       {
           countsPrefixDescriptors[groupID].inclusivePrefix = countsPrefixDescriptors[groupID].aggregate;
           countsPrefixDescriptors[groupID].statusFlag = 2;
       }
       else
       {
           countsPrefixDescriptors[groupID].statusFlag = 1;
       }
   }
}


void DecoupledLookback(uint groupID, uint groupThreadID)
{
    //if (groupThreadID == 0 && groupID > 0)
    //{
    //    uint exclusivePrefix = 0;

    //    for (int lookbackID = (groupID - 1); lookbackID >= 0; --lookbackID)
    //    {
    //        uint currentStatus = 0;

    //        do
    //        {
    //            InterlockedCompareExchange(prefixDescriptor[lookbackID].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
    //        } while (currentStatus == 0);


    //        if (currentStatus == 1)
    //        {
    //            exclusivePrefix += prefixDescriptor[lookbackID].aggregate;
    //            continue;
    //        }
    //        else if (currentStatus == 2)
    //        {
    //            exclusivePrefix += prefixDescriptor[lookbackID].inclusivePrefix;
    //            break;
    //        }
    //    }

    //    prefixDescriptor[groupID].exclusivePrefix = exclusivePrefix;
    //    prefixDescriptor[groupID].inclusivePrefix = prefixDescriptor[groupID].aggregate + exclusivePrefix;
    //    InterlockedCompareStore(prefixDescriptor[groupID].statusFlag, 1, 2);
    //}
}

void LocalDownSweep(uint groupID, uint groupThreadID, uint threadID)
{
    //if (groupThreadID == (LocalThreadCount - 1))
    //{
    //    localPrefixSums[LocalThreadCount - 1] = 0;
    //}
    //GroupMemoryBarrierWithGroupSync();

    //for (uint stride = LocalThreadCount / 2; stride > 0; stride /= 2)
    //{
    //    uint index = (groupThreadID + 1) * stride * 2 - 1;
    //    if (index < LocalThreadCount)
    //    {
    //        uint t = localPrefixSums[index - stride];
    //        localPrefixSums[index - stride] = localPrefixSums[index];
    //        localPrefixSums[index] += t;
    //    }
    //    GroupMemoryBarrierWithGroupSync();
    //}

    //if (groupThreadID < (LocalThreadCount - 1))
    //{
    //    prefixSums[threadID] = localPrefixSums[groupThreadID + 1] + prefixDescriptor[groupID].exclusivePrefix;
    //}
    //else
    //{
    //    prefixSums[threadID] = prefixDescriptor[groupID].inclusivePrefix;
    //}
}

void Reorder(uint groupID, uint groupThreadID, uint threadID)
{

}