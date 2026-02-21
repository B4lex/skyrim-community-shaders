#pragma once

#include "API/TubusAPI.h"
#include <RE/F/FormTypes.h>
#include <vector>

struct Tubus : Feature
{
	ConstantBuffer* UtilityPerGeometryCB = nullptr;
	ConstantBuffer* LightingPerGeometryCB = nullptr;
	TubusAPI::Tubus* TubusLib = nullptr;
	float TransitionProgress = 0.0f;
	float DebounceProgress = 0.0f;
	RE::NiPoint3 CameraPlayerCollisionPoint;
	RE::NiPoint3 PlayerCameraCollisionPoint;

	// Tubus();
	////////////////////////////////////////////////// Boilerplate
	// Metadata
	std::string GetName() override { return "Tubus"; }
	std::string GetShortName() override { return "Tubus"; }
	std::string_view GetCategory() const override { return "Lighting"; }
	std::string GetFeatureModLink() override { return MakeNexusModURL("999999"); }
	std::pair<std::string, std::vector<std::string>> GetFeatureSummary() override
	{
		return { "Camera-to-character occlusion reveal shader.",
			{
				"Discards occluding meshes between camera and player.",
				"World-space capsule culling between collision points.",
			} };
	}

	// Functionality
	bool SupportsVR() override { return true; }
	std::string_view GetShaderDefineName() override { return "TUBUS"; }
	bool HasShaderDefine(RE::BSShader::Type t) override
	{
		return t == RE::BSShader::Type::Utility || t == RE::BSShader::Type::Lighting;
	};

	// Settings & UI
	void RestoreDefaultSettings() override;
	void LoadSettings(json& o_json) override;
	void SaveSettings(json& o_json) override;
	void DrawSettings() override;

	// Resources
	void SetupResources() override;

	////////////////////////////////////////////////// Feature Specific Data
	struct Settings
	{
		float Radius = 210.0f;
		float EdgeWidth = 75.0f;
		float TransitionSpeed = 0.6f;
	} settings;

	struct CommonBufferData
	{
		float3 CameraPlayerCollision;
		float Radius;
		float3 PlayerCameraCollision;
		float EdgeWidth;
		uint32_t ShouldEnableCulling;
		float3 PlayerWorldPosition;
	};

	static_assert(sizeof(CommonBufferData) % 16 == 0, "CommonBufferData must be aligned to 16 bytes.");

	CommonBufferData GetCommonBufferData();
	void BSUtilityShader_SetupGeometry(const RE::BSRenderPass* a_pass) const;
	void BSLightingShader_SetupGeometry(const RE::BSRenderPass* a_pass) const;
};
