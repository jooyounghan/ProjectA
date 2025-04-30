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
    uint spriteIndexInterpolaterID;
    uint spriteIndexInterpolaterDegree;
    float spriteEmitterInterpInformDummy;
};

struct InterpPropertyHeader
{
    uint controlPointsCount;
    uint interpolateMethod;
};

struct D1Dim1Prop
{
    InterpPropertyHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][2];
};

struct D3Dim1Prop
{
    InterpPropertyHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][4];
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

static float1 EvaluateD1Dim1(float t, float coeff[2])
{
    return coeff[0] * t + coeff[1];
}

static float1 EvaluateD3Dim1(float t, float coeff[4])
{
    return Get3DegreeFunction(coeff[0], coeff[1], coeff[2], coeff[3], t);
}

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

float GetInterpolatedDim1(
    uint degree, uint interpolatedID, 
    float timeSpent, float maxLife, 
    StructuredBuffer<D1Dim1Prop> d1Dim1Props,
    StructuredBuffer<D3Dim1Prop> d3Dim1Props
)
{
    const uint CubicSplineMethod = 1;

    if (degree == 1)
    {
        D1Dim1Prop interpProp = d1Dim1Props[interpolatedID];
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                return EvaluateD1Dim1(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD1Dim1(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 3)
    {
        D3Dim1Prop interpProp = d3Dim1Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolateMethod;
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

                return EvaluateD3Dim1(t, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD3Dim1(0.f, interpProp.coefficient[stepsCount]);
    }

    return 0.f;
}

float2 GetInterpolatedDim2(
    uint degree, uint interpolatedID, 
    float2 timeSpent2, float maxLife,
    StructuredBuffer<D1Dim2Prop> d1Dim2Props,
    StructuredBuffer<D3Dim2Prop> d3Dim2Props
)
{
    const uint CubicSplineMethod = 1;
    const float timeSpent = timeSpent2.x;

    if (degree == 1)
    {
        D1Dim2Prop interpProp = d1Dim2Props[interpolatedID];
        const uint stepsCount = interpProp.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpProp.xProfiles[stepIdx];
            float x2 = interpProp.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                return EvaluateD1Dim2(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD1Dim2(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 3)
    {
        D3Dim2Prop interpProp = d3Dim2Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolateMethod;
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

                return EvaluateD3Dim2(t, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD3Dim2(0.f, interpProp.coefficient[stepsCount]);
    }

    return float2(0.f, 0.f);
}

float4 GetInterpolatedDim4(
    uint degree, uint interpolatedID, 
    float4 timeSpent4, float maxLife,
    StructuredBuffer<D1Dim4Prop> d1Dim4Props,
    StructuredBuffer<D3Dim4Prop> d3Dim4Props
)
{
    const uint CubicSplineMethod = 1;
    const float timeSpent = timeSpent4.x;

    if (degree == 1)
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
                return EvaluateD1Dim4(timeSpent, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD1Dim4(maxLife, interpProp.coefficient[stepsCount]);
    }

    else if (degree == 3)
    {
        D3Dim4Prop interpProp = d3Dim4Props[interpolatedID];
        const uint interpolateMethod = interpProp.header.interpolateMethod;
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

                return EvaluateD3Dim4(t, interpProp.coefficient[stepIdx]);
            }
        }

        return EvaluateD3Dim4(0.f, interpProp.coefficient[stepsCount]);
    }

    return float4(0.f, 0.f, 0.f, 1.f);
}