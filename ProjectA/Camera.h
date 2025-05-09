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
		ID3D11RenderTargetView* backBufferRTV,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		UINT filmWidth,
		UINT filmHeight,
		float fovAngle,
		float nearZ,
		float farZ,
		UINT blurCount
	) noexcept;
	~CCamera() override = default;

protected:
	ShotFilm m_shotFilm;
	UINT m_blurCount;

protected:
	std::vector<AFilm*> m_attachedFilms;

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

public:
	void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

protected:
	void UpdateAngle(int mouseX, int mouseY);
	void UpdateKeyStatus(WPARAM keyInformation, bool isDown);

public:
	void AttachFilm(const std::vector<AFilm*>& films) { m_attachedFilms.insert(m_attachedFilms.end(), films.begin(), films.end()); }
	void ClearFilm(ID3D11DeviceContext* deviceContext) noexcept;
	void DevelopFilm(ID3D11DeviceContext* deviceContext);
	void BlendFilm(ID3D11DeviceContext* deviceContext);

public:
	void ApplyCamera(ID3D11DeviceContext* deviceContext);
	void ClearCamera(ID3D11DeviceContext* deviceContext);
	void Print(ID3D11DeviceContext* deviceContext);
};

