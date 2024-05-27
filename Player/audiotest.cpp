#include "audiotest.h"

namespace
{
	bool bMenuOpen = false;
	SoundEvent* seClick;
}

void audiotest::Init()
{
	if (!global_fmodLoaded)
	{
		InitFMOD();
	}

	seClick = new SoundEvent("event:/e_click");
}

void audiotest::GUI()
{
	if (!global_fmodLoaded)
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
			masterBank->Reload();
			stringsBank->Reload();

			seClick->Reload();
		}

		ImGui::End();
	}
}

void audiotest::Destroy()
{
	delete seClick;
}
