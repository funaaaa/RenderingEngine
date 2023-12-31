﻿#include "RenderingEngine.h"
#include <fstream>
#include <filesystem>
#include <dxcapi.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

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






	/*-- 後でまとめるやつら --*/

	//シェーダー読み込み
	CompileShaderFromFile(L"Resource/ShaderFiles/BasicShader/VertexShader.hlsl", L"vs_6_0", m_vs, m_errorBlob);
	CompileShaderFromFile(L"Resource/ShaderFiles/BasicShader/PixelShader.hlsl", L"ps_6_0", m_ps, m_errorBlob);

	//頂点データとインデックスデータを設定。
	m_vertex.emplace_back(Vertex({ 0.0f, 0.25f, 0.0f }, { 0.80f, 0.71f, 0.64f, 1.0f }));
	m_vertex.emplace_back(Vertex({ 0.25f, -0.25f, 0.0f }, { 0.60f, 0.45f, 0.50f, 1.0f }));
	m_vertex.emplace_back(Vertex({ -0.25f, -0.25f, 0.0f }, { 0.90f, 0.85f, 0.80f, 1.0f }));
	m_index.emplace_back(0);
	m_index.emplace_back(1);
	m_index.emplace_back(2);

	//頂点バッファを生成。
	UINT vertexBufferSize = static_cast<UINT>(m_vertex.size());
	UINT indexBufferSize = static_cast<UINT>(m_index.size());
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize * sizeof(Vertex)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	//インデックスバッファを生成。
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize * sizeof(uint32_t)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer));


	//データ転送
	void* pVertexData;
	CD3DX12_RANGE vbRange(0, 0);
	m_vertexBuffer->Map(0, &vbRange, &pVertexData);
	memcpy(pVertexData, m_vertex.data(), vertexBufferSize * sizeof(Vertex));
	m_vertexBuffer->Unmap(0, nullptr);
	void* pIndexData;
	CD3DX12_RANGE ibRange(0, 0);
	m_indexBuffer->Map(0, &ibRange, &pIndexData);
	memcpy(pIndexData, m_index.data(), indexBufferSize * sizeof(uint32_t));
	m_indexBuffer->Unmap(0, nullptr);

	//頂点バッファビューを生成。
	m_vertexBufferView.BufferLocation =
		m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize * sizeof(Vertex);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	//インデックスバッファビューを生成。
	m_indexBufferView.BufferLocation =
		m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = indexBufferSize * sizeof(uint32_t);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//ルートシグネチャの構築
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		0, nullptr,   //pParameters
		0, nullptr,   //pStaticSamplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &m_errorBlob);
	//RootSignature の生成
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	//インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, m_pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, offsetof(Vertex, m_color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};

	//パイプラインステートオブジェクトの生成.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	//シェーダーのセット
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());
	//ブレンドステート設定
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//ラスタライザーステート
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//出力先は1ターゲット
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//デプスバッファのフォーマットを設定
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//ルートシグネチャのセット
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//マルチサンプル設定
	psoDesc.SampleDesc = { 1,0 };
	psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない＆警告も出ないので注意.
	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));

	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(width), float(height));
	m_scissorRect = CD3DX12_RECT(0, 0, LONG(width), LONG(height));

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






	//パイプラインステートのセット
	m_commandList->SetPipelineState(m_pipeline.Get());
	//ルートシグネチャのセット
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	//ビューポートとシザーのセット
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	//プリミティブタイプ、頂点・インデックスバッファのセット
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	//描画命令の発行
	m_commandList->DrawIndexedInstanced(static_cast<UINT>(m_index.size()), 1, 0, 0, 0);




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

HRESULT Engine::RenderingEngine::CompileShaderFromFile(const std::wstring& arg_fileName, const std::wstring& arg_profile, Microsoft::WRL::ComPtr<ID3DBlob>& arg_shaderBlob, Microsoft::WRL::ComPtr<ID3DBlob>& arg_errorMsg)
{

	std::filesystem::path filePath(arg_fileName);
	std::ifstream infile(filePath);
	std::vector<char> srcData;
	if (!infile)
		throw std::runtime_error("shader not found");
	srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
	infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

	//DXCによるコンパイル処理
	Microsoft::WRL::ComPtr<IDxcLibrary> library;
	Microsoft::WRL::ComPtr<IDxcCompiler> compiler;
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
	Microsoft::WRL::ComPtr<IDxcOperationResult> dxcResult;

	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	library->CreateBlobWithEncodingFromPinned(srcData.data(), UINT(srcData.size()), CP_ACP, &source);
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	LPCWSTR compilerFlags[] = {
  #if _DEBUG
	  L"/Zi", L"/O0",
  #else
	  L"/O2" //リリースビルドでは最適化
  #endif
	};
	compiler->Compile(source.Get(), filePath.wstring().c_str(),
		L"main", arg_profile.c_str(),
		compilerFlags, _countof(compilerFlags),
		nullptr, 0, //Defines
		nullptr,
		&dxcResult);

	HRESULT hr;
	dxcResult->GetStatus(&hr);
	if (SUCCEEDED(hr))
	{
		dxcResult->GetResult(
			reinterpret_cast<IDxcBlob**>(arg_shaderBlob.GetAddressOf())
		);
	}
	else
	{
		dxcResult->GetErrorBuffer(
			reinterpret_cast<IDxcBlobEncoding**>(arg_errorMsg.GetAddressOf())
		);

		//GetBufferPointerとGetBufferSizeを使って内容にアクセス
		const char* pData = static_cast<const char*>(arg_errorMsg->GetBufferPointer());
		size_t size = arg_errorMsg->GetBufferSize();

		//バッファを文字列として読む
		std::string content(pData, pData + size);

		//VisualStudioの出力ウィンドウに出力
		OutputDebugStringA(content.c_str());

	}
	return hr;
}
