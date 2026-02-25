#pragma once

struct Tubus : public Feature
{
	////////////////////////////////////////////////// Boilerplate
	// Metadata
	virtual inline std::string GetName() override { return "Tubus"; }
	virtual inline std::string GetShortName() override { return "Tubus"; }
	virtual inline std::string_view GetCategory() const override { return "Lighting"; }
	virtual inline std::string GetFeatureModLink() override { return MakeNexusModURL("999999"); }
	virtual inline std::pair<std::string, std::vector<std::string>> GetFeatureSummary() override
	{
		return {
			"Camera-to-character occlusion reveal shader.",
			{
				"Discards occluding meshes between camera and player.",
				"Configurable screen-space radius and falloff.",
			}
		};
	}

	// Functionality
	virtual bool inline SupportsVR() override { return true; }
	virtual inline std::string_view GetShaderDefineName() override { return "TUBUS"; }
	virtual inline bool HasShaderDefine(RE::BSShader::Type t) override { return t == RE::BSShader::Type::Utility; };

	// Settings & UI
	virtual void RestoreDefaultSettings() override;
	virtual void LoadSettings(json& o_json) override;
	virtual void SaveSettings(json& o_json) override;
	virtual void DrawSettings() override;

	// Resources
	virtual void SetupResources() override;

	////////////////////////////////////////////////// Feature Specific Data
	struct Settings
	{
		float Radius = 0.15f;
		float Falloff = 0.05f;
	} settings;

	struct CommonBufferData
	{
		float3 PlayerWorldPosition;
		float Radius;
		float Falloff;
		float3 _pad;
	};
	static_assert(sizeof(CommonBufferData) % 16 == 0,
		"CommonBufferData must be aligned to 16 bytes.");

	CommonBufferData GetCommonBufferData();
};
