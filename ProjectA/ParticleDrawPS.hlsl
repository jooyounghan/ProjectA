#include "ParticleDrawCommon.hlsli"


float4 main(ParticleGSOut input) : SV_TARGET
{
    float factor = input.life;
    float dist = length(0.5f - input.texCoord) * 2.f;
    float scale = smoothstep(0.f, 1.f, 1.f - dist);
    float4 color = input.color;

    return color * scale * factor;
}