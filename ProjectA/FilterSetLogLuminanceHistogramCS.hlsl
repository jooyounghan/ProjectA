#include "DefineLinkedWithCpp.hlsli"
#include "FilterToneMappingCommon.hlsli"

cbuffer luminanceFilterProperty : register(b2)
{
    float minLogLum;
    float maxLogLum;
    uint2 lumDummy;
};

Texture2D<float4> hdrShotFilm : register(t0);
RWStructuredBuffer<uint> logLuminanceHistogram : register(u0);

const static uint groupSize =  GroupTexWidth * GroupTexHeight;

static uint colorToBin(float3 hdrRGB) 
{
    float lum = GetLuminance(hdrRGB);

    if (lum < 1E-3f) 
    {
        return 0;
    }

    float inverseLogLumRange = 1.f / (maxLogLum - minLogLum);
    float logLum = clamp((log2(lum) - minLogLum) * inverseLogLumRange, 0.0, 1.0);

    return uint(logLum * (float(groupSize) - 1.f) + 1.0);
}

groupshared uint luminanceHistogramShared[groupSize];

[numthreads(GroupTexWidth, GroupTexHeight, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    uint groupXID = Gid.x;
    uint groupYID = Gid.y;
    uint groupIdx = GroupTexWidth * groupYID + groupXID;

    uint groupThreadXID = GTid.x;
    uint groupThreadYID = GTid.y;
    uint groupThreadIdx = GroupTexWidth * groupThreadYID + groupThreadXID;

    luminanceHistogramShared[groupThreadIdx] = 0;
    GroupMemoryBarrierWithGroupSync();

    uint2 xyCoord = DTid.xy;
    float3 hdrRGB = hdrShotFilm[xyCoord].xyz;
    
    uint binIndex = colorToBin(hdrRGB);
    InterlockedAdd(luminanceHistogramShared[binIndex], 1);

    GroupMemoryBarrierWithGroupSync();

    InterlockedAdd(logLuminanceHistogram[groupThreadIdx], luminanceHistogramShared[groupThreadIdx]);
}

