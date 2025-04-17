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

bool ShapedVectorSelector::SetShapedVectorProperty(EShapedVector selectedShapedVector, SShapedVectorProperty& shapedVectorProperty)
{
	bool isChanged = false;
	switch (selectedShapedVector)
	{
	case EShapedVector::None:
		ShapedVectorSelector::ResetShapedVector(shapedVectorProperty);
		break;
	case EShapedVector::Sphere:
		isChanged |= ShapedVectorSelector::SetSphereShapedVector(shapedVectorProperty);
		break;
	case EShapedVector::HemiSphere:
		isChanged |= ShapedVectorSelector::SetHemiSphereShapedVector(shapedVectorProperty);
		break;
	case EShapedVector::Cone:
		isChanged |= ShapedVectorSelector::SetConeShapedVector(shapedVectorProperty);
		break;
	}
	return isChanged;
}

void ShapedVectorSelector::ResetShapedVector(SShapedVectorProperty& shapedVectorProperty)
{
	AutoZeroMemory(shapedVectorProperty);
}

bool ShapedVectorSelector::SetSphereShapedVector(SShapedVectorProperty& shapedVectorProperty)
{
	static XMFLOAT3 origin = XMFLOAT3(0.f, 0.f, 0.f);
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	shapedVectorProperty.origin = origin;
	shapedVectorProperty.upVector = XMFLOAT3(0.f, 1.f, 0.f);
	shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_2PI);
	shapedVectorProperty.minMaxRadius = minMaxRadius;
	
	return isChanged;
}

bool ShapedVectorSelector::SetHemiSphereShapedVector(SShapedVectorProperty& shapedVectorProperty)
{
	static XMFLOAT3 origin = XMFLOAT3(0.f, 0.f, 0.f);
	static XMVECTOR upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	XMVector3Normalize(upVector);

	shapedVectorProperty.origin = origin;
	XMStoreFloat3(&shapedVectorProperty.upVector, upVector);
	shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, 0.f);
	shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PI);
	shapedVectorProperty.minMaxRadius = minMaxRadius;

	return isChanged;
}

bool ShapedVectorSelector::SetConeShapedVector(SShapedVectorProperty& shapedVectorProperty)
{
	static XMFLOAT3 origin = XMFLOAT3(0.f, 0.f, 0.f);
	static XMVECTOR upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	static float centerAngle = 15.f;
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 10.f);

	bool isChanged = false;

	isChanged |= DragFloat3("구 중심", &origin.x, 0.1f, -1000.f, 1000.f, "%.1f");
	isChanged |= DragFloat3("Up 벡터", upVector.m128_f32, 0.01f, -1.f, 1.f, "%.2f");
	isChanged |= DragFloat("각도(Up 벡터)", &centerAngle, 0.1f, 0.f, 360.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최소)", &minMaxRadius.x, 0.1f, 0.f, 1000.f, "%.1f");
	isChanged |= DragFloat("구 반지름(최대)", &minMaxRadius.y, 0.1f, minMaxRadius.x, 1000.f, "%.1f");

	XMVector3Normalize(upVector);

	shapedVectorProperty.origin = origin;
	XMStoreFloat3(&shapedVectorProperty.upVector, upVector);
	shapedVectorProperty.minInitRadian = XMFLOAT2(0.f, XM_PIDIV2 - (centerAngle * XM_PI / 180.f));
	shapedVectorProperty.maxInitRadian = XMFLOAT2(XM_2PI, XM_PIDIV2 + (centerAngle * XM_PI / 180.f));
	shapedVectorProperty.minMaxRadius = minMaxRadius;

	return isChanged;
}
