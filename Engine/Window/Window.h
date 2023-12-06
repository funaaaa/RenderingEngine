#pragma once
#include "../DirectX/d3dx12.h"
#include "../Math/Vec.h"

namespace Engine {
	class RenderingEngine;
}

namespace Engine {

	class Window {

	private:

		HWND m_hwnd;

		//�E�B���h�E�T�C�Y
		Math::Vec2<int> WINDOW_RECT = Math::Vec2<int>(1280, 720);

	public:

		//�E�B���h�E���쐬�B
		void Create(HINSTANCE arg_hInstance, Engine::RenderingEngine* arg_engine, LPCWSTR arg_windowName);
		void Show(Engine::RenderingEngine* arg_engine, int arg_nCmdShow);

		HWND GetHWND() { return m_hwnd; }
		Math::Vec2<int> GetWindowRect() { return WINDOW_RECT; }

	};

}