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
	};
	typedef void* (*_RequestPluginAPI)();

	[[nodiscard]] inline void* RequestPluginAPI()
	{
		auto pluginHandle = GetModuleHandle(L"tubus-lib.dll");
		auto requestAPIFunction = reinterpret_cast<_RequestPluginAPI>(GetProcAddress(pluginHandle, "RequestPluginAPI"));
		return requestAPIFunction();
	}
}
