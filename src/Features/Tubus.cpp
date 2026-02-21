#include "Tubus.h"

#include "Globals.h"
#include "State.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Tubus::Settings, Radius, EdgeWidth, TransitionSpeed)

// Tubus::Tubus() {  }

void Tubus::RestoreDefaultSettings() { settings = {}; }

void Tubus::LoadSettings(json& o_json) { settings = o_json; }

void Tubus::SaveSettings(json& o_json) { o_json = settings; }

void Tubus::DrawSettings()
{
	ImGui::SeparatorText("Occlusion Reveal");
	ImGui::SliderFloat("Radius", &settings.Radius, 50.0f, 300.0f, "%.1f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("World-space radius of the capsule between camera and player collision points.");
	ImGui::SliderFloat("Edge Width", &settings.EdgeWidth, 1.0f, 100.0f, "%.1f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"World-space width of the noisy dissolve band around the capsule. Larger values create a wider dithered "
			"transition.");
	ImGui::SliderFloat("Transition Speed", &settings.TransitionSpeed, 0.1f, 5.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("How fast the culling capsule animates in and out. Higher values = faster transition.");
}

void Tubus::SetupResources()
{
	UtilityPerGeometryCB = new ConstantBuffer(ConstantBufferDesc<TubusAPI::PerGeometryData>());
	LightingPerGeometryCB = new ConstantBuffer(ConstantBufferDesc<TubusAPI::PerGeometryData>());
}

Tubus::CommonBufferData Tubus::GetCommonBufferData()
{
	CommonBufferData data = {};
	TubusLib = static_cast<TubusAPI::Tubus*>(TubusAPI::RequestPluginAPI());
	const bool shouldCull = TubusLib->ShouldEnableCulling();

	const float dt = RE::GetSecondsSinceLastFrame();
	const float transitionTarget = shouldCull ? 1.0f : 0.0f;
	const float debounceTarget = shouldCull ? 1.0f : 0.0f;
	const float transitionSpeed = settings.TransitionSpeed * dt;
	const float debounceSpeed = settings.TransitionSpeed * dt * 1.33f;

	if (DebounceProgress < debounceTarget)
		DebounceProgress = std::min(DebounceProgress + debounceSpeed, debounceTarget);
	else if (DebounceProgress > debounceTarget)
		DebounceProgress = std::max(DebounceProgress - debounceSpeed, debounceTarget);

	if (DebounceProgress == 1.0f && TransitionProgress < transitionTarget)
		TransitionProgress = std::min(TransitionProgress + transitionSpeed, transitionTarget);
	else if (DebounceProgress == 0.0f && TransitionProgress > transitionTarget)
		TransitionProgress = std::max(TransitionProgress - transitionSpeed, transitionTarget);

	data.Radius = settings.Radius * TransitionProgress;
	data.EdgeWidth = settings.EdgeWidth;
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

void Tubus::BSUtilityShader_SetupGeometry(const RE::BSRenderPass* a_pass) const
{
	UtilityPerGeometryCB->Update(TubusLib->GetPerGeometryData(a_pass->geometry));
	ID3D11Buffer* buffer = { UtilityPerGeometryCB->CB() };
	auto context = globals::d3d::context;
	context->PSSetConstantBuffers(3, 1, &buffer);
}

void Tubus::BSLightingShader_SetupGeometry(const RE::BSRenderPass* a_pass) const
{
	LightingPerGeometryCB->Update(TubusLib->GetPerGeometryData(a_pass->geometry));
	ID3D11Buffer* buffer = { LightingPerGeometryCB->CB() };
	auto context = globals::d3d::context;
	context->PSSetConstantBuffers(13, 1, &buffer);
}
