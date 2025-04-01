#pragma once
#include "DynamicBuffer.h"

#include <DirectXMath.h>

enum class EKey
{
	W,
	D,
	S,
	A,
};

class Camera
{
public:
	Camera(
		const DirectX::XMVECTOR& positionIn,
		const DirectX::XMVECTOR& angleIn,
		UINT viewportWidth,
		UINT viewportHeight,
		float fovAngle,
		float nearZ,
		float farZ
	) noexcept;
	~Camera() = default;

protected:
	D3D11_VIEWPORT m_viewport;

protected:
	struct
	{
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMVECTOR position;
		DirectX::XMVECTOR angle;
	} m_cameraViewPropertiesCPU;
	D3D11::CDynamicBuffer m_cameraViewPropertiesGPU;
	bool m_isViewChanged;

protected:
	struct
	{
		DirectX::XMMATRIX projMatrix;
		float fovAngle;
		float nearZ;
		float farZ;
		float dummy;
	} m_cameraProjPropertiesCPU;
	D3D11::CDynamicBuffer m_cameraProjPropertiesGPU;
	bool m_isProjChanged;

public:
	inline bool GetViewChanged() noexcept { return m_isViewChanged; }
	inline bool GetProjChanged() noexcept { return m_isProjChanged; }

	inline ID3D11Buffer* GetViewBuffer() noexcept { return m_cameraViewPropertiesGPU.GetBuffer(); }
	inline ID3D11Buffer* GetProjBuffer() noexcept { return m_cameraProjPropertiesGPU.GetBuffer(); }


protected:
	float m_cameraSpeed;
	float m_mouseNdcX;
	float m_mouseNdcY;
	bool m_isFirstViewOptionOn;
	bool m_isMoveKeyPressed[4];

protected:
	static const DirectX::XMVECTOR GDefaultForward;
	static const DirectX::XMVECTOR GDefaultUp;
	static const DirectX::XMVECTOR GDefaultRight;
	DirectX::XMVECTOR m_currentForward;
	DirectX::XMVECTOR m_currentUp;
	DirectX::XMVECTOR m_currentRight;

public:
	void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);
	void UpdateCamera(ID3D11DeviceContext* deviceContext, float dt);

protected:
	void UpdateViewMatrix(ID3D11DeviceContext* deviceContext) noexcept;
	void UpdateProjMatrix(ID3D11DeviceContext* deviceContext) noexcept;
	void UpdateAngle(int mouseX, int mouseY);
	void UpdateKeyStatus(WPARAM keyInformation, bool isDown);

};

