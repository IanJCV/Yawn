#include "game.h"
#include "debugscene.h"

#define GUI ImGui

DebugScene scene;

Game::Game()
{
	scene.OnEnter();
}

void Game::LoadResources()
{

}

void Game::Update(float dt)
{
	scene.Update(dt);
}

void Game::Render()
{

}

void Game::Shutdown()
{
	scene.OnExit();
}
