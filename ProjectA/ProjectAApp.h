#pragma once
#include "BaseApp.h"

#include <d3d11.h>
#include <memory>
#include <vector>

#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "ComputeShader.h"

#include "DynamicBuffer.h"

class IUpdatable;
class CCamera;
class CParticleManager;

namespace D3D11
{
	class CGraphicsPSOObject;
}

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
	struct  
	{
		float dt;
		float appWidth;
		float appHeight;
		float dummy;
	} m_appParamsCPU;
	D3D11::CDynamicBuffer m_appParamsGPU;

#pragma region 테스트 변수
	std::unique_ptr<CCamera> m_camera;
	std::unique_ptr<CParticleManager> m_particleManager;

private:
	D3D11::CVertexShader m_drawParticleVS;
	D3D11::CGeometryShader m_drawParticleGS;
	D3D11::CPixelShader m_drawParticlePS;
	std::unique_ptr<D3D11::CGraphicsPSOObject> m_drawParticlePSO;
#pragma endregion
};
