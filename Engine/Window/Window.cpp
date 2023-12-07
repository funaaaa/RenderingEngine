#include "Window.h"
#include <Windows.h>

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	//ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void Engine::Window::Create(HINSTANCE arg_hInstance, Engine::RenderingEngine* arg_engine, LPCWSTR arg_windowName)
{


	WNDCLASSEX window{};
	window.cbSize = sizeof(WNDCLASSEX);
	window.lpfnWndProc = (WNDPROC)WindowProc;
	window.lpszClassName = L"DirectX";
	window.hInstance = GetModuleHandle(nullptr);
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	//window.hIcon = LoadIcon(window.hInstance, TEXT("icon"));

	//ウィンドウクラスをOSに登録
	int CheckError = RegisterClassEx(&window);
	if (!CheckError)
	{
		return assert(0);
	}

	//ウィンドウを生成。
	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = { 0,0,WINDOW_RECT.x, WINDOW_RECT.y };
	AdjustWindowRect(&rect, dwStyle, FALSE);
	m_hwnd = CreateWindow(window.lpszClassName, arg_windowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, arg_hInstance, arg_engine);

}

void Engine::Window::Show(Engine::RenderingEngine* arg_engine, int arg_nCmdShow)
{

	SetWindowLongPtr(m_hwnd, GWLP_USERDATA,
		reinterpret_cast<LONG_PTR>(arg_engine));
	ShowWindow(m_hwnd, arg_nCmdShow);

}
