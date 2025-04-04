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
#pragma  endregion

using namespace std;
using namespace App;
using namespace D3D11;
using namespace DirectX;

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
#pragma region App 초기화
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

#pragma region ImGui 초기화
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesKorean());

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(m_mainWindow);
	ImGui_ImplDX11_Init(m_device, m_deviceContext);

	// ImGuiIO, Font Initializing
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::EndFrame();
#pragma endregion

#pragma region 테스트 초기화
	CParticleManager::InitializeEmitterDrawPSO(m_device);
	CParticleManager::InitializePoolingPSO(m_device);
	CParticleManager::InitializeEmitterSourcingPSO(m_device);
	CParticleManager::InitializeParticleDrawPSO(m_device);
#pragma endregion

#pragma region 인스턴스 초기화
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		m_width, m_height, 90.f, 0.01f, 100.000f
		);

	m_particleManager = make_unique<CParticleManager>(4, 1024 * 1024);

	m_particleManager->AddParticleEmitter(
		XMVectorSet(-4.f, 0.f, 10.f, 1.f),
		XMVectorZero(),
		XMVectorSet(3.f, 3.f, 0.f, 1.f),
		m_device, m_deviceContext
	);

	m_particleManager->AddParticleEmitter(
		XMVectorSet(0.f, 0.f, 14.f, 1.f),
		XMVectorZero(),
		XMVectorSet(0.f, 3.f, -3.f, 1.f),
		m_device, m_deviceContext
	);
	
	m_particleManager->AddParticleEmitter(
		XMVectorSet(0.f, 0.f, 6.f, 1.f),
		XMVectorZero(),
		XMVectorSet(0.f, 3.f, 3.f, 1.f),
		m_device, m_deviceContext
	);

	m_particleManager->AddParticleEmitter(
		XMVectorSet(4.f, 0.f, 10.f, 1.f),
		XMVectorZero(),
		XMVectorSet(-3.f, 3.f, 0.f, 1.f),
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
#pragma region 앱 버퍼 업데이트
	m_appParamsCPU.dt = deltaTime;
	m_appParamsCPU.appWidth = static_cast<float>(m_width);
	m_appParamsCPU.appHeight = static_cast<float>(m_height);
	m_appParamsGPU.Stage(m_deviceContext);
	m_appParamsGPU.Upload(m_deviceContext);
#pragma endregion

#pragma region 인스턴스 업데이트
	for (auto& updatable : m_updatables)
	{
		updatable->Update(m_deviceContext, deltaTime);
	}
#pragma endregion

#pragma region 카메라 초기화 및 설정
	static vector<ID3D11RenderTargetView*> mainRTVs = { m_camera->GetRTV() };
	for (auto& mainRTV : mainRTVs)
	{
		m_deviceContext->ClearRenderTargetView(mainRTV, clearColor);
	}
	m_deviceContext->ClearDepthStencilView(m_camera->GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0x00);
	m_deviceContext->OMSetRenderTargets(static_cast<UINT>(mainRTVs.size()), mainRTVs.data(), m_camera->GetDSV());
	m_deviceContext->RSSetViewports(1, &m_camera->GetViewport());
#pragma endregion

#pragma region ParticleManager 메인 로직
	m_particleManager->DrawEmittersDebugCube(m_camera->GetPropertiesBuffer(), m_deviceContext);

	ID3D11Buffer* systemCbs = m_appParamsGPU.GetBuffer();
	m_deviceContext->CSSetConstantBuffers(0, 1, &systemCbs);
	m_particleManager->ExecuteParticleSystem(m_deviceContext);

	ID3D11Buffer* tempCbs[] = { m_appParamsGPU.GetBuffer(), m_camera->GetPropertiesBuffer() };
	ID3D11Buffer* tempNullCbs[] = { nullptr, nullptr };
	m_deviceContext->VSSetConstantBuffers(0, 2, tempCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, tempCbs);
	m_particleManager->DrawParticles(m_deviceContext);
	m_deviceContext->VSSetConstantBuffers(0, 2, tempNullCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, tempNullCbs);

#pragma endregion

#pragma region 카메라 -> 백버퍼 복사 및 UI 그리기
	m_deviceContext->CopyResource(m_backBuffer, m_camera->GetRenderTargetTexture());

	DrawUI();
#pragma endregion

	HRESULT hResult = m_swapchain->Present(1, 0);
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

	ImGui::NewFrame();

	ImGui::Begin("System");

	ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
		1000.0f / ImGui::GetIO().Framerate,
		ImGui::GetIO().Framerate);

	ImGui::End();

	ImGui::Render();
	ImGui::EndFrame();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void CProjectAApp::AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (m_camera) m_camera->HandleInput(msg, wParam, lParam);
}