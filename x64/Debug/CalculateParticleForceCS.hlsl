#include "CaculateForceCommon.hlsli"

#define airDensity 1.225
#define sphereCd 0.47

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	if (index < Pcurrent)
	{
		const uint particleIndex = currentIndices[index].index;
		Particle currentParticle = particles[particleIndex];

		// 가속도 계산
		const uint emitterID = currentParticle.emitterID;
		const uint emitterType = currentParticle.emitterType;
		const float3 velocity = currentParticle.velocity;
		const float density = currentParticle.density;
		const float radius = currentParticle.radius;
        const float volume = radius * radius * radius * 4.f * 3.141592f / 3.f;
        const float area = 3.141592f * radius * radius;
        const float mass = volume * density;
		const float3 gravity = float3(0.f, -9.8, 0.f);
		
		float3 force = float3(0.f, 0.f, 0.f);
		
        force += mass * gravity;
        force -= volume * airDensity * gravity;
        force -= airDensity * sphereCd * area * length(velocity) * velocity / 2.f;

		if (emitterType == 0)
		{

        }
		else if (emitterType == 1)
		{
			float3 particleWorldPos = currentParticle.worldPos;
			float3 parentEmitterWorldPos = emitterWorldPos[emitterID].xyz;
			float3 r = (particleWorldPos - parentEmitterWorldPos);
			float distance = length(r);
			float3 gravityAcc = -r / min((distance * distance * distance), 1.f);
			float3 curlAcc = CurlNoise(particleWorldPos, 1.0f);
            force += mass * (gravityAcc + curlAcc * 2.f);
        }

        currentParticle.accelerate = force / mass;
		
		particles[particleIndex] = currentParticle;
	}
}