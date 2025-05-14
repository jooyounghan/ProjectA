static float3 luminanceFactor = float3(0.2126f, 0.7152f, 0.0722f);

static float3x3 RGBToXYZMatrix = float3x3(
	float3(0.4124564, 0.3575761, 0.1804375),
	float3(0.2126729, 0.7151522, 0.0721750),
	float3(0.0193339, 0.1191920, 0.9503041)
);

static float3x3 XYZToRGBMatrix = float3x3(
	float3(3.2404542, -1.5371385, -0.4985314),
	float3(-0.9692660, 1.8760108, 0.0415560),
	float3(0.0556434, -0.2040259, 1.0572252)
);

float GetLuminance(float3 rgb)
{
	return dot(rgb, luminanceFactor);
}

float3 ConvertRGB2Yxy(float3 rgb)
{
	float3 XYZ = mul(rgb, RGBToXYZMatrix);
	float XYZSum = dot(XYZ, float3(1, 1, 1));
	float3 Yxy = float3(XYZ.y, XYZ.x / XYZSum, XYZ.y / XYZSum);
	return Yxy;
}

float3 ConvertYxy2RGB(float3 Yxy)
{
	float Y = Yxy.x;
	float x = Yxy.y;
	float y = Yxy.z;

	float X = x * Y / y;
	float Z = (1 - x - y) * Y / y;
	float3 XYZ = float3(X, Y, Z);
	float3 rgb = mul(XYZ, XYZToRGBMatrix);
	return rgb;
}

float3 SimpleReinhard1(float3 rgb)
{
	return rgb / (1.f + rgb);
}

float3 SimpleReinhard2(float3 rgb, float avgLuminance)
{
	float3 Yxy = ConvertRGB2Yxy(rgb);
	float3 luminance = Yxy.x;
	float lp = luminance / (9.6f * avgLuminance);
	Yxy.x = lp / (1 + lp);
	rgb = ConvertYxy2RGB(Yxy);
	return rgb;
}

float3 ExtendedReinhard(float3 rgb, float3 whiteHdrRgb)
{
	float cWhite = 1.f / whiteHdrRgb;
	return rgb * (1.f + rgb * cWhite * cWhite) / (1.f + rgb);
}

float3 ChangeLuminance(float3 rgb, float luminanceOut)
{
	float luminance = GetLuminance(rgb);
	return rgb * (luminanceOut / luminance);
}

float3 LuminanceReinhard(float3 rgb, float whiteLuminance)
{
	float luminance = GetLuminance(rgb);
	float cWhite = 1.f / whiteLuminance;
	float numerator = luminance * (1.0f + (luminance * cWhite * cWhite));
	float newLuminance = numerator / (1.0f + luminance);
	return ChangeLuminance(rgb, newLuminance);
}
