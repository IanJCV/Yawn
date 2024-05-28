#pragma once
#include "scene.h"

class DebugScene : public IScene
{
public:
	void OnEnter() override;
	void Update(float dt) override;
	void OnExit() override;
};