#pragma once
#include "BaseApp.h"
#include "Camera.h"

#include <d3d11.h>
#include <memory>

class ProjectAApp : public App::CBaseApp
{
protected:
	ProjectAApp() noexcept;
	ProjectAApp(const ProjectAApp& baseApp) = delete;
	ProjectAApp(ProjectAApp&& baseApp) = delete;

public:
	virtual ~ProjectAApp() = default;

public:
	static ProjectAApp* GetInstance() noexcept;

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
	ID3D11RenderTargetView* m_backBufferRTV = nullptr;
	IDXGISwapChain* m_swapchain = nullptr;

private:
	std::unique_ptr<Camera> m_camera;
};
