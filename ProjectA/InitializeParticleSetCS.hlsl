#include "ParticleCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#ifdef SPRITE_EMITTER
StructuredBuffer<SpriteEmitterInterpInform> emitterInterpInforms : register(t0);
#else
StructuredBuffer<ParticleEmitterInterpInform> emitterInterpInforms : register(t0);
#endif	

Texture2D<float> depthView : register(t1);
Texture2D<float4> normalView : register(t2);

StructuredBuffer<D1Dim4Prop> d1Dim4Props : register(t3);
StructuredBuffer<D3Dim4Prop> d3Dim4Props : register(t4);

#ifdef SPRITE_EMITTER
StructuredBuffer<D1Dim2Prop> d1Dim2Props : register(t5);
StructuredBuffer<D3Dim2Prop> d3Dim2Props : register(t6);
StructuredBuffer<D1Dim1Prop> d1Dim1Props : register(t7);
StructuredBuffer<D3Dim1Prop> d3Dim1Props : register(t8);
#endif	


RWStructuredBuffer<Particle> totalParticles : register(u0);
AppendStructuredBuffer<uint> deathIndexSet : register(u1);

#ifdef SPRITE_EMITTER
AppendStructuredBuffer<SpriteAliveIndex> aliveIndexSet : register(u2);
#else
AppendStructuredBuffer<uint> aliveIndexSet : register(u2);
#endif

SamplerState clampSampler : register(s0);

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint emitterType;
    uint2 emitterManagerPropertyDummy;
};

bool CheckIntersection(Texture2D<float> depthTexture, float3 p1, float3 p2)
{
    const int steps = 10;
    for (int i = 0; i <= steps; ++i)
    {
        float t = i / (float)steps;
        float3 p = lerp(p1, p2, t);
        float2 uv = p.xy;

        if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
            continue;

        float depthAtUV = depthTexture.SampleLevel(clampSampler, uv, 0);
        if (p.z > depthAtUV)
            return true;
    }
    return false;
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
            const uint emitterID = currentParticle.emitterID;

            #ifdef SPRITE_EMITTER
            const SpriteEmitterInterpInform interpInform = emitterInterpInforms[emitterID];
            #else
            const ParticleEmitterInterpInform interpInform = emitterInterpInforms[emitterID];
            #endif	
            
            const float maxLife = interpInform.maxLife;

            const uint colorInterpolaterNotSelected = ~0;
            const uint colorInterpolaterID = interpInform.colorInterpolaterID;
            const uint colorDegree = interpInform.colorInterpolaterDegree;
            const float timeSpent = maxLife - currentParticle.life;

            if (colorInterpolaterID != colorInterpolaterNotSelected)
            {
                currentParticle.color = GetInterpolatedDim4(
                    colorDegree, colorInterpolaterID, 
                    float4(timeSpent, timeSpent, timeSpent, timeSpent), maxLife,
                    d1Dim4Props, d3Dim4Props
                );
            }

            #ifdef SPRITE_EMITTER
            const uint spriteSizeInterpolaterID = interpInform.spriteSizeInterpolaterID;
            const uint spriteSizeDegree = interpInform.spriteSizeInterpolaterDegree;
            const uint spriteIndexInterpolaterID = interpInform.spriteIndexInterpolaterID;
            const uint spriteIndexDegree = interpInform.spriteIndexInterpolaterDegree;

            if (spriteSizeInterpolaterID != colorInterpolaterNotSelected)
            {
                currentParticle.xyScale = GetInterpolatedDim2(
                    spriteSizeDegree, spriteSizeInterpolaterID, 
                    float2(timeSpent, timeSpent), maxLife,
                    d1Dim2Props, d3Dim2Props
                );
            }

            if (spriteIndexInterpolaterID != colorInterpolaterNotSelected)
            {
                currentParticle.spriteIndex = GetInterpolatedDim1(
                    spriteIndexDegree, spriteIndexInterpolaterID, 
                    timeSpent, maxLife,
                    d1Dim1Props, d3Dim1Props
                );
            }
            #endif	

            // 가속도를 통한 적분 ===========================================================================================
            float3 prevWorldPos = currentParticle.worldPos;
            currentParticle.velocity += currentParticle.accelerate * dt;
            float3 currentPos = prevWorldPos + currentParticle.velocity * dt; 
            currentParticle.worldPos = currentPos;
            // ==============================================================================================================

            // Depth 기반 충돌 처리 =========================================================================================
            float4 prevViewProjPos = mul(float4(prevWorldPos, 1.f), viewProjMatrix);
            float prevWInv = 1.f / prevViewProjPos.w;
            float3 ndcPrevViewProjPos = prevViewProjPos.xyz * prevWInv;
            ndcPrevViewProjPos.x = (ndcPrevViewProjPos.x + 1.f) / 2.f;
            ndcPrevViewProjPos.y = (-ndcPrevViewProjPos.y + 1.f) / 2.f;

            float4 currentViewProjPos = mul(float4(currentPos, 1.f), viewProjMatrix);
            float currentWInv = 1.f / currentViewProjPos.w;
            float3 ndcCurrentViewProjPos = currentViewProjPos.xyz * currentWInv;
            ndcCurrentViewProjPos.x = (ndcCurrentViewProjPos.x + 1.f) / 2.f;
            ndcCurrentViewProjPos.y = (-ndcCurrentViewProjPos.y + 1.f) / 2.f;

            // out 확인 후 위치 조정해줘서 조금 더 정확도 향상
            if (CheckIntersection(depthView, ndcPrevViewProjPos, ndcCurrentViewProjPos))
            {
                currentParticle.worldPos = prevWorldPos;
                float3 normalVector = -normalView.SampleLevel(clampSampler, ndcPrevViewProjPos.xy, 0).xyz;
                currentParticle.velocity = -0.5f * reflect(currentParticle.velocity, normalVector);
            }
            // ==============================================================================================================
            
            #ifdef SPRITE_EMITTER
            float depth = 1.f - ndcCurrentViewProjPos.z;
            SpriteAliveIndex spriteAliveIndex;
            spriteAliveIndex.index = threadID;
            spriteAliveIndex.depth = FloatToSortableUint(depth);
            aliveIndexSet.Append(spriteAliveIndex);
            #else
            aliveIndexSet.Append(threadID);
            #endif
            
            totalParticles[threadID] = currentParticle;
        }		
    }
}
