#pragma once
#include "common.h"
#include "globals.h"
#include "game.h"

namespace engine
{

	// D3D Globals
	extern ENGINE_API DeviceContext* deferredContext;
	extern ENGINE_API SwapChain* swapChain;
	extern ENGINE_API RenderTargetView* renderTarget;
	extern ENGINE_API DepthTargetView* depthTarget;
	extern ENGINE_API ID3D11DepthStencilState* depthState;

	extern ENGINE_API ID3D11RasterizerState* rasterizerState;

	extern ENGINE_API ShaderBlob* vsBlob;
	extern ENGINE_API ShaderBlob* psBlob;
	extern ENGINE_API ShaderBlob* error_blob;
	extern ENGINE_API VertexShader* vertexShader;
	extern ENGINE_API PixelShader* pixelShader;

	extern ENGINE_API DirectX::Mouse mouse;
	extern ENGINE_API DirectX::Keyboard keyboard;

	inline ENGINE_API bool g_DirectXInitialized;
	ENGINE_API void DirectXSetup(HWND hwnd);

	ENGINE_API void ResizeWindow();

	ENGINE_API void ClearBackground(DirectX::SimpleMath::Color color);

	ENGINE_API void DoGameSetup();

	ENGINE_API ImGuiContext* GetImGuiContext();

	ENGINE_API bool DoGameLoop(Game& game, float dt);

	ENGINE_API void EngineShutdown();
}