#include "TestGame.h"
#include "audio.h"

engine::Model* car;
engine::Model* playground;
engine::Shader* shader;

engine::Camera* camera;

DirectX::Keyboard keyboard;
DirectX::Mouse mouse;

engine::audio::SoundEvent* randomSound;

using namespace DirectX::SimpleMath;

TestGame::TestGame()
{
    Vector3 scale;
    Vector3 pos;
    Quaternion rot;
    Matrix::CreateLookAt(Vector3(0.f, 1.f, -2.f), Vector3::Zero, Vector3::UnitY).Decompose(scale, rot, pos);

    camera = new engine::Camera(pos, rot);
}

GameLoad TestGame::LoadResources()
{
    DebugOut(L"!!! TEST GAME INITIALIZED !!!\n");

    shader = new engine::Shader();
    const wchar_t* file = L"shaders/shaders.hlsl";
    if (!shader->Load(file, file, engine::device))
    {
        DebugOut(L"No worky!!!\n");
    }

    car = engine::LoadModel("models/icosphere.fbx", shader);
    if (!car)
    {
        DebugOut(L"Model not loaded!\n");
        assert(false);
    }

    playground = engine::LoadModel("models/playground.fbx", shader);
    if (!playground)
    {
        DebugOut(L"Model not loaded!\n");
        assert(false);
    }

    car->modelMatrix = car->modelMatrix * Matrix::CreateScale(0.2f);
    playground->modelMatrix = playground->modelMatrix * Matrix::CreateScale(0.2f);

    randomSound = new engine::audio::SoundEvent("event:/e_click");

    engine::Renderer::Get().SetCamera(camera);

    GameLoad gl;
    gl.keyboard = &keyboard;
    gl.mouse = &mouse;

    return gl;
}

float GetAxis(bool min, bool max)
{
    if (min && max)
        return 0.f;
    return (min ? -1.f : max ? 1.f : 0.f);
}

bool TestGame::Update(float dt)
{
    auto kb = keyboard.GetState();
    auto ms = mouse.GetState();

    mouse.SetMode(kb.LeftControl ? DirectX::Mouse::MODE_ABSOLUTE : DirectX::Mouse::MODE_RELATIVE);

    Vector3 movement = Vector3(GetAxis(kb.A, kb.D), GetAxis(kb.Q, kb.E), GetAxis(kb.S, kb.W));
    Vector2 rotation;

    if (mouse.GetState().positionMode == DirectX::Mouse::MODE_RELATIVE)
    {
        rotation = Vector2(ms.x, ms.y);
    }
    else
    {
        mouse.SetVisible(true);
    }

    camera->Update(dt, movement, rotation);

    return false;
}

void TestGame::Render(engine::Renderer& rend, ImGuiContext* ctx, float dt)
{
    ImGui::SetCurrentContext(ctx);
    rend.SubmitForRendering(car);
    rend.SubmitForRendering(playground);

    static bool open = true;
    if (ImGui::Begin("wnd", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::Text("frametime: %.4f", dt);
        ImGui::Text("fps: %.2f", 1.f / dt);
        ImGui::Text("camera rot: %.2f, %.2f, %.2f", camera->Forward.x, camera->Forward.y, camera->Forward.z);
        ImGui::Text("camera pos: %.2f, %.2f, %.2f", camera->Position.x, camera->Position.y, camera->Position.z);

        if (ImGui::Button("Play sound"))
        {
            randomSound->Play();
        }

        if (ImGui::Button("Reload shader"))
        {
            shader->Reload();
        }
        ImGui::End();
    }
}

void TestGame::Shutdown()
{
}