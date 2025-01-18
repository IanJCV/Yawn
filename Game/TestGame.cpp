#include "TestGame.h"
#include "engine.h"
#include "audio.h"
#include <filesystem>
#include <imguizmo/ImGuizmo.h>

engine::Model* car;
engine::Model* playground;
engine::Model* slurp;
engine::Model* cube;

engine::Shader* shader;

engine::Texture* gridTexture;

engine::Camera* camera;

DirectX::Keyboard keyboard;
DirectX::Mouse mouse;

engine::audio::SoundEvent* randomSound;

engine::ConfigEntry cfg_MouseSens{ .category = "General", .name = "f_MouseSensitivity", .type = engine::EntryType::CENTRY_TYPE_FLOAT };
engine::ConfigEntry cfg_WindowWidth{ .category = "Graphics", .name = "i_WindowWidth", .type = engine::EntryType::CENTRY_TYPE_INT };
engine::ConfigEntry cfg_WindowHeight{ .category = "Graphics", .name = "i_WindowHeight", .type = engine::EntryType::CENTRY_TYPE_INT };

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

    engine::ini_open("config.ini", &cfg);

    if (cfg_WindowWidth.iValue != 0 && cfg_WindowHeight.iValue != 0)
    {
        engine::SetWindowSize(cfg_WindowWidth.iValue, cfg_WindowHeight.iValue);
    }

    shader = new engine::Shader();
    const wchar_t* file = L"shaders/shaders.hlsl";
    shader->Load(file, file, engine::device);

    car = engine::LoadModel("models/icosphere.fbx", shader);

    playground = engine::LoadModel("models/playground.fbx", shader);

    slurp = engine::LoadModel("models/slurp.fbx", shader);
    slurp->Scale(0.25f);
    slurp->Translate(Vector3(0.f, 0.f, -15.f));

    engine::Shader* cubeShader = new engine::Shader();
    cubeShader->Load(L"uh", L"erm", engine::device);

    cube = engine::LoadModel("models/cube.fbx", cubeShader);
    cube->Scale(2.f);
    cube->Translate(Vector3(1.f, 0.f, -20.f));

    gridTexture = new engine::Texture("textures/grid.png");

    car->SetTexture(gridTexture);
    playground->SetTexture(gridTexture);

    randomSound = new engine::audio::SoundEvent("event:/e_click");

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

static float* MatrixToFloatArr(Matrix m, float** r)
{
    DirectX::XMFLOAT4X4 temp;
    DirectX::XMStoreFloat4x4(&temp, m);

    float result[] =
    {
        temp._11, temp._21, temp._31, temp._41,
        temp._12, temp._22, temp._32, temp._42,
        temp._13, temp._23, temp._33, temp._43,
        temp._14, temp._24, temp._34, temp._44
    };
    *r = result;
    return result;
}

void EditTransform(const engine::Camera& camera, Matrix* matrix)
{
    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
    if (ImGui::IsKeyPressed(ImGuiKey_1))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_2))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_3))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    Quaternion rotation;
    Vector3 translation, scale;
    matrix->Decompose(scale, rotation, translation);
    ImGuizmo::DecomposeMatrixToComponents((float*)matrix, &translation.x, &rotation.x, &scale.x);
    ImGui::InputFloat3("Tr", &translation.x);
    ImGui::InputFloat3("Rt", &rotation.x);
    ImGui::InputFloat3("Sc", &scale.x);
    ImGuizmo::RecomposeMatrixFromComponents(&translation.x, &rotation.x, &scale.x, (float*)matrix);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
    }
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate((float*)&camera.View, (float*)&camera.Projection, mCurrentGizmoOperation, mCurrentGizmoMode, (float*)matrix, NULL, NULL);
}

void TestGame::Render(engine::Renderer& rend, ImGuiContext* ctx, float dt)
{
    static bool drawCar = true;

    ImGui::SetCurrentContext(ctx);
    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);
    rend.DrawSkybox();

    if (drawCar)
    {
        rend.SubmitForRendering(car);
    }
    rend.SubmitForRendering(playground, true);
    rend.SubmitForRendering(slurp);
    rend.SubmitForRendering(cube);

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

    EditTransform(*camera, &car->modelMatrix);
}

void TestGame::Shutdown()
{
}