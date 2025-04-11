#include "ParticleDrawCommon.hlsli"

float3 GetViewNormal(float2 uv)
{
    uv = uv * 2.0 - 1.0;
    float3 n = float3(uv.x, uv.y, 1.0 - abs(uv.x) - abs(uv.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
}


float4 main(ParticleGSOut input) : SV_TARGET
{
    float factor = input.life;
    float dist = length(0.5f - input.texCoord) * 2.f;
    float scale = smoothstep(0.f, 1.f, 1.f - dist);
    
    float3 viewNormal = GetViewNormal(input.texCoord);

    float4 worldNormal = mul(float4(viewNormal, 0.f), invTransposeViewMatrix);
    float lightPower = dot(worldNormal, float4(0.f, -1.f, 0.f, 0.f));
   
    return float4(input.color * scale * factor, scale * factor);
    //return float4(float3(0.f, 1.f, 1.f)* lightPower * scale * factor, scale * factor);
}