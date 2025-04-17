#pragma once
#include "BaseSelector.h"

enum class EShapedVector
{
	None,
	Sphere,
	HemiSphere,
	Cone
};

struct SShapedVectorProperty
{
	DirectX::XMFLOAT3 origin;
	DirectX::XMFLOAT3 upVector;
	DirectX::XMFLOAT2 minInitRadian;
	DirectX::XMFLOAT2 maxInitRadian;
	DirectX::XMFLOAT2 minMaxRadius;
};

class ShapedVectorSelector : public BaseSelector<EShapedVector>
{
public:
	static std::unordered_map<EShapedVector, std::string> GShapedVectorStringMaps;
	
public:
	static bool SetShapedVectorProperty(EShapedVector selectedShapedVector, SShapedVectorProperty& shapedVectorProperty);

protected:
	static void ResetShapedVector(SShapedVectorProperty& shapedVectorProperty);
	static bool SetSphereShapedVector(SShapedVectorProperty& shapedVectorProperty);
	static bool SetHemiSphereShapedVector(SShapedVectorProperty& shapedVectorProperty);
	static bool SetConeShapedVector(SShapedVectorProperty& shapedVectorProperty);
};

