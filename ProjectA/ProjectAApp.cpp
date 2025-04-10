#include "ProjectAApp.h"

#include "GEngine.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#pragma region Test
#include "MacroUtilities.h"

#include "GraphicsPSOObject.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"

#include "Camera.h"
#include "ParticleManager.h"

#include <sstream>
#pragma  endregion

#pragma execution_character_set("utf-8")

using namespace std;
using namespace App;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

CProjectAApp::CProjectAApp() noexcept
	: CBaseApp(), m_drawParticleVS(1),
	m_appParamsGPU(sizeof(m_appParamsCPU), 1, &m_appParamsCPU)
{
}

CProjectAApp* CProjectAApp::GetInstance() noexcept
{
	static CProjectAApp ProjectAApp;
	return &ProjectAApp;
}

void CProjectAApp::Create(
	UINT width,
	UINT height,
	const wchar_t* className,
	const wchar_t* applicaitonName
) noexcept
{
	CBaseApp::Create(width, height, className, applicaitonName);
}

void CProjectAApp::Init()
{
#pragma region App �ʱ�ȭ
	GEngine* engine = GEngine::GetInstance();

	m_device = engine->GetDevice();
	m_deviceContext = engine->GetDeviceContext();
	m_backBuffer = engine->GetBackBufferTexture();
	m_backBufferRTV = engine->GetBackBufferRTV();
	m_swapchain = engine->GetSwapChain();

	m_wndSizeUpdateHandler = [&, engine](UINT width, UINT height)
	{
		m_width = width;
		m_height = height;
		m_swapchain->ResizeBuffers(
			engine->GetBackBufferCount(),
			m_width, m_height,
			engine->GetBackBufferFormat(),
			engine->GetSwapChainFlag());
	};

	m_appParamsGPU.InitializeBuffer(m_device);

	CRasterizerState::InitializeDefaultRasterizerStates(m_device);
	CBlendState::InitializeDefaultBlendStates(m_device);
	CDepthStencilState::InitializeDefaultDepthStencilState(m_device);
	CSamplerState::InitializeSamplerState(m_device);

#pragma endregion

#pragma region ImGui �ʱ�ȭ
	IMGUI_CHECKVERSION();

	CreateContext();
	StyleColorsDark();

	ImGuiIO& io = GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesKorean());

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(m_mainWindow);
	ImGui_ImplDX11_Init(m_device, m_deviceContext);

	// ImGuiIO, Font Initializing
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	NewFrame();
	EndFrame();
#pragma endregion

#pragma region �׽�Ʈ �ʱ�ȭ
	CParticleManager::InitializeEmitterDrawPSO(m_device);
	CParticleManager::InitializePoolingPSO(m_device);
	CParticleManager::InitializeEmitterSourcingPSO(m_device);
	CParticleManager::InitializeParticleDrawPSO(m_device);
#pragma endregion

#pragma region �ν��Ͻ� �ʱ�ȭ
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		m_width, m_height, 90.f, 0.01f, 100000.000f
		);

	m_particleManager = make_unique<CParticleManager>(500, 1024 * 1024);
	
	m_particleManager->AddParticleEmitter(
		0 /* Emitter Type */,
		XMVectorSet(0.f, 0.f, 2.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		m_device, m_deviceContext
	);

	m_updatables.emplace_back(m_camera.get());
	m_updatables.emplace_back(m_particleManager.get());
	for (auto& updatable : m_updatables)
	{
		updatable->Initialize(m_device, m_deviceContext);
	}
#pragma endregion

#pragma endregion
}

constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 1.f };

