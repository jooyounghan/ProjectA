#include "EmitterDrawCommon.hlsli"

EmitterVSOutput main(EmitterVSInput emitterInput )
{
    matrix worldTransform =
    {
        emitterInput.world0,
        emitterInput.world1,
        emitterInput.world2,
        emitterInput.world3
    };

    EmitterVSOutput result;

	float4 worldPos = mul(float4(emitterInput.modelPos, 1.f), worldTransform);
    result.viewProjPos = mul(worldPos, viewProjMatrix);
    result.normalVec = emitterInput.normalVec;
	return result;
}