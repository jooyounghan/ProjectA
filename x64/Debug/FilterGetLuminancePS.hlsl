#include "FilterCommon.hlsli"
#include "FilterToneMappingCommon.hlsli"

Texture2D traceTexture : register(t0);
SamplerState clampSampler : register(s0);

float main(PostProcessVertexOutput input) : SV_TARGET
{
    float3 rgb = traceTexture.Sample(clampSampler, input.f2TexCoord);
    return GetLuminance(rgb);
}