#include "DefineLinkedWithCpp.hlsli"
#include "FilterCommon.hlsli"

cbuffer MotionBlurFilmProperty : register(b2)
{
    uint samplingCount;
    float dissipationFactor;
    uint2 motionBlurFilterDummy;
};

Texture2D<float2> motionVectorTexture : register(t0);
Texture2D sceneTexture : register(t1);

RWTexture2D<float4> motionBlurredTexture : register(u0);

SamplerState clampSampler : register(s0);

[numthreads(GroupTexWidth, GroupTexHeight, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

    float2 location = float2(DTid.x, DTid.y);
    float2 prevLocation = location;

    float2 invVelocity = motionVectorTexture[location];
    invVelocity.x =  -invVelocity.x;
    float velocityLength = length(invVelocity);

    uint dynamicStepCount = min(samplingCount, uint(velocityLength));

    float2 sampleStep = invVelocity / dynamicStepCount;    
    float4 sceneColor = sceneTexture[location];
    for (uint step = 0; step < dynamicStepCount; ++step)
    {   
        prevLocation += sampleStep;
        motionBlurredTexture[prevLocation] = sceneColor;
        sceneColor *= dissipationFactor;
    }
}