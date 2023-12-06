#include "Window.h"

void Engine::Window::Create(HINSTANCE arg_hInstance, Engine::RenderingEngine* arg_engine, LPCWSTR arg_windowName)
{

	//ウィンドウを生成。
	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = { 0,0,WINDOW_RECT.x, WINDOW_RECT.y };
	AdjustWindowRect(&rect, dwStyle, FALSE);
	m_hwnd = CreateWindow(arg_windowName, arg_windowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, arg_hInstance, arg_engine);

}

void Engine::Window::Show(Engine::RenderingEngine* arg_engine, int arg_nCmdShow)
{

	SetWindowLongPtr(m_hwnd, GWLP_USERDATA,
		reinterpret_cast<LONG_PTR>(arg_engine));
	ShowWindow(m_hwnd, arg_nCmdShow);

}
