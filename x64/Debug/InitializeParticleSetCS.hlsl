#include "ParticleCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#ifdef SPRITE_EMITTER
StructuredBuffer<SpriteEmitterInterpInform> emitterInterpInforms : register(t0);
#else
StructuredBuffer<ParticleEmitterInterpInform> emitterInterpInforms : register(t0);
#endif	

StructuredBuffer<D1Dim4Prop> d1Dim4Props : register(t1);
StructuredBuffer<D3Dim4Prop> d3Dim4Props : register(t2);
StructuredBuffer<D1Dim2Prop> d1Dim2Props : register(t3);
StructuredBuffer<D3Dim2Prop> d3Dim2Props : register(t4);
StructuredBuffer<D1Dim1Prop> d1Dim1Props : register(t5);
StructuredBuffer<D3Dim1Prop> d3Dim1Props : register(t6);

RWStructuredBuffer<Particle> totalParticles : register(u0);
AppendStructuredBuffer<uint> deathIndexSet : register(u1);

#ifdef SPRITE_EMITTER
AppendStructuredBuffer<SpriteAliveIndex> aliveIndexSet : register(u2);
#else
AppendStructuredBuffer<uint> aliveIndexSet : register(u2);
#endif

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint emitterType;
    uint2 emitterManagerPropertyDummy;
};

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
            // N차 이상 보간 일 경우, 색상에 대한 보간 ======================================================================
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

            // ==============================================================================================================

            // 가속도를 통한 적분 ===========================================================================================
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;
            // ==============================================================================================================
            
            #ifdef SPRITE_EMITTER
            float4 viewProjPos = mul(float4(currentParticle.worldPos, 1.f), viewProjMatrix);
            float depth = 1.f - (viewProjPos.z / viewProjPos.w);
            SpriteAliveIndex spriteAliveIndex;
            spriteAliveIndex.index = threadID;
            spriteAliveIndex.depth = asuint(depth);
            aliveIndexSet.Append(spriteAliveIndex);
            #else
            aliveIndexSet.Append(threadID);
            #endif
            
            totalParticles[threadID] = currentParticle;
        }		
    }
}
