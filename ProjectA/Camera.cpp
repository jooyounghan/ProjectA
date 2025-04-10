#include "Camera.h"

#include "TextureUtilities.h"
#include "BufferMacroUtilities.h"
#include "MacroUtilities.h"
#include "GlobalVariable.h"

#include <Windows.h>
#include <xmmintrin.h>
#include <algorithm>
#include <unordered_map>

using namespace std;
using namespace DirectX;
using namespace D3D11;

CCamera::CCamera(
	const XMVECTOR& position, 
	const XMVECTOR& angle, 
	UINT viewportWidth, 
	UINT viewportHeight, 
	float fovAngle, 
	float nearZ, 
	float farZ
) noexcept
	: m_isPropertiesChanged(false),
	m_cameraSpeed(10.f),
	m_mouseNdcX(0.f), m_mouseNdcY(0.f), 
	m_currentForward(GDirection::GDefaultForward),
	m_currentUp(GDirection::GDefaultUp),
	m_currentRight(GDirection::GDefaultRight)
{
	AutoZeroMemory(m_viewport);
	AutoZeroMemory(m_cameraPropertiesCPU);
	AutoZeroMemory(m_isMoveKeyPressed);

	m_position = position;
	m_angle = angle;
	m_fovAngle = fovAngle;
	m_nearZ = nearZ;
	m_farZ = farZ;

	m_isPropertiesChanged = true;

	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<FLOAT>(viewportWidth);
	m_viewport.Height = static_cast<FLOAT>(viewportHeight);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}

void CCamera::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_MOUSEMOVE:
		UpdateAngle(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		UpdateKeyStatus(wParam, true);
		break;
	case WM_KEYUP:
		UpdateKeyStatus(wParam, false);
		break;
	default:
		break;
	}
}

void CCamera::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_propertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_cameraPropertiesCPU));
	m_propertiesGPU->InitializeBuffer(device);

	TextureUtilities::CreateTexture2D(
		static_cast<UINT>(m_viewport.Width),
		static_cast<UINT>(m_viewport.Height), 
		1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, 
		DXGI_FORMAT_R8G8B8A8_UNORM, 
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 
		device, m_renderTarget.GetAddressOf()
	);

	TextureUtilities::CreateTexture2D(
		static_cast<UINT>(m_viewport.Width),
		static_cast<UINT>(m_viewport.Height), 
		1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, 
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_BIND_DEPTH_STENCIL,
		device, m_depthStencil.GetAddressOf()
	);

	TextureUtilities::CreateRenderTargetView(
		device, m_renderTarget.Get(), m_renderTargetRTV.GetAddressOf()
	);
	TextureUtilities::CreateShaderResourceView(
		device, deviceContext, m_renderTarget.Get(), m_renderTargetSRV.GetAddressOf()
	);
	TextureUtilities::CreateUnorderedAccessView(
		device, m_renderTarget.Get(), m_renderTargetUAV.GetAddressOf()
	);
	TextureUtilities::CreateDepthStencilView(
		device, m_depthStencil.Get(), m_depthStencilView.GetAddressOf()
	);
}

void CCamera::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	const XMVECTOR quaternion = XMQuaternionRotationRollPitchYawFromVector(m_angle);
	m_currentForward = XMVector3Rotate(GDirection::GDefaultForward, quaternion);
	m_currentUp = XMVector3Rotate(GDirection::GDefaultUp, quaternion);
	m_currentRight = XMVector3Rotate(GDirection::GDefaultRight, quaternion);

	float cameraSpeed = m_cameraSpeed * dt;
	XMVECTOR movement = XMVectorZero();
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::W)]) movement += m_currentForward * cameraSpeed;
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::D)]) movement += m_currentRight * cameraSpeed;
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::S)]) movement -= m_currentForward * cameraSpeed;
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::A)]) movement -= m_currentRight * cameraSpeed;

	if (XMVectorGetX(XMVector3Length(movement)) > 1E-3)
	{
		m_position = XMVectorAdd(m_position, movement);
		m_isPropertiesChanged = true;
	}
	if (m_isPropertiesChanged)
	{
		XMMATRIX viewMatrix = XMMatrixLookToLH(
			m_position,
			m_currentForward,
			m_currentUp
		);

		XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(
			m_fovAngle,
			m_viewport.Width / m_viewport.Height,
			m_nearZ,
			m_farZ
		);

		XMMATRIX viewProjMatrix = viewMatrix * projMatrix;
		m_cameraPropertiesCPU.viewProjMatrix = XMMatrixTranspose(viewProjMatrix);
		m_cameraPropertiesCPU.invTransposeViewMatrix = XMMatrixInverse(nullptr, viewMatrix);

		m_propertiesGPU->Stage(deviceContext);
		m_propertiesGPU->Upload(deviceContext);
		m_isPropertiesChanged = false;
	}
}

void CCamera::UpdateAngle(int mouseX, int mouseY)
{
	m_mouseNdcX = mouseX * 2.0f / m_viewport.Width - 1.0f;
	m_mouseNdcY = -mouseY * 2.0f / m_viewport.Height + 1.0f;

	m_mouseNdcX = std::clamp(m_mouseNdcX, -1.0f, 1.0f);
	m_mouseNdcY = std::clamp(m_mouseNdcY, -1.0f, 1.0f);

	if (m_isMoveKeyPressed[4])
	{
		m_angle = XMVectorSet(-m_mouseNdcY * XM_PI, m_mouseNdcX * XM_2PI, 0.f, 0.f);
		m_isPropertiesChanged = true;
	}
}

void CCamera::UpdateKeyStatus(WPARAM keyInformation, bool isDown)
{
	static const unordered_map<WPARAM, EKey> WinMsgToKeyInput
	{
		{ 'A', EKey::A}, 
		{ 'D', EKey::D },
		{ 'S', EKey::S },
		{ 'W', EKey::W },
		{ 'F', EKey::F } 
	};

	try
	{
		const EKey& keyIndex = WinMsgToKeyInput.at(keyInformation);
		m_isMoveKeyPressed[static_cast<size_t>(keyIndex)] = isDown;
	}
	catch (const std::exception&)
	{
		// Do Nothing
	}
}