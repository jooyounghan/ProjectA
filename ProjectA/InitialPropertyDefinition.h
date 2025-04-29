#pragma once
#include "ShapedVectorProperty.h"
#include <DirectXMath.h>

#define InitLife 10.f
#define LoopInfinity static_cast<UINT8>(~0)
#define InitPositionShapedVector EShapedVector::Sphere
#define InitSpeedShapedVector EShapedVector::None
#define InitOrigin DirectX::XMFLOAT3(0.f, 0.f, 0.f)
#define InitUpVector DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f)
#define InitXYScale DirectX::XMFLOAT2(0.f, 0.f)
#define InitColor DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f)