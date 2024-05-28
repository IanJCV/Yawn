#include "audiotest.h"

namespace
{
	bool bMenuOpen = false;
	engine::audio::SoundEvent* seClick;
}

void audiotest::Init()
{
	if (!engine::audio::global_fmodLoaded)
	{
		engine::audio::InitFMOD();
	}

	seClick = new engine::audio::SoundEvent("event:/e_click");
}

void audiotest::GUI()
{
	if (!engine::audio::global_fmodLoaded)
	{
		return;
	}

	if (ImGui::Begin("Audio Test", &bMenuOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("Instance count: %d", seClick->GetInstanceCount());

		if (ImGui::Button("Play"))
		{
			seClick->Play();
		}

		if (ImGui::Button("Reload"))
		{
			engine::audio::masterBank->Reload();
			engine::audio::stringsBank->Reload();

			seClick->Reload();
		}

		ImGui::End();
	}
}

void audiotest::Destroy()
{
	delete seClick;
}
