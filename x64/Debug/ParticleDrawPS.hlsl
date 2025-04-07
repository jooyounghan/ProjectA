#include "ParticleDrawCommon.hlsli"

float4 main(ParticleGSOut input) : SV_TARGET
{
    float factor = input.life / 3.f;
    float dist = length(0.5f - input.texCoord) * 2.f;
    float scale = smoothstep(0.f, 1.f, 1.f - dist);
    return float4(float3(0.f, 1.f, 1.f) * scale * factor, scale * factor);
}