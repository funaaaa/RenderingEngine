#include "RenderingEngine.h"

Engine::RenderingEngine::RenderingEngine()
{
}

void Engine::RenderingEngine::Initialize(HWND arg_hwnd)
{

#ifdef _DEBUG

	//�f�o�b�O���C���[�L�����B
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugLayer))))
	{
		m_debugLayer->EnableDebugLayer();
	}
	//GPUBased���؂�L�����B
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_gpuBasedValidation))))
	{
		m_gpuBasedValidation->SetEnableGPUBasedValidation(true);
	}

#endif

	//DXGIFactory�𐶐�
#ifdef _DEBUG
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
#else
	CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory));
#endif

	//�g�p����n�[�h�E�F�A�A�_�v�^�[������
	UINT adapterIndex = 0;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	while (DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(adapterIndex, &adapter)) {

		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1(&desc);
		++adapterIndex;
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		//D3D12���g���邩�H
		HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	//�g�p����A�_�v�^�[����
	adapter.As(&m_adapter);

	//�f�o�C�X�𐶐��B
	D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

	//�R�}���h�L���[�̐���
	D3D12_COMMAND_QUEUE_DESC queueDesc{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

	//�X���b�v�`�F�[���𐶐�
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

	//�����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v�𐶐�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	scDesc.BufferCount,		//�X���b�v�`�F�[���Ŏw�肵���o�b�t�@��
	D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	0
	};
	m_device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�����_�[�^�[�Q�b�g�r���[�𐶐�
	m_renderTargets.resize(FRAMEBUFFER_COUNT);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT index = 0; index < FRAMEBUFFER_COUNT; ++index) {

		m_swapchain->GetBuffer(index, IID_PPV_ARGS(&m_renderTargets[index]));
		m_device->CreateRenderTargetView(m_renderTargets[index].Get(), nullptr, rtvHandle);

	}

	//�[�x�p�̃f�B�X�N���v�^�q�[�v�𐶐�
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));

	//�[�x�o�b�t�@�𐶐�
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

	//�[�x�r���[�𐶐�
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc
	{
	DXGI_FORMAT_D32_FLOAT,
	D3D12_DSV_DIMENSION_TEXTURE2D,
	D3D12_DSV_FLAG_NONE,
	{0}
	};
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc,	dsvHandle);

	//�R�}���h�A���P�[�^�[�𐶐��B
	m_commandAllocator.resize(FRAMEBUFFER_COUNT);
	for (int i = 0; i < FrameBufferCount; ++i)
	{
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocators[i])
		);
	}




}
