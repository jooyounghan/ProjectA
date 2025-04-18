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

bool ShapedVectorSelector::SetShapedVectorProperty(
	XMFLOAT3& origin,
	XMVECTOR& upVector,
	EShapedVector selectedShapedVector, 
	SShapedVectorProperty& shapedVectorProperty
)
{
	bool isChanged = false;
	switch (selectedShapedVector)
	{
	case EShapedVector::None:
		ShapedVectorSelector::ResetShapedVector(origin, upVector, shapedVectorProperty);
		break;
	case EShapedVector::Sphere:
		isChanged |= ShapedVectorSelector::SetSphereShapedVector(origin, upVector, shapedVectorProperty);
		break;
	case EShapedVector::HemiSphere:
		isChanged |= ShapedVectorSelector::SetHemiSphereShapedVector(origin, upVector, shapedVectorProperty);
		break;
	case EShapedVector::Cone:
		isChanged |= ShapedVectorSelector::SetConeShapedVector(origin, upVector, shapedVectorProperty);
		break;
	}
	return isChanged;
}

void ShapedVectorSelector::ResetShapedVector(
	XMFLOAT3& origin,
	XMVECTOR& upVector,
	SShapedVectorProperty& shapedVectorProperty
)
{
	AutoZeroMemory(origin);
	upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	AutoZeroMemory(shapedVectorProperty);
}

bool ShapedVectorSelector::SetSphereShapedVector(
	XMFLOAT3& origin,
	XMVECTOR& upVector,
	SShapedVectorProperty& shapedVectorProperty
)
{
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	shapedVectorProperty.transformation = XMMatrixTranspose(XMMatrixTranslation(origin.x, origin.y, origin.z));
	shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_2PI);
	shapedVectorProperty.minMaxRadius = minMaxRadius;
	
	return isChanged;
}

bool ShapedVectorSelector::SetHemiSphereShapedVector(
	XMFLOAT3& origin,
	XMVECTOR& upVector,
	SShapedVectorProperty& shapedVectorProperty
)
{
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	XMVector3Normalize(upVector);
	XMVECTOR defaultUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR originVector = XMLoadFloat3(&origin);

	shapedVectorProperty.transformation = XMMatrixTranspose(
		XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			GetRotationQuaternion(defaultUp, upVector),
			originVector
		)
	);
	shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PI);
	shapedVectorProperty.minMaxRadius = minMaxRadius;

	return isChanged;
}

bool ShapedVectorSelector::SetConeShapedVector(
	XMFLOAT3& origin,
	XMVECTOR& upVector,
	SShapedVectorProperty& shapedVectorProperty
)
{
	static float centerAngle = 15.f;
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("각도(Up 벡터)", &centerAngle, 0.1f, 0.f, 360.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	XMVector3Normalize(upVector);
	XMVECTOR defaultUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR originVector = XMLoadFloat3(&origin);

	shapedVectorProperty.transformation = XMMatrixTranspose(
		XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			GetRotationQuaternion(defaultUp, upVector),
			originVector
		)
	);

	shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, XM_PIDIV2 - (centerAngle * XM_PI / 180.f));
	shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PIDIV2 + (centerAngle * XM_PI / 180.f));
	shapedVectorProperty.minMaxRadius = minMaxRadius;

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
