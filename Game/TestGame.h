#pragma once
#include <game.h>
#include <imgui/imgui.h>
#include "game_common.h"

class GAME_API TestGame : public Game
{
public:
	// Inherited via Game
	GameLoad LoadResources() override;
	bool Update(float dt) override;
	void Render(engine::Renderer& rend, ImGuiContext* ctx, float dt) override;
	void Shutdown() override;
	TestGame();
};