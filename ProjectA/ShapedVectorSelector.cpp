#include "ShapedVectorSelector.h"
#include "MacroUtilities.h"
#include "imgui.h"

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
	const string& selectorName, 
	XMFLOAT3& origin, 
	XMVECTOR& upVector, 
	SShapedVectorProperty& shapedVectorProperty
)
	: BaseSelector<EShapedVector>(selectorName, GShapedVectorStringMaps),
	m_selectorName(selectorName),
	m_origin(origin),
	m_upVector(upVector),
	m_shapedVectorProperty(shapedVectorProperty)
{
}

bool ShapedVectorSelector::SetShapedVectorProperty(EShapedVector selectedShapedVector)
{
	bool isChanged = false;
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
	return isChanged;
}

void ShapedVectorSelector::ResetShapedVector()
{
	AutoZeroMemory(m_origin);
	m_upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	AutoZeroMemory(m_shapedVectorProperty);
}

bool ShapedVectorSelector::SetSphereShapedVector()
{
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &m_origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	m_upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	m_shapedVectorProperty.transformation = XMMatrixTranspose(XMMatrixTranslation(m_origin.x, m_origin.y, m_origin.z));
	m_shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	m_shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_2PI);
	m_shapedVectorProperty.minMaxRadius = minMaxRadius;
	
	return isChanged;
}

bool ShapedVectorSelector::SetHemiSphereShapedVector()
{
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &m_origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", m_upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

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
	m_shapedVectorProperty.minMaxRadius = minMaxRadius;

	return isChanged;
}

bool ShapedVectorSelector::SetConeShapedVector()
{
	static float centerAngle = 15.f;
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &m_origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", m_upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("각도(Up 벡터)", &centerAngle, 0.1f, 0.f, 360.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

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

	m_shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, XM_PIDIV2 - (centerAngle * XM_PI / 180.f));
	m_shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PIDIV2 + (centerAngle * XM_PI / 180.f));
	m_shapedVectorProperty.minMaxRadius = minMaxRadius;

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
