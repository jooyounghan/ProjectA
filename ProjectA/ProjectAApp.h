#pragma once
#include "BaseApp.h"

#include <d3d11.h>
#include <memory>
#include <vector>

class IUpdatable;
class CCamera;
class CParticleEmitter;

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
	virtual void Create(
		UINT width,
		UINT height,
		const wchar_t* className,
		const wchar_t* applicaitonName
	) noexcept override;

public:
	virtual void Init() override;
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
	std::vector<IUpdatable*> m_updatables;

private:
	std::unique_ptr<CCamera> m_camera;
	std::unique_ptr<CParticleEmitter> m_particleEmitter;
};
