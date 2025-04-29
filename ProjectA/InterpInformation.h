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
	unsigned int colorInterpolaterID;
	unsigned int colorInterpolaterDegree;
	unsigned int spriteSizeInterpolaterID;
	unsigned int spriteSizeInterpolaterDegree;
	unsigned int spriteIndexInterpolaterID;
	unsigned int spriteIndexInterpolaterDegree;
	float spriteInterpDummy;
};