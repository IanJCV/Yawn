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

#include <atlbase.h>
#include <atlconv.h>

#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

#include <format>

#include "audiotest.h"

#pragma comment( lib, "engine.lib" )

using namespace DirectX;
using namespace SimpleMath;

bool global_windowResize = false;

// Input
enum GameAction {
    GameActionMoveCamFwd,
    GameActionMoveCamBack,
    GameActionMoveCamLeft,
    GameActionMoveCamRight,
    GameActionTurnCamLeft,
    GameActionTurnCamRight,
    GameActionLookUp,
    GameActionLookDown,
    GameActionRaiseCam,
    GameActionLowerCam,
    GameActionCount
};
static bool global_keyIsDown[GameActionCount] = {};

Game game;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    //wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"yawnengineapp";
    if (!RegisterClass(&wc))
        return 1;

    HWND hwnd = CreateWindow(wc.lpszClassName, L"Player", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, NULL);

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid[0].dwFlags = RIDEV_INPUTSINK;
    Rid[0].hwndTarget = hwnd;
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
    
    assert(hwnd != NULL);

    DirectXSetup(hwnd);

    game.LoadResources();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, deviceContext);

    SimpleMath::Color color = { 0xab / 255.0f, 0xce / 255.0f, 0xb4 / 255.0f, 1.0f };

    struct Constants
    {
        Matrix model;
        Matrix mvpMatrix;
        Color color;
    };

    ConstantBuffer constBuf(device, sizeof(Constants));

    // Audio
    audiotest::Init();

    // Camera
    SimpleMath::Matrix cameraMatrix = SimpleMath::Matrix::CreatePerspectiveFieldOfView(DEG2RAD * 70.0f, g_WindowWidth / g_WindowHeight, 0.01f, 100.0f);
    SimpleMath::Vector3 cameraPos = { 0, 0, 2 };
    SimpleMath::Vector3 cameraFwd = { 0, 0, -1 };
    Vector3 cameraRight;
    float cameraPitch = 0.f;
    float cameraYaw = 0.f;

    float cameraSensitivity = 0.002f;

    // Variables
    Color quadColor = Color { 1.0f, 1.0f, 1.0f, 1.0f };

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

    double lastShaderReloadFailTime = -999.0;

    Keyboard keyboard;
    Mouse mouse;

    Keyboard::KeyboardStateTracker keyboardTracker;
    Mouse::ButtonStateTracker mouseTracker;

    mouse.SetWindow(hwnd);
    mouse.SetMode(Mouse::MODE_RELATIVE);

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

        //keyboardTracker.Update(keyboard.GetState());
        //mouseTracker.Update(mouse.GetState());

        // audio
        if (global_fmodLoaded && fmodSystem)
        {
            fmodSystem->update();
        }

        /*** TODO: RENDER A FRAME HERE WITH DIRECT3D ***/

        Matrix projectionMat = {};

        if (global_windowResize)
        {
            ResizeWindow();

            projectionMat = Matrix::CreatePerspectiveFieldOfView(d2r(70.0f), float(g_WindowWidth) / float(g_WindowHeight), 0.01f, 50.0f);

            global_windowResize = false;
        }

        // Update camera
        {
            auto kState = keyboard.GetState();

            Mouse::Get().SetMode(kState.LeftControl ? Mouse::MODE_ABSOLUTE : Mouse::MODE_RELATIVE);

            auto mState = Mouse::Get().GetState();

            if (mState.positionMode == Mouse::MODE_RELATIVE)
			{
				cameraPitch -= float(mState.y) * cameraSensitivity;
				cameraYaw -= float(mState.x) * cameraSensitivity;
            }
            else
            {
                Mouse::Get().SetVisible(true);
            }

            auto m = Matrix::CreateFromYawPitchRoll(cameraYaw, cameraPitch, 0.f);
            cameraFwd = m.Forward();

            cameraRight = m.Right();

            if (cameraPitch >  d2r(88))
                cameraPitch =  d2r(88);
            if (cameraPitch < -d2r(88))
                cameraPitch = -d2r(88);

            SimpleMath::Vector3 camFwdXZ = cameraFwd;
            camFwdXZ.y = 0.0f;
            camFwdXZ.Normalize();

            SimpleMath::Vector3 cameraRightXZ;
            camFwdXZ.Cross({ 0, 1, 0 }, cameraRightXZ);

            const float CAM_MOVE_SPEED = 2.f; // in metres per second
            const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
            if (global_keyIsDown[GameActionMoveCamFwd])
                cameraPos += cameraFwd * CAM_MOVE_AMOUNT;
            if (global_keyIsDown[GameActionMoveCamBack])
                cameraPos -= cameraFwd * CAM_MOVE_AMOUNT;
            if (global_keyIsDown[GameActionMoveCamLeft])
                cameraPos -= cameraRight * CAM_MOVE_AMOUNT;
            if (global_keyIsDown[GameActionMoveCamRight])
                cameraPos += cameraRight * CAM_MOVE_AMOUNT;
            if (global_keyIsDown[GameActionRaiseCam])
                cameraPos.y += CAM_MOVE_AMOUNT;
            if (global_keyIsDown[GameActionLowerCam])
                cameraPos.y -= CAM_MOVE_AMOUNT;
        }

        Matrix viewMat = Matrix::CreateLookAt(cameraPos, cameraPos + cameraFwd, Vector3::UnitY);
        projectionMat = Matrix::CreatePerspectiveFieldOfView(d2r(70.0f), float(g_WindowWidth) / float(g_WindowHeight), 0.1f, 10.f);

        // Spin the quad
        Matrix modelMat = Matrix::CreateRotationY(0.2f * (float)(PI * currentTimeInSeconds));

        // Calculate model-view-projection matrix to send to shader
        Matrix modelViewProj = modelMat * viewMat * projectionMat;


        void* c = nullptr;
        constBuf.BeginUpdate(deviceContext, &c);
        Constants* cst = (Constants*)c;
        cst->model = modelMat;
        cst->mvpMatrix = modelViewProj;
        cst->color = quadColor;
        constBuf.EndUpdate(deviceContext);


        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        /* clear the back buffer to cornflower blue for the new frame */

        bool open = true;
        if (ImGui::Begin("stuff", &open))
        {
            ImGui::Text("frametime: %f", dt);
            ImGui::Text("fps: %f", 1.0f / dt);

            ImGui::Text("camera pos x: %.2f y: %.2f z: %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
            ImGui::Text("camera pitch: %.2f yaw: %.2f", cameraPitch, cameraYaw);

            ImGui::DragFloat("Camera sensitivity", &cameraSensitivity, 0.01f, 0.001f, 0.5f, "%.2f");

            ImGui::Separator();

            ImGui::Text("window size: %dx%d", g_WindowWidth, g_WindowHeight);
            auto state = Mouse::Get().GetState();
            ImGui::Text("mouse: %d, %d", state.x, state.y);

            ImGui::ColorPicker4("Clear Color", color);

            ImGui::ColorPicker4("Quad Color", quadColor);

            if (ImGui::Button("Load FMOD"))
            {
                InitFMOD();
            }

            ImGui::Separator();

            if (currentTimeInSeconds <= lastShaderReloadFailTime + 2.0f)
            {
                ImGui::Text("Shader reload failed!");
            }

            if (ImGui::Button("Reload Shaders"))
            {
                if (!ReloadShaders())
                {
                    lastShaderReloadFailTime = currentTimeInSeconds;
                }
            }

            for (size_t i = 0; i < GameActionCount; i++)
            {
                ImGui::Text("action %d: %d", i, global_keyIsDown[i]);
            }

            ImGui::End();
        }
        
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
        deviceContext->RSSetViewports(1, &viewport);

        deviceContext->RSSetState(rasterizerState);

        deviceContext->OMSetRenderTargets(1, &renderTarget, depthTarget);

        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        deviceContext->VSSetShader(vertexShader, NULL, 0);
        deviceContext->PSSetShader(pixelShader, NULL, 0);

        //deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
        constBuf.Use(deviceContext);

        deviceContext->IASetInputLayout(input_layout_ptr);

        // deviceContext->IASetVertexBuffers(0, 1, &vertex_buffer_ptr, &vertex_stride, &vertex_offset);
        // deviceContext->IASetIndexBuffer(index_buffer_ptr, DXGI_FORMAT_R16_UINT, 0);

        deviceContext->IASetVertexBuffers(0, icosphere->meshCount, icosphere->vBuffers.data(), icosphere->strides.data(), icosphere->offsets.data());
        deviceContext->IASetIndexBuffer(icosphere->iBuffers[0], DXGI_FORMAT_R32_UINT, 0);

        deviceContext->DrawIndexed(icosphere->meshes[0]->index_count, 0, 0);

        // Rendering
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swapChain->Present(0, 0);

        mouse.EndOfInputFrame();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    audiotest::Destroy();

    DestroyFMOD();

    return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        Keyboard::ProcessMessage(message, wParam, lParam);
    }

    switch (message)
    {
    case WM_ACTIVATE:
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
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);

        {
            Keyboard::State state = Keyboard::Get().GetState();

			global_keyIsDown[GameActionMoveCamFwd] = state.W;
			global_keyIsDown[GameActionMoveCamLeft] = state.A;
			global_keyIsDown[GameActionMoveCamBack] = state.S;
			global_keyIsDown[GameActionMoveCamRight] = state.D;
			global_keyIsDown[GameActionRaiseCam] = state.E;
			global_keyIsDown[GameActionLowerCam] = state.Q;

            if (state.Escape)
            {
                PostQuitMessage(0);
            }
        }
		break;

    case WM_MOUSEACTIVATE:
        // When you click to activate the window, we want Mouse to ignore that event.
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