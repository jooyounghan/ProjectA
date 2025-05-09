#pragma once
#include "AFilm.h"
#include "DynamicBuffer.h"
#include "UAVOption.h"

#include <memory>

class MotionBlurFilm : public AFilm
{
public:
	MotionBlurFilm(
		UINT samplingCount,
		float dissipationFactor,
		UINT width,
		UINT height,
		DXGI_FORMAT sceneFormat,
		UINT bitLevel,
		UINT channelCount
	);
	virtual ~MotionBlurFilm() override = default;

protected:
	Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption, D3D11::UAVOption> m_motionBlurredFilm;

protected:
	struct
	{
		UINT m_samplingCount;
		float m_dissipationFactor;
		float dummy[2];
	} m_motionBlurFilmPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_motionBlurFilmPropertiesGPU;

public:
	virtual void Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext) override;
	virtual void ClearFilm(ID3D11DeviceContext * deviceContext) override;

public:
	virtual void Blend(
		ID3D11DeviceContext* deviceContext,
		AFilm* blendTargetFilm,
		const D3D11_VIEWPORT & blendTargetViewport
	) override;
};

