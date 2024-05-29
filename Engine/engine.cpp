#include "engine.h"

#include "filesystem"
#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/CommonStates.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include "audio.h"


namespace engine
{
	using namespace DirectX;
	using namespace SimpleMath;

	ENGINE_API DeviceContext* deferredContext;
	ENGINE_API SwapChain* swapChain;
	ENGINE_API RenderTargetView* renderTarget;
	ENGINE_API DepthTargetView* depthTarget;

	ENGINE_API ID3D11DepthStencilState* depthState;

	ENGINE_API ID3D11RasterizerState* rasterizerState;

	ENGINE_API ShaderBlob* vsBlob;
	ENGINE_API ShaderBlob* psBlob;
	ENGINE_API ShaderBlob* error_blob;
	ENGINE_API VertexShader* vertexShader;
	ENGINE_API PixelShader* pixelShader;

	ENGINE_API DirectX::Mouse mouse;
	ENGINE_API DirectX::Keyboard keyboard;
	ENGINE_API void DirectXSetup(HWND hwnd)
	{
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		// Init DX11
		DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
		swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
		swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
		swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swap_chain_descr.SampleDesc.Count = 1;
		swap_chain_descr.SampleDesc.Quality = 0;
		swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_descr.BufferCount = 1;
		swap_chain_descr.OutputWindow = hwnd;
		swap_chain_descr.Windowed = true;

		D3D_FEATURE_LEVEL feature_level;
		D3D_FEATURE_LEVEL feature_levels[] =
		{
			D3D_FEATURE_LEVEL_11_1
		};

		UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

		IDXGIFactory1* factory;
		CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));

		UINT i = 0;
		IDXGIAdapter1* pAdapter;
		std::vector <IDXGIAdapter1*> vAdapters;
		while (factory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);

			DXGI_ADAPTER_DESC1 desc;
			HRESULT h = pAdapter->GetDesc1(&desc);
			assert(SUCCEEDED(h));
			DebugOut(L"Adapter %d found: name: %s\n", i, desc.Description);

			++i;
		}

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			feature_levels,
			ARRAYSIZE(feature_levels),
			D3D11_SDK_VERSION,
			&swap_chain_descr,
			&swapChain,
			&device,
			&feature_level,
			&immediateContext);

		assert(S_OK == hr && swapChain && device && immediateContext);

		//hr = device->CreateDeferredContext(0, &deferredContext);

		//assert(SUCCEEDED(hr));

		DebugOut(L"feature level: %d\n", feature_level);

		// create a framebuffer
		ID3D11Texture2D* framebuffer;
		hr = swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)&framebuffer);
		assert(SUCCEEDED(hr));

		hr = device->CreateRenderTargetView(
			framebuffer, 0, &renderTarget);
		assert(SUCCEEDED(hr));
		framebuffer->Release();

		// resize
		D3D11_TEXTURE2D_DESC desc;
		framebuffer->GetDesc(&desc);
		engine::g_WindowWidth = desc.Width;
		engine::g_WindowHeight = desc.Height;

		std::unique_ptr<CommonStates> states(new CommonStates(device));

		immediateContext->OMSetBlendState(states->Opaque(), Color(0, 0, 0, 0), 0xFFFFFFFF);
		immediateContext->OMSetDepthStencilState(states->DepthDefault(), 0);

		rasterizerState = states->CullClockwise();
		immediateContext->RSSetState(rasterizerState);

		auto samplerState = states->LinearWrap();
		immediateContext->PSSetSamplers(0, 1, &samplerState);

		D3D11_TEXTURE2D_DESC depthTextureDesc;
		ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
		depthTextureDesc.Width = desc.Width;
		depthTextureDesc.Height = desc.Height;
		depthTextureDesc.MipLevels = 1;
		depthTextureDesc.ArraySize = 1;
		depthTextureDesc.SampleDesc.Count = 1;
		depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* DepthStencilTexture;
		hr = device->CreateTexture2D(&depthTextureDesc, NULL, &DepthStencilTexture);

		assert(SUCCEEDED(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = depthTextureDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

		hr = device->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &depthTarget);
		DepthStencilTexture->Release();

		assert(SUCCEEDED(hr));

		g_DirectXInitialized = true;

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device, immediateContext);
	}

	ENGINE_API void ResizeWindow()
	{
		immediateContext->OMSetRenderTargets(0, 0, 0);
		renderTarget->Release();
		depthTarget->Release();

		// render target
		HRESULT res = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		assert(SUCCEEDED(res));

		ID3D11Texture2D* frameBuffer;
		res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer);
		assert(SUCCEEDED(res));

		res = device->CreateRenderTargetView(frameBuffer, NULL, &renderTarget);

		assert(SUCCEEDED(res));

		D3D11_TEXTURE2D_DESC desc;
		frameBuffer->GetDesc(&desc);
		DebugOut(L"framebuffer resized: %dx%d\n", desc.Width, desc.Height);
		engine::g_WindowWidth = desc.Width;
		engine::g_WindowHeight = desc.Height;

		frameBuffer->Release();

		// depth
		D3D11_TEXTURE2D_DESC depthTextureDesc;
		ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
		depthTextureDesc.Width = desc.Width;
		depthTextureDesc.Height = desc.Height;
		depthTextureDesc.MipLevels = 1;
		depthTextureDesc.ArraySize = 1;
		depthTextureDesc.SampleDesc.Count = 1;
		depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* DepthStencilTexture;
		res = device->CreateTexture2D(&depthTextureDesc, NULL, &DepthStencilTexture);

		assert(SUCCEEDED(res));

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = depthTextureDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

		res = device->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &depthTarget);

		assert(SUCCEEDED(res));

		DepthStencilTexture->Release();

		Renderer& rend = Renderer::Get();
		if (rend.camera)
		{
			rend.camera->RecalculateProjection();
		}
	}

	ENGINE_API void ClearBackground(DirectX::SimpleMath::Color color)
	{
		immediateContext->ClearRenderTargetView(renderTarget, color);
		immediateContext->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	ENGINE_API ImGuiContext* GetImGuiContext()
	{
		return ImGui::GetCurrentContext();
	}

	ENGINE_API bool DoGameLoop(Game& game, float deltaTime)
	{
		using namespace std::chrono;

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool running = game.Update(deltaTime);

		ClearBackground(Color(0.f, 0.f, 0.f, 0.f));

		D3D11_VIEWPORT viewport = {
		  0.0f,
		  0.0f,
		  float(engine::g_WindowWidth),
		  float(engine::g_WindowHeight),
		  0.0f,
		  1.0f };

		immediateContext->RSSetViewports(1, &viewport);
		immediateContext->RSSetState(rasterizerState);
		immediateContext->OMSetRenderTargets(1, &renderTarget, depthTarget);
		immediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto rend = engine::Renderer::Get();

		rend.SetContext(&immediateContext);
		rend.PreUpdate();

		game.Render(rend, ImGui::GetCurrentContext(), deltaTime);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapChain->Present(0, 0);

		Mouse::Get().EndOfInputFrame();

		audio::fmodSystem->update();

		return true;
	}

	ENGINE_API void DoGameSetup()
	{

	}

	ENGINE_API void EngineShutdown()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}