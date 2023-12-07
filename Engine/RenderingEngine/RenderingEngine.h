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

		//�f�o�C�X�֘A
		Microsoft::WRL::ComPtr<IDXGIFactory3> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

		//�X���b�v�`�F�[��
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapchain;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_renderTargets;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;

		//�f�B�X�N���v�^�q�[�v
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		UINT m_rtvHeapSize;

		//�R�}���h�֘A
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_commandAllocator;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Fence1>> m_frameFence;
		std::vector<UINT> m_frameFenceValue;

		//�f�o�b�O�֘A
		Microsoft::WRL::ComPtr<ID3D12Debug> m_debugLayer;
		Microsoft::WRL::ComPtr<ID3D12Debug3> m_gpuBasedValidation;

		const UINT FRAMEBUFFER_COUNT = 2;

	public:

		RenderingEngine();
		virtual ~RenderingEngine() {};

		void Initialize(HWND arg_hwnd);
		void Terminate() {};

		void Render();

		//�ȉ��h����ŏ͂��Ƃɓ��e���I�[�o�[���[�h����炵���B
		virtual void Prepare() {};
		virtual void Cleanup() {};
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& arg_command) {};

	};

}