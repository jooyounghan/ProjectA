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
	ShapedVectorSelector(
		const std::string& selectorName,
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		SShapedVectorProperty& shapedVectorProperty
	);
	~ShapedVectorSelector() override = default;

public:
	static std::unordered_map<EShapedVector, std::string> GShapedVectorStringMaps;
	
protected:
	std::string m_selectorName;
	DirectX::XMFLOAT3& m_origin;
	DirectX::XMVECTOR& m_upVector;
	SShapedVectorProperty& m_shapedVectorProperty;

public:
	bool SetShapedVectorProperty(EShapedVector selectedShapedVector);

protected:
	void ResetShapedVector();
	bool SetSphereShapedVector();
	bool SetHemiSphereShapedVector();
	bool SetConeShapedVector();

protected:
	static DirectX::XMVECTOR GetRotationQuaternion(
		const DirectX::XMVECTOR& from, 
		const DirectX::XMVECTOR& to
	);
};

