#include "ShapedVectorSelector.h"
#include "MacroUtilities.h"
#include "imgui.h"

#include <format>

using namespace std;
using namespace DirectX;
using namespace ImGui;

unordered_map<EShapedVector, string> ShapedVectorSelector::GShapedVectorStringMaps
{
	{ EShapedVector::None, "선택 안함" },
	{ EShapedVector::Sphere, "Sphere" },
	{ EShapedVector::HemiSphere, "HemiSphere" },
	{ EShapedVector::Cone, "Cone" }
};

ShapedVectorSelector::ShapedVectorSelector(
	const std::string& selectorName,
	const std::string& radiusName,
	XMFLOAT3& origin, 
	XMVECTOR& upVector, 
	SShapedVectorProperty& shapedVectorProperty
)
	: BaseSelector<EShapedVector>(selectorName, GShapedVectorStringMaps),
	m_selectorName(selectorName),
	m_radiusName(radiusName),
	m_origin(origin),
	m_upVector(upVector),
	m_shapedVectorProperty(shapedVectorProperty),
	m_centerAngle(15.f)
{

}

bool ShapedVectorSelector::SetShapedVectorProperty(
	const string& shapedVectorName, 
	EShapedVector selectedShapedVector
)
{
	bool isChanged = false;

	PushID(format("{}SetVectorProperty", shapedVectorName).c_str());
	switch (selectedShapedVector)
	{
	case EShapedVector::None:
		ShapedVectorSelector::ResetShapedVector();
		break;
	case EShapedVector::Sphere:
		isChanged |= ShapedVectorSelector::SetSphereShapedVector();
		break;
	case EShapedVector::HemiSphere:
		isChanged |= ShapedVectorSelector::SetHemiSphereShapedVector();
		break;
	case EShapedVector::Cone:
		isChanged |= ShapedVectorSelector::SetConeShapedVector();
		break;
	}
	PopID();

	return isChanged;
}

void ShapedVectorSelector::ResetShapedVector()
{
	AutoZeroMemory(m_origin);
	m_upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	m_shapedVectorProperty.transformation = XMMatrixIdentity();
	m_shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	m_shapedVectorProperty.maxInitRadian = XMFLOAT2(0.f, 0.f);
	m_shapedVectorProperty.minMaxRadius = XMFLOAT2(0.f, 0.f);
}

bool ShapedVectorSelector::SetSphereShapedVector()
{
	bool isChanged = false;

	isChanged |= DragFloat3(format("{} 중심", m_selectorName).c_str(), &m_origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat(format("최소 {}", m_radiusName).c_str(), &m_shapedVectorProperty.minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat(format("최대 {}", m_radiusName).c_str(), &m_shapedVectorProperty.minMaxRadius.y, 0.1f, m_shapedVectorProperty.minMaxRadius.x, 1000.f, "%.1f");

	m_upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	m_shapedVectorProperty.transformation = XMMatrixTranspose(XMMatrixTranslation(m_origin.x, m_origin.y, m_origin.z));
	m_shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	m_shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_2PI);
	
	return isChanged;
}

bool ShapedVectorSelector::SetHemiSphereShapedVector()
{
	bool isChanged = false;

	isChanged |= DragFloat3(format("{} 중심", m_selectorName).c_str(), &m_origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", m_upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat(format("최소 {}", m_radiusName).c_str(), &m_shapedVectorProperty.minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat(format("최대 {}", m_radiusName).c_str(), &m_shapedVectorProperty.minMaxRadius.y, 0.1f, m_shapedVectorProperty.minMaxRadius.x, 1000.f, "%.1f");

	XMVector3Normalize(m_upVector);
	XMVECTOR defaultUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR originVector = XMLoadFloat3(&m_origin);

	m_shapedVectorProperty.transformation = XMMatrixTranspose(
		XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			GetRotationQuaternion(defaultUp, m_upVector),
			originVector
		)
	);
	m_shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	m_shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PI);

	return isChanged;
}

bool ShapedVectorSelector::SetConeShapedVector()
{
	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &m_origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", m_upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("각도(Up 벡터)", &m_centerAngle, 0.1f, 0.f, 360.f, "%.1f");
	isChanged |= DragFloat(format("최소 {}", m_radiusName).c_str(), &m_shapedVectorProperty.minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat(format("최대 {}", m_radiusName).c_str(), &m_shapedVectorProperty.minMaxRadius.y, 0.1f, m_shapedVectorProperty.minMaxRadius.x, 1000.f, "%.1f");

	XMVector3Normalize(m_upVector);
	XMVECTOR defaultUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR originVector = XMLoadFloat3(&m_origin);

	m_shapedVectorProperty.transformation = XMMatrixTranspose(
		XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			GetRotationQuaternion(defaultUp, m_upVector),
			originVector
		)
	);

	m_shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, XM_PIDIV2 - (m_centerAngle * XM_PI / 180.f));
	m_shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PIDIV2 + (m_centerAngle * XM_PI / 180.f));

	return isChanged;
}

XMVECTOR ShapedVectorSelector::GetRotationQuaternion(const XMVECTOR& from, const XMVECTOR& to)
{
	XMVECTOR v = XMVector3Normalize(from);
	XMVECTOR d = XMVector3Normalize(to);

	float cosTheta = XMVectorGetX(XMVector3Dot(v, d));
	XMVECTOR axis = XMVector3Cross(v, d);

	if (XMVector3Equal(axis, XMVectorZero()))
	{
		if (cosTheta > 0.9999f)
			return XMQuaternionIdentity();
		else
		{
			axis = XMVector3Normalize(XMVectorSet(1.f, 0.f, 0.f, 0.f));
			if (XMVector3Equal(v, axis))
				axis = XMVectorSet(0.f, 0.f, 1.f, 0.f);
		}
	}

	float angle = acosf(cosTheta);
	return XMQuaternionRotationAxis(XMVector3Normalize(axis), angle);
}
