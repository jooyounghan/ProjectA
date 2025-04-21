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
		const uint LineInteractionFlag = 4;
		const uint PointInteractionFlag = 5;

		const uint NForceVortexKind = 0;
		const uint NForceLineInteractionKind = 1;
		const uint NForcePointInteraction = 2;

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
		uint lineInteractionFlag = (forceFlag >> LineInteractionFlag);
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
				VortexForce vortexForce = forceProperty.nVortexForce[vortexIdx];
				float3 origin = vortexForce.vortexOrigin;
				float3 originToPos = position - origin;
				float3 vortexAxis = vortexForce.vortexAxis;
				float vortexRadius = vortexForce.vortexRadius;

				float3 vortexDir = cross(vortexAxis, originToPos);
				float vortexDirLength = length(vortexDir);

				float3 centripetalDir = dot(originToPos, vortexAxis) * vortexAxis - originToPos;
				float vortexDistance = length(centripetalDir);
				
				if (vortexDirLength > 1E-3f && vortexDistance > 1E-3f)
				{
					vortexDir = normalize(vortexDir);
					centripetalDir = normalize(centripetalDir);

			        float tangentialVelocity = vortexForce.vortexCoefficient * dt;
					float centripetalForceMagnitude = tangentialVelocity * tangentialVelocity / vortexDistance;
					force += vortexForce.vortexCoefficient * vortexDir;
					force += (centripetalForceMagnitude + vortexForce.vortexTightness) * centripetalDir;
				}

			}
		}


		if (lineInteractionFlag & 1)
		{
			force += forceProperty.curlNoiseCoefficient * CurlNoise(position, max(forceProperty.curlNoiseOctave, 0.1f));
		}

		if (pointInteractionFlag & 1)
		{
			force += forceProperty.curlNoiseCoefficient * CurlNoise(position, max(forceProperty.curlNoiseOctave, 0.1f));
		}


        currentParticle.accelerate = force;
		
		particles[particleIndex] = currentParticle;
	}
}