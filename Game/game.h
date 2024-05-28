#pragma once
#include <imgui/imgui.h>
#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

class Game
{
public:
	Game();

	void LoadResources();

	void Update(float dt);

	void Render();

	void Shutdown();
};