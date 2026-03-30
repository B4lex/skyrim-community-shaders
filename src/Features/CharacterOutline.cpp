#include "CharacterOutline.h"

#include "Deferred.h"
#include "Globals.h"
#include "State.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CharacterOutline::Settings, OutlineColorR, OutlineColorG, OutlineColorB, OutlineOpacity, Thickness, OutlineNPCs)

void CharacterOutline::RestoreDefaultSettings() { settings = {}; }

void CharacterOutline::LoadSettings(json& o_json) { settings = o_json; }

void CharacterOutline::SaveSettings(json& o_json) { o_json = settings; }

void CharacterOutline::DrawSettings()
{
	ImGui::SeparatorText("Character Outline");

	float color[3] = { settings.OutlineColorR, settings.OutlineColorG, settings.OutlineColorB };
	if (ImGui::ColorEdit3("Outline Color", color)) {
		settings.OutlineColorR = color[0];
		settings.OutlineColorG = color[1];
		settings.OutlineColorB = color[2];
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Color of the outline around the player character.");

	ImGui::SliderFloat("Outline Opacity", &settings.OutlineOpacity, 0.0f, 1.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Opacity of the outline effect. 0 = fully transparent, 1 = fully opaque.");

	int thickness = static_cast<int>(settings.Thickness);
	if (ImGui::SliderInt("Outline Thickness", &thickness, 1, 5)) {
		settings.Thickness = static_cast<uint32_t>(thickness);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Width of the outline in pixels.");

	ImGui::Checkbox("Outline NPCs", &settings.OutlineNPCs);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Also draw outlines around all NPCs.");
}

void CharacterOutline::SetupResources()
{
	outlineSettingsCB = new ConstantBuffer(ConstantBufferDesc<OutlineCBData>());
}

void CharacterOutline::ClearShaderCache()
{
	if (outlineCS) {
		outlineCS->Release();
		outlineCS = nullptr;
	}
}

ID3D11ComputeShader* CharacterOutline::GetComputeOutline()
{
	if (!outlineCS) {
		logger::debug("Compiling CharacterOutline OutlineCompositeCS");
		outlineCS = static_cast<ID3D11ComputeShader*>(
			Util::CompileShader(L"Data\\Shaders\\CharacterOutline\\OutlineCompositeCS.hlsl", {}, "cs_5_0"));
	}
	return outlineCS;
}

void CharacterOutline::DrawOutline()
{
	ZoneScoped;
	TracyD3D11Zone(globals::state->tracyCtx, "Character Outline");

	auto renderer = globals::game::renderer;
	auto context = globals::d3d::context;
	auto deferred = globals::deferred;

	auto main = renderer->GetRuntimeData().renderTargets[deferred->forwardRenderTargets[0]];
	auto playerMask = renderer->GetRuntimeData().renderTargets[MASKS2];

	// Update outline settings constant buffer
	OutlineCBData cbData = {};
	cbData.OutlineColor = { settings.OutlineColorR, settings.OutlineColorG, settings.OutlineColorB, settings.OutlineOpacity };
	cbData.Thickness = static_cast<float>(settings.Thickness);
	outlineSettingsCB->Update(cbData);

	// Clear previous SRV bindings to avoid resource conflicts
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->CSSetShaderResources(0, 16, nullSRVs);

	// Bind player mask as input SRV
	ID3D11ShaderResourceView* srvs[1]{ playerMask.SRV };
	context->CSSetShaderResources(0, 1, srvs);

	// Bind main render target as UAV for read/write
	ID3D11UnorderedAccessView* uavs[1]{ main.UAV };
	context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

	// Bind outline settings constant buffer
	ID3D11Buffer* buffer = outlineSettingsCB->CB();
	context->CSSetConstantBuffers(1, 1, &buffer);

	// Dispatch outline compute shader
	context->CSSetShader(GetComputeOutline(), nullptr, 0);
	auto dispatchCount = Util::GetScreenDispatchCount(true);
	context->Dispatch(dispatchCount.x, dispatchCount.y, 1);

	// Cleanup bindings
	srvs[0] = nullptr;
	context->CSSetShaderResources(0, 1, srvs);
	uavs[0] = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);
	buffer = nullptr;
	context->CSSetConstantBuffers(1, 1, &buffer);
	context->CSSetShader(nullptr, nullptr, 0);
}
