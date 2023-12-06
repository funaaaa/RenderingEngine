#pragma once
#include <wrl.h>
#include "../DirectX/d3dx12.h"
#include "../Math/Vec.h"

namespace Engine {

	class RenderingEngine {

	protected:

		Math::Vec2<int> WINDOW_RECT = Math::Vec2<int>(1280, 720);
		const UINT FRAMEBUFFER_COUNT = 2;

	public:

		RenderingEngine();
		virtual ~RenderingEngine();

		void Initialize(HWND arg_hwnd);
		void Terminate();

		virtual void Render();

		//以下派生先で章ごとに内容をオーバーラードするらしい。
		virtual void Prepare() {};
		virtual void Cleanup() {};
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& arg_command) {};

		//ウィンドウサイズを取得。
		Math::Vec2<int> GetWindowRect() { return WINDOW_RECT; }

	};

}