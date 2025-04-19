#pragma once
#ifndef BASEAPP_H
#define BASEAPE_H

#include "AppManagerDllHelper.h"

#include <Windows.h>
#include <optional>
#include <functional>

template class APPMANAGER_API std::function<void(const UINT& width, const UINT& height)>;

namespace App
{
	using WndSizeUpdateHandler = std::function<void(const UINT& width, const UINT& height)>;

	class APPMANAGER_API CBaseApp
	{
	protected:
		CBaseApp() noexcept;
		CBaseApp(const CBaseApp& baseApp) = delete;
		CBaseApp(CBaseApp&& baseApp) = delete;

	public:
		virtual ~CBaseApp() = default;

	public:
		static CBaseApp* MainApp;

	protected:
		WNDCLASSEX m_windowClass;
		HWND m_mainWindow = NULL;

	public:
		inline HWND GetWindowHandle() const noexcept { return m_mainWindow; }
		
	protected:
		UINT m_width;
		UINT m_height;

	public:
		UINT& GetWidth() { return m_width; }
		UINT& GetHeight() { return m_height; }

	protected:
		LARGE_INTEGER m_frequency;
		LARGE_INTEGER m_prevTime;

	protected:
		WndSizeUpdateHandler m_wndSizeUpdateHandler = [&](UINT, UINT) {};

	public:
		inline void SetWindowSizeUpdateHandler(WndSizeUpdateHandler handler) noexcept { m_wndSizeUpdateHandler = handler; }

	public:
		void Run();

	public:
		LRESULT WINAPI AppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	public:
		virtual void Create(UINT width, UINT height, const wchar_t* className, const wchar_t* applicaitonName) noexcept;

	public:
		virtual void Init() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Quit() = 0;
		virtual void AppProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	private:
		void OnWindowSizeUpdate() noexcept;
		float GetDeltaTime() noexcept;
	};
}
#endif