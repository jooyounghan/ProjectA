#pragma once
#include "Updatable.h"
#include "DynamicBuffer.h"

#include <DirectXMath.h>

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
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		UINT viewportWidth,
		UINT viewportHeight,
		float fovAngle,
		float nearZ,
		float farZ
	) noexcept;
	~CCamera() = default;

protected:
	D3D11_VIEWPORT m_viewport;

public:
	inline const D3D11_VIEWPORT& GetViewport() const noexcept { return m_viewport; }

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
	} m_cameraPropertiesCPU;
	D3D11::CDynamicBuffer m_propertiesGPU;
	bool m_isPropertiesChanged;

public:
	inline bool GetPropertiesChanged() const noexcept { return m_isPropertiesChanged; }
	inline ID3D11Buffer* GetPropertiesBuffer() const noexcept { return m_propertiesGPU.GetBuffer(); }

protected:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTarget;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencil;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_renderTargetSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_renderTargetUAV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

public:
	inline ID3D11Texture2D* GetRenderTargetTexture() const noexcept { return m_renderTarget.Get(); }
	inline ID3D11Texture2D* GetDepthStencilTexture() const noexcept { return m_depthStencil.Get(); }
	inline ID3D11RenderTargetView* GetRTV() const noexcept { return m_renderTargetRTV.Get(); }
	inline ID3D11ShaderResourceView* GetSRV() const noexcept { return m_renderTargetSRV.Get(); }
	inline ID3D11UnorderedAccessView* GetUAV() const noexcept { return m_renderTargetUAV.Get(); }
	inline ID3D11DepthStencilView* GetDSV() const noexcept { return m_depthStencilView.Get(); }

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

};

