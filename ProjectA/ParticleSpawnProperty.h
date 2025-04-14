#pragma once
#include "Updatable.h"
#include "DynamicBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>

struct SEmitRate
{
	float time;
	int emitRate;
};

class CParticleSpawnProperty : public IUpdatable
{
public:
	CParticleSpawnProperty(
		const DirectX::XMFLOAT2& minEmitRadians,
		const DirectX::XMFLOAT2& maxEmitRadians,
		const DirectX::XMFLOAT2& minMaxEmitSpeeds,
		const std::vector<SEmitRate>& emitRateProfiles = std::vector<SEmitRate>(),
		bool isPlayLoop = false,
		float loopTime = 0.f
	);
	~CParticleSpawnProperty() = default;

protected:
	bool m_isPlayLoop;
	float m_loopTime;
	float m_currentPlayTime = 0.f;
	UINT m_currentEmitRate = 0;
	UINT m_frameEmitCount = 0;
	UINT m_emittedCountPerSec = 0;
	std::vector<SEmitRate> m_emitRateProfiles;

public:
	inline void SetLoopPlay(bool isPlayLoop, float loopTime = 0.f) { m_isPlayLoop = isPlayLoop; m_loopTime = loopTime; }
	inline void SetEmitRateProfiles(const std::vector<SEmitRate>& emitRateProfiles) { m_emitRateProfiles = emitRateProfiles; }

public:
	inline UINT GetCurrentEmitRate() const noexcept { return m_currentEmitRate; }

protected:
	struct
	{
		DirectX::XMFLOAT2 minEmitRadians;
		DirectX::XMFLOAT2 maxEmitRadians;
		DirectX::XMFLOAT2 minMaxEmitSpeeds;
		DirectX::XMFLOAT2 dummy;
	} m_particleSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_particleSpawnPropertyGPU;
	bool m_isParticleSpawnPropertyChanged = false;

public:
	inline ID3D11Buffer* GetParticleSpawnPropertyBuffer() const noexcept { return m_particleSpawnPropertyGPU->GetBuffer(); }

public:
	void SetMinEmitRadians(const DirectX::XMFLOAT2& minEmitRadians);
	void SetMaxEmitRadians(const DirectX::XMFLOAT2& maxEmitRadians);
	void SetMinMaxEmitSpeed(const DirectX::XMFLOAT2& minMaxEmitSpeeds);

public:
	inline const DirectX::XMFLOAT2& GetMinEmitRadians() const noexcept { return m_particleSpawnPropertyCPU.minEmitRadians; }
	inline const DirectX::XMFLOAT2& GetMaxEmitRadians() const noexcept { return m_particleSpawnPropertyCPU.maxEmitRadians; }
	inline const DirectX::XMFLOAT2& GetMinMaxEmitSpeed() const noexcept { return m_particleSpawnPropertyCPU.minMaxEmitSpeeds; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

private:
	void UpdateEmitCount(float dt);

};

