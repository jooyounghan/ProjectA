#include "Camera.h"

#include "TextureUtilities.h"
#include "MacroUtilities.h"
#include "GlobalVariable.h"

#include "ConstantBuffer.h"
#include "DynamicBuffer.h"

#include "FilterCommonData.h"
#include "GraphicsPSOObject.h"

#include <algorithm>
#include <unordered_map>

using namespace std;
using namespace DirectX;
using namespace D3D11;


CCamera::CCamera(
	ID3D11RenderTargetView* backBufferRTV,
	const XMVECTOR& position, 
	const XMVECTOR& angle, 
	UINT viewportWidth, 
	UINT viewportHeight, 
	float fovAngle, 
	float nearZ, 
	float farZ,
	UINT blurCount
) noexcept
	: 
	m_shotFilm(
		viewportWidth, viewportHeight, 
		DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_FORMAT_D24_UNORM_S8_UINT, 
		1, 4, backBufferRTV
	),
	m_blurCount(blurCount),
	m_isPropertiesChanged(false),
	m_cameraSpeed(10.f),
	m_mouseNdcX(0.f), m_mouseNdcY(0.f), 
	m_currentForward(GDirection::GDefaultForward),
	m_currentUp(GDirection::GDefaultUp),
	m_currentRight(GDirection::GDefaultRight)
{
	ZeroMem(m_cameraPropertiesCPU);
	ZeroMem(m_isMoveKeyPressed);

	m_position = position;
	m_angle = angle;
	m_fovAngle = fovAngle;
	m_nearZ = nearZ;
	m_farZ = farZ;

	m_isPropertiesChanged = true;
}

ID3D11Buffer* CCamera::GetPropertiesBuffer() const noexcept { return m_propertiesGPU->GetBuffer(); }

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

	m_shotFilm.Initialize(device, deviceContext);
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
			m_shotFilm.GetFilmAspectRatio(),
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
	m_mouseNdcX = mouseX * 2.0f / m_shotFilm.GetFilmWidth() - 1.0f;
	m_mouseNdcY = -mouseY * 2.0f / m_shotFilm.GetFilmHeight() + 1.0f;

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

void CCamera::ClearFilm(ID3D11DeviceContext* deviceContext) noexcept
{ 	
	for (auto& attachedFilm : m_attachedFilms)
	{
		attachedFilm->ClearFilm(deviceContext);
	}
	m_attachedFilms.clear(); 
}

void CCamera::DevelopFilm(ID3D11DeviceContext* deviceContext)
{
	for (auto& attachedFilm : m_attachedFilms)
	{
		attachedFilm->Develop(deviceContext);
	}
}

void CCamera::BlendFilm(ID3D11DeviceContext* deviceContext)
{
	const D3D11_VIEWPORT& shotFilmViewport = m_shotFilm.GetFilmViewPort();
	ID3D11RenderTargetView* shotFilmRTV = m_shotFilm.GetFilmRTV();
	for (auto& attachedFilm : m_attachedFilms)
	{
		attachedFilm->Blend(deviceContext, &m_shotFilm, shotFilmViewport);
	}
}

void CCamera::ApplyCamera(ID3D11DeviceContext* deviceContext)
{
	vector<ID3D11RenderTargetView*> rtvs = { m_shotFilm.GetFilmRTV() };
	vector<D3D11_VIEWPORT> viewports = { m_shotFilm.GetFilmViewPort() };
	for (auto& attachedFilm : m_attachedFilms)
	{
		rtvs.emplace_back(attachedFilm->GetFilmRTV());
		viewports.emplace_back(attachedFilm->GetFilmViewPort());
	}

	deviceContext->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), m_shotFilm.GetFilmDSV());
	deviceContext->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}

void CCamera::ClearCamera(ID3D11DeviceContext* deviceContext)
{
	m_shotFilm.ClearFilm(deviceContext);
}


void CCamera::Print(ID3D11DeviceContext* deviceContext)
{
	m_shotFilm.Develop(deviceContext);
}
