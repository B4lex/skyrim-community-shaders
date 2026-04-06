#pragma once

namespace SkyrimARPGShaderAPI
{
	typedef void* (*RequestPluginAPI_)();

	[[nodiscard]] inline void* RequestPluginAPI()
	{
		const auto pluginHandle = GetModuleHandle(L"skyrim-arpg-shader-lib.dll");
		const auto requestAPIFunction = reinterpret_cast<RequestPluginAPI_>(GetProcAddress(pluginHandle, "RequestPluginAPI"));
		return requestAPIFunction();
	}
}