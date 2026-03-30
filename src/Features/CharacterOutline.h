#pragma once

struct CharacterOutline : Feature
{
	ConstantBuffer* outlineSettingsCB = nullptr;
	ID3D11ComputeShader* outlineCS = nullptr;

	////////////////////////////////////////////////// Boilerplate
	// Metadata
	std::string GetName() override { return "Character Outline"; }
	std::string GetShortName() override { return "CharacterOutline"; }
	std::string_view GetCategory() const override { return "Lighting"; }
	std::string GetFeatureModLink() override { return ""; }
	std::pair<std::string, std::vector<std::string>> GetFeatureSummary() override
	{
		return { "Screen-space outline around characters.",
			{
				"Highlights the player character and NPCs with a configurable outline.",
				"Uses per-geometry detection and edge detection compute shader.",
			} };
	}

	// Functionality
	bool SupportsVR() override { return true; }
	std::string_view GetShaderDefineName() override { return "CHARACTER_OUTLINE"; }
	bool HasShaderDefine(RE::BSShader::Type t) override
	{
		return t == RE::BSShader::Type::Lighting;
	};

	// Settings & UI
	void RestoreDefaultSettings() override;
	void LoadSettings(json& o_json) override;
	void SaveSettings(json& o_json) override;
	void DrawSettings() override;

	// Resources
	void SetupResources() override;
	void ClearShaderCache() override;

	////////////////////////////////////////////////// Feature Specific Data
	struct Settings
	{
		float OutlineColorR = 1.0f;
		float OutlineColorG = 1.0f;
		float OutlineColorB = 1.0f;
		float OutlineOpacity = 0.8f;
		uint32_t Thickness = 2;
		bool OutlineNPCs = true;
	} settings;

	struct alignas(16) OutlineCBData
	{
		float4 OutlineColor;
		float Thickness;
		float3 pad;
	};

	static_assert(sizeof(OutlineCBData) % 16 == 0, "OutlineCBData must be aligned to 16 bytes.");

	void DrawOutline();
	ID3D11ComputeShader* GetComputeOutline();
};
