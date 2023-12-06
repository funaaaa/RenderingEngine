#include "Engine/DirectX/d3dx12.h"
#include "Engine/RenderingEngine/RenderingEngine.h"

HRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// ⼀ 般 的 な W Mメッセージプロシージャの実装 (省略)
	// :
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

	//エンジン本体
	Engine::RenderingEngine engine;

	//デバッグレイヤー有効化。
	Microsoft::WRL::ComPtr<ID3D12Debug> debugLayer;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();
	}
	//GPUBased検証を有効化。
	Microsoft::WRL::ComPtr<ID3D12Debug3> gpuBasedValidation;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&gpuBasedValidation))))
	{
		gpuBasedValidation->SetEnableGPUBasedValidation(true);
	}

	//ウィンドウを生成。
	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = { 0,0,engine.GetWindowRect().x, engine.GetWindowRect().y };
	AdjustWindowRect(&rect, dwStyle, FALSE);
	auto hwnd = CreateWindow(L"RenderingEngine", L"RenderingEngine", dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, hInstance, &engine);

	//エンジンを初期化。
	engine.Initialize(hwnd);

	//ウィンドウを表示。
	SetWindowLongPtr(hwnd, GWLP_USERDATA,
		reinterpret_cast<LONG_PTR>(&engine));
	ShowWindow(hwnd, nCmdShow);

	//ゲームループ
	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);			DispatchMessage(&msg);
		}
		engine.Render();
	}

	//終了処理。
	engine.Terminate();
	return msg.wParam;
}