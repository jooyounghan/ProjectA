#include "PoolingCommon.hlsli"

StructuredBuffer<uint> aliveFlags : register(t0);
StructuredBuffer<uint> prefixSums : register(t1);
RWStructuredBuffer<Particle> totalParticles : register(u2);
RWStructuredBuffer<ParticleSelector> currnetIndices : register(u3);

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint groupID = Gid.x;
	uint groupThreadID = GTid.x;
	uint threadID = DTid.x;

    if (threadID == 0)
	{
		IndexedParticleThreadGroupXCount = ceil(Pcurrent / float(LocalThreadCount));
    }

    if (threadID < particleMaxCount)
	{
        if (aliveFlags[threadID])
		{
            uint index = prefixSums[threadID] - 1;
            ParticleSelector particleSelector;
            particleSelector.index = threadID;
            particleSelector.dummy = 0;

            Particle currentParticle = totalParticles[threadID];
            float4 viewPos = mul(float4(currentParticle.worldPos, 1.f), viewProjMatrix);
            viewPos.xyz /= viewPos.w;

            particleSelector.emitterType = currentParticle.emitterID;
            particleSelector.depth = viewPos.z;
                        
            totalParticles[threadID].worldPos = viewPos.xyz;
            currnetIndices[index] = particleSelector;
        }
	}
}