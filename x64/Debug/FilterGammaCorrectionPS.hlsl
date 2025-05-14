#include "FilterCommon.hlsli"
#include "FilterToneMappingCommon.hlsli"

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);


float4 main(PostProcessVertexOutput input) : SV_TARGET
{   
    float3 hdrColor = filterSource.Sample(clampSampler, input.f2TexCoord);
//    float3 ldrColor = SimpleReinhard(hdrColor);
//    float3 ldrColor = ExtendedReinhard(hdrColor, float3(1.f, 1.f, 1.f));
    float3 ldrColor = LuminanceReinhard(hdrColor, 10000.f);
    float3 gammaCorrecetedColor = pow(ldrColor, 1.0f / 2.2f);
    return float4(gammaCorrecetedColor, 1.f);
}