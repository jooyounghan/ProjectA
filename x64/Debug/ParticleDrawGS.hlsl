#include "ParticleCommon.hlsli"
#include "ParticleDrawCommon.hlsli"

[maxvertexcount(4)]
#ifdef SPRITE_EMITTER
void main(point SpriteVSOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<SpriteGSOut> output)
#else
void main(point ParticleVSOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<ParticleGSOut> output)
#endif
{
#ifdef SPRITE_EMITTER
    SpriteVSOut inputData = input[0];
    SpriteGSOut element;
    element.spriteIndex = inputData.spriteIndex;
	element.emitterID = inputData.emitterID;
#else
    ParticleVSOut inputData = input[0];
    ParticleGSOut element;
    element.velocity = inputData.velocity;
#endif

    const float2 xyScale = inputData.xyScale;
    const float4 dx = float4(10.f / appWidth, 0.f, 0.f, 0.f) * xyScale.x;
    const float4 dy = float4(0.f, 10.f / appHeight, 0.f, 0.f) * xyScale.y;
    element.color = inputData.color;

    float4 offsets[4] = 
    {
        -dx - dy,
        -dx + dy,
         dx - dy,
         dx + dy
    };

    float2 texCoords[4] = 
    {
        float2(0.f, 1.f),
        float2(0.f, 0.f),
        float2(1.f, 1.f),
        float2(1.f, 0.f)
    };


    [unroll]
    for (int i = 0; i < 4; ++i) 
    {
        element.viewPos = inputData.viewPos + offsets[i];
        element.texCoord = texCoords[i];
        output.Append(element);
    }
    
    output.RestartStrip();

}