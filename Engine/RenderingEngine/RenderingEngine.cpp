#include "RenderingEngine.h"

Engine::RenderingEngine::RenderingEngine()
{
}

void Engine::RenderingEngine::Initialize(HWND arg_hwnd)
{

#ifdef _DEBUG

	//デバッグレイヤー有効化。
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugLayer))))
	{
		m_debugLayer->EnableDebugLayer();
	}
	//GPUBased検証を有効化。
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_gpuBasedValidation))))
	{
		m_gpuBasedValidation->SetEnableGPUBasedValidation(true);
	}

#endif

	//DXGIFactoryを生成
#ifdef _DEBUG
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
#else
	CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory));
#endif

	//使用するハードウェアアダプターを検索
	UINT adapterIndex = 0;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	while (DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(adapterIndex, &adapter)) {

		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1(&desc);
		++adapterIndex;
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		//D3D12が使えるか？
		HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	//使用するアダプターを代入
	adapter.As(&m_adapter);

	//デバイスを生成。
	D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

	//コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC queueDesc{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

	//スワップチェーンを生成
	RECT rect = {};
	GetClientRect(arg_hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	DXGI_SWAP_CHAIN_DESC1 scDesc{};
	scDesc.BufferCount = FRAMEBUFFER_COUNT;
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.SampleDesc.Count = 1;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain;
	m_factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(), arg_hwnd,
		&scDesc, nullptr, nullptr, &swapchain);
	swapchain.As(&m_swapchain);

	//レンダーターゲットビュー用ディスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	scDesc.BufferCount,		//スワップチェーンで指定したバッファ数
	D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	0
	};
	m_device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//レンダーターゲットビューを生成
	m_renderTargets.resize(FRAMEBUFFER_COUNT);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT index = 0; index < FRAMEBUFFER_COUNT; ++index) {

		m_swapchain->GetBuffer(index, IID_PPV_ARGS(&m_renderTargets[index]));
		m_device->CreateRenderTargetView(m_renderTargets[index].Get(), nullptr, rtvHandle);

	}

	//深度用のディスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));

	//深度バッファを生成
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		width, height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.Format = resDesc.Format;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	);

	//深度ビューを生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc
	{
	DXGI_FORMAT_D32_FLOAT,
	D3D12_DSV_DIMENSION_TEXTURE2D,
	D3D12_DSV_FLAG_NONE,
	{0}
	};
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc,	dsvHandle);

	//コマンドアロケーターを生成。
	m_commandAllocator.resize(FRAMEBUFFER_COUNT);
	for (int i = 0; i < FrameBufferCount; ++i)
	{
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocators[i])
		);
	}




}
