#include "ParticleCommon.hlsli"

struct VortexForceProperty
{
    float3 vortexOrigin;
    float3 vortexAxis;
    float vortexRadius;
    float vortexDeathHorizonRadius;
    float vortexCoefficient;
    float vortexTightness;
};

struct LineInteractionForceProperty
{
    float3 lineInteractionOrigin;
    float3 lineInteractionAxis;
    float interactionDistance;
    float interactionCoefficient;
};

struct PointInteractionForceProperty
{
    float3 pointInteractionCenter;
    float interactionRadius;
    float interactionCoefficient;
};

struct ForceProperty
{
    uint forceFlag;
    float3 gravityForce;
    float dragCoefficient;
    float curlNoiseOctave;
    float curlNoiseCoefficient;
    uint nForceCount;
    VortexForceProperty nVortexForce[MaxNForceCount];
    LineInteractionForceProperty nLineInteractionForce[MaxNForceCount];
    PointInteractionForceProperty nPointInteractionForce[MaxNForceCount];
};

uint GetRadixCount(uint n)
{
    if (n == 0)
        return 0;
    return firstbithigh(n) + 1;
}

uint GetNForceCount(uint nForceCount, uint forceKind)
{
    const uint radixCount = GetRadixCount(MaxNForceCount);
    const uint shift = radixCount * forceKind;
    const uint shiftedForceCount = nForceCount >> shift;
    const uint filledMask = (1 << radixCount) - 1;

    return (shiftedForceCount & filledMask);
}

// 수정 필요 ==============================================================
float hash(float3 p)
{
    p = frac(p * 0.3183099 + float3(0.1, 0.2, 0.3));
    p *= 17.0;
    return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float Noise(float3 p)
{
    float3 pi = floor(p);
    float3 pf = frac(p);

    float3 w = pf * pf * (3.0 - 2.0 * pf);

    float n000 = hash(pi + float3(0, 0, 0));
    float n001 = hash(pi + float3(0, 0, 1));
    float n010 = hash(pi + float3(0, 1, 0));
    float n011 = hash(pi + float3(0, 1, 1));
    float n100 = hash(pi + float3(1, 0, 0));
    float n101 = hash(pi + float3(1, 0, 1));
    float n110 = hash(pi + float3(1, 1, 0));
    float n111 = hash(pi + float3(1, 1, 1));

    float x00 = lerp(n000, n100, w.x);
    float x01 = lerp(n001, n101, w.x);
    float x10 = lerp(n010, n110, w.x);
    float x11 = lerp(n011, n111, w.x);

    float y0 = lerp(x00, x10, w.y);
    float y1 = lerp(x01, x11, w.y);

    return lerp(y0, y1, w.z);
}

float3 CurlNoise(float3 pos, float eps)
{
    float3 dx = float3(eps, 0, 0);
    float3 dy = float3(0, eps, 0);
    float3 dz = float3(0, 0, eps);

    float3 dp_dy = float3(
        Noise(pos + dy + dz) - Noise(pos + dy - dz),
        Noise(pos + dz + dx) - Noise(pos + dz - dx),
        Noise(pos + dx + dy) - Noise(pos + dx - dy)
        );

    float3 dp_dz = float3(
        Noise(pos - dy + dz) - Noise(pos - dy - dz),
        Noise(pos - dz + dx) - Noise(pos - dz - dx),
        Noise(pos - dx + dy) - Noise(pos - dx - dy)
        );

    float3 curl;
    curl.x = (dp_dy.z - dp_dz.y);
    curl.y = (dp_dy.x - dp_dz.z);
    curl.z = (dp_dy.y - dp_dz.x);

    return normalize(curl);
}
// 수정 필요 ==============================================================