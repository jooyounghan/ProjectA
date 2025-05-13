#include "EmitterDrawCommon.hlsli"

float3x3 Inverse3x3(float3x3 m)
{
    float3 r0 = cross(m[1], m[2]);
    float3 r1 = cross(m[2], m[0]);
    float3 r2 = cross(m[0], m[1]);

    float invDet = 1.0 / dot(r2, m[2]);

    return float3x3(r0, r1, r2) * invDet;
}

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
    
    float3x3 rotationScale = (float3x3)worldTransform;
    float3x3 normalMatrix = transpose(Inverse3x3(transpose(rotationScale)));

    result.normalVec = mul(emitterInput.normalVec, normalMatrix);
	return result;
}