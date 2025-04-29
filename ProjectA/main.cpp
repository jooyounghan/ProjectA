#include "ProjectAApp.h"
#include "GEngine.h"
#include <iostream>
#include <pix.h>

int main()
{
	UINT AppWidth = 1920;
	UINT AppHeight = 1080;

	CProjectAApp* testApp = CProjectAApp::GetInstance();
	testApp->Create(AppWidth, AppHeight, L"Test", L"Engine");

	UINT& AppWidthRef = testApp->GetWidth();
	UINT& AppHeightRef = testApp->GetHeight();

	D3D11::GEngine* engine = D3D11::GEngine::GetInstance();
	engine->InitEngine(AppWidthRef, AppHeightRef, 240, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM, 2,
		testApp->GetWindowHandle(),
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	testApp->Init();
	testApp->Run();
	testApp->Quit();
}