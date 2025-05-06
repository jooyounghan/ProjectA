#pragma once
#include "IUpdatable.h"

#include "Texture2DInstance.h"
#include "RTVOption.h"
#include "SRVOption.h"
#include "DSVOption.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace D3D11
{
	class CDynamicBuffer;
}

enum class EKey
{
	W,
	D,
	S,
	A,
	F
};

class CCamera : public IUpdatable
{
public:
	CCamera(
		ID3D11RenderTargetView* backBufferRTV,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		UINT viewportWidth,
		UINT viewportHeight,
		float fovAngle,
		float nearZ,
		float farZ,
		UINT blurCount
	) noexcept;
	~CCamera() override = default;

protected:
	ID3D11RenderTargetView* m_backBufferRTV;
	D3D11_VIEWPORT m_viewport;

public:
	inline const D3D11_VIEWPORT& GetViewport() const noexcept { return m_viewport; }

protected:
	UINT m_width;
	UINT m_height;
	UINT m_blurCount;

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;
	float m_fovAngle;
	float m_nearZ;
	float m_farZ;

protected:
	struct
	{
		DirectX::XMMATRIX viewProjMatrix;
		DirectX::XMMATRIX invTransposeViewMatrix;
	} m_cameraPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_propertiesGPU;
	bool m_isPropertiesChanged;

public:
	ID3D11Buffer* GetPropertiesBuffer() const noexcept;

protected:
	std::unique_ptr<Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption>> m_renderTargetTexture;
	std::unique_ptr<Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption>> m_filteredTexture;
	std::unique_ptr<Texture2DInstance<D3D11::DSVOption>> m_depthStencil;

protected:
	std::vector<std::unique_ptr<Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption>>> m_blurredTextures;
	std::vector<D3D11_VIEWPORT> m_blurredViewports;

public:
	ID3D11Texture2D* GetRenderTargetTexture() const noexcept;
	ID3D11RenderTargetView* GetRenderTargetRTV() const noexcept;
	ID3D11ShaderResourceView* GetRenderTargetSRV() const noexcept;
public:
	ID3D11Texture2D* GetFilteredTexture() const noexcept;
	ID3D11RenderTargetView* GetFilteredRTV() const noexcept;
	ID3D11ShaderResourceView* GetFilteredSRV() const noexcept;

public:
	ID3D11Texture2D* GetDepthStencilTexture() const noexcept;
	ID3D11DepthStencilView* GetDSV() const noexcept;

protected:
	float m_cameraSpeed;
	float m_mouseNdcX;
	float m_mouseNdcY;
	bool m_isMoveKeyPressed[5];

protected:
	DirectX::XMVECTOR m_currentForward;
	DirectX::XMVECTOR m_currentUp;
	DirectX::XMVECTOR m_currentRight;

public:
	void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

protected:
	void UpdateAngle(int mouseX, int mouseY);
	void UpdateKeyStatus(WPARAM keyInformation, bool isDown);


public:
	void ClearCamera(ID3D11DeviceContext* deviceContext);
	void Blur(ID3D11DeviceContext* deviceContext);
	void GammaCorrection(ID3D11DeviceContext* deviceContext);
};

