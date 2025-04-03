#include "EmitterDrawCommon.hlsli"

float4 main(EmitterVSInput emitterInput ) : SV_POSITION
{
    matrix worldTransform =
    {
        emitterInput.world0,
        emitterInput.world1,
        emitterInput.world2,
        emitterInput.world3
    };

	float4 worldPos = mul(float4(emitterInput.modelPos, 1.f), worldTransform);
	return mul(worldPos, viewProjMatrix);
}