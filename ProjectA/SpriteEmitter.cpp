#include "SpriteEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "SpriteRuntimeSpawnProperty.h"
#include "ForceUpdateProperty.h"
#include "EmitterTypeDefinition.h"

#include "stb_image.h"
#include "stb_image_resize2.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;
using namespace ImGui;

SpriteEmitter::SpriteEmitter(
	UINT emitterID, 
	const DirectX::XMVECTOR& position, 
	const DirectX::XMVECTOR& angle, 
	const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler, 
	const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, const XMFLOAT2&, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterUpdatedHandler,
	const std::function<void(UINT, unsigned char*, UINT, UINT, UINT)>& spriteTextureLoadedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::SpriteEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler,
		gpuColorInterpolaterSelectedHandler,
		gpuColorInterpolaterUpdatedHandler
	),
	m_spriteSizeInterpolaterID(InterpPropertyNotSelect),
	m_onSpriteSizeInterpolaterSelected(gpuSpriteSizeInterpolaterSelectedHandler),
	m_onSpriteSizeInterpolaterUpdated(gpuSpriteSizeInterpolaterUpdatedHandler),
	m_spriteIndexInterpolaterID(InterpPropertyNotSelect),
	m_onSpriteIndexInterpolaterSelected(gpuSpriteIndexInterpolaterSelectedHandler),
	m_onSpriteIndexInterpolaterUpdated(gpuSpriteIndexInterpolaterUpdatedHandler),
	m_spriteTextureAspectRatio(1.f),
	m_onSpriteTextureLoaded(spriteTextureLoadedHandler)
{
}


void SpriteEmitter::LoadSpriteTexture(const string& spriteTextureDirectory)
{
	if (!spriteTextureDirectory.empty())
	{
		m_spriteTextureDirectory = spriteTextureDirectory;

		int width, height, channel;
		stbi_uc* spriteTextureBuffer = stbi_load(m_spriteTextureDirectory.c_str(), &width, &height, &channel, 4);
		m_spriteTextureAspectRatio = width / float(height);
		m_onSpriteTextureLoaded(GetEmitterID(), spriteTextureBuffer, width, height, channel);
		stbi_image_free(spriteTextureBuffer);
	}
}

void SpriteEmitter::SetSpriteTextureSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>&& spriteTextureSRV)
{
	m_spriteTextureSRV = move(spriteTextureSRV);
}

void SpriteEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<CSpriteRuntimeSpawnProperty>(
		[this](bool isColorGPUInterpolaterOn, EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater) 
		{ 
			m_initialSpawnProperty->SetUseInitialColor(!isColorGPUInterpolaterOn);
			m_onGpuColorInterpolaterSelected(GetEmitterID(), m_colorInterpolaterID, isColorGPUInterpolaterOn, colorIntperpolationMethod, colorInterpolater);
		},
		[this](bool isColorGPUInterpolaterOn, float maxLife, EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater)
		{
			m_onGpuColorInterpolaterUpdated(GetEmitterID(), m_colorInterpolaterID, isColorGPUInterpolaterOn, maxLife, colorIntperpolationMethod, colorInterpolater);
		},
		[this](bool isSpriteSizeGPUInterpolaterOn, EInterpolationMethod spriteSizeIntperpolationMethod, IInterpolater<2>* spriteSizeInterpolater)
		{
			m_initialSpawnProperty->SetUseInitialSize(!isSpriteSizeGPUInterpolaterOn);
			m_onSpriteSizeInterpolaterSelected(GetEmitterID(), m_spriteSizeInterpolaterID, isSpriteSizeGPUInterpolaterOn, spriteSizeIntperpolationMethod, spriteSizeInterpolater);
		},
		[this](bool isSpriteSizeGPUInterpolaterOn, float maxLife, EInterpolationMethod spriteSizeIntperpolationMethod, IInterpolater<2>* spriteSizeInterpolater) 
		{ 
			m_onSpriteSizeInterpolaterUpdated(GetEmitterID(), m_spriteSizeInterpolaterID, isSpriteSizeGPUInterpolaterOn, maxLife, spriteSizeIntperpolationMethod, spriteSizeInterpolater);
		},
		[this](bool isSpriteIndexGPUInterpolaterOn, EInterpolationMethod spriteIndexIntperpolationMethod, IInterpolater<1>* spriteIndexInterpolater)
		{
			m_onSpriteIndexInterpolaterSelected(GetEmitterID(), m_spriteIndexInterpolaterID, isSpriteIndexGPUInterpolaterOn, spriteIndexIntperpolationMethod, spriteIndexInterpolater);
		},
			[this](bool isSpriteIndexGPUInterpolaterOn, float maxLife, const XMFLOAT2& spriteTextureCount, EInterpolationMethod spriteIndexIntperpolationMethod, IInterpolater<1>* spriteIndexInterpolater)
		{
			m_onSpriteIndexInterpolaterUpdated(GetEmitterID(), m_spriteIndexInterpolaterID, isSpriteIndexGPUInterpolaterOn, maxLife, spriteTextureCount, spriteIndexIntperpolationMethod, spriteIndexInterpolater);
		}
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);
}

void SpriteEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	AEmitter::Initialize(device, deviceContext);
	LoadSpriteTexture(m_spriteTextureDirectory);
}

void SpriteEmitter::DrawUIImpl()
{
	AEmitter::DrawUIImpl();

	SeparatorText("스프라이트 텍스쳐");
	ImVec2 regionAvailSize = GetContentRegionAvail();
	if (!m_spriteTextureSRV)
	{

	}
	else
	{
		Image((ImTextureID)m_spriteTextureSRV.Get(), ImVec2(regionAvailSize.x, regionAvailSize.x / m_spriteTextureAspectRatio));
	}
}

void SpriteEmitter::Serialize(std::ofstream& ofs)
{
	AEmitter::Serialize(ofs);
	SerializeHelper::SerializeString(ofs, m_spriteTextureDirectory);
}

void SpriteEmitter::Deserialize(std::ifstream& ifs)
{
	AEmitter::Deserialize(ifs);
	m_spriteTextureDirectory = SerializeHelper::DeserializeString(ifs);
}