void CProjectAApp::Update(float deltaTime)
{
#pragma region �� ���� ������Ʈ
	m_appParamsCPU.dt = deltaTime;
	m_appParamsCPU.appWidth = static_cast<float>(m_width);
	m_appParamsCPU.appHeight = static_cast<float>(m_height);
	m_appParamsCPU.particleMaxCount = m_particleManager->GetParticleMaxCount();
	m_appParamsGPU.Stage(m_deviceContext);
	m_appParamsGPU.Upload(m_deviceContext);
#pragma endregion

#pragma region �ν��Ͻ� ������Ʈ
	for (auto& updatable : m_updatables)
	{
		updatable->Update(m_deviceContext, deltaTime);
	}
#pragma endregion

#pragma region ī�޶� �ʱ�ȭ �� ����
	static vector<ID3D11RenderTargetView*> mainRTVs = { m_camera->GetRTV() };
	for (auto& mainRTV : mainRTVs)
	{
		m_deviceContext->ClearRenderTargetView(mainRTV, clearColor);
	}
	m_deviceContext->ClearDepthStencilView(m_camera->GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0x00);
	m_deviceContext->OMSetRenderTargets(static_cast<UINT>(mainRTVs.size()), mainRTVs.data(), m_camera->GetDSV());
	m_deviceContext->RSSetViewports(1, &m_camera->GetViewport());
#pragma endregion

#pragma region ParticleManager ���� ����
	ID3D11Buffer* singleNullCb = nullptr;

	ID3D11Buffer* cameraCb = m_camera->GetPropertiesBuffer();
	m_deviceContext->VSSetConstantBuffers(0, 1, &cameraCb);
		m_particleManager->DrawEmittersDebugCube(m_camera->GetPropertiesBuffer(), m_deviceContext);
	m_deviceContext->VSSetConstantBuffers(0, 1, &singleNullCb);

	ID3D11Buffer* appParamsCb = m_appParamsGPU.GetBuffer();
	m_deviceContext->CSSetConstantBuffers(0, 1, &appParamsCb);
	m_deviceContext->GSSetConstantBuffers(0, 1, &appParamsCb);
		m_particleManager->ExecuteParticleSystem(m_deviceContext);
		m_deviceContext->VSSetConstantBuffers(1, 1, &cameraCb);
		m_deviceContext->GSSetConstantBuffers(1, 1, &cameraCb);
		m_deviceContext->PSSetConstantBuffers(1, 1, &cameraCb);
			m_particleManager->DrawParticles(m_deviceContext);
		m_deviceContext->VSSetConstantBuffers(1, 1, &singleNullCb);
		m_deviceContext->GSSetConstantBuffers(1, 1, &singleNullCb);
		m_deviceContext->PSSetConstantBuffers(1, 1, &singleNullCb);
	m_deviceContext->CSSetConstantBuffers(0, 1, &singleNullCb);
	m_deviceContext->GSSetConstantBuffers(0, 1, &singleNullCb);
#pragma endregion

#pragma region ī�޶� -> ����� ���� �� UI �׸���
	m_deviceContext->CopyResource(m_backBuffer, m_camera->GetRenderTargetTexture());

	DrawUI();
#pragma endregion

	HRESULT hResult = m_swapchain->Present(0, 0);
	if (FAILED(hResult)) throw exception("Present Failed");
}

void CProjectAApp::Quit()
{
}

void CProjectAApp::DrawUI()
{
	static vector<ID3D11RenderTargetView*> rtvs{ m_backBufferRTV };
	m_deviceContext->OMSetRenderTargets(1, rtvs.data(), nullptr);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	NewFrame();

	Begin("System");

	Text("Average %.3f ms/frame (%.1f FPS)",
		1000.0f / GetIO().Framerate,
		GetIO().Framerate);

	DrawEmitterHandler();

	End();

	Render();
	EndFrame();

	ImGui_ImplDX11_RenderDrawData(GetDrawData());
}

