#include "SimulateCommon.hlsli"

#define airDensity 1.225
#define airViscosity 1.81E-5f

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	if (index < Pcurrent)
	{
		uint particleIndex = currentIndices[index];
		Particle currentParticle = particles[particleIndex];

		
		// ���ӵ� ���
		float3 accelerate = float3(0.f, 0.f, 0.f);
		const float density = currentParticle.density;
		const float radius = currentParticle.radius;
		const float3 gravity = float3(0.f, -9.8, 0.f);
		const float3 velocity = currentParticle.velocity;
		accelerate += gravity;
		accelerate -= (airDensity / density) * gravity;
		accelerate -= (4.5 * airViscosity / (density * radius * radius)) * velocity;

		currentParticle.accelerate = accelerate;

		// ���ӵ��� ���� ����
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