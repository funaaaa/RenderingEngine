#include "RenderingEngine.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

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
	m_device->SetName(L"Device");

	//コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC queueDesc{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	m_commandQueue->SetName(L"MainCommandQueue");

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
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV,scDesc.BufferCount,D3D12_DESCRIPTOR_HEAP_FLAG_NONE,0 };
	m_device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvHeap->SetName(L"RTVHeap");
	m_rtvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//レンダーターゲットビューを生成
	m_renderTargets.resize(FRAMEBUFFER_COUNT);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT index = 0; index < FRAMEBUFFER_COUNT; ++index) {

		m_swapchain->GetBuffer(index, IID_PPV_ARGS(&m_renderTargets[index]));
		m_device->CreateRenderTargetView(m_renderTargets[index].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvHeapSize);

	}
	m_renderTargets[0]->SetName(L"SwapchainRenderTarget0");
	m_renderTargets[1]->SetName(L"SwapchainRenderTarget1");

	//深度用のディスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
	m_dsvHeap->SetName(L"DSVHeap");

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
	m_depthStencil->SetName(L"DepthStencil");

	//深度ビューを生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//コマンドアロケーターを生成
	m_commandAllocator.resize(FRAMEBUFFER_COUNT);
	for (UINT index = 0; index < FRAMEBUFFER_COUNT; ++index)
	{
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocator[index])
		);
	}
	m_commandAllocator[0]->SetName(L"CommandAllocator0");
	m_commandAllocator[1]->SetName(L"CommandAllocator1");

	//フレーム同期用のフェンスを生成
	m_frameFence.resize(FRAMEBUFFER_COUNT);
	m_frameFenceValue.resize(FRAMEBUFFER_COUNT);
	for (UINT index = 0; index < FRAMEBUFFER_COUNT; ++index)
	{
		m_device->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_frameFence[index]));
		m_frameFenceValue[index] = 0;
	}
	m_frameFence[0]->SetName(L"FrameFence0");
	m_frameFence[1]->SetName(L"FrameFence1");

	//コマンドリストの生成
	m_device->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator[0].Get(),
		nullptr, IID_PPV_ARGS(&m_commandList));
	m_commandList->SetName(L"MainCommandList");
	m_commandList->Close();



}

void Engine::RenderingEngine::Render()
{

	//スワップチェーンインデックスを取得
	UINT frameIndex = m_swapchain->GetCurrentBackBufferIndex();

	//コマンドリストの中身をクリア
	m_commandAllocator[frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocator[frameIndex].Get(), nullptr);

	//スワップチェーン表示可能状態からレンダーターゲット描画可能状態へ
	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrierToRT);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, m_rtvHeapSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	//カラーバッファのクリア
	const float clearColor[] = { 0.30f, 0.25f, 0.22f,1.0f };
	m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	//デプスバッファのクリア
	m_commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//描画先をセット
	m_commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	//レンダーターゲット描画可能状態からスワップチェーン表示可能状態へ
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_commandList->ResourceBarrier(1, &barrierToPresent);

	//描画コマンドの積み込み終了
	m_commandList->Close();

	//コマンドリストをキューに投入して実行
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, lists);
	//画面へ表示
	m_swapchain->Present(1, 0);

	//現在のフレームフェンスにGPUが到達後設定される値をセット
	auto& fence = m_frameFence[frameIndex];
	const auto currentValue = ++m_frameFenceValue[frameIndex];
	m_commandQueue->Signal(fence.Get(), currentValue);
	//次処理するコマンドアロケーターは既に実⾏完了済みかを対になっているフェンスで確認
	frameIndex = (frameIndex + 1) % FRAMEBUFFER_COUNT;
	const auto finishExpected = m_frameFenceValue[frameIndex];
	const auto nextFenceValue = m_frameFence[frameIndex]->GetCompletedValue();
	if (nextFenceValue < finishExpected)
	{
		//GPUが処理中のため、イベントで待機
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		m_frameFence[frameIndex]->SetEventOnCompletion(finishExpected, event);
		WaitForSingleObject(event, INFINITE);

	}


}
