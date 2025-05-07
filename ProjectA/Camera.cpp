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
	m_backBufferRTV(backBufferRTV),
	m_width(viewportWidth),
	m_height(viewportHeight),
	m_blurCount(blurCount),
	m_isPropertiesChanged(false),
	m_cameraSpeed(10.f),
	m_mouseNdcX(0.f), m_mouseNdcY(0.f), 
	m_currentForward(GDirection::GDefaultForward),
	m_currentUp(GDirection::GDefaultUp),
	m_currentRight(GDirection::GDefaultRight)
{
	ZeroMem(m_viewport);
	ZeroMem(m_cameraPropertiesCPU);
	ZeroMem(m_isMoveKeyPressed);

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

ID3D11Buffer* CCamera::GetPropertiesBuffer() const noexcept { return m_propertiesGPU->GetBuffer(); }

ID3D11Texture2D* CCamera::GetRenderTargetTexture() const noexcept { return m_renderTargetTexture->GetTexture2D(); }
ID3D11RenderTargetView* CCamera::GetRenderTargetRTV() const noexcept { return m_renderTargetTexture->GetRTV(); }
ID3D11ShaderResourceView* CCamera::GetRenderTargetSRV() const noexcept { return m_renderTargetTexture->GetSRV(); }
ID3D11Texture2D* CCamera::GetFilteredTexture() const noexcept { return m_filteredTexture->GetTexture2D(); }
ID3D11RenderTargetView* CCamera::GetFilteredRTV() const noexcept { return m_filteredTexture->GetRTV(); }
ID3D11ShaderResourceView* CCamera::GetFilteredSRV() const noexcept { return m_filteredTexture->GetSRV(); }
ID3D11Texture2D* CCamera::GetDepthStencilTexture() const noexcept { return m_depthStencil->GetTexture2D(); }
ID3D11DepthStencilView* CCamera::GetDSV() const noexcept { return m_depthStencil->GetDSV(); }

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

	m_renderTargetTexture = make_unique<Texture2DInstance<RTVOption, SRVOption>>(
		m_width, m_height,
		1, 1, NULL, NULL,
		D3D11_USAGE_DEFAULT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1, 4
	);
	m_renderTargetTexture->InitializeByOption(device, deviceContext);

	m_filteredTexture = make_unique<Texture2DInstance<RTVOption, SRVOption>>(
		m_width, m_height,
		1, 1, NULL, NULL,
		D3D11_USAGE_DEFAULT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1, 4
	);
	m_filteredTexture->InitializeByOption(device, deviceContext);

	m_depthStencil = make_unique<Texture2DInstance<DSVOption>>(
		m_width, m_height,
		1, 1, NULL, NULL,
		D3D11_USAGE_DEFAULT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		1, 4
	);
	m_depthStencil->InitializeByOption(device, deviceContext);
	UINT blurWidth = m_width;
	UINT blurHeight = m_height;

	D3D11_VIEWPORT blurredViewport;
	blurredViewport.TopLeftX = 0.f;
	blurredViewport.TopLeftY = 0.f;
	blurredViewport.MinDepth = 0.f;
	blurredViewport.MaxDepth = 1.f;

	m_blurredViewports.emplace_back(m_viewport);
	for (UINT idx = 0; idx < m_blurCount; ++idx)
	{
		blurWidth /= 2;
		blurHeight /= 2;

		blurredViewport.Width = static_cast<FLOAT>(blurWidth);
		blurredViewport.Height = static_cast<FLOAT>(blurHeight);

		m_blurredTextures.emplace_back(
			make_unique<Texture2DInstance<RTVOption, SRVOption>>(
				blurWidth, blurHeight,
				1, 1, NULL, NULL,
				D3D11_USAGE_DEFAULT,
				DXGI_FORMAT_R8G8B8A8_UNORM,

				1, 4
			)
		);
		m_blurredViewports.emplace_back(blurredViewport);
	}
	for (UINT idx = 0; idx < m_blurCount; ++idx)
	{
		m_blurredTextures[idx]->InitializeByOption(device, deviceContext);
	}

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

void CCamera::ClearCamera(ID3D11DeviceContext* deviceContext)
{
	constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 1.f };

	vector<ID3D11RenderTargetView*> cameraRTVs = { m_renderTargetTexture->GetRTV(), m_filteredTexture->GetRTV() };
	for (auto& mainRTV : cameraRTVs)
	{
		deviceContext->ClearRenderTargetView(mainRTV, clearColor);
	}
	ID3D11DepthStencilView* cameraDSV = m_depthStencil->GetDSV();
	deviceContext->ClearDepthStencilView(cameraDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0x00);

	for (auto& blurredTexture : m_blurredTextures)
	{
		deviceContext->ClearRenderTargetView(blurredTexture->GetRTV(), clearColor);
	}
}

