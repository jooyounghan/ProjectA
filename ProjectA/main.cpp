#include "ProjectAApp.h"
#include "GEngine.h"
#include <iostream>
#include <pix.h>

int main()
{
	UINT AppWidth = 1920;
	UINT AppHeight = 1080;

	CProjectAApp* testApp = CProjectAApp::GetInstance();
	testApp->Init(AppWidth, AppHeight, L"Test", L"Engine");
	testApp->Run();
	testApp->Quit();
}