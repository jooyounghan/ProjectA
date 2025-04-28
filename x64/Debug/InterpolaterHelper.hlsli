#include "DefineLinkedWithCpp.hlsli"

#define MaxStepCount MaxControlPointsCount - 1

struct ParticleEmitterInterpInform
{
    float maxLife;
    uint colorInterpolaterID;
    uint colorInterpolaterDegree;
    float particleEmitterInterpInformDummy;
};

struct SpriteEmitterInterpInform
{
    float maxLife;
    uint colorInterpolaterID;
    uint colorInterpolaterDegree;
    uint spriteSizeInterpolaterID;
    uint spriteSizeInterpolaterDegree;
    float3 spriteEmitterInterpInformDummy;
};

struct InterpPropertyHeader
{
    uint controlPointsCount;
    uint interpolateMethod;
};

struct D1Dim2Prop
{
    InterpPropertyHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][2][2];
};

struct D3Dim2Prop
{
    InterpPropertyHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][2][4];
};


struct D1Dim4Prop
{
    InterpPropertyHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][4][2];
};

struct D3Dim4Prop
{
    InterpPropertyHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][4][4];
};

static float Get3DegreeFunction(float a, float b, float c, float d, float x) { return (((a * x) + b) * x + c) * x + d; }

static float2 EvaluateD1Dim2(float t, float coeff[2][2])
{
    return float2(
        coeff[0][0] * t + coeff[0][1],
        coeff[1][0] * t + coeff[1][1]
    );
}

static float2 EvaluateD3Dim2(float t, float coeff[2][4])
{
    return float2(
        Get3DegreeFunction(coeff[0][0], coeff[0][1], coeff[0][2], coeff[0][3], t),
        Get3DegreeFunction(coeff[1][0], coeff[1][1], coeff[1][2], coeff[1][3], t)
    );
}

static float4 EvaluateD1Dim4(float t, float coeff[4][2])
{
    return float4(
        coeff[0][0] * t + coeff[0][1],
        coeff[1][0] * t + coeff[1][1],
        coeff[2][0] * t + coeff[2][1],
        coeff[3][0] * t + coeff[3][1]
        );
}

static float4 EvaluateD3Dim4(float t, float coeff[4][4])
{
    return float4(
        Get3DegreeFunction(coeff[0][0], coeff[0][1], coeff[0][2], coeff[0][3], t),
        Get3DegreeFunction(coeff[1][0], coeff[1][1], coeff[1][2], coeff[1][3], t),
        Get3DegreeFunction(coeff[2][0], coeff[2][1], coeff[2][2], coeff[2][3], t),
        Get3DegreeFunction(coeff[3][0], coeff[3][1], coeff[3][2], coeff[3][3], t)
      );
}

