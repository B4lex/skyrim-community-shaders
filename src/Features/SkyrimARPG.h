#pragma once

#include "API/OcclusionReveal.h"
#include "API/RequestAPI.h"
#include <RE/F/FormTypes.h>
#include <vector>

struct SkyrimARPG : Feature
{
	// Tubus resources
	ConstantBuffer* UtilityPerGeometryCB = nullptr;
	ConstantBuffer* LightingPerGeometryCB = nullptr;
	SkyrimARPGShaderAPI::Internal* TubusLib = nullptr;
	float TransitionProgress = 0.0f;
	float DebounceProgress = 0.0f;
	RE::NiPoint3 CameraPlayerCollisionPoint;
	RE::NiPoint3 PlayerCameraCollisionPoint;

	// CharacterOutline resources
	ConstantBuffer* outlineSettingsCB = nullptr;
	ID3D11ComputeShader* outlineCS = nullptr;
	ID3D11Texture2D* characterMaskTex = nullptr;
	ID3D11ShaderResourceView* characterMaskSRV = nullptr;
	ID3D11UnorderedAccessView* characterMaskUAV = nullptr;

	////////////////////////////////////////////////// Boilerplate
	// Metadata
	std::string GetName() override { return "SkyrimARPG"; }
	std::string GetShortName() override { return "SkyrimARPG"; }
	std::string_view GetCategory() const override { return "Lighting"; }
	std::string GetFeatureModLink() override { return MakeNexusModURL("999999"); }
	std::pair<std::string, std::vector<std::string>> GetFeatureSummary() override
	{
		return { "ARPG-style character visibility effects.",
			{
				"Camera-to-character occlusion reveal shader.",
				"Screen-space outline around characters.",
			} };
	}

	// Functionality
	bool SupportsVR() override { return true; }
	std::string_view GetShaderDefineName() override { return "SKYRIM_ARPG"; }
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
	void ClearShaderCache() override;

	////////////////////////////////////////////////// Tubus Settings
	struct TubusSettings
	{
		float Radius = 210.0f;
		float EdgeWidth = 75.0f;
		float TransitionSpeed = 0.6f;
	} tubusSettings;

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

	struct OutlineSettings
	{
		uint32_t Thickness = 2;
	} outlineSettings;

	struct alignas(16) OutlineCBData
	{
		float Thickness;
		float3 pad;
	};

	static_assert(sizeof(OutlineCBData) % 16 == 0, "OutlineCBData must be aligned to 16 bytes.");

	void DrawOutline();
	ID3D11ComputeShader* GetComputeOutline();
};
