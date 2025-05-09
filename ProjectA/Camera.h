#pragma once
#include "IUpdatable.h"
#include "ShotFilm.h"

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
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		float fovAngle,
		float nearZ,
		float farZ
	) noexcept;

	~CCamera() override = default;

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
	float m_cameraSpeed;
	float m_mouseNdcX;
	float m_mouseNdcY;
	bool m_isMoveKeyPressed[5];

protected:
	DirectX::XMVECTOR m_currentForward;
	DirectX::XMVECTOR m_currentUp;
	DirectX::XMVECTOR m_currentRight;

protected:
	D3D11_VIEWPORT m_viewport;

public:
	void SetViewport(const D3D11_VIEWPORT& viewport) noexcept;

public:
	void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

protected:
	void UpdateAngle(int mouseX, int mouseY);
	void UpdateKeyStatus(WPARAM keyInformation, bool isDown);
};

