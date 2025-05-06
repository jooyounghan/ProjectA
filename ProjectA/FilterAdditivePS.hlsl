#include "FilterCommon.hlsli"

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);

float4 main(PostProcessVertexOutput output) : SV_TARGET
{   
    return 1.2f * filterSource.Sample(clampSampler, output.f2TexCoord);
}