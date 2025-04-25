#include "DefineLinkedWithCpp.hlsli"

#define MaxStepCount MaxControlPointsCount - 1

struct EmitterInterpInform
{
    float maxLife;
    uint colorInterpolaterID;
    uint colorInterpolaterDegree;
    float emitterInterpolaterInformationDummy;
};

struct InterpPropertyHeader
{
    uint controlPointsCount;
    uint interpolaterFlag;
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

StructuredBuffer<EmitterInterpInform> emitterInterpInforms : register(t0);
StructuredBuffer<D1Dim4Prop> d1Dim4Props : register(t1);
StructuredBuffer<D3Dim4Prop> d3Dim4Props : register(t2);

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


float4 GetInterpolated(uint degree, uint interpolatedID, float4 timeSpent4, float maxLife)
{
    const uint CubicSplineMethod = 2;
    const uint CatmullRomMethod = 3;
    const float timeSpent = timeSpent4.x;

    if (degree == 2)
    {
        D1Dim4Prop interpProp = d1Dim4Props[interpolatedID];
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                return Evaluate1Degree(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return Evaluate1Degree(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 4)
    {
        D3Dim4Prop interpProp = d3Dim4Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolaterFlag;
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                float t = (interpolateMethod == CubicSplineMethod)
                    ? timeSpent - x1
                    : (timeSpent - x1) / (x2 - x1);

                return Evaluate3Degree(t, interpProp.coefficient[stepIdx]);
            }
        }

        return Evaluate3Degree(0.f, interpProp.coefficient[stepsCount]);
    }

    return float4(0.f, 0.f, 0.f, 1.f);
}