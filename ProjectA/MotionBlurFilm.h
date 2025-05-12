#pragma once
#include "BaseFilm.h"
#include "DynamicBuffer.h"
#include "UAVOption.h"

#include <memory>

class CMotionBlurFilm : public CBaseFilm
{
public:
	CMotionBlurFilm(
		UINT samplingCount,
		float dissipationFactor,
		float maxMotionTrailLength,
		UINT width,
		UINT height,
		DXGI_FORMAT sceneFormat,
		UINT bitLevel,
		UINT channelCount
	);
	virtual ~CMotionBlurFilm() override = default;

protected:
	Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption, D3D11::UAVOption> m_motionBlurredFilm;

protected:
	struct
	{
		UINT m_samplingCount;
		float m_dissipationFactor;
		float m_maxMotionTrailLength;
		float dummy;
	} m_motionBlurFilmPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_motionBlurFilmPropertiesGPU;

public:
	virtual void Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext) override;
	virtual void ClearFilm(ID3D11DeviceContext * deviceContext) override;

public:
	virtual void Blend(
		ID3D11DeviceContext* deviceContext,
		CBaseFilm* blendTargetFilm
	) override;
};

