#include "Tubus.h"

#include "Globals.h"
#include "State.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
	Tubus::Settings,
	Radius,
	Falloff)

////////////////////////////////////////////////////////////////////////////////////

void Tubus::RestoreDefaultSettings()
{
	settings = {};
}

void Tubus::LoadSettings(json& o_json)
{
	settings = o_json;
}

void Tubus::SaveSettings(json& o_json)
{
	o_json = settings;
}

void Tubus::DrawSettings()
{
	ImGui::SeparatorText("Occlusion Reveal");
	ImGui::SliderFloat("Radius", &settings.Radius, 0.0f, 0.5f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Screen-space radius of the reveal circle around the player.");
	ImGui::SliderFloat("Falloff", &settings.Falloff, 0.0f, 0.2f, "%.3f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Soft edge width at the boundary of the reveal circle.");
}

void Tubus::SetupResources()
{
}

Tubus::CommonBufferData Tubus::GetCommonBufferData()
{
	CommonBufferData data = {};
	data.Radius = settings.Radius;
	data.Falloff = settings.Falloff;

	if (auto player = RE::PlayerCharacter::GetSingleton()) {
		auto pos = player->GetPosition();
		data.PlayerWorldPosition = { pos.x, pos.y, pos.z };
	}

	return data;
}
