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

#include "ParticleEmitterManager.h"
#include "EmitterManagerCommonData.h"

#include "AEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"
#include "ForceUpdateProperty.h"

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

	sizeof(SInterpProperty<4, 2>);
	sizeof(SInterpProperty<4, 4>);

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
	CEmitterManagerCommonData::Intialize(TotalParticleCount, m_device);

	CGPUInterpolater<4, 2>::InitializeGPUInterpProperty(m_device, MaxEmitterCount);
	CGPUInterpolater<4, 4>::InitializeGPUInterpProperty(m_device, MaxEmitterCount);
#pragma endregion

#pragma region 인스턴스 초기화
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 0.f, -10.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		m_width, m_height, 90.f, 0.01f, 100000.000f
	);
	m_camera->Initialize(m_device, m_deviceContext);

	ParticleEmitterManager& particleEmitterManager = ParticleEmitterManager::GetParticleEmitterManager();
	particleEmitterManager.Initialize(m_device, m_deviceContext);
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
	m_appParamsCPU.particleTotalCount = TotalParticleCount;

	m_appParamsGPU->Stage(m_deviceContext);
	m_appParamsGPU->Upload(m_deviceContext);
#pragma endregion

#pragma region 인스턴스 업데이트
	m_camera->Update(m_deviceContext, deltaTime);

	ParticleEmitterManager& particleEmitterManager = ParticleEmitterManager::GetParticleEmitterManager();
	particleEmitterManager.Update(m_deviceContext, deltaTime);
#pragma endregion

#pragma region 글로벌 변수 업데이트
	CGPUInterpolater<4, 2>::UpdateInterpProperty(m_deviceContext);
	CGPUInterpolater<4, 4>::UpdateInterpProperty(m_deviceContext);
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
	particleEmitterManager.DrawEmitters(m_deviceContext);
	m_deviceContext->VSSetConstantBuffers(0, 1, &singleNullCb);

	ID3D11Buffer* commonCbs[] = { m_appParamsGPU->GetBuffer(), m_camera->GetPropertiesBuffer() };
	ID3D11Buffer* commonNullCbs[] = { nullptr, nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonCbs);
	particleEmitterManager.InitializeAliveFlag(m_deviceContext);
	particleEmitterManager.SourceParticles(m_deviceContext);
	particleEmitterManager.CalculateForces(m_deviceContext);
	particleEmitterManager.DrawParticles(m_deviceContext);
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

		if (Button("이미터 생성"))
		{
			switch (emttierType)
			{
			case EEmitterType::ParticleEmitter:
			{
				ParticleEmitterManager& particleEmitterManager = ParticleEmitterManager::GetParticleEmitterManager();
				particleEmitterManager.AddEmitter(XMVectorZero(), XMVectorZero(), m_device, m_deviceContext);
				break;
			}
			case EEmitterType::SpriteEmitter:
				break;
			default:
				break;
			}
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("이미터 생성 종료"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	constexpr int NotSelected = -1;
	static int particleEmitterIndex = -1;

	ParticleEmitterManager& particleEmitterManager = ParticleEmitterManager::GetParticleEmitterManager();

	vector<unique_ptr<AEmitter>>& emitters = particleEmitterManager.GetEmitters();
	SeparatorText("파티클 이미터");
	if (emitters.size() > 0)
	{
		PushID("Emitter Handler");

		if (BeginCombo("Select Particle Emitter", particleEmitterIndex == NotSelected ? "Choose Emitter" : format("Emitter {}", particleEmitterIndex + 1).c_str(), NULL))
		{
			for (int idx = 0; idx < emitters.size(); idx++)
			{
				const bool isSelected = (idx == particleEmitterIndex);
				if (Selectable(format("Select Emitter {}", idx + 1).c_str(), isSelected))
				{
					particleEmitterIndex = idx;
				}

				if (isSelected)
					SetItemDefaultFocus();
			}
			EndCombo();
		}
		PopID();
	}

	AEmitter* emitter = (particleEmitterIndex == NotSelected) ? nullptr : emitters[particleEmitterIndex].get();
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

		emitter->GetInitialSpawnProperty()->DrawPropertyUI();
		emitter->GetEmitterUpdateProperty()->DrawPropertyUI();
		emitter->GetRuntimeSpawnProperty()->DrawPropertyUI();
		emitter->GetForceUpdateProperty()->DrawPropertyUI();
	}
}

void CProjectAApp::AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (m_camera) m_camera->HandleInput(msg, wParam, lParam);

	switch(msg) 
	{
    case WM_DESTROY:
		PostQuitMessage(0); // 메시지 루프 종료 요청
		return ;
	}
}