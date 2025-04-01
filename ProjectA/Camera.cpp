#include "Camera.h"

#include "MacroUtilities.h"

#include <Windows.h>
#include <xmmintrin.h>
#include <algorithm>
#include <unordered_map>

using namespace std;
using namespace DirectX;

const XMVECTOR Camera::GDefaultForward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
const XMVECTOR Camera::GDefaultUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
const XMVECTOR Camera::GDefaultRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);

Camera::Camera(
	const XMVECTOR& positionIn, 
	const XMVECTOR& angleIn, 
	UINT viewportWidth, 
	UINT viewportHeight, 
	float fovAngle, 
	float nearZ, 
	float farZ
) noexcept
	: m_cameraViewPropertiesGPU(sizeof(m_cameraViewPropertiesCPU), 1, &m_cameraViewPropertiesGPU),
	m_cameraProjPropertiesGPU(sizeof(m_cameraProjPropertiesCPU), 1, &m_cameraProjPropertiesCPU),
	m_mouseNdcX(0.f), m_mouseNdcY(0.f), m_isFirstViewOptionOn(false),
	m_currentForward(GDefaultForward), m_currentUp(GDefaultUp), m_currentRight(GDefaultRight)
{
	AutoZeroMemory(m_viewport);
	AutoZeroMemory(m_cameraViewPropertiesCPU);
	AutoZeroMemory(m_cameraProjPropertiesCPU);
	AutoZeroMemory(m_isMoveKeyPressed);

	m_cameraViewPropertiesCPU.position = positionIn;
	m_cameraViewPropertiesCPU.angle = angleIn;
	m_isViewChanged = true;

	m_cameraProjPropertiesCPU.fovAngle = fovAngle;
	m_cameraProjPropertiesCPU.nearZ = nearZ;
	m_cameraProjPropertiesCPU.farZ = farZ;
	m_isProjChanged = true;

	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<FLOAT>(viewportWidth);
	m_viewport.Height = static_cast<FLOAT>(viewportHeight);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}

void Camera::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_MOUSEMOVE:
		UpdateAngle(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		UpdateKeyStatus(wParam, true);
	case WM_KEYUP:
		UpdateKeyStatus(wParam, false);
	default:
		break;
	}
}

void Camera::UpdateCamera(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::W)]) m_cameraViewPropertiesCPU.position += m_currentForward * m_cameraSpeed;
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::D)]) m_cameraViewPropertiesCPU.position += m_currentRight * m_cameraSpeed;
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::S)]) m_cameraViewPropertiesCPU.position -= m_currentForward * m_cameraSpeed;
	if (m_isMoveKeyPressed[static_cast<size_t>(EKey::A)]) m_cameraViewPropertiesCPU.position -= m_currentRight * m_cameraSpeed;

	if (m_isViewChanged) UpdateViewMatrix(deviceContext);
	if (m_isProjChanged) UpdateProjMatrix(deviceContext);


}

void Camera::UpdateViewMatrix(ID3D11DeviceContext* deviceContext) noexcept
{
	const XMVECTOR quaternion = XMQuaternionRotationRollPitchYawFromVector(m_cameraViewPropertiesCPU.angle);
	m_currentForward = XMVector3Rotate(GDefaultForward, quaternion);
	m_currentUp = XMVector3Rotate(GDefaultUp, quaternion);
	m_currentRight = XMVector3Rotate(GDefaultRight, quaternion);

	m_cameraViewPropertiesCPU.viewMatrix = XMMatrixLookToLH(
		m_cameraViewPropertiesCPU.position,
		m_currentForward,
		m_currentUp
	);

	m_cameraViewPropertiesGPU.Stage(deviceContext);
	m_cameraViewPropertiesGPU.Upload(deviceContext);
}

void Camera::UpdateProjMatrix(ID3D11DeviceContext* deviceContext) noexcept
{
	m_cameraProjPropertiesCPU.projMatrix = XMMatrixPerspectiveFovLH(
		m_cameraProjPropertiesCPU.fovAngle,
		m_viewport.Width / m_viewport.Height,
		m_cameraProjPropertiesCPU.nearZ,
		m_cameraProjPropertiesCPU.farZ
	);

	m_cameraProjPropertiesGPU.Stage(deviceContext);
	m_cameraProjPropertiesGPU.Upload(deviceContext);
}

void Camera::UpdateAngle(int mouseX, int mouseY)
{
	m_mouseNdcX = mouseX * 2.0f / m_viewport.Width - 1.0f;
	m_mouseNdcY = -mouseY * 2.0f / m_viewport.Height + 1.0f;

	m_mouseNdcX = std::clamp(m_mouseNdcX, -1.0f, 1.0f);
	m_mouseNdcY = std::clamp(m_mouseNdcY, -1.0f, 1.0f);

	if (m_isFirstViewOptionOn)
	{
		m_cameraViewPropertiesCPU.angle = XMVectorAdd(
			m_cameraViewPropertiesCPU.angle, 
			XMVectorSet(-m_mouseNdcY * XM_PIDIV2, m_mouseNdcX * XM_2PI, 0.f, 0.f)
		);
		m_isViewChanged = true;
	}
}

void Camera::UpdateKeyStatus(WPARAM keyInformation, bool isDown)
{
	static const unordered_map<WPARAM, EKey> WinMsgToKeyInput
	{
		{ 0x41, EKey::A }, // A
		{ 0x44, EKey::D }, // D
		{ 0x53, EKey::S }, // S
		{ 0x57, EKey::W } // W
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