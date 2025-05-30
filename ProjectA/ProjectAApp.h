#pragma once
#include "BaseApp.h"
#include <d3d11.h>
#include <memory>
#include <vector>
#include <string>

namespace D3D11
{
	class CGraphicsPSOObject;
	class CDynamicBuffer;
}

class IUpdatable;
class CCamera;
class CBaseFilm;
class CShotFilm;
class CEmitterSelector;
class AEmitterManager;

class CProjectAApp : public App::CBaseApp
{
protected:
	CProjectAApp() noexcept;
	CProjectAApp(const CProjectAApp& baseApp) = delete;
	CProjectAApp(CProjectAApp&& baseApp) = delete;

public:
	virtual ~CProjectAApp() = default;

public:
	static CProjectAApp* GetInstance() noexcept;

public:
	virtual void Init(
		UINT width, UINT height,
		const wchar_t* className,
		const wchar_t* applicaitonName
	) override;
	virtual void Update(float deltaTime) override;
	virtual void Quit() override;
	virtual void AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	void DrawUI();

private:
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	ID3D11Texture2D* m_backBuffer = nullptr;
	ID3D11RenderTargetView* m_backBufferRTV = nullptr;
	IDXGISwapChain* m_swapchain = nullptr;

private:
	struct alignas(16)
	{
		float dt;
		float appWidth;
		float appHeight;
		UINT appDummy;
	} m_appParamsCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_appParamsGPU;

#pragma region 테스트 변수
	std::unique_ptr<CCamera> m_camera;
	std::unique_ptr<CShotFilm> m_shotFilm;
	std::unique_ptr<CBaseFilm> m_normalVectorFilm;
	std::unique_ptr<CEmitterSelector> m_emitterSelector;
	std::vector<std::unique_ptr<AEmitterManager>> m_emitterManagers;

private:
	void DrawEmitterHandler();
	void DrawEmitterSelector(const std::string& emitterName, int& emitterSelectIndex, AEmitterManager* emitterManager);
#pragma endregion
};