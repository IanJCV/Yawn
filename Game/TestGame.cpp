#include "TestGame.h"
#include "audio.h"
#include <filesystem>
#include <imguizmo/ImGuizmo.h>

engine::Model* car;
engine::Model* playground;
engine::Model* slurp;

engine::Shader* shader;
engine::Texture* gridTexture;
engine::CubemapTexture* skyTexture;

engine::Camera* camera;

DirectX::Keyboard keyboard;
DirectX::Mouse mouse;

engine::audio::SoundEvent* randomSound;

bool initialized= false;

using namespace DirectX::SimpleMath;

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

GameLoad TestGame::LoadResources()
{
    initialized = true;

    shader = new engine::Shader();
    const wchar_t* file = L"shaders/shaders.hlsl";
    shader->Load(file, file, engine::device);

    car = engine::LoadModel("models/icosphere.fbx", shader);

    playground = engine::LoadModel("models/playground.fbx", shader);

    slurp = engine::LoadModel("models/slurp.fbx", shader);
    slurp->Scale(0.25f);
    slurp->Translate(Vector3(0.f, 0.f, -15.f));

    gridTexture = new engine::Texture("textures/grid.png");

    car->SetTexture(gridTexture);
    playground->SetTexture(gridTexture);

    skyTexture = new engine::CubemapTexture(
        {
            "textures/skyboxes/thing/clouds1_rt.bmp",
            "textures/skyboxes/thing/clouds1_lf.bmp",
            "textures/skyboxes/thing/clouds1_up.bmp",
            "textures/skyboxes/thing/clouds1_dn.bmp",
            "textures/skyboxes/thing/clouds1_ft.bmp",
            "textures/skyboxes/thing/clouds1_bk.bmp",
        });

    randomSound = new engine::audio::SoundEvent("event:/e_click");

    engine::Renderer::Get().SetCamera(camera);

    GameLoad gl;
    gl.keyboard = &keyboard;
    gl.mouse = &mouse;

    auto thing = engine::Shader::Find("shaders");

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

    //mouse.SetMode(kb.LeftControl ? DirectX::Mouse::MODE_ABSOLUTE : DirectX::Mouse::MODE_RELATIVE);
    mouse.SetMode(ms.rightButton ? DirectX::Mouse::MODE_RELATIVE : DirectX::Mouse::MODE_ABSOLUTE);

    Vector3 movement;
    Vector2 rotation;

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

static std::vector<float> MatrixToFloatArr(Matrix m, float** r)
{
    DirectX::XMFLOAT4X4 temp;
    DirectX::XMStoreFloat4x4(&temp, m);

    std::vector<float> result =
    {
        temp._11, temp._21, temp._31, temp._41,
        temp._12, temp._22, temp._32, temp._42,
        temp._13, temp._23, temp._33, temp._43,
        temp._14, temp._24, temp._34, temp._44
    };
    *r = result.data();
    return result;
}

void TestGame::Render(engine::Renderer& rend, ImGuiContext* ctx, float dt)
{
    static bool drawCar = true;

    ImGui::SetCurrentContext(ctx);
    ImGuizmo::BeginFrame();
    rend.DrawSkybox(skyTexture);

    if (drawCar)
    {
        rend.SubmitForRendering(car);
    }
    rend.SubmitForRendering(playground, true);
    rend.SubmitForRendering(slurp);



    static bool open = true;
    if (ImGui::Begin("wnd", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::Text("frametime: %.4f", dt);
        ImGui::Text("fps: %.2f", 1.f / dt);
        ImGui::Text("camera rot: %.2f, %.2f, %.2f", camera->Forward.x, camera->Forward.y, camera->Forward.z);
        ImGui::Text("camera pos: %.2f, %.2f, %.2f", camera->Position.x, camera->Position.y, camera->Position.z);

        ImGui::Checkbox("VSync", &g_Vsync);

        ImGui::Checkbox("Draw car", &drawCar);

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



    auto& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::SetOrthographic(false);

    float* view;
    std::vector<float> t = MatrixToFloatArr(camera->View, &view);

    float* projection;
    MatrixToFloatArr(camera->Projection, &projection);

    float* model;
    MatrixToFloatArr(car->modelMatrix, &model);

    ImGuizmo::Manipulate(view, projection, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, model);

    ImGuizmo::DrawCubes(view, projection, model, 1);
}

void TestGame::Shutdown()
{
}