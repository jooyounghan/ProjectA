#pragma once
#include <DirectXMath.h>

enum class EShapedVector
{
	None,
	Manual,
	Sphere,
	HemiSphere,
	Cone
};

struct SShapedVectorProperty
{
	DirectX::XMMATRIX transformation;
	DirectX::XMFLOAT2 minInitRadian;
	DirectX::XMFLOAT2 maxInitRadian;
	DirectX::XMFLOAT2 minMaxRadius;
};
