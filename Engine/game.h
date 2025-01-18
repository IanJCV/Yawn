#pragma once
#include <imgui/imgui.h>
#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>
#include "common.h"

#include "scene.h"
#include "rendering.h"
#include "config.h"

struct ENGINE_API GameLoad
{
	DirectX::Keyboard* keyboard;
	DirectX::Mouse* mouse;
	engine::Config* config;
};

class ENGINE_API Game
{
public:
	Game();

	virtual GameLoad LoadResources() = 0;

	virtual bool Update(float dt) = 0;

	virtual void Render(engine::Renderer& rend, ImGuiContext* ctx, float dt) = 0;

	virtual void OnWindowFocused() = 0;

	virtual void OnWindowUnfocused() = 0;

	virtual void Shutdown() = 0;
};