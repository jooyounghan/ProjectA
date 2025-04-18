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
	DirectX::XMMATRIX transformation;
	DirectX::XMFLOAT2 minInitRadian;
	DirectX::XMFLOAT2 maxInitRadian;
	DirectX::XMFLOAT2 minMaxRadius;
	DirectX::XMFLOAT2 dummy;
};

class ShapedVectorSelector : public BaseSelector<EShapedVector>
{
public:
	static std::unordered_map<EShapedVector, std::string> GShapedVectorStringMaps;
	
public:
	static bool SetShapedVectorProperty(
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		EShapedVector selectedShapedVector, 
		SShapedVectorProperty& shapedVectorProperty
	);

protected:
	static void ResetShapedVector(
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		SShapedVectorProperty& shapedVectorProperty
	);
	static bool SetSphereShapedVector(
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		SShapedVectorProperty& shapedVectorProperty
	);
	static bool SetHemiSphereShapedVector(
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		SShapedVectorProperty& shapedVectorProperty
	);
	static bool SetConeShapedVector(
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		SShapedVectorProperty& shapedVectorProperty
	);

protected:
	static DirectX::XMVECTOR GetRotationQuaternion(
		const DirectX::XMVECTOR& from, 
		const DirectX::XMVECTOR& to
	);
};

