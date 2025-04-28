#include "DefineLinkedWithCpp.hlsli"

#define MaxStepCount MaxControlPointsCount - 1

struct ParticleEmitterInterpInform
{
    float maxLife;
    uint colorInterpolaterID;
    uint colorInterpolaterDegree;
    float particleEmitterInterpInformDummy;
};

struct InterpPropertyHeader
{
    uint controlPointsCount;
    uint interpolateMethod;
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

static float4 Evaluate1Degree(float time, float coeff[4][2])
{
    return float4(
        coeff[0][0] * time + coeff[0][1],
        coeff[1][0] * time + coeff[1][1],
        coeff[2][0] * time + coeff[2][1],
        coeff[3][0] * time + coeff[3][1]
        );
}

static float4 Evaluate3Degree(float t, float coeff[4][4])
{
    return float4(
        Get3DegreeFunction(coeff[0][0], coeff[0][1], coeff[0][2], coeff[0][3], t),
        Get3DegreeFunction(coeff[1][0], coeff[1][1], coeff[1][2], coeff[1][3], t),
        Get3DegreeFunction(coeff[2][0], coeff[2][1], coeff[2][2], coeff[2][3], t),
        Get3DegreeFunction(coeff[3][0], coeff[3][1], coeff[3][2], coeff[3][3], t)
      );
}

