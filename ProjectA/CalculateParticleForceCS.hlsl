#include "CaculateForceCommon.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]

StructuredBuffer<uint> particleDrawIndirectArgs: register(t0);
StructuredBuffer<uint> currentIndices : register(t1);
StructuredBuffer<ForceProperty> emitterForces : register(t2);

RWStructuredBuffer<Particle> particles : register(u0);

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	if (index < Pcurrent)
	{
		const uint GravityFlag = 0;
		const uint DragFlag = 1;
		const uint CurlNoiseFlag = 2;
		const uint VortexFlag = 3;
		const uint PointInteractionFlag = 4;

		const uint NForceVortexKind = 0;
		const uint NForcePointInteraction = 1;

		const uint particleIndex = currentIndices[index];

		Particle currentParticle = particles[particleIndex];

		// 가속도 계산
		const uint emitterID = currentParticle.emitterID;
		const uint emitterType = currentParticle.emitterType;
        const float3 position = currentParticle.worldPos;
		const float3 velocity = currentParticle.velocity;
		float3 force = float3(0.f, 0.f, 0.f);

		ForceProperty forceProperty = emitterForces[emitterID];

		uint forceFlag = forceProperty.forceFlag;

		uint gravityFlag = (forceFlag >> GravityFlag);
		uint dragFlag = (forceFlag >> DragFlag);
		uint curlNoiseFlag = (forceFlag >> CurlNoiseFlag);
		uint vortextFlag = (forceFlag >> VortexFlag);
		uint pointInteractionFlag = (forceFlag >> PointInteractionFlag);

		if (gravityFlag & 1)
		{
			force += forceProperty.gravityForce;
		}

		if (dragFlag & 1)
		{
			force -= forceProperty.dragCoefficient * length(velocity) * velocity;
		}
		
		if (curlNoiseFlag & 1)
		{
			force += forceProperty.curlNoiseCoefficient * CurlNoise(position, max(forceProperty.curlNoiseOctave, 0.1f));
		}

		if (vortextFlag & 1)
		{
			const uint vortexCount = GetNForceCount(forceProperty.nForceCount, NForceVortexKind);

			[unroll]
			for (uint vortexIdx = 0; vortexIdx < vortexCount; ++vortexIdx)
			{
                VortexForceProperty vortexForceProperty = forceProperty.nVortexForce[vortexIdx];
                float3 origin = vortexForceProperty.vortexOrigin;
                float3 vortexAxis = vortexForceProperty.vortexAxis;
                float vortexRadius = vortexForceProperty.vortexRadius;
                float vortexDeathHorizonRadius = vortexForceProperty.vortexDeathHorizonRadius;
                float vortexCoefficient = vortexForceProperty.vortexCoefficient;
                float vortexTightness = vortexForceProperty.vortexTightness;
				
				float3 originToPos = position - origin;

				float3 vortexDir = cross(vortexAxis, originToPos);
				float vortexDirLength = length(vortexDir);

				float3 centripetalDir = dot(originToPos, vortexAxis) * vortexAxis - originToPos;
				float vortexDistance = length(centripetalDir);
				
				if (vortexDirLength > 1E-3f && vortexDistance > 1E-3f)
				{
					vortexDir = normalize(vortexDir);
					centripetalDir = normalize(centripetalDir);
					
                    float scale = lerp(1.f, 0.f, saturate(vortexDistance / max(vortexRadius, 1E-3)));
                    force += scale * vortexCoefficient * vortexDir;
                    					
                    float currentTangentialSpeed = dot(velocity + scale * vortexCoefficient * dt, vortexDir);
                    float centripetalForceMagnitude = currentTangentialSpeed * currentTangentialSpeed / vortexDistance;
                    force += (centripetalForceMagnitude + scale * vortexTightness) * centripetalDir;
                }

				if (vortexDistance < vortexDeathHorizonRadius)
				{
					currentParticle.life = 0.f;
				}
			}
		}

		if (pointInteractionFlag & 1)
		{
			const uint pointInteractionCount = GetNForceCount(forceProperty.nForceCount, NForcePointInteraction);

			[unroll]
			for (uint pointInteractionIdx = 0; pointInteractionIdx < pointInteractionCount; ++pointInteractionIdx)
			{
                PointInteractionForceProperty pointInteractionForceProperty = forceProperty.nPointInteractionForce[pointInteractionIdx];
                float3 origin = pointInteractionForceProperty.pointInteractionCenter;
                float interactionRadius = pointInteractionForceProperty.interactionRadius;
                float interactionCoefficient = pointInteractionForceProperty.interactionCoefficient;
				
				float3 posToOrigin = origin - position;
				float distance = length(posToOrigin);
				
				if (distance > 1E-3)
				{
					float scale = lerp(1.f, 0.f, saturate(distance / max(interactionRadius, 1E-3)));
					force += scale * interactionCoefficient * normalize(posToOrigin);
				}
			}
		}

        currentParticle.accelerate = force;
		
		particles[particleIndex] = currentParticle;
	}
}