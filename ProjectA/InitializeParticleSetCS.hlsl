#include "ParticleCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]

RWStructuredBuffer<uint> particleDrawIndirectArgs: register(u0);
RWStructuredBuffer<Particle> totalParticles : register(u1);
RWStructuredBuffer<uint> aliveFlags : register(u2);
AppendStructuredBuffer<uint> deathParticleSet : register(u3);
RWStructuredBuffer<PrefixDesciptor> prefixDescriptor : register(u4);

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
	uint threadID = DTid.x;

    // Initialize��, Prefix�� ���� �ʱ�ȭ �׸� ���� �ʱ�ȭ ����
    if(groupThreadID == 0)
    {
        if (threadID == 0)
        {
            Pcurrent = 0;
        }
        PrefixDesciptor pd = prefixDescriptor[groupID];
        pd.aggregate = 0;
        pd.statusFlag = 0;
        pd.exclusivePrefix = 0;
        pd.inclusivePrefix = 0;
        prefixDescriptor[groupID] = pd;
    }


	if (threadID < particleMaxCount)
    {
		Particle currentParticle = totalParticles[threadID];
		
        currentParticle.life -= dt;
		if (currentParticle.life < 1E-3f)
		{
            deathParticleSet.Append(threadID);
            aliveFlags[threadID] = 0;
        }
		else
		{
            aliveFlags[threadID] = 1;
            
            // N�� �̻� ���� �� ���, ���� ���� ���� ======================================================================
            const uint emitterID = currentParticle.emitterID;
            const EmitterInterpInform interpInform = emitterInterpInforms[emitterID];
            const float maxLife = interpInform.maxLife;

            const uint colorInterpolaterNotSelected = ~0;
            const uint colorInterpolaterID = interpInform.colorInterpolaterID;
            const uint colorDegree = interpInform.colorInterpolaterDegree;
            const float timeSpent = maxLife - currentParticle.life;

            if (colorInterpolaterID != colorInterpolaterNotSelected)
            {
                currentParticle.color = GetInterpolated(colorDegree, colorInterpolaterID, float4(timeSpent, timeSpent, timeSpent, timeSpent), maxLife);
            }
            // ==============================================================================================================

            // ���ӵ��� ���� ���� ===========================================================================================
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;
            // ==============================================================================================================

            totalParticles[threadID] = currentParticle;
        }		
    }
}
