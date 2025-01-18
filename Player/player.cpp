#include <engine.h>
#include <constant_buffer.h>
#include <game.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>

#include <windows.h>

#include <hidusage.h>
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

#include <format>

#include "audiotest.h"
#include <TestGame.h>

#pragma comment( lib, "engine.lib" )
#pragma comment( lib, "game.lib" )

using namespace DirectX;
using namespace SimpleMath;

bool global_windowResize = false;

TestGame game;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    //wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"yawnengineapp";
    if (!RegisterClass(&wc))
        return 1;

    HWND hwnd = CreateWindow(wc.lpszClassName, L"Player", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1280, 720, 0, 0, hInstance, NULL);

    RAWINPUTDEVICE Rid{};
    Rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid.usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid.dwFlags = RIDEV_INPUTSINK;
    Rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
    
    assert(hwnd != NULL);

    engine::DirectXSetup(hwnd);
    engine::audio::InitFMOD();

    // Timing
    LONGLONG startPerfCount = 0;
    LONGLONG perfCounterFrequency = 0;
    {
        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount);
        startPerfCount = perfCount.QuadPart;
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);
        perfCounterFrequency = perfFreq.QuadPart;
    }
    double currentTimeInSeconds = 0.0;

    engine::DoGameSetup();
    ImGui::SetCurrentContext(engine::GetImGuiContext());

    GameLoad gl = game.LoadResources();
    Keyboard* keyboard = gl.keyboard;
    Mouse* mouse = gl.mouse;

    Keyboard::Set(keyboard);
    Mouse::Set(mouse);

    Mouse::Get().SetWindow(hwnd);

    MSG msg = {};
    bool should_close = false;
    while (!should_close)
    {
        float dt;
        {
            double previousTimeInSeconds = currentTimeInSeconds;
            LARGE_INTEGER perfCount;
            QueryPerformanceCounter(&perfCount);

            currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
            dt = (float)(currentTimeInSeconds - previousTimeInSeconds);
        }

        /**** handle user input and other window events ****/
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { should_close = true; break; }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (global_windowResize)
        {
            engine::ResizeWindow();

            global_windowResize = false;
        }

        if (!engine::DoGameLoop(game, dt))
        {
            should_close = true;
        }

        /*
        // audio
        if (global_fmodLoaded && fmodSystem)
        {
            fmodSystem->update();
        }

        Matrix projectionMat = {};


        Matrix viewMat = Matrix::CreateLookAt(cameraPos, cameraPos + cameraFwd, Vector3::UnitY);
        projectionMat = Matrix::CreatePerspectiveFieldOfView(d2r(70.0f), float(g_WindowWidth) / float(g_WindowHeight), 0.1f, 10.f);

        // Spin the quad
        Matrix modelMat = Matrix::CreateRotationY(0.2f * (float)(PI * currentTimeInSeconds));

        // Calculate model-view-projection matrix to send to shader
        Matrix modelViewProj = modelMat * viewMat * projectionMat;


        void* c = nullptr;
        constBuf.BeginUpdate(immediateContext, &c);
        Constants* cst = (Constants*)c;
        cst->model = modelMat;
        cst->mvpMatrix = modelViewProj;
        cst->color = quadColor;
        constBuf.EndUpdate(immediateContext);

        
        audiotest::GUI();

        ClearBackground(color);

        RECT winRect;
        GetClientRect(hwnd, &winRect);
        D3D11_VIEWPORT viewport = {
          0.0f,
          0.0f,
          (FLOAT)(winRect.right - winRect.left),
          (FLOAT)(winRect.bottom - winRect.top),
          0.0f,
          1.0f };
        immediateContext->RSSetViewports(1, &viewport);

        immediateContext->RSSetState(rasterizerState);

        immediateContext->OMSetRenderTargets(1, &renderTarget, depthTarget);

        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        immediateContext->VSSetShader(vertexShader, NULL, 0);
        immediateContext->PSSetShader(pixelShader, NULL, 0);

        //immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
        constBuf.Use(immediateContext);

        immediateContext->IASetInputLayout(input_layout_ptr);

        // immediateContext->IASetVertexBuffers(0, 1, &vertex_buffer_ptr, &vertex_stride, &vertex_offset);
        // immediateContext->IASetIndexBuffer(index_buffer_ptr, DXGI_FORMAT_R16_UINT, 0);

        immediateContext->IASetVertexBuffers(0, icosphere->meshCount, icosphere->vBuffers.data(), icosphere->strides.data(), icosphere->offsets.data());
        immediateContext->IASetIndexBuffer(icosphere->iBuffers[0], DXGI_FORMAT_R32_UINT, 0);

        immediateContext->DrawIndexed(icosphere->meshes[0]->index_count, 0, 0);

        // Rendering
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swapChain->Present(0, 0);

        mouse.EndOfInputFrame();
        */
    }

    game.Shutdown();

    engine::audio::DestroyFMOD();

    return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        Keyboard::ProcessMessage(message, wParam, lParam);
    }

    switch (message)
    {
    case WM_ACTIVATE:
        if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
        {
            game.OnWindowFocused();
        }
        else if (wParam == WA_INACTIVE)
        {
            game.OnWindowUnfocused();
        }
    case WM_ACTIVATEAPP:
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse::ProcessMessage(message, wParam, lParam);
        break;
    }

    switch (message)
    {
    case WM_MOUSEACTIVATE:
        return MA_ACTIVATEANDEAT;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		global_windowResize = true;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

    return 0;
}