#pragma once
#include <DirectXMath.h>


struct SParticleInterpInformation
{
	float maxLife;
	unsigned int colorInterpolaterID;
	unsigned int colorInterpolaterDegree;
	float particleInterpDummy;
};

struct SSpriteInterpInformation
{
	float maxLife;
	uint32_t colorInterpolaterID;
	uint32_t colorInterpolaterDegree;
	uint32_t spriteSizeInterpolaterID;
	uint32_t spriteSizeInterpolaterDegree;
	uint32_t spriteIndexInterpolaterID;
	uint32_t spriteIndexInterpolaterDegree;
	DirectX::XMFLOAT2 spriteTextureCount;
	float dummy[3];
};