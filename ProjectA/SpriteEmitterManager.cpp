#include "SpriteEmitterManager.h"

#include "EmitterManagerCommonData.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"

#include "ComputeShader.h"
#include "GraphicsPSOObject.h"
#include "ConstantBuffer.h"

#include "SpriteEmitter.h"
#include "GPUInterpPropertyManager.h"
#include "MacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

SpriteEmitterManager::SpriteEmitterManager(
	UINT maxEmitterCount,
	UINT maxParticleCount
)
	: AEmitterManager("SpriteEmitterManager", static_cast<UINT>(EEmitterType::SpriteEmitter), maxEmitterCount, maxParticleCount)
{
	SSpriteInterpInformation spriteInterpInformation;
	AutoZeroMemory(spriteInterpInformation);

	m_emitterInterpInformationCPU.resize(m_maxEmitterCount, spriteInterpInformation);
	m_forcePropertyChangedEmitterIDs.reserve(m_maxEmitterCount);
}

SpriteEmitterManager& SpriteEmitterManager::GetSpriteEmitterManager()
{
	static SpriteEmitterManager spriteEmitterManager(MaxSpriteEmitterCount, MaxParticleCount);
	return spriteEmitterManager;
}

void SpriteEmitterManager::ReclaimEmitterID(UINT emitterID) noexcept
{
	AutoZeroMemory(m_emitterInterpInformationCPU[emitterID]);
	AddInterpolaterInformChangedEmitterID(emitterID);

	AEmitterManager::ReclaimEmitterID(emitterID);
}

UINT SpriteEmitterManager::AddEmitter(DirectX::XMVECTOR position, DirectX::XMVECTOR angle, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	UINT spriteEmitterID = IssueAvailableEmitterID();
	unique_ptr<SpriteEmitter> spriteEmitter = make_unique<SpriteEmitter>(
		spriteEmitterID,
		position, angle,
		[this](UINT emitterID, const XMMATRIX& worldTransform) {
			m_worldTransformCPU[emitterID] = worldTransform;
			AddWorldTransformChangedEmitterID(emitterID);
		},
		[this](UINT emitterID, const SEmitterForceProperty& forceProperty)
		{
			m_forcePropertyCPU[emitterID] = forceProperty;
			AddForceChangedEmitterID(emitterID);
		},
		[this](UINT emitterID, EInterpolationMethod colorInterpolationMethod, bool isColorGPUInterpolaterOn)
		{
			SetColorGPUInterpolateOption(emitterID, colorInterpolationMethod, isColorGPUInterpolaterOn);
		},
		[this](UINT colorInterpolaterID, EInterpolationMethod colorInterpolationMethod, IInterpolater<4>* colorInterpolater)
		{
			UpdateColorGPUInterpolater(colorInterpolaterID, colorInterpolationMethod, colorInterpolater);
		},
		[this](UINT emitterID, EInterpolationMethod spriteSizeInterpolationMethod, bool isspriteSizeGPUInterpolaterOn)
		{
			SetSpriteSizeGPUInterpolateOption(emitterID, spriteSizeInterpolationMethod, isspriteSizeGPUInterpolaterOn);
		},
		[this](UINT spriteSizeInterpolaterID, EInterpolationMethod spriteSizeInterpolationMethod, IInterpolater<2>* spriteSizeInterpolater)
		{
			UpdateSpriteSizeGPUInterpolater(spriteSizeInterpolaterID, spriteSizeInterpolationMethod, spriteSizeInterpolater);
		},
		[this](UINT emitterID, float maxLife, UINT colorInterpolaterID, UINT colorCoefficientCount, UINT spriteSizeInterpolaterID, UINT spriteSizeCoefficientCount)
		{
			m_emitterInterpInformationCPU[emitterID].maxLife = maxLife;
			m_emitterInterpInformationCPU[emitterID].colorInterpolaterID = colorInterpolaterID;
			m_emitterInterpInformationCPU[emitterID].colorInterpolaterDegree = colorCoefficientCount;
			m_emitterInterpInformationCPU[emitterID].spriteSizeInterpolaterID = spriteSizeInterpolaterID;
			m_emitterInterpInformationCPU[emitterID].spriteSizeInterpolaterDegree = spriteSizeCoefficientCount;
			AddInterpolaterInformChangedEmitterID(emitterID);
		}
	);

	spriteEmitter->Initialize(device, deviceContext);

	m_emitters.emplace_back(std::move(spriteEmitter));
	return spriteEmitterID;

}

void SpriteEmitterManager::SetSpriteSizeGPUInterpolateOption(UINT emitterID, EInterpolationMethod spriteSizeInterpolationMethod, bool isSpriteSizeGPUInterpolaterOn)
{

}

void SpriteEmitterManager::UpdateSpriteSizeGPUInterpolater(UINT spriteSizeInterpolaterID, EInterpolationMethod spriteSizeInterpolationMethod, IInterpolater<2>* spriteSizeInterpolater)
{

}

void SpriteEmitterManager::InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	AEmitterManager::InitializeImpl(device, deviceContext);

	m_emitterInterpInformationGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SSpriteInterpInformation)),
		m_maxEmitterCount,
		m_emitterInterpInformationCPU.data()
	);
	m_emitterInterpInformationGPU->InitializeBuffer(device);

	m_spriteSizeD1Dim4PorpertyManager = make_unique<CGPUInterpPropertyManager<2, 2>>(m_maxEmitterCount);
	m_spriteSizeD3Dim4PorpertyManager = make_unique<CGPUInterpPropertyManager<2, 4>>(m_maxEmitterCount);

	m_spriteSizeD1Dim4PorpertyManager->Initialize(device, deviceContext);
	m_spriteSizeD3Dim4PorpertyManager->Initialize(device, deviceContext);
}

void SpriteEmitterManager::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	AEmitterManager::UpdateImpl(deviceContext, dt);

	m_spriteSizeD1Dim4PorpertyManager->Update(deviceContext, dt);
	m_spriteSizeD3Dim4PorpertyManager->Update(deviceContext, dt);
}

void SpriteEmitterManager::InitializeAliveFlag(ID3D11DeviceContext* deviceContext)
{
}

void SpriteEmitterManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
}
