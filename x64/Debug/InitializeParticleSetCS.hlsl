#include "ParticleCommon.hlsli"
#include "InterpolaterHelper.hlsli"

StructuredBuffer<ParticleEmitterInterpInform> particleEmitterInterpInforms : register(t0);
StructuredBuffer<D1Dim4Prop> d1Dim4Props : register(t1);
StructuredBuffer<D3Dim4Prop> d3Dim4Props : register(t2);

RWStructuredBuffer<Particle> totalParticles : register(u0);
AppendStructuredBuffer<uint> deathIndexSet : register(u1);
AppendStructuredBuffer<uint> aliveIndexSet : register(u2);


float4 GetInterpolated(uint degree, uint interpolatedID, float4 timeSpent4, float maxLife)
{
    const uint CubicSplineMethod = 2;
    const uint CatmullRomMethod = 3;
    const float timeSpent = timeSpent4.x;

    if (degree == 2)
    {
        D1Dim4Prop interpProp = d1Dim4Props[interpolatedID];
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                return Evaluate1Degree(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return Evaluate1Degree(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 4)
    {
        D3Dim4Prop interpProp = d3Dim4Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolaterFlag;
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                float t = (interpolateMethod == CubicSplineMethod)
                    ? timeSpent - x1
                    : (timeSpent - x1) / (x2 - x1);

                return Evaluate3Degree(t, interpProp.coefficient[stepIdx]);
            }
        }

        return Evaluate3Degree(0.f, interpProp.coefficient[stepsCount]);
    }

    return float4(0.f, 0.f, 0.f, 1.f);
}

[numthreads(LocalThreadCount, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint groupID = Gid.x;
    uint groupThreadID = GTid.x;
	uint threadID = DTid.x;

	if (threadID < particleMaxCount)
    {
		Particle currentParticle = totalParticles[threadID];
		
        currentParticle.life -= dt;
		if (currentParticle.life < 1E-3f)
		{
            deathIndexSet.Append(threadID);
        }
		else
		{
            aliveIndexSet.Append(threadID);
            
            // N차 이상 보간 일 경우, 색상에 대한 보간 ======================================================================
            const uint emitterID = currentParticle.emitterID;
            const ParticleEmitterInterpInform interpInform = particleEmitterInterpInforms[emitterID];
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

            // 가속도를 통한 적분 ===========================================================================================
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;
            // ==============================================================================================================

            totalParticles[threadID] = currentParticle;
        }		
    }
}