void CCamera::Blur(ID3D11DeviceContext* deviceContext)
{
	vector<ID3D11RenderTargetView*> blurFilteredRTVs;
	vector<ID3D11ShaderResourceView*> blurFilteredSRVs;


	blurFilteredRTVs.reserve(m_blurCount + 1);
	blurFilteredSRVs.reserve(m_blurCount + 1);

	blurFilteredRTVs.emplace_back(m_renderTargetTexture->GetRTV());
	blurFilteredSRVs.emplace_back(m_renderTargetTexture->GetSRV());

	for (auto& blurredTexture : m_blurredTextures)
	{
		blurFilteredRTVs.emplace_back(blurredTexture->GetRTV());
		blurFilteredSRVs.emplace_back(blurredTexture->GetSRV());
	}

	ID3D11Buffer* vertexBuffers[] = {
		CFilterCommonData::GFilterQuadPositionBuffer->GetBuffer(),
		CFilterCommonData::GFilterQuadUVCoordBuffer->GetBuffer()
	};
	ID3D11Buffer* vertexNullBuffers[] = { nullptr, nullptr };
	ID3D11Buffer* indexBuffer = CFilterCommonData::GFilterQuadIndexBuffer->GetBuffer();

	UINT strides[] = { static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMFLOAT2)) };
	UINT nullStrides[] = { NULL, NULL };
	UINT offsets[] = { 0, 0 };
	UINT nullOffsets[] = { NULL, NULL };

	// Blur =========================================================================
	constexpr float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	CFilterCommonData::GFilterBlurPSO->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11RenderTargetView* nullRTV = nullptr;

	for (UINT downIdx = 0; downIdx < m_blurCount; ++downIdx)
	{
		deviceContext->OMSetRenderTargets(1, &blurFilteredRTVs[downIdx + 1], nullptr);
		deviceContext->RSSetViewports(1, &m_blurredViewports[downIdx + 1]);

		deviceContext->PSSetShaderResources(0, 1, &blurFilteredSRVs[downIdx]);

		deviceContext->DrawIndexedInstanced(
			static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
			1, NULL, NULL, NULL
		);

		deviceContext->PSSetShaderResources(0, 1, &nullSRV);
		deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	}

	for (UINT upIdx = 0; upIdx < m_blurCount; ++upIdx)
	{
		deviceContext->OMSetRenderTargets(1, &blurFilteredRTVs[blurFilteredRTVs.size() - 2 - upIdx], nullptr);
		deviceContext->RSSetViewports(1, &m_blurredViewports[m_blurredViewports.size() - 2 - upIdx]);

		deviceContext->PSSetShaderResources(0, 1, &blurFilteredSRVs[blurFilteredSRVs.size() - 1 - upIdx]);

		deviceContext->DrawIndexedInstanced(
			static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
			1, NULL, NULL, NULL
		);

		deviceContext->PSSetShaderResources(0, 1, &nullSRV);
		deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	}

	CFilterCommonData::GFilterBlurPSO->RemovePSO(deviceContext);
	// ======================================================================================

	deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);
}

void CCamera::GammaCorrection(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* vertexBuffers[] = {
		CFilterCommonData::GFilterQuadPositionBuffer->GetBuffer(),
		CFilterCommonData::GFilterQuadUVCoordBuffer->GetBuffer()
	};
	ID3D11Buffer* vertexNullBuffers[] = { nullptr, nullptr };
	ID3D11Buffer* indexBuffer = CFilterCommonData::GFilterQuadIndexBuffer->GetBuffer();

	UINT strides[] = { static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMFLOAT2)) };
	UINT nullStrides[] = { NULL, NULL };
	UINT offsets[] = { 0, 0 };
	UINT nullOffsets[] = { NULL, NULL };

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

	CFilterCommonData::GFilterGammaCorrectionPSO->ApplyPSO(deviceContext);

	ID3D11ShaderResourceView* gammaCorrectionSRV = m_renderTargetTexture->GetSRV();
	ID3D11ShaderResourceView* gammaCorrectionNullSRV = nullptr;
	ID3D11RenderTargetView* gammaCorrectionNullRTV = nullptr;

	deviceContext->OMSetRenderTargets(1, &m_backBufferRTV, nullptr);
	deviceContext->RSSetViewports(1, &m_viewport);
	deviceContext->PSSetShaderResources(0, 1, &gammaCorrectionSRV);

	deviceContext->DrawIndexedInstanced(
		static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
		1, NULL, NULL, NULL
	);

	deviceContext->PSSetShaderResources(0, 1, &gammaCorrectionNullSRV);
	deviceContext->OMSetRenderTargets(1, &gammaCorrectionNullRTV, nullptr);

	CFilterCommonData::GFilterGammaCorrectionPSO->RemovePSO(deviceContext);

	deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);

}