void CProjectAApp::DrawEmitterHandler()
{
	constexpr int NotSelected = -1;
	static int emitterIndex = -1;
	const vector<unique_ptr<CParticleEmitter>>& particleEmitters = m_particleManager->GetParticleEmitters();

	if (particleEmitters.size() > 0)
	{
		PushID("Emitter Handler");

		ostringstream emitterName;
		emitterName << "Emitter " << emitterIndex;
		if (BeginCombo("Select Emitter", emitterIndex == NotSelected ? "Choose Emitter" : emitterName.str().c_str(), NULL))
		{
			for (int idx = 0; idx < particleEmitters.size(); idx++)
			{
				ostringstream emitterNamei;
				emitterNamei << "Emitter " << idx;
				const bool isSelected = (idx == emitterIndex);
				if (Selectable(emitterNamei.str().c_str(), isSelected))
				{
					emitterIndex = idx;
				}

				if (isSelected)
					SetItemDefaultFocus();
			}
			EndCombo();
		}
		PopID();
	}

	if (emitterIndex >= 0)
	{
		CParticleEmitter* emitter = particleEmitters[emitterIndex].get();
		CParticleSpawnProperty* emitterProperty = emitter->GetParticleSpawnProperty();
		
		ImGui::SeparatorText("��ġ / �ڼ�");
		XMVECTOR emitterPos = emitter->GetPosition();
		XMVECTOR emitterAngle = emitter->GetAngle();
		if (DragFloat3("Emitter Position", emitterPos.m128_f32, 0.1f, -100.f, 100.f)) emitter->SetPosition(emitterPos);
		if (DragFloat3("Emitter Angle", emitterAngle.m128_f32, 0.1f, -360.f, 360.f)) emitter->SetAngle(emitterAngle);

		ImGui::SeparatorText("Emitter ���� �е�");
		float particleDensity = emitter->GetParticleDensity();
		if (DragFloat("���� �е�", &particleDensity, 0.001f, 0.f, 2.f)) emitter->SetParticleDensity(particleDensity);


		ImGui::SeparatorText("Emitter ���� ���� ��");
		bool isMinEmitAngleChanged = false;
		bool isMaxEmitAngleChanged = false;
		XMFLOAT2 minEmitRadians = emitterProperty->GetMinEmitRadians();
		XMFLOAT2 maxEmitRadians = emitterProperty->GetMaxEmitRadians();
		minEmitRadians.x *= (360.f / XM_2PI);
		minEmitRadians.y *= (360.f / XM_2PI);
		maxEmitRadians.x *= (360.f / XM_2PI);
		maxEmitRadians.y *= (360.f / XM_2PI);
		if (DragFloat("���� �ּ� ��(a)", &minEmitRadians.x, 0.1f, 0.f, 360.f)) isMinEmitAngleChanged = true;
		if (DragFloat("���� �ּ� ��(b)", &minEmitRadians.y, 0.1f, 0.f, 360.f)) isMinEmitAngleChanged = true;
		if (DragFloat("���� �ִ� ��(a)", &maxEmitRadians.x, 0.1f, minEmitRadians.x, 360.f)) isMaxEmitAngleChanged = true;
		if (DragFloat("���� �ִ� ��(b)", &maxEmitRadians.y, 0.1f, minEmitRadians.y, 360.f)) isMaxEmitAngleChanged = true;
		minEmitRadians.x *= (XM_2PI / 360.f);
		minEmitRadians.y *= (XM_2PI / 360.f);
		maxEmitRadians.x *= (XM_2PI / 360.f);
		maxEmitRadians.y *= (XM_2PI / 360.f);

		if (isMinEmitAngleChanged) emitterProperty->SetMinEmitRadians(minEmitRadians);
		if (isMaxEmitAngleChanged) emitterProperty->SetMaxEmitRadians(maxEmitRadians);

		ImGui::SeparatorText("Emitter ���� ���� �ӷ�");
		float emitSpeed = emitterProperty->GetEmitSpeed();
		if (DragFloat("���� �ӵ�", &emitSpeed, 0.1f, 0.f, 100.f)) emitterProperty->SetEmitSpeed(emitSpeed);
	}
}

void CProjectAApp::AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (m_camera) m_camera->HandleInput(msg, wParam, lParam);
}