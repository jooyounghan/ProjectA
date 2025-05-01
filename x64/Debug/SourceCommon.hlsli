float rand(float2 seed)
{
    return frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453);
}

float2 hash22(float2 p)
{
    const float2 k1 = float2(127.1, 311.7);
    const float2 k2 = float2(269.5, 183.3);

    float2 sinInput = float2(dot(p, k1), dot(p, k2));
    float2 s = sin(sinInput) * 43758.5453;
    return frac(s);
}
