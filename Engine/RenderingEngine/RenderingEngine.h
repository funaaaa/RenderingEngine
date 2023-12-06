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

		//�ȉ��h����ŏ͂��Ƃɓ��e���I�[�o�[���[�h����炵���B
		virtual void Prepare() {};
		virtual void Cleanup() {};
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& arg_command) {};

		//�E�B���h�E�T�C�Y���擾�B
		Math::Vec2<int> GetWindowRect() { return WINDOW_RECT; }

	};

}