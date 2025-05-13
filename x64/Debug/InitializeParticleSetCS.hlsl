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

bool CheckIntersection(
    Texture2D<float> depthTexture, 
    float3 prevUVWithDepth, 
    float3 currentUVWithDepth, 
    out float3 intersectUVWithDepth
)
{
    const int steps = 10;

    float invSteps = 1.f / (float)steps;
    float3 prevLerpedUVWithDepth = prevUVWithDepth;
    intersectUVWithDepth = float3(0.f, 0.f, 0.f);
    
    for (int i = 1; i <= steps; ++i)
    {
        float currentStep = i * invSteps;
        float3 currentLerpedUVWithDepth = lerp(prevUVWithDepth, currentUVWithDepth, currentStep);
        
        float2 prevUV = prevLerpedUVWithDepth.xy;
        float prevDepth = prevLerpedUVWithDepth.z;
        float2 currentUV = currentLerpedUVWithDepth.xy;
        float currentDepth = currentLerpedUVWithDepth.z;
        
        if (any(prevUV < 0 || prevUV > 1) || any(currentUV < 0 || currentUV > 1))
            continue;

        float prevRefDepth = depthTexture.SampleLevel(clampSampler, prevUV, 0);
        float currentRefDepth = depthTexture.SampleLevel(clampSampler, currentUV, 0);
        
        if (prevDepth <= prevRefDepth && currentDepth > currentRefDepth)
        {
            intersectUVWithDepth = prevLerpedUVWithDepth;
            return true;
        }
        prevLerpedUVWithDepth = currentLerpedUVWithDepth;
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
            currentParticle.velocity += currentParticle.accelerate * dt;
            float4 prevWorldPos = float4(currentParticle.worldPos, 1.f);
            float4 currentVelocity = float4(currentParticle.velocity, 0.f);
            float4 currentPos = prevWorldPos + currentVelocity * dt;
            
            currentParticle.worldPos = currentPos.xyz;
            // ==============================================================================================================

            // Depth 기반 충돌 처리 =========================================================================================
            float4 clippedPrevPos = mul(prevWorldPos, viewProjMatrix);
            float4 currentClippedPos = mul(currentPos, viewProjMatrix);
            float3 ndcPrevPos = clippedPrevPos / clippedPrevPos.w;
            float3 ndcCurrentPos = currentClippedPos / currentClippedPos.w;

            float2 prevPosUV = 0.5f * (ndcPrevPos.xy * float2(1, -1) + 1.f);
            float2 currentPosUV = 0.5f * (ndcCurrentPos.xy * float2(1, -1) + 1.f);
            
            float3 prevUVWithDepth = float3(prevPosUV, ndcPrevPos.z);
            float3 currentUVWithDepth = float3(currentPosUV, ndcCurrentPos.z);

            float3 intersectUVWithDepth = currentUVWithDepth;
            if (CheckIntersection(depthView, prevUVWithDepth, currentUVWithDepth, intersectUVWithDepth))
            {
                float2 intersectUV = intersectUVWithDepth.xy;
                float3 normalVector = normalView.SampleLevel(clampSampler, intersectUV, 0).xyz;
                
                float2 ndcIntersectXY = (2.f * intersectUV - 1.f) * float2(1, -1);
                float4 clippedIntersectPos = float4(ndcIntersectXY, intersectUVWithDepth.z, 1.f);
                float4 worldIntersectPos = mul(clippedIntersectPos, invViewProjMatrix);
                worldIntersectPos /= worldIntersectPos.w;
                currentParticle.worldPos = worldIntersectPos;
                currentParticle.velocity = 0.5f * reflect(currentParticle.velocity, normalVector);
            }
            // ==============================================================================================================
            
            #ifdef SPRITE_EMITTER
            float depth = 1.f - intersectUVWithDepth.z;
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
