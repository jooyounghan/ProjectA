#pragma once
#include "BaseSelector.h"
#include "ShapedVectorProperty.h"

class CShapedVectorSelector : public CBaseSelector<EShapedVector>
{
public:
	CShapedVectorSelector(
		const std::string& selectorName,
		const std::string& radiusName,
		DirectX::XMFLOAT3& origin,
		DirectX::XMVECTOR& upVector,
		SShapedVectorProperty& shapedVectorProperty
	);
	~CShapedVectorSelector() override = default;

public:
	static std::unordered_map<EShapedVector, std::string> GShapedVectorStringMaps;
	
protected:
	std::string m_selectorName;
	std::string m_radiusName;

protected:
	DirectX::XMFLOAT3& m_origin;
	DirectX::XMVECTOR& m_upVector;
	SShapedVectorProperty& m_shapedVectorProperty;

protected:
	float m_centerAngle;

public:
	bool SetShapedVectorProperty(EShapedVector selectedShapedVector);

protected:
	void ResetShapedVector();
	bool SetManualShapedVector();
	bool SetSphereShapedVector();
	bool SetHemiSphereShapedVector();
	bool SetConeShapedVector();

protected:
	static DirectX::XMVECTOR GetRotationQuaternion(
		const DirectX::XMVECTOR& from, 
		const DirectX::XMVECTOR& to
	);
};

