#include "FilterCommon.hlsli"
#include "FilterToneMappingCommon.hlsli"

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);


float4 main(PostProcessVertexOutput input) : SV_TARGET
{   
    float3 hdrColor = filterSource.Sample(clampSampler, input.f2TexCoord);
    float3 ldrColor = SimpleReinhard2(hdrColor, 0.1f);
    float3 gammaCorrecetedColor = pow(ldrColor, 1.0f / 2.2f);
    return float4(gammaCorrecetedColor, 1.f);
}