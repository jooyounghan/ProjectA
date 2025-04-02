#include "ProjectAApp.h"
#include "GEngine.h"
#include <iostream>

int main()
{
	constexpr UINT AppWidth = 1920;
	constexpr UINT AppHeight = 1200;

	CProjectAApp* testApp = CProjectAApp::GetInstance();
	testApp->Create(AppWidth, AppHeight, L"Test", L"Engine");

	D3D11::GEngine* engine = D3D11::GEngine::GetInstance();
	engine->InitEngine(AppWidth, AppHeight, 240, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM, 2,
		testApp->GetWindowHandle(),
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	testApp->Init();
	testApp->Run();
	testApp->Quit();
}