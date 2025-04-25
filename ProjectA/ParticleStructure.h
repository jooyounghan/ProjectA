#pragma once
#include <DirectXMath.h>

struct SParticle
{
	DirectX::XMVECTOR color;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 accelerate;
	uint32_t emitterType;
	uint32_t emitterID;
	float life;
};
