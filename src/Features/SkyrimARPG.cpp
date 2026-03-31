#include "SkyrimARPG.h"

#include "Deferred.h"
#include "Globals.h"
#include "State.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SkyrimARPG::TubusSettings, Radius, EdgeWidth, TransitionSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SkyrimARPG::OutlineSettings, OutlineOpacity, Thickness)

void SkyrimARPG::RestoreDefaultSettings()
{
	tubusSettings = {};
	outlineSettings = {};
}

void SkyrimARPG::LoadSettings(json& o_json)
{
	if (o_json.contains("OcclusionReveal"))
		tubusSettings = o_json["OcclusionReveal"];
	if (o_json.contains("CharacterOutline"))
		outlineSettings = o_json["CharacterOutline"];
}

void SkyrimARPG::SaveSettings(json& o_json)
{
	o_json["OcclusionReveal"] = tubusSettings;
	o_json["CharacterOutline"] = outlineSettings;
}

void SkyrimARPG::DrawSettings()
{
	ImGui::SeparatorText("Occlusion Reveal");
	ImGui::SliderFloat("Radius", &tubusSettings.Radius, 50.0f, 300.0f, "%.1f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("World-space radius of the capsule between camera and player collision points.");
	ImGui::SliderFloat("Edge Width", &tubusSettings.EdgeWidth, 1.0f, 100.0f, "%.1f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"World-space width of the noisy dissolve band around the capsule. Larger values create a wider dithered "
			"transition.");
	ImGui::SliderFloat("Transition Speed", &tubusSettings.TransitionSpeed, 0.1f, 5.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("How fast the culling capsule animates in and out. Higher values = faster transition.");

	ImGui::SeparatorText("Character Outline");

	ImGui::SliderFloat("Outline Opacity", &outlineSettings.OutlineOpacity, 0.0f, 1.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Opacity of the outline effect. 0 = fully transparent, 1 = fully opaque.");

	int thickness = static_cast<int>(outlineSettings.Thickness);
	if (ImGui::SliderInt("Outline Thickness", &thickness, 1, 5)) {
		outlineSettings.Thickness = static_cast<uint32_t>(thickness);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Width of the outline in pixels.");

}

void SkyrimARPG::SetupResources()
{
	// Tubus constant buffers
	UtilityPerGeometryCB = new ConstantBuffer(ConstantBufferDesc<TubusAPI::PerGeometryData>());
	LightingPerGeometryCB = new ConstantBuffer(ConstantBufferDesc<TubusAPI::PerGeometryData>());

	// CharacterOutline resources
	outlineSettingsCB = new ConstantBuffer(ConstantBufferDesc<OutlineCBData>());

	auto renderer = globals::game::renderer;
	auto device = globals::d3d::device;

	auto& main = renderer->GetRuntimeData().renderTargets[RE::RENDER_TARGETS::kMAIN];

	D3D11_TEXTURE2D_DESC texDesc{};
	main.texture->GetDesc(&texDesc);
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &characterMaskTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {
		.Format = texDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
	};
	DX::ThrowIfFailed(device->CreateShaderResourceView(characterMaskTex, &srvDesc, &characterMaskSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
		.Format = texDesc.Format,
		.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
		.Texture2D = { .MipSlice = 0 }
	};
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(characterMaskTex, &uavDesc, &characterMaskUAV));
}

void SkyrimARPG::ClearShaderCache()
{
	if (outlineCS) {
		outlineCS->Release();
		outlineCS = nullptr;
	}
}

// ---- Tubus methods ----

SkyrimARPG::CommonBufferData SkyrimARPG::GetCommonBufferData()
{
	CommonBufferData data = {};
	TubusLib = static_cast<TubusAPI::Tubus*>(TubusAPI::RequestPluginAPI());
	const bool shouldCull = TubusLib->ShouldEnableCulling();

	const float dt = RE::GetSecondsSinceLastFrame();
	const float transitionTarget = shouldCull ? 1.0f : 0.0f;
	const float debounceTarget = shouldCull ? 1.0f : 0.0f;
	const float transitionSpeed = tubusSettings.TransitionSpeed * dt;
	const float debounceSpeed = tubusSettings.TransitionSpeed * dt * 1.33f;

	if (DebounceProgress < debounceTarget)
		DebounceProgress = std::min(DebounceProgress + debounceSpeed, debounceTarget);
	else if (DebounceProgress > debounceTarget)
		DebounceProgress = std::max(DebounceProgress - debounceSpeed, debounceTarget);

	if (DebounceProgress == 1.0f && TransitionProgress < transitionTarget)
		TransitionProgress = std::min(TransitionProgress + transitionSpeed, transitionTarget);
	else if (DebounceProgress == 0.0f && TransitionProgress > transitionTarget)
		TransitionProgress = std::max(TransitionProgress - transitionSpeed, transitionTarget);

	data.Radius = tubusSettings.Radius * TransitionProgress;
	data.EdgeWidth = tubusSettings.EdgeWidth;
	data.ShouldEnableCulling = shouldCull || TransitionProgress > 0.0f;

	if (const auto player = RE::PlayerCharacter::GetSingleton()) {
		const auto playerPos = player->GetPosition();
		data.PlayerWorldPosition = { playerPos.x, playerPos.y, playerPos.z };
		RE::NiPoint3 collisionPoint;
		if (TubusLib->GetCameraPlayerCollisionPoint(player, collisionPoint)) {
			CameraPlayerCollisionPoint = collisionPoint;
		}
		if (TubusLib->GetPlayerCameraCollisionPoint(player, collisionPoint)) {
			PlayerCameraCollisionPoint = collisionPoint;
		}
		data.CameraPlayerCollision = { CameraPlayerCollisionPoint.x, CameraPlayerCollisionPoint.y,
			CameraPlayerCollisionPoint.z };
		data.PlayerCameraCollision = { PlayerCameraCollisionPoint.x, PlayerCameraCollisionPoint.y,
			PlayerCameraCollisionPoint.z };
	}

	return data;
}

void SkyrimARPG::BSUtilityShader_SetupGeometry(const RE::BSRenderPass* a_pass) const
{
	UtilityPerGeometryCB->Update(TubusLib->GetPerGeometryData(a_pass->geometry));
	ID3D11Buffer* buffer = { UtilityPerGeometryCB->CB() };
	auto context = globals::d3d::context;
	context->PSSetConstantBuffers(3, 1, &buffer);
}

void SkyrimARPG::BSLightingShader_SetupGeometry(const RE::BSRenderPass* a_pass) const
{
	LightingPerGeometryCB->Update(TubusLib->GetPerGeometryData(a_pass->geometry));
	ID3D11Buffer* buffer = { LightingPerGeometryCB->CB() };
	auto context = globals::d3d::context;
	context->PSSetConstantBuffers(13, 1, &buffer);
}

// ---- CharacterOutline methods ----

ID3D11ComputeShader* SkyrimARPG::GetComputeOutline()
{
	if (!outlineCS) {
		logger::debug("Compiling SkyrimARPG OutlineCompositeCS");
		outlineCS = static_cast<ID3D11ComputeShader*>(
			Util::CompileShader(L"Data\\Shaders\\SkyrimARPG\\OutlineCompositeCS.hlsl", {}, "cs_5_0"));
	}
	return outlineCS;
}

void SkyrimARPG::DrawOutline()
{
	ZoneScoped;
	TracyD3D11Zone(globals::state->tracyCtx, "Character Outline");

	auto renderer = globals::game::renderer;
	auto context = globals::d3d::context;
	auto deferred = globals::deferred;

	auto main = renderer->GetRuntimeData().renderTargets[deferred->forwardRenderTargets[0]];

	// Update outline settings constant buffer
	OutlineCBData cbData = {};
	cbData.Thickness = static_cast<float>(outlineSettings.Thickness);
	outlineSettingsCB->Update(cbData);

	// Clear previous SRV bindings to avoid resource conflicts
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->CSSetShaderResources(0, 16, nullSRVs);

	// Bind dedicated character mask as input SRV
	ID3D11ShaderResourceView* srvs[1]{ characterMaskSRV };
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
