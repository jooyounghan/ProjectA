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

#include "DynamicBuffer.h"

#include "GraphicsPSOObject.h"
#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"

#include "Camera.h"
#include "ShotFilm.h"
#include "EmitterSelector.h"

#include "ParticleEmitterManager.h"
#include "SpriteEmitterManager.h"

#include "EmitterManagerCommonData.h"
#include "FilterCommonData.h"

#include "AEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "RuntimeSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include "GPUInterpPropertyManager.h"

#include <format>
#pragma  endregion

using namespace std;
using namespace App;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;


#define CurrentEmitterTypeCount 2
#define MaxParticleEmitterCount 1000
#define MaxSpriteEmitterCount 1000

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
void CProjectAApp::Init(
	UINT width, UINT height,
	const wchar_t* className,
	const wchar_t* applicaitonName
)
{
	CBaseApp::Init(width, height, className, applicaitonName);

#pragma region App 초기화

	GEngine* engine = D3D11::GEngine::GetInstance();
	engine->InitEngine(m_width, m_height, 240, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM, 2,
		m_mainWindow,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

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
	CEmitterManagerCommonData::Intialize(m_device);
	CFilterCommonData::Intialize(m_device);
#pragma endregion

#pragma region 인스턴스 초기화
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 10.f, 0.f, 1.f),
		XMVectorSet(0.1f, 0.f, 0.f, 1.f),
		120.f, 0.01f, 100000.f
	);
	m_camera->Initialize(m_device, m_deviceContext);

	m_shotFilm = make_unique<CShotFilm>(
		m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R24G8_TYPELESS, 1, 4, m_backBufferRTV
	);
	m_shotFilm->Initialize(m_device, m_deviceContext);

	m_camera->SetViewport(m_shotFilm->GetFilmViewPort());

	m_emitterManagers.emplace_back(make_unique<ParticleEmitterManager>(
		m_width, m_height, MaxParticleEmitterCount, MaxParticleCount
	));

	m_emitterManagers.emplace_back(make_unique<SpriteEmitterManager>(
		m_width, m_height, MaxSpriteEmitterCount, MaxParticleCount
	));
	for (auto& emitterManager : m_emitterManagers)
	{
		emitterManager->Initialize(m_device, m_deviceContext);
	}

#pragma endregion

#pragma endregion
}

void CProjectAApp::Update(float deltaTime)
{
#pragma region 앱 버퍼 업데이트
	m_appParamsCPU.dt = deltaTime;
	m_appParamsCPU.appWidth = static_cast<float>(m_width);
	m_appParamsCPU.appHeight = static_cast<float>(m_height);

	m_appParamsGPU->Stage(m_deviceContext);
	m_appParamsGPU->Upload(m_deviceContext);
#pragma endregion

#pragma region 인스턴스 업데이트
	m_camera->Update(m_deviceContext, deltaTime);

	for (auto& emitterManager : m_emitterManagers)
	{
		emitterManager->Update(m_deviceContext, deltaTime);
	}
#pragma endregion

	ID3D11RenderTargetView* shotFilmRTV = m_shotFilm->GetFilmRTV();
	ID3D11RenderTargetView* shotFilmNullRTV = nullptr;
	ID3D11DepthStencilView* shotFilmDSV = m_shotFilm->GetFilmDSV();
	const D3D11_VIEWPORT& shotFilmViewport = m_shotFilm->GetFilmViewPort();

#pragma region 카메라 초기화 및 설정
	m_shotFilm->ClearFilm(m_deviceContext);
#pragma endregion

	ID3D11Buffer* cameraCb = m_camera->GetPropertiesBuffer();
	ID3D11Buffer* singleNullCb = nullptr;
	CShotFilm* shortFilm = m_shotFilm.get();

#pragma region 방출기 그리기
	m_deviceContext->VSSetConstantBuffers(0, 1, &cameraCb);
	for (auto& emitterManager : m_emitterManagers)
	{
		emitterManager->DrawEmitters(shortFilm, m_deviceContext);
	}
	m_deviceContext->VSSetConstantBuffers(0, 1, &singleNullCb);
#pragma endregion

#pragma region 입자 그리기
	ID3D11Buffer* commonCbs[] = { m_appParamsGPU->GetBuffer(), m_camera->GetPropertiesBuffer() };
	ID3D11Buffer* commonNullCbs[] = { nullptr, nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonCbs);


	for (auto& emitterManager : m_emitterManagers)
	{
		emitterManager->InitializeAliveFlag(shortFilm, m_deviceContext);
		emitterManager->SourceParticles(m_deviceContext);
		emitterManager->CalculateIndirectArgs(m_deviceContext);
		emitterManager->CalculateForces(m_deviceContext);
		emitterManager->FinalizeParticles(m_deviceContext);
		emitterManager->DrawParticles(shortFilm, m_deviceContext);
	}
	m_deviceContext->CSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonNullCbs);
#pragma endregion

#pragma region 전체 화면에 대한 후처리 수행
	m_shotFilm->Develop(m_deviceContext);
#pragma endregion

#pragma region UI 그리기
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
	ID3D11RenderTargetView* uiRTV = m_backBufferRTV;
	ID3D11RenderTargetView* uiNullRTV = nullptr;

	m_deviceContext->OMSetRenderTargets(1, &uiRTV, nullptr);

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

	m_deviceContext->OMSetRenderTargets(1, &uiNullRTV, nullptr);
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
				AEmitterManager* particleEmitterManager = m_emitterManagers[static_cast<size_t>(EEmitterType::ParticleEmitter)].get();
				UINT particleEmitterID = particleEmitterManager->AddEmitter(XMVectorZero(), XMVectorZero(), m_device, m_deviceContext);
				particleEmitterManager->GetEmitter(particleEmitterID)->Initialize(m_device, m_deviceContext);
				break;
			}
			case EEmitterType::SpriteEmitter:
			{
				AEmitterManager* spriteEmitterManager = m_emitterManagers[static_cast<size_t>(EEmitterType::SpriteEmitter)].get();
				UINT spriteEmitterID = spriteEmitterManager->AddEmitter(XMVectorZero(), XMVectorZero(), m_device, m_deviceContext);
				spriteEmitterManager->GetEmitter(spriteEmitterID)->Initialize(m_device, m_deviceContext);
				break;
			}
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

	static int emitterSelectIndex[CurrentEmitterTypeCount] = { -1 , -1 } ;
	static string emitterNames[CurrentEmitterTypeCount] = { "파티클 이미터", "스프라이트 이미터" };

	for (size_t idx = 0; idx < CurrentEmitterTypeCount; ++idx)
	{
		DrawEmitterSelector(emitterNames[idx], emitterSelectIndex[idx], m_emitterManagers[idx].get());
	}
}
void CProjectAApp::DrawEmitterSelector(const std::string& emitterName, int& emitterSelectIndex, AEmitterManager* emitterManager)
{
	constexpr int NotSelected = -1;

	PushID(format("{} Handler", emitterName).c_str());
	vector<unique_ptr<AEmitter>>& particleEmitters = emitterManager->GetEmitters();
	SeparatorText(emitterName.c_str());
	if (particleEmitters.size() > 0)
	{

		if (BeginCombo(format("{} 선택", emitterName).c_str(), emitterSelectIndex == NotSelected ? "Choose Emitter" : format("{} {}", emitterName, emitterSelectIndex + 1).c_str(), NULL))
		{
			for (int idx = 0; idx < particleEmitters.size(); idx++)
			{
				const bool isSelected = (idx == emitterSelectIndex);
				if (Selectable(format("Select Emitter {}", idx + 1).c_str(), isSelected))
				{
					emitterSelectIndex = idx;
				}

				if (isSelected)
					SetItemDefaultFocus();
			}
			EndCombo();
		}

		if (emitterSelectIndex != NotSelected)
		{
			SameLine();
			if (Button("이미터 삭제"))
			{
				UINT emitterID = particleEmitters[emitterSelectIndex]->GetEmitterID();
				emitterManager->RemoveEmitter(emitterID);
				emitterSelectIndex = NotSelected;
			}
		}
	}

	AEmitter* emitter = (emitterSelectIndex == NotSelected) ? nullptr : particleEmitters[emitterSelectIndex].get();

	if (emitter)
	{
		emitter->DrawUI();
	}
	PopID();
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