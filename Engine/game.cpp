#include "game.h"

Game::Game()
{

}

GameLoad Game::LoadResources()
{
	return GameLoad{ 0 };
}

bool Game::Update(float dt)
{
	return true;
}

void Game::Render(engine::Renderer& rend, ImGuiContext* ctx, float dt)
{

}

void Game::Shutdown()
{

}
