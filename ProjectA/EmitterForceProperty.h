#pragma once
#include "DefineLinkedWithShader.h"
#include <DirectXMath.h>

struct SVortexForce
{
	DirectX::XMFLOAT3 vortexOrigin;
	DirectX::XMFLOAT3 vortexAxis;
	float vortexRadius;
	float vortexDeathHorizonRadius;
	float vortextCoefficient;
	float vortexTightness;
};

struct SPointInteractionForce
{
	DirectX::XMFLOAT3 pointInteractionCenter;
	float interactionRadius;
	float interactionCoefficient;
};

struct SEmitterForceProperty
{
	uint32_t forceFlag;
	DirectX::XMFLOAT3 gravityForce;
	float dragCoefficient;
	float curlNoiseOctave;
	float curlNoiseCoefficient;
	uint32_t nForceCount;
	SVortexForce nVortexForce[MaxNForceCount];
	SPointInteractionForce nPointInteractionForce[MaxNForceCount];
};