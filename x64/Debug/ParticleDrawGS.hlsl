#include "ParticleCommon.hlsli"
#include "ParticleDrawCommon.hlsli"

[maxvertexcount(4)]
#ifdef SPRITE_EMITTER
void main(point SpriteVSOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<SpriteGSOut> output)
#else
void main(point ParticleVSOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<ParticleGSOut> output)
#endif
{
    const float defaultLength = 10.f;
    const float defaultTailLength = defaultLength / 2.f;
    
#ifdef SPRITE_EMITTER
    SpriteVSOut inputData = input[0];
    SpriteGSOut element;
    element.spriteIndex = inputData.spriteIndex;
	element.emitterID = inputData.emitterID;
#else
    ParticleVSOut inputData = input[0];
    ParticleGSOut element;
#endif

    float4 inputViewPos = inputData.viewPos;
    float2 viewportScale = float2(1.f/ appWidth, 1.f / appHeight);
    float2 xyScale = inputData.xyScale;
    float2 dxdy = defaultLength * viewportScale * xyScale;

    element.color = inputData.color;

    float4 offsets[4] = 
    {
        float4(-dxdy.x, -dxdy.y, 0.f, 0.f),
        float4(-dxdy.x, dxdy.y, 0.f, 0.f),
        float4(dxdy.x, -dxdy.y, 0.f, 0.f),
        float4(dxdy.x, dxdy.y, 0.f, 0.f)
    };

    float2 texCoords[4] = 
    {
        float2(0.f, 1.f),
        float2(0.f, 0.f),
        float2(1.f, 1.f),
        float2(1.f, 0.f)
    };

#ifdef PARTICLE_EMITTER
    float4 negativeNDCVelocity = -float4(inputData.ndcVelocity, 0.f, 0.f);
#endif

    [unroll]
    for (int i = 0; i < 4; ++i) 
    {
#ifdef PARTICLE_EMITTER
        offsets[i] += defaultTailLength * negativeNDCVelocity * dot(offsets[i], negativeNDCVelocity);
#endif
        element.viewPos = inputData.viewPos + offsets[i];
        element.texCoord = texCoords[i];
        output.Append(element);
    }
    
    output.RestartStrip();

}