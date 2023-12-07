#include "Engine/DirectX/d3dx12.h"
#include "Engine/RenderingEngine/RenderingEngine.h"
#include "Engine/Window/Window.h"
#include <memory>

HRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// ⼀ 般 的 な W Mメッセージプロシージャの実装 (省略)
	// :


	HRESULT result = HRESULT();

	return result;

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

	//エンジン本体
	std::unique_ptr<Engine::RenderingEngine> engine;
	engine = std::make_unique<Engine::RenderingEngine>();

	//ウィンドウを作成。
	Engine::Window window;
	window.Create(hInstance, engine.get(), L"RenderingEngine");

	//エンジンを初期化。
	engine->Initialize(window.GetHWND());

	//ウィンドウを表示。
	window.Show(engine.get(), nCmdShow);

	//ゲームループ
	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);			DispatchMessage(&msg);
		}
		engine->Render();
	}

	//終了処理。
	//engine.Terminate();
	//return msg.wParam;
	return 0;
}