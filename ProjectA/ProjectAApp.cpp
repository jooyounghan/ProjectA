#include "ProjectAApp.h"

#include "GEngine.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <vector>

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

ProjectAApp::ProjectAApp() noexcept
	: CBaseApp()
{
}

ProjectAApp* ProjectAApp::GetInstance() noexcept
{
	static ProjectAApp ProjectAApp;
	return &ProjectAApp;
}

void ProjectAApp::Create(
	UINT width,
	UINT height,
	const wchar_t* className,
	const wchar_t* applicaitonName
) noexcept
{
	CBaseApp::Create(width, height, className, applicaitonName);
}

void ProjectAApp::Init()
{
#pragma region App 초기화
	GEngine* engine = GEngine::GetInstance();

	m_device = engine->GetDevice();
	m_deviceContext = engine->GetDeviceContext();
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
	
	XMVECTOR initValue = XMVectorSet(0.f, 0.f, 0.f, 0.f);

	m_camera = make_unique<Camera>(
		initValue, initValue, m_width, m_height, 90.f, 0.01f, 100.000f
	);
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
}

constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 1.f };

void ProjectAApp::Update(float deltaTime)
{
	m_deviceContext->ClearRenderTargetView(m_backBufferRTV, clearColor);
	HRESULT hResult = m_swapchain->Present(1, 0);
	if (FAILED(hResult)) throw exception("Present Failed");
}

void ProjectAApp::Quit()
{
}

void ProjectAApp::DrawUI()
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

void ProjectAApp::AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}