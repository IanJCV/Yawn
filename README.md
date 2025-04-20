## Overview  
This is a simple DX11 "game framework" that I use to play around with DirectX features. It's made to be a MonoGame-like way to make basic games using C++.  

It's built using Win32, the latest DirectX 11, Dear Imgui for UI, and FMOD for audio.

The project structure is:  
- Player EXE contains the startup procedures, loading configs, and window events
- Engine DLL contains all the engine internals and APIs, like graphics, input, window management, asset loading, etc.
- Game DLL, contains the "game logic" -- it implements a `Game` class, which hooks into the engine and lets the user create their own logic.

It's not meant to be fully dynamic, the Player and Engine can change per project (and they should), to tailor their behaviour to each game, 
so it isn't supposed to load the games fully automatically or switch between them at runtime, nor will it.

## Example (Game/TestGame.cpp)  
```cpp
#include "TestGame.h"
#include "engine.h"
#include "audio.h"
#include <filesystem>
#include <imguizmo/ImGuizmo.h>

engine::Model* playground;

engine::Shader* shader;

engine::Texture* gridTexture;

engine::Camera* camera;

DirectX::Keyboard keyboard;
DirectX::Mouse mouse;

engine::audio::SoundEvent* randomSound;

// Config variables are defined here per game.
engine::ConfigEntry cfg_MouseSens{ .category = "General", .name = "f_MouseSensitivity", .type = engine::EntryType::CENTRY_TYPE_FLOAT };
engine::ConfigEntry cfg_WindowWidth{ .category = "Graphics", .name = "i_WindowWidth", .type = engine::EntryType::CENTRY_TYPE_INT };
engine::ConfigEntry cfg_WindowHeight{ .category = "Graphics", .name = "i_WindowHeight", .type = engine::EntryType::CENTRY_TYPE_INT };

// This is only for loading the data from a file.
engine::Config cfg =
{
    .entryCount = 3,
    .entries =
    {
        &cfg_MouseSens,
        &cfg_WindowWidth,
        &cfg_WindowHeight
    }
};

bool initialized= false;

using namespace DirectX::SimpleMath;

// This runs before engine initialization, so be careful about the APIs you use.
TestGame::TestGame()
{
    Vector3 scale;
    Vector3 pos;
    Quaternion rot;
    Matrix::CreateLookAt(Vector3(0.f, 1.f, -2.f), Vector3::Zero, Vector3::UnitY).Decompose(scale, rot, pos);

    camera = new engine::Camera(pos, rot);

    DebugOut(L"PATH: %s\n", std::filesystem::current_path().wstring().c_str());
}

void TestGame::OnWindowFocused()
{
    if (!initialized)
        return;
    DebugOut(L"Window in focus\n");
    mouse.SetMode(DirectX::Mouse::MODE_RELATIVE);
}

void TestGame::OnWindowUnfocused()
{
    if (!initialized)
        return;
    DebugOut(L"Window out of focus\n");
    mouse.SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    mouse.SetVisible(true);
}

// This runs only once on engine startup.
GameLoad TestGame::LoadResources()
{
    initialized = true;

    // Load data into the config struct
    engine::ini_open("config.ini", &cfg);

    if (cfg_WindowWidth.iValue != 0 && cfg_WindowHeight.iValue != 0)
    {
        engine::SetWindowSize(cfg_WindowWidth.iValue, cfg_WindowHeight.iValue);
    }

    // No materials yet, only shaders. Textures are attached to each model
    shader = new engine::Shader();
    const wchar_t* file = L"shaders/shaders.hlsl";
    shader->Load(file, file, engine::device);

    playground = engine::LoadModel("models/playground.fbx", shader);

    playground->Scale(0.25f);
    playground->Translate(Vector3(0.f, 0.f, -15.f));

    gridTexture = new engine::Texture("textures/grid.png");

    playground->SetTexture(gridTexture);

    // Sounds are defined in the FMOD project!
    randomSound = new engine::audio::SoundEvent("event:/e_click");

    // Renderer is a singleton, here we just set some global variables like the camera and skybox.
    // You can reset these anytime for gameplay purposes.
    engine::Renderer& rend = engine::Renderer::Get();

    rend.SetCamera(camera);
    rend.SetSkyTexture(new engine::CubemapTexture(
        {
            "textures/skyboxes/uhh/rt.0001.tif",
            "textures/skyboxes/uhh/lf.0001.tif",
            "textures/skyboxes/uhh/up.0001.tif",
            "textures/skyboxes/uhh/dn.0001.tif",
            "textures/skyboxes/uhh/ft.0001.tif",
            "textures/skyboxes/uhh/bk.0001.tif",
        }));

    // This is a struct meant to be passed to the Player level for initialization.
    GameLoad gl;
    gl.keyboard = &keyboard;
    gl.mouse = &mouse;
    gl.config = &cfg;

    auto thing = engine::Shader::Find("shaders");

    return gl;
}

float GetAxis(bool min, bool max)
{
    if (min && max)
        return 0.f;
    return (min ? -1.f : max ? 1.f : 0.f);
}

// Runs every frame before rendering.
bool TestGame::Update(float dt)
{
    auto kb = keyboard.GetState();
    auto ms = mouse.GetState();

    mouse.SetMode(ms.rightButton ? DirectX::Mouse::MODE_RELATIVE : DirectX::Mouse::MODE_ABSOLUTE);

    Vector3 movement;
    Vector2 rotation;

    // Camera movement
    if (mouse.GetState().positionMode == DirectX::Mouse::MODE_RELATIVE)
    {
        movement = Vector3(GetAxis(kb.A, kb.D), GetAxis(kb.Q, kb.E), GetAxis(kb.S, kb.W));
        if (kb.LeftShift)
        {
            movement *= 2.f;
        }
        rotation = Vector2(ms.x, ms.y);
    }
    else
    {
        mouse.SetVisible(true);
    }

    camera->Update(dt, movement, rotation);

    return false;
}

// Runs every frame after update
void TestGame::Render(engine::Renderer& rend, ImGuiContext* ctx, float dt)
{
    ImGui::SetCurrentContext(ctx);
    rend.DrawSkybox();

    // Easy to just send a model off to be drawn
    rend.SubmitForRendering(playground, true);

    static bool open = true;
    if (ImGui::Begin("wnd", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::Text("frametime: %.4f", dt);
        ImGui::Text("fps: %.2f", 1.f / dt);
        ImGui::Text("camera rot: %.2f, %.2f, %.2f", camera->Forward.x, camera->Forward.y, camera->Forward.z);
        ImGui::Text("camera pos: %.2f, %.2f, %.2f", camera->Position.x, camera->Position.y, camera->Position.z);

        ImGui::Checkbox("VSync", &g_Vsync);

        if (ImGui::Button("Play sound"))
        {
            randomSound->Play();
        }

        if (ImGui::Button("Reload shader"))
        {
            shader->Reload();
        }

        if (ImGui::Button("Reload skybox"))
        {
            rend.m_SkyboxShader->Reload();
        }
        ImGui::End();
    }
}

// Runs before the engine shuts down
void TestGame::Shutdown()
{
}
```
