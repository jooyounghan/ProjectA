#pragma once
#include "AEmitter.h"
#include "InterpInformation.h"



class SpriteEmitter : public AEmitter
{
public:
	SpriteEmitter(
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
		const std::function<void(UINT, UINT, bool, float, UINT, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterUpdatedHandler,
		const std::function<void(UINT, unsigned char*, UINT, UINT, UINT)>& spriteTextureLoadedHandler
	);
	~SpriteEmitter() override = default;


protected:
	UINT m_spriteSizeInterpolaterID;
	std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterSelected;
	std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterUpdated;

protected:
	UINT m_spriteIndexInterpolaterID;
	std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<1>*)> m_onSpriteIndexInterpolaterSelected;
	std::function<void(UINT, UINT, bool, float, UINT, EInterpolationMethod, IInterpolater<1>*)> m_onSpriteIndexInterpolaterUpdated;

public:
	inline void SetSpriteSizeInterpolaterID(UINT spriteSizeInterpolaterID) noexcept { m_spriteSizeInterpolaterID = spriteSizeInterpolaterID; }
	inline void SetSpriteIndexInterpolaterID(UINT spriteIndexInterpolaterID) noexcept { m_spriteIndexInterpolaterID = spriteIndexInterpolaterID; }

protected:
	std::string m_spriteTextureDirectory;
	float m_spriteTextureAspectRatio;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_spriteTextureSRV;

protected:
	std::function<void(UINT, unsigned char*, UINT, UINT, UINT)> m_onSpriteTextureLoaded;

protected:
	void LoadSpriteTexture(const std::string& spriteTextureDirectory);

public:
	void SetSpriteTextureSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>&& spriteTextureSRV);

public:
	virtual void CreateProperty() override;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

protected:
	virtual void DrawUIImpl() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

