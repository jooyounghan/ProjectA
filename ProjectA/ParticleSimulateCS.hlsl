#include "SimulateCommon.hlsli"

#define airDensity 1.225
#define airViscosity 1.81E-5f * 1E2

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	if (index < Pcurrent)
	{
		const uint particleIndex = currentIndices[index];
		Particle currentParticle = particles[particleIndex];

		// 가속도 계산
		const uint emitterID = currentParticle.emitterID;
		const uint emitterType = currentParticle.emitterType;
		const float density = currentParticle.density;
		const float radius = currentParticle.radius;
		float3 accelerate = float3(0.f, 0.f, 0.f);

		if (emitterType == 0)
		{
			const float3 gravity = float3(0.f, -9.8, 0.f);
			const float3 velocity = currentParticle.velocity;
			accelerate += gravity;
			accelerate -= (airDensity / density) * gravity;
			accelerate -= (4.5 * airViscosity / (density * radius * radius)) * velocity;
		}
		else if (emitterType == 1)
		{
			float3 particleWorldPos = currentParticle.worldPos;
			float3 parentEmitterWorldPos = emitterWorldPos[emitterID].xyz;
			float3 gravityAcc = (parentEmitterWorldPos - particleWorldPos);
			float3 curlAcc = CurlNoise(particleWorldPos, 0.1f);
			accelerate += (gravityAcc + curlAcc * 3.f);
		}

		currentParticle.accelerate = accelerate;

		// 가속도를 통한 적분
		currentParticle.velocity += currentParticle.accelerate * dt;
		currentParticle.worldPos += currentParticle.velocity * dt;

		if (currentParticle.worldPos.y < 0.f)
		{
			currentParticle.worldPos.y = 1E-3;
			currentParticle.velocity.x = currentParticle.velocity.x * 0.2f;
			currentParticle.velocity.y = currentParticle.velocity.y * -0.2f;
			currentParticle.velocity.z = currentParticle.velocity.z * 0.2f;
		}

		particles[particleIndex] = currentParticle;
	}
}