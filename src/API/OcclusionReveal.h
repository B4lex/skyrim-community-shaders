#pragma once

namespace SkyrimARPGShaderAPI
{
	struct alignas(16) PerGeometryData
	{
		RE::NiColorA OutlineColor = {0};
		bool EnableCulling = false;
		char pad0[3] = {};
		bool ForceEnableCulling = false;
		char pad1[11] = {};
	};

	static_assert(sizeof(PerGeometryData) % 16 == 0, "PerGeometryData must be aligned to 16 bytes.");

	struct Internal
	{
		virtual ~Internal() = default;

		// occlusion reveal
		virtual bool ShouldEnableCulling() noexcept = 0;
		virtual PerGeometryData GetPerGeometryData(RE::BSGeometry*) noexcept = 0;
		virtual bool GetFirstCollisionPoint(const RE::NiPoint3&, const RE::NiPoint3&, RE::NiPoint3&) noexcept = 0;
		virtual std::vector<RE::NiPoint3> GetPlayerRayCastPoints(RE::PlayerCharacter*) noexcept = 0;
		virtual bool GetPlayerCameraCollisionPoint(RE::PlayerCharacter*, RE::NiPoint3&) noexcept = 0;
		virtual bool GetCameraPlayerCollisionPoint(RE::PlayerCharacter*, RE::NiPoint3&) noexcept = 0;
		// object outline
		virtual bool IsObjectOutlined(RE::TESObjectREFR*) noexcept = 0;
	};
}
