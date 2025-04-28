#include "ParticleCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#ifdef SPRITE_EMITTER
StructuredBuffer<SpriteEmitterInterpInform> particleEmitterInterpInforms : register(t0);
#else
StructuredBuffer<ParticleEmitterInterpInform> particleEmitterInterpInforms : register(t0);
#endif	

StructuredBuffer<D1Dim4Prop> d1Dim4Props : register(t1);
StructuredBuffer<D3Dim4Prop> d3Dim4Props : register(t2);
StructuredBuffer<D1Dim2Prop> d1Dim2Props : register(t3);
StructuredBuffer<D3Dim2Prop> d3Dim2Props : register(t4);

RWStructuredBuffer<Particle> totalParticles : register(u0);
AppendStructuredBuffer<uint> deathIndexSet : register(u1);
AppendStructuredBuffer<uint> aliveIndexSet : register(u2);

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint emitterType;
    uint2 emitterManagerPropertyDummy;
};

float2 GetInterpolatedDim2(uint degree, uint interpolatedID, float2 timeSpent2, float maxLife)
{
    const uint CubicSplineMethod = 1;
    const uint CatmullRomMethod = 3;
    const float timeSpent = timeSpent2.x;

    if (degree == 1)
    {
        D1Dim2Prop interpProp = d1Dim2Props[interpolatedID];
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                return EvaluateD1Dim2(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD1Dim2(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 3)
    {
        D3Dim2Prop interpProp = d3Dim2Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolateMethod;
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

                return EvaluateD3Dim2(t, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD3Dim2(0.f, interpProp.coefficient[stepsCount]);
    }

    return float2(0.f, 0.f);
}

float4 GetInterpolatedDim4(uint degree, uint interpolatedID, float4 timeSpent4, float maxLife)
{
    const uint CubicSplineMethod = 1;
    const uint CatmullRomMethod = 3;
    const float timeSpent = timeSpent4.x;

    if (degree == 1)
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
                return EvaluateD1Dim4(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD1Dim4(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 3)
    {
        D3Dim4Prop interpProp = d3Dim4Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolateMethod;
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

                return EvaluateD3Dim4(t, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD3Dim4(0.f, interpProp.coefficient[stepsCount]);
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

            #ifdef SPRITE_EMITTER
            const SpriteEmitterInterpInform interpInform = particleEmitterInterpInforms[emitterID];
            #else
            const ParticleEmitterInterpInform interpInform = particleEmitterInterpInforms[emitterID];
            #endif	
            
            const float maxLife = interpInform.maxLife;

            const uint colorInterpolaterNotSelected = ~0;
            const uint colorInterpolaterID = interpInform.colorInterpolaterID;
            const uint colorDegree = interpInform.colorInterpolaterDegree;
            const float timeSpent = maxLife - currentParticle.life;

            if (colorInterpolaterID != colorInterpolaterNotSelected)
            {
                currentParticle.color = GetInterpolatedDim4(colorDegree, colorInterpolaterID, float4(timeSpent, timeSpent, timeSpent, timeSpent), maxLife);
            }

            #ifdef SPRITE_EMITTER
            const uint spriteSizeInterpolaterID = interpInform.spriteSizeInterpolaterID;
            const uint spriteSizeDegree = interpInform.spriteSizeInterpolaterDegree;

            if (spriteSizeInterpolaterID != colorInterpolaterNotSelected)
            {
                currentParticle.xyScale = GetInterpolatedDim2(spriteSizeDegree, spriteSizeInterpolaterID, float2(timeSpent, timeSpent), maxLife);
            }
            #endif	

            // ==============================================================================================================

            // 가속도를 통한 적분 ===========================================================================================
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;
            // ==============================================================================================================

            totalParticles[threadID] = currentParticle;
        }		
    }
}
