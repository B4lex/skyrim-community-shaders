#pragma once

namespace TubusAPI
{
	struct alignas(16) PerGeometryData
	{
		bool EnableCulling = false;
		char pad0[3] = {};
		bool ForceEnableCulling = false;
		char pad1[11] = {};
	};

	struct ObjectOutlineConfig
	{
		RE::NiColorA Color;
	};

	struct ObjectOutline : ObjectOutlineConfig
	{
		RE::TESObjectREFR* Object;
		ObjectOutline(RE::TESObjectREFR*, RE::NiColorA) noexcept;
	};

	static_assert(sizeof(PerGeometryData) % 16 == 0, "PerGeometryData must be aligned to 16 bytes.");

	struct Tubus
	{
		virtual ~Tubus() = default;

		virtual bool ShouldEnableCulling() noexcept = 0;
		virtual PerGeometryData GetPerGeometryData(RE::BSGeometry*) noexcept = 0;
		virtual bool GetFirstCollisionPoint(const RE::NiPoint3&, const RE::NiPoint3&, RE::NiPoint3&) noexcept = 0;
		virtual std::vector<RE::NiPoint3> GetPlayerRayCastPoints(RE::PlayerCharacter*) noexcept = 0;
		virtual bool GetPlayerCameraCollisionPoint(RE::PlayerCharacter*, RE::NiPoint3&) noexcept = 0;
		virtual bool GetCameraPlayerCollisionPoint(RE::PlayerCharacter*, RE::NiPoint3&) noexcept = 0;
		virtual bool IsObjectOutlined(RE::TESObjectREFR*) noexcept = 0;
		virtual void SetOutlineObjects(const std::vector<ObjectOutline>&) noexcept = 0;
		virtual void ResetOutlineObjects() noexcept = 0;
	};
	typedef void* (*RequestPluginAPI_)();

	[[nodiscard]] inline void* RequestPluginAPI()
	{
		const auto pluginHandle = GetModuleHandle(L"tubus-lib.dll");
		const auto requestAPIFunction = reinterpret_cast<RequestPluginAPI_>(GetProcAddress(pluginHandle, "RequestPluginAPI"));
		return requestAPIFunction();
	}
}
