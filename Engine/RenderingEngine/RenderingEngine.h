#pragma once
#include <wrl.h>
#include <DirectXTex.h>
#include <dxgi1_3.h>
#include <dxgi1_5.h>
#include <vector>
#include "../DirectX/d3dx12.h"
#include "../Math/Vec.h"

namespace Engine {

	class RenderingEngine {

	protected:

		//デバイス関連
		Microsoft::WRL::ComPtr<IDXGIFactory3> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

		//スワップチェーン
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapchain;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_renderTargets;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;

		//ディスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		UINT m_rtvHeapSize;

		//コマンド関連
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_commandAllocator;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Fence1>> m_frameFence;
		std::vector<UINT> m_frameFenceValue;

		//デバッグ関連
		Microsoft::WRL::ComPtr<ID3D12Debug> m_debugLayer;
		Microsoft::WRL::ComPtr<ID3D12Debug3> m_gpuBasedValidation;

		const UINT FRAMEBUFFER_COUNT = 2;

	public:

		RenderingEngine();
		virtual ~RenderingEngine() {};

		void Initialize(HWND arg_hwnd);
		void Terminate() {};

		void Render();

		//以下派生先で章ごとに内容をオーバーラードするらしい。
		virtual void Prepare() {};
		virtual void Cleanup() {};
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& arg_command) {};

	};

}