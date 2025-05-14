float3 SimpleReinhard(float3 rgb)
{
	return rgb / (1.f + rgb);
}

float3 ExtendedReinhard(float3 rgb, float3 whiteHdrRgb)
{
	float cWhite = 1.f / whiteHdrRgb;
	return rgb * (1.f + rgb * cWhite * cWhite) / (1.f + rgb);
}

static float3 luminanceFactor = float3(0.2126f, 0.7152f, 0.0722f);

float GetLumniance(float3 rgb)
{
	return dot(rgb, luminanceFactor);
}

float3 ChangeLuminance(float3 rgb, float luminanceOut)
{
	float luminance = GetLumniance(rgb);
	return rgb * (luminanceOut / luminance);
}

float3 LuminanceReinhard(float3 rgb, float whiteLuminance)
{
	float luminance = GetLumniance(rgb);
	float cWhite = 1.f / whiteLuminance;
	float numerator = luminance * (1.0f + (luminance * cWhite * cWhite));
	float newLuminance = numerator / (1.0f + luminance);
	return ChangeLuminance(rgb, newLuminance);
}