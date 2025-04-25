#include "CaculateForceCommon.hlsli"

#define Pcurrent particleDrawIndirectArgs[0]
#define EPSILON 1e-3f

StructuredBuffer<uint> particleDrawIndirectArgs: register(t0);
StructuredBuffer<uint> currentIndices : register(t1);
StructuredBuffer<matrix> emitterWorldTransforms : register(t2);
StructuredBuffer<ForceProperty> emitterForces : register(t3);

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

		const uint emitterID = currentParticle.emitterID;
		const uint emitterType = currentParticle.emitterType;

		const float3 position = currentParticle.worldPos;
		const float3 velocity = currentParticle.velocity;
		const matrix emitterWorldTransform = emitterWorldTransforms[emitterID];
		const ForceProperty forceProperty = emitterForces[emitterID];
        const uint forceFlag = forceProperty.forceFlag;
		float3 force = float3(0.f, 0.f, 0.f);



        if (IsForceEnabled(forceFlag, GravityFlag))
        {
            force += forceProperty.gravityForce;
        }

        if (IsForceEnabled(forceFlag, DragFlag))
        {
            force -= forceProperty.dragCoefficient * length(velocity) * velocity;
        }

        if (IsForceEnabled(forceFlag, CurlNoiseFlag))
        {
            float octave = max(forceProperty.curlNoiseOctave, 0.1f);
            force += forceProperty.curlNoiseCoefficient * CurlNoise(position, octave);
        }

        if (IsForceEnabled(forceFlag, VortexFlag))
        {
            uint vortexCount = GetNForceCount(forceProperty.nForceCount, NForceVortexKind);

            [unroll]
            for (uint i = 0; i < vortexCount; ++i)
            {
                VortexForceProperty v = forceProperty.nVortexForce[i];

                float3 origin = GetWorldPosition(v.vortexOrigin, emitterWorldTransform);
                float3 toPos = position - origin;

                float3 vortexDir = cross(v.vortexAxis, toPos);
                float vortexDirLen = length(vortexDir);

                float3 centripetalDir = dot(toPos, v.vortexAxis) * v.vortexAxis - toPos;
                float vortexDist = length(centripetalDir);

                if (vortexDirLen > EPSILON && vortexDist > EPSILON)
                {
                    vortexDir = normalize(vortexDir);
                    centripetalDir = normalize(centripetalDir);

                    float scale = lerp(1.f, 0.f, saturate(vortexDist / max(v.vortexRadius, EPSILON)));
                    force += scale * v.vortexCoefficient * vortexDir;

                    float tangentSpeed = dot(velocity + scale * v.vortexCoefficient * dt, vortexDir);
                    float centripetalMag = tangentSpeed * tangentSpeed / vortexDist;
                    force += (centripetalMag + scale * v.vortexTightness) * centripetalDir;
                }

                if (vortexDist < v.vortexDeathHorizonRadius)
                {
                    currentParticle.life = 0.f;
                }
            }
        }

        if (IsForceEnabled(forceFlag, PointInteractionFlag))
        {
            uint piCount = GetNForceCount(forceProperty.nForceCount, NForcePointInteraction);

            [unroll]
            for (uint i = 0; i < piCount; ++i)
            {
                PointInteractionForceProperty pi = forceProperty.nPointInteractionForce[i];

                float3 origin = GetWorldPosition(pi.pointInteractionCenter, emitterWorldTransform);
                float3 toOrigin = origin - position;
                float dist = length(toOrigin);

                if (dist > EPSILON)
                {
                    float scale = lerp(1.f, 0.f, saturate(dist / max(pi.interactionRadius, EPSILON)));
                    force += scale * pi.interactionCoefficient * normalize(toOrigin);
                }
            }
        }

        currentParticle.accelerate = force;
		
		particles[particleIndex] = currentParticle;
	}
}