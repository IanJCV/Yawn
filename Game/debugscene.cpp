#include "debugscene.h"
#include "globals.h"
#include <imgui/imgui.h>
#include <camera.h>
#include <DirectXTK/Mouse.h>
#include <DirectXTK/Keyboard.h>

using namespace DirectX::SimpleMath;

void DebugScene::OnEnter()
{

}

void DebugScene::Update(float dt)
{
    bool open = true;
    if (ImGui::Begin("stuff", &open))
    {
        ImGui::Text("frametime: %f", dt);
        ImGui::Text("fps: %f", 1.0f / dt);

        //ImGui::Text("camera pos x: %.2f y: %.2f z: %.2f", camera->Position.x, camera->Position.y, camera->Position.z);
        //ImGui::Text("camera pitch: %.2f yaw: %.2f", camera->Pitch, camera->Yaw);

        //ImGui::DragFloat("Camera sensitivity", &camera->Sensitivity, 0.01f, 0.001f, 0.5f, "%.2f");

        ImGui::Separator();

        //ImGui::Text("window size: %dx%d", engine::g_WindowWidth, engine::g_WindowHeight);
        auto state = DirectX::Mouse::Get().GetState();
        ImGui::Text("mouse: %d, %d", state.x, state.y);

        ImGui::Separator();

        ImGui::End();
    }
}

void DebugScene::OnExit()
{
    //delete camera;
}
