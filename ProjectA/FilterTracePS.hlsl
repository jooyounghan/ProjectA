#include "FilterCommon.hlsli"

Texture2D traceTexture : register(t0);
SamplerState clampSampler : register(s0);

float4 main(PostProcessVertexOutput output) : SV_TARGET
{   
    return traceTexture.Sample(clampSampler, output.f2TexCoord);
}