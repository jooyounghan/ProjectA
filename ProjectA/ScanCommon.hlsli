#include "ParticleSimulateCommon.hlsli"

struct PartitionDescriptor
{
    int     aggregate;
    uint    statusFlag; /* X : 0, A : 1, P : 2*/
    int     exclusivePrefix;
    int     inclusivePrefix;
};

StructuredBuffer<uint> aliveFlags : register(t0);
RWStructuredBuffer<uint> prefixSums : register(u1);
RWStructuredBuffer<PartitionDescriptor> partitionDescriptor : register(u2);

groupshared int localPrefixSums[LocalThreadCount];

void InitializePartitionDescriptor(uint groupID, uint groupThreadID)
{
    if (groupThreadID == 0)
    {
        PartitionDescriptor pd = partitionDescriptor[groupID];
        pd.aggregate = 0;
        pd.statusFlag = 0;
        pd.exclusivePrefix = 0;
        pd.inclusivePrefix = 0;
        partitionDescriptor[groupID] = pd;
    }
}

void LocalUpSweep(uint groupID, uint groupThreadID, uint threadID)
{
    localPrefixSums[groupThreadID] = aliveFlags[threadID];
    GroupMemoryBarrierWithGroupSync();

	[unroll]
	for (uint stride = 1; stride < LocalThreadCount; stride *= 2)
	{
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            localPrefixSums[index] += localPrefixSums[index - stride];
        }
        GroupMemoryBarrierWithGroupSync();
	}
    
    if (groupThreadID == 0)
    {
        partitionDescriptor[groupID].aggregate = localPrefixSums[LocalThreadCount - 1];
        if (groupID == 0)
        {
            partitionDescriptor[groupID].inclusivePrefix = partitionDescriptor[groupID].aggregate;
            partitionDescriptor[groupID].statusFlag = 2;
        }
        else
        {
            partitionDescriptor[groupID].statusFlag = 1;            
        }
    }
}

void DecoupledLookback(uint groupID, uint groupThreadID)
{
    if (groupThreadID == 0 && groupID > 0)
    {
        uint exclusivePrefix = 0;
        
        for (int lookbackID = (groupID - 1); lookbackID >= 0; --lookbackID)
        {
            uint currentStatus = 0;
            
            do
            {                
                InterlockedCompareExchange(partitionDescriptor[lookbackID].statusFlag, 0xFFFFFFFF, 0xFFFFFFFF, currentStatus);
            }
            while (currentStatus == 0);


            if (currentStatus == 1)
            {
                exclusivePrefix += partitionDescriptor[lookbackID].aggregate;
                continue;
            }
            else if (currentStatus == 2)
            {
                exclusivePrefix += partitionDescriptor[lookbackID].inclusivePrefix;
                break;
            }
        }
        
        partitionDescriptor[groupID].exclusivePrefix = exclusivePrefix;
        partitionDescriptor[groupID].inclusivePrefix = partitionDescriptor[groupID].aggregate + exclusivePrefix;
        InterlockedCompareStore(partitionDescriptor[groupID].statusFlag, 1, 2);
    }
}

void LocalDownSweep(uint groupID, uint groupThreadID, uint threadID)
{
    if (groupThreadID == (LocalThreadCount - 1))
    {
        localPrefixSums[LocalThreadCount - 1] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = LocalThreadCount / 2; stride > 0; stride /= 2)
    {
        uint index = (groupThreadID + 1) * stride * 2 - 1;
        if (index < LocalThreadCount)
        {
            uint t = localPrefixSums[index - stride];
            localPrefixSums[index - stride] = localPrefixSums[index];
            localPrefixSums[index] += t;
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupThreadID < (LocalThreadCount - 1))
    {
        prefixSums[threadID] = localPrefixSums[groupThreadID + 1] + partitionDescriptor[groupID].exclusivePrefix;
    }
    else
    {
        prefixSums[threadID] = partitionDescriptor[groupID].inclusivePrefix;       
    }
}
