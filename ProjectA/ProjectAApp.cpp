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
#include "EmitterSelector.h"

#include "ParticleEmitterManager.h"
#include "SpriteEmitterManager.h"
#include "EmitterManagerCommonData.h"

#include "AEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"
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

#pragma region App �ʱ�ȭ

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

	sizeof(SInterpProperty<4, 2>);
	sizeof(SInterpProperty<4, 4>);

	CRasterizerState::InitializeDefaultRasterizerStates(m_device);
	CBlendState::InitializeDefaultBlendStates(m_device);
	CDepthStencilState::InitializeDefaultDepthStencilState(m_device);
	CSamplerState::InitializeSamplerState(m_device);

#pragma endregion

#pragma region ImGui �ʱ�ȭ
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

#pragma region UI ���� Ŭ���� �ʱ�ȭ
	m_emitterSelector = make_unique<CEmitterSelector>("������ �̹��� ������ �����ϼ���");
#pragma endregion

#pragma region �۷ι� ���� �ʱ�ȭ
	CEmitterManagerCommonData::Intialize(m_device);
#pragma endregion

#pragma region �ν��Ͻ� �ʱ�ȭ
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 0.f, -20.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		m_width, m_height, 90.f, 0.01f, 100000.000f
	);
	m_camera->Initialize(m_device, m_deviceContext);


	AEmitterManager* emitterManagers[] = {
		&ParticleEmitterManager::GetParticleEmitterManager(),
		&SpriteEmitterManager::GetSpriteEmitterManager()
	};

	for (auto& emitterManager : emitterManagers)
	{
		emitterManager->Initialize(m_device, m_deviceContext);
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

	m_appParamsGPU->Stage(m_deviceContext);
	m_appParamsGPU->Upload(m_deviceContext);
#pragma endregion

#pragma region �ν��Ͻ� ������Ʈ
	m_camera->Update(m_deviceContext, deltaTime);

	AEmitterManager* emitterManagers[] = {
		&ParticleEmitterManager::GetParticleEmitterManager(),
		&SpriteEmitterManager::GetSpriteEmitterManager()
	};

	for (auto& emitterManager : emitterManagers)
	{
		emitterManager->Update(m_deviceContext, deltaTime);
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
	ID3D11Buffer* cameraCb = m_camera->GetPropertiesBuffer();
	ID3D11Buffer* singleNullCb = nullptr;
	m_deviceContext->VSSetConstantBuffers(0, 1, &cameraCb);
	for (auto& emitterManager : emitterManagers)
	{
		emitterManager->DrawEmitters(m_deviceContext);
	}
	m_deviceContext->VSSetConstantBuffers(0, 1, &singleNullCb);

	ID3D11Buffer* commonCbs[] = { m_appParamsGPU->GetBuffer(), m_camera->GetPropertiesBuffer() };
	ID3D11Buffer* commonNullCbs[] = { nullptr, nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonCbs);
	for (auto& emitterManager : emitterManagers)
	{
		emitterManager->InitializeAliveFlag(m_deviceContext);
		emitterManager->SourceParticles(m_deviceContext);
		emitterManager->CalculateForces(m_deviceContext);
		emitterManager->FinalizeParticles(m_deviceContext);
		emitterManager->DrawParticles(m_deviceContext);
	}
	m_deviceContext->CSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonNullCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonNullCbs);

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
	if (Button("�̹��� ����"))
		OpenPopup("CreateEmitter");

	if (BeginPopupModal("CreateEmitter", NULL, ImGuiWindowFlags_MenuBar))
	{
		static EEmitterType emttierType = EEmitterType::ParticleEmitter;
		static std::unique_ptr<AEmitter> createdEmitter = nullptr;
		m_emitterSelector->SelectEnums(emttierType);

		static ImGuiChildFlags childFlag = ImGuiChildFlags_::ImGuiChildFlags_Border | ImGuiChildFlags_::ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_::ImGuiChildFlags_AutoResizeY;

		if (Button("�̹��� ����"))
		{
			switch (emttierType)
			{
			case EEmitterType::ParticleEmitter:
			{
				ParticleEmitterManager& particleEmitterManager = ParticleEmitterManager::GetParticleEmitterManager();
				UINT particleEmitterID = particleEmitterManager.AddEmitter(XMVectorZero(), XMVectorZero(), m_device, m_deviceContext);
				particleEmitterManager.GetEmitter(particleEmitterID)->Initialize(m_device, m_deviceContext);
				break;
			}
			case EEmitterType::SpriteEmitter:
			{
				SpriteEmitterManager& spriteEmitterManager = SpriteEmitterManager::GetSpriteEmitterManager();
				UINT spriteEmitterID = spriteEmitterManager.AddEmitter(XMVectorZero(), XMVectorZero(), m_device, m_deviceContext);
				spriteEmitterManager.GetEmitter(spriteEmitterID)->Initialize(m_device, m_deviceContext);
				break;
			}
			default:
				break;
			}
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("�̹��� ���� ����"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	static int emitterSelectIndex[CurrentEmitterTypeCount] = { -1 , -1 } ;
	static string emitterNames[CurrentEmitterTypeCount] = { "��ƼŬ �̹���", "��������Ʈ �̹���" };
	AEmitterManager* emitterManagers[CurrentEmitterTypeCount] = {
		&ParticleEmitterManager::GetParticleEmitterManager(),
		&SpriteEmitterManager::GetSpriteEmitterManager()
	};

	for (size_t idx = 0; idx < CurrentEmitterTypeCount; ++idx)
	{
		DrawEmitterSelector(emitterNames[idx], emitterSelectIndex[idx], emitterManagers[idx]);
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

		if (BeginCombo(format("{} ����", emitterName).c_str(), emitterSelectIndex == NotSelected ? "Choose Emitter" : format("{} {}", emitterName, emitterSelectIndex + 1).c_str(), NULL))
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
			if (Button("�̹��� ����"))
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
		PostQuitMessage(0); // �޽��� ���� ���� ��û
		return ;
	}
}