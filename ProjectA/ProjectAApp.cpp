#include "ProjectAApp.h"

#include "GEngine.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "implot.h"
#include "implot_internal.h"

#pragma region Test
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "DynamicBuffer.h"

#include "GraphicsPSOObject.h"
#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"

#include "Camera.h"
#include "EmitterSelector.h"

#include "EmitterManager.h"
#include "EmitterManagerStaticData.h"

#include "AEmitter.h"
#include "EmitterStaticData.h"

#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"

#include "GPUInterpolater.h"

#include <format>
#pragma  endregion

#define TotalParticleCount 1024 * 1024
#define MaxEmitterCount 1000

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
{
}

CProjectAApp* CProjectAApp::GetInstance() noexcept
{
	static CProjectAApp ProjectAApp;
	return &ProjectAApp;
}

void CProjectAApp::Create(
	UINT width, UINT height, 
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

	m_appParamsGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_appParamsCPU));
	m_appParamsGPU->InitializeBuffer(m_device);

	sizeof(SInterpolaterProperty<4, 2>);
	sizeof(SInterpolaterProperty<4, 4>);

	CRasterizerState::InitializeDefaultRasterizerStates(m_device);
	CBlendState::InitializeDefaultBlendStates(m_device);
	CDepthStencilState::InitializeDefaultDepthStencilState(m_device);
	CSamplerState::InitializeSamplerState(m_device);

#pragma endregion

#pragma region ImGui 초기화
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImPlot::CreateContext();
	StyleColorsDark();

	ImGuiIO& io = GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesKorean());

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(m_mainWindow);
	ImGui_ImplDX11_Init(m_device, m_deviceContext);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	NewFrame();
	EndFrame();
#pragma endregion

#pragma region UI 관련 클래스 초기화
	m_emitterSelector = make_unique<CEmitterSelector>("생성할 이미터 종류를 선택하세요");
#pragma endregion

#pragma region 글로벌 변수 초기화
	EmitterStaticData::InitializeGlobalEmitterProperty(MaxEmitterCount, m_device);
	EmitterStaticData::InitializeEmitterDrawPSO(m_device);

	CGPUInterpolater<4, 2>::InitializeGPUInterpolater(m_device, MaxEmitterCount);
	CGPUInterpolater<4, 4>::InitializeGPUInterpolater(m_device, MaxEmitterCount);

	CEmitterManagerStaticData::InitializeSetInitializingPSO(m_device);
	CEmitterManagerStaticData::InitializePoolingCS(m_device);
	CEmitterManagerStaticData::InitializeEmitterSourcingCS(m_device);
	CEmitterManagerStaticData::InitializeParticleSimulateCS(m_device);
	CEmitterManagerStaticData::InitializeRadixSortCS(m_device);
	CEmitterManagerStaticData::InitializeParticleDrawPSO(m_device);
#pragma endregion

#pragma region 인스턴스 초기화
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 0.f, -10.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		m_width, m_height, 90.f, 0.01f, 100000.000f
	);

	m_particleManager = make_unique<CEmitterManager>(10, TotalParticleCount);

	m_updatables.emplace_back(m_camera.get());
	m_updatables.emplace_back(m_particleManager.get());

	for (auto& updatable : m_updatables)
	{
		updatable->Initialize(m_device, m_deviceContext);
	}
#pragma endregion

#pragma endregion
}

constexpr FLOAT clearColor[4] = { 1.f, 1.f, 1.f, 1.f };

void CProjectAApp::Update(float deltaTime)
{
#pragma region 앱 버퍼 업데이트
	m_appParamsCPU.dt = deltaTime;
	m_appParamsCPU.appWidth = static_cast<float>(m_width);
	m_appParamsCPU.appHeight = static_cast<float>(m_height);
	m_appParamsCPU.particleTotalCount = TotalParticleCount;

	m_appParamsGPU->Stage(m_deviceContext);
	m_appParamsGPU->Upload(m_deviceContext);
#pragma endregion

#pragma region 인스턴스 업데이트
	for (auto& updatable : m_updatables)
	{
		updatable->Update(m_deviceContext, deltaTime);
	}
#pragma endregion

#pragma region 글로벌 변수 업데이트
	EmitterStaticData::UpdateGlobalEmitterProperty(m_deviceContext);
	CGPUInterpolater<4, 2>::UpdateInterpolaterProperty(m_deviceContext);
	CGPUInterpolater<4, 4>::UpdateInterpolaterProperty(m_deviceContext);
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
	ID3D11Buffer* cameraCb = m_camera->GetPropertiesBuffer();
	ID3D11Buffer* singleNullCb = nullptr;
	m_deviceContext->VSSetConstantBuffers(0, 1, &cameraCb);
		EmitterStaticData::DrawEmittersDebugCube(m_deviceContext);
	m_deviceContext->VSSetConstantBuffers(0, 1, &singleNullCb);

	ID3D11Buffer* commonCbs[] = { m_appParamsGPU->GetBuffer(), m_camera->GetPropertiesBuffer() };
	ID3D11Buffer* commonNullCbs[] = { nullptr, nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonCbs);
		m_particleManager->ExecuteParticleSystem(m_deviceContext);
		m_particleManager->CaculateParticlesForce(m_deviceContext);
		m_particleManager->DrawParticles(m_deviceContext);
	m_deviceContext->CSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonNullCbs);
#pragma endregion

#pragma region 카메라 -> 백버퍼 복사 및 UI 그리기
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
	if (Button("이미터 생성"))
		OpenPopup("CreateEmitter");

	if (BeginPopupModal("CreateEmitter", NULL, ImGuiWindowFlags_MenuBar))
	{
		static EEmitterType emttierType = EEmitterType::ParticleEmitter;
		static std::unique_ptr<AEmitter> createdEmitter = nullptr;
		m_emitterSelector->SelectEnums(emttierType);

		static ImGuiChildFlags childFlag = ImGuiChildFlags_::ImGuiChildFlags_Border | ImGuiChildFlags_::ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_::ImGuiChildFlags_AutoResizeY;
		if (BeginChild("##CreateEmitterChild", ImVec2(0.f, 0.f), childFlag))
		{
			if (m_emitterSelector->CreateEmitter(emttierType, createdEmitter))
			{
				m_particleManager->AddParticleEmitter(createdEmitter, m_device, m_deviceContext);
				ImGui::CloseCurrentPopup();
			}

			EndChild();
		}

		if (ImGui::Button("이미터 생성 종료"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	constexpr int NotSelected = -1;
	static int emitterIndex = -1;
	vector<unique_ptr<AEmitter>>& emitters = m_particleManager->GetEmitters();

	if (emitters.size() > 0)
	{
		PushID("Emitter Handler");

		if (BeginCombo("Select Emitter", emitterIndex == NotSelected ? "Choose Emitter" : format("Emitter {}", emitterIndex + 1).c_str(), NULL))
		{
			for (int idx = 0; idx < emitters.size(); idx++)
			{
				const bool isSelected = (idx == emitterIndex);
				if (Selectable(format("Select Emitter {}", idx + 1).c_str(), isSelected))
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

	AEmitter* emitter = (emitterIndex == NotSelected) ? nullptr : emitters[emitterIndex].get();
	if (emitter)
	{
		XMVECTOR emitterPos = emitter->GetPosition();
		if (DragFloat3("이미터 위치", emitterPos.m128_f32, 0.1f, -1000.f, 1000.f, "%.1f"))
		{
			emitter->SetPosition(emitterPos);
		}

		XMVECTOR emitterAngle = emitter->GetAngle();
		emitterAngle = XMVectorScale(emitterAngle, 180.f / XM_PI);
		if (DragFloat3("이미터 각도", emitterAngle.m128_f32, 0.1f, -360, 360.f, "%.1f"))
		{
			emitterAngle = XMVectorScale(emitterAngle, XM_PI / 180.f);
			emitter->SetAngle(emitterAngle);
		}

		emitter->GetAEmitterSpawnProperty()->DrawPropertyUI();
		emitter->GetAEmitterUpdateProperty()->DrawPropertyUI();
		emitter->GetAParticleSpawnProperty()->DrawPropertyUI();
		emitter->GetAParticleUpdateProperty()->DrawPropertyUI();
	}
}

void CProjectAApp::AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (m_camera) m_camera->HandleInput(msg, wParam, lParam);
}