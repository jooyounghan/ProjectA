#include "AEmitter.h"
#include "MacroUtilities.h"

#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "RuntimeSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include <exception>

using namespace std;
using namespace DirectX;
using namespace D3D11;
using namespace ImGui;

AEmitter::AEmitter(
	UINT emitterType,
	UINT emitterID,
	const XMVECTOR& position,
	const XMVECTOR& angle,
	const function<void(UINT, const XMMATRIX&)>& worldTransformChangedHandler,
	const function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
	const function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
) :
	m_emitterType(emitterType),
	m_emitterPropertyCPU{ emitterID, 0, 0, 0 },
	m_isSpawned(false),
	m_position(position),
	m_angle(angle),
	m_isEmitterWorldTransformChanged(false),
	m_colorInterpolaterID(InterpPropertyNotSelect),
	m_onWorldTransformChanged(worldTransformChangedHandler),
	m_onForcePropertyChanged(forcePropertyChangedHandler),
	m_onGpuColorInterpolaterSelected(gpuColorInterpolaterSelectedHandler),
	m_onGpuColorInterpolaterUpdated(gpuColorInterpolaterUpdatedHandler)
{

}

void AEmitter::SetPosition(const XMVECTOR& position) noexcept
{
	m_position = position;
	m_isEmitterWorldTransformChanged = true;
}

void AEmitter::SetAngle(const XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isEmitterWorldTransformChanged = true;
}

void AEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertyCPU));
	m_emitterPropertyGPU->InitializeBuffer(device);

	m_initialSpawnProperty->Initialize(device, deviceContext);
	m_emitterUpdateProperty->Initialize(device, deviceContext);
	m_runtimeSpawnProperty->Initialize(device, deviceContext);
	m_forceUpdateProperty->Initialize(device, deviceContext);
}

#define UPDATE_PROPRTY(PROPERTY, DC, DT) if (PROPERTY) PROPERTY->Update(DC, DT);

void AEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterWorldTransformChanged)
	{
		const UINT& emitterID = m_emitterPropertyCPU.emitterID;
		const XMMATRIX emitterWorldTransform = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);

		m_onWorldTransformChanged(emitterID, emitterWorldTransform);
		m_isEmitterWorldTransformChanged = false;
	}

	m_initialSpawnProperty->Update(deviceContext, dt);
	m_emitterUpdateProperty->Update(deviceContext, dt);
	m_runtimeSpawnProperty->Update(deviceContext, dt);
	m_forceUpdateProperty->Update(deviceContext, dt);
}

void AEmitter::Serialize(std::ofstream& ofs)
{
	m_initialSpawnProperty->Serialize(ofs);
	m_emitterUpdateProperty->Serialize(ofs);
	m_runtimeSpawnProperty->Serialize(ofs);
	m_forceUpdateProperty->Serialize(ofs);
}

void AEmitter::Deserialize(std::ifstream& ifs)
{
	m_initialSpawnProperty->Deserialize(ifs);
	m_emitterUpdateProperty->Deserialize(ifs);
	m_runtimeSpawnProperty->Deserialize(ifs);
	m_forceUpdateProperty->Deserialize(ifs);

	m_isEmitterWorldTransformChanged = true;
}

void AEmitter::DrawUI()
{
	DrawUIImpl();

	m_initialSpawnProperty->DrawUI();
	m_emitterUpdateProperty->DrawUI();
	m_runtimeSpawnProperty->DrawUI();
	m_forceUpdateProperty->DrawUI();
}

void AEmitter::DrawUIImpl()
{
	XMVECTOR position = m_position;
	XMVECTOR angle = m_angle;

	if (DragFloat3("��ġ", position.m128_f32, 0.1f, -1000.f, 1000.f, "%.1f"))
	{
		SetPosition(position);
	}

	angle = XMVectorScale(angle, 180.f / XM_PI);
	if (DragFloat3("����", angle.m128_f32, 0.1f, -360, 360.f, " % .1f"))
	{
		angle = XMVectorScale(angle, XM_PI / 180.f);
		SetAngle(angle);
	}
}
