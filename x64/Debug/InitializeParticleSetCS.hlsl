#include "ParticleCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]

StructuredBuffer<Degree1Dim4InterpolaterProperty> firstDegreeInterpolater : register(t0);
StructuredBuffer<Degree3Dim4InterpolaterProperty> thirdDegreeInterpolater : register(t1);

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

    // Initialize시, Prefix와 같은 초기화 항목 또한 초기화 수행
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
            
            // 색상에 대한 보간
            uint colorInterpolaterDegree = currentParticle.colorInterpolaterDegree;
            uint colorInterpolaterID = currentParticle.colorInterpolaterID;

            float timeSpent = currentParticle.maxLife - currentParticle.life;

            if (colorInterpolaterDegree == 2)
            {
                // Linear
                Degree1Dim4InterpolaterProperty interpolaterProperty = firstDegreeInterpolater[colorInterpolaterID];
                currentParticle.color = float4(timeSpent, 0.f, 0.f, 1.f);
            }
            else if (colorInterpolaterDegree == 4)
            {
                Degree3Dim4InterpolaterProperty interpolaterProperty = thirdDegreeInterpolater[colorInterpolaterID];

                if (interpolaterProperty.header.interpolaterFlag == 2)
                {
                    // Catmull-Rom
                    currentParticle.color = float4(0.f, timeSpent, 0.f, 1.f);
                }
                else if (interpolaterProperty.header.interpolaterFlag == 3)
                {
                    // Cubic
                    currentParticle.color = float4(0.f, 0.f, timeSpent, 1.f);
                }
            }


            // 가속도를 통한 적분
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;

            //if (currentParticle.worldPos.y < 0.f)
            //{
            //    currentParticle.worldPos.y = 1E-3;
            //    currentParticle.velocity.x = currentParticle.velocity.x * 0.2f;
            //    currentParticle.velocity.y = currentParticle.velocity.y * -0.2f;
            //    currentParticle.velocity.z = currentParticle.velocity.z * 0.2f;
            //}

            totalParticles[threadID] = currentParticle;
        }		
    }
}
