#include "ParticleDrawCommon.hlsli"


float4 main(ParticleGSOut input) : SV_TARGET
{
    float factor = input.life;
    float dist = length(0.5f - input.texCoord) * 2.f;
    float scale = smoothstep(0.f, 1.f, 1.f - dist);
    float3 color = input.color;
    float alpha = scale;
    alpha = min(alpha, alpha * factor);
    color = min(color, color * factor);
    return float4(color * scale, alpha);
    //return float4(float3(0.f, 1.f, 1.f)* lightPower * scale * factor, scale * factor);
}