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

#pragma region 테스트 초기화
	CParticleManager::InitializeSetInitializingPSO(m_device);
	CParticleManager::InitializeEmitterDrawPSO(m_device);
	CParticleManager::InitializePoolingCS(m_device);
	CParticleManager::InitializeEmitterSourcingCS(m_device);
	CParticleManager::InitializeParticleSimulateCS(m_device);
	CParticleManager::InitializeParticleDrawPSO(m_device);
	CParticleManager::InitializeRadixSortCS(m_device);
#pragma endregion

#pragma region 인스턴스 초기화
	m_camera = make_unique<CCamera>(
		XMVectorSet(0.f, 15.f, 0.f, 1.f),
		XMVectorSet(60.f * XM_2PI / 360.f, 0.f, 0.f, 1.f),
		m_width, m_height, 90.f, 0.01f, 100000.000f
		);

	m_particleManager = make_unique<CParticleManager>(500, 1024 * 1024);
	
#pragma region Radius 1E-3
	UINT emitterID1 = m_particleManager->AddParticleEmitter(
		0 /* Emitter Type */,
		7874.0f/* Steel Density */,
		1E-4f/* particle Radius */,
		XMVectorSet(-10.f, 0.f, 15.f, 1.f),
		XMVectorSet(0.f, -90.f * XM_2PI / 360.f, 0.f, 1.f),
		XMFLOAT2(0.f, 0.f),
		XMFLOAT2(XM_2PI, XM_2PI),
		XMFLOAT2(0, 1), 0,
		m_device, m_deviceContext
	);

	CParticleEmitter* emitter =  m_particleManager->GetEmitter(emitterID1);
	CEmitterSpawnProperty* emitterSpawnProperty = emitter->GetEmitterSpawnProperty();
	CParticleSpawnProperty* particleSpawnProperty = emitter->GetParticleSpawnProperty();

	particleSpawnProperty->SetMinEmitRadians(XMFLOAT2(-10.f * XM_PI / 180.f, 120.f * XM_PI / 180.f));
	particleSpawnProperty->SetMaxEmitRadians(XMFLOAT2(10.f * XM_PI / 180.f, 180.f * XM_PI / 180.f));
	particleSpawnProperty->SetEmitSpeed(30.f);
	particleSpawnProperty->SetLoopPlay(true, 4.f);
	particleSpawnProperty->SetEmitRateProfiles(
		vector<SEmitRate>{
			{0.f, 0}, { 2.f, 0 }, { 2.5f, 3000 }, { 3.f, 0 }, { 4.f, 0 }
	});
#pragma endregion

#pragma region Radius 1
	UINT emitterID2 = m_particleManager->AddParticleEmitter(
		0 /* Emitter Type */,
		7874.0f/* Steel Density */,
		1.f/* particle Radius */,
		XMVectorSet(0.f, 0.f, 15.f, 1.f),
		XMVectorSet(0.f, -90.f * XM_2PI / 360.f, 0.f, 1.f),
		XMFLOAT2(0.f, 0.f),
		XMFLOAT2(XM_2PI, XM_2PI),
		XMFLOAT2(0, 1), 0,
		m_device, m_deviceContext
	);

	emitter = m_particleManager->GetEmitter(emitterID2);
	emitterSpawnProperty = emitter->GetEmitterSpawnProperty();
	particleSpawnProperty = emitter->GetParticleSpawnProperty();

	particleSpawnProperty->SetMinEmitRadians(XMFLOAT2(-10.f * XM_PI / 180.f, 120.f * XM_PI / 180.f));
	particleSpawnProperty->SetMaxEmitRadians(XMFLOAT2(10.f * XM_PI / 180.f, 180.f * XM_PI / 180.f));
	particleSpawnProperty->SetEmitSpeed(30.f);
	particleSpawnProperty->SetLoopPlay(true, 4.f);
	particleSpawnProperty->SetEmitRateProfiles(
		vector<SEmitRate>{
			{0.f, 0}, { 2.f, 0 }, { 2.5f, 3000 }, { 3.f, 0 }, { 4.f, 0 }
	});
#pragma endregion

#pragma region Radius 0.005 And Ligter Than Air
	UINT emitterID3 = m_particleManager->AddParticleEmitter(
		0 /* Emitter Type */,
		1.f/* Lighter Than Air */,
		1.f/* particle Radius */,
		XMVectorSet(10.f, 0.f, 15.f, 1.f),
		XMVectorSet(0.f, -90.f * XM_2PI / 360.f, 0.f, 1.f),
		XMFLOAT2(0.f, 0.f),
		XMFLOAT2(XM_2PI, XM_2PI),
		XMFLOAT2(0, 1), 0,
		m_device, m_deviceContext
	);

	emitter = m_particleManager->GetEmitter(emitterID3);
	emitterSpawnProperty = emitter->GetEmitterSpawnProperty();
	particleSpawnProperty = emitter->GetParticleSpawnProperty();

	particleSpawnProperty->SetMinEmitRadians(XMFLOAT2(-10.f * XM_PI / 180.f, 120.f * XM_PI / 180.f));
	particleSpawnProperty->SetMaxEmitRadians(XMFLOAT2(10.f * XM_PI / 180.f, 180.f * XM_PI / 180.f));
	particleSpawnProperty->SetEmitSpeed(10.f);
	particleSpawnProperty->SetLoopPlay(true, 4.f);
	particleSpawnProperty->SetEmitRateProfiles(
		vector<SEmitRate>{
			{0.f, 0}, { 2.f, 0 }, { 2.5f, 3000 }, { 3.f, 0 }, { 4.f, 0 }
	});
#pragma endregion

#pragma region Gravity Field
	UINT emitterID4 = m_particleManager->AddParticleEmitter(
		1 /* Emitter Type */,
		1.225f,
		1.f/* particle Radius */,
		XMVectorSet(0.f, 10.f, 10.f, 1.f),
		XMVectorSet(0.f, -90.f * XM_2PI / 360.f, 0.f, 1.f),
		XMFLOAT2(0.f, 0.f),
		XMFLOAT2(XM_2PI, XM_2PI),
		XMFLOAT2(0, 5), 1000,
		m_device, m_deviceContext
	);
#pragma endregion


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
	m_appParamsCPU.particleMaxCount = m_particleManager->GetParticleMaxCount();
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
	ID3D11Buffer* singleNullCb = nullptr;

	ID3D11Buffer* cameraCb = m_camera->GetPropertiesBuffer();
	m_deviceContext->VSSetConstantBuffers(0, 1, &cameraCb);
		m_particleManager->DrawEmittersDebugCube(m_deviceContext);
	m_deviceContext->VSSetConstantBuffers(0, 1, &singleNullCb);

	ID3D11Buffer* commonCbs[] = { m_appParamsGPU.GetBuffer(), m_camera->GetPropertiesBuffer() };
	ID3D11Buffer* commonNullCbs[] = { nullptr, nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->VSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->GSSetConstantBuffers(0, 2, commonCbs);
	m_deviceContext->PSSetConstantBuffers(0, 2, commonCbs);
		m_particleManager->ExecuteParticleSystem(m_deviceContext);
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
		
		ImGui::SeparatorText("위치 / 자세");
		XMVECTOR emitterPos = emitter->GetPosition();
		XMVECTOR emitterAngle = emitter->GetAngle();
		emitterAngle = XMVectorScale(emitterAngle, 360.f / XM_2PI);
		if (DragFloat3("Emitter 위치", emitterPos.m128_f32, 0.1f, -100.f, 100.f)) emitter->SetPosition(emitterPos);
		if (DragFloat3("Emitter 자세", emitterAngle.m128_f32, 0.1f, -360.f, 360.f))
		{
			emitterAngle = XMVectorScale(emitterAngle, XM_2PI / 360.f);
			emitter->SetAngle(emitterAngle);
		}

		ImGui::SeparatorText("Emitter 입자 밀도(kg/m3)");
		float particleDensity = emitter->GetParticleDensity();
		if (DragFloat("입자 밀도", &particleDensity, 0.1f, 0.1f, 10000.f)) emitter->SetParticleDensity(particleDensity);

		ImGui::SeparatorText("Emitter 입자 반지름(m)");
		float particleRadius = emitter->GetParticleRadius();
		if (DragFloat("입자 반지름", &particleRadius, 1E-4f, 1E-4f, 1.f, "%.4f")) emitter->SetParticleRadius(particleRadius);

		ImGui::SeparatorText("Emitter 입자 방출 각");
		bool isMinEmitAngleChanged = false;
		XMFLOAT2 minEmitRadians = emitterProperty->GetMinEmitRadians();
		minEmitRadians.x *= (360.f / XM_2PI);
		minEmitRadians.y *= (360.f / XM_2PI);
		if (DragFloat("방출 최소 각(a)", &minEmitRadians.x, 0.1f, -360.f, 360.f)) isMinEmitAngleChanged = true;
		if (DragFloat("방출 최소 각(b)", &minEmitRadians.y, 0.1f, -360.f, 360.f)) isMinEmitAngleChanged = true;

		bool isMaxEmitAngleChanged = false;
		XMFLOAT2 maxEmitRadians = emitterProperty->GetMaxEmitRadians();
		maxEmitRadians.x *= (360.f / XM_2PI);
		maxEmitRadians.y *= (360.f / XM_2PI);

		if (maxEmitRadians.x < minEmitRadians.x)
		{
			isMaxEmitAngleChanged = true;
			maxEmitRadians.x = minEmitRadians.x;
		}
		if (maxEmitRadians.y < minEmitRadians.y)
		{
			isMaxEmitAngleChanged = true;
			maxEmitRadians.y = minEmitRadians.y;
		}

		if (DragFloat("방출 최대 각(a)", &maxEmitRadians.x, 0.1f, minEmitRadians.x, 360.f)) isMaxEmitAngleChanged = true;
		if (DragFloat("방출 최대 각(b)", &maxEmitRadians.y, 0.1f, minEmitRadians.y, 360.f)) isMaxEmitAngleChanged = true;
		minEmitRadians.x *= (XM_2PI / 360.f);
		minEmitRadians.y *= (XM_2PI / 360.f);
		maxEmitRadians.x *= (XM_2PI / 360.f);
		maxEmitRadians.y *= (XM_2PI / 360.f);

		if (isMinEmitAngleChanged) emitterProperty->SetMinEmitRadians(minEmitRadians);
		if (isMaxEmitAngleChanged) emitterProperty->SetMaxEmitRadians(maxEmitRadians);

		ImGui::SeparatorText("Emitter 입자 방출 속력(m/s)");
		float emitSpeed = emitterProperty->GetEmitSpeed();
		if (DragFloat("방출 속도", &emitSpeed, 0.1f, 0.f, 100.f)) emitterProperty->SetEmitSpeed(emitSpeed);
	}
}

void CProjectAApp::AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (m_camera) m_camera->HandleInput(msg, wParam, lParam);
}