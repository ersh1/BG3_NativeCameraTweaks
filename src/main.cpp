#include "Hooks.h"
#include "Settings.h"
#include "Utils.h"

unsigned int __stdcall InitThread(void* param)
{
	const auto settings = Settings::Main::GetSingleton();
	if (*settings->WatchForConfigChanges) {
		settings->WatchForChanges();
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE a_hModule, DWORD a_ul_reason_for_call, LPVOID a_lpReserved)
{
	if (a_ul_reason_for_call == DLL_PROCESS_ATTACH) {
#ifndef NDEBUG
		while (!IsDebuggerPresent()) {
			Sleep(100);
		}
#endif

		// stuff
		dku::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

		INFO("process : {}", dku::Hook::GetProcessName())
	    const auto processPath = dku::Hook::GetProcessPath();
		INFO("process path : {}", processPath)

		std::string productVersion;
		if (Utils::GetProductVersion(processPath, productVersion)) {
			INFO("process version : {}", productVersion)
		} else {
			WARN("process version not found!")
		}

		const auto settings = Settings::Main::GetSingleton();
		settings->Load();

		_beginthreadex(NULL, 0, InitThread, NULL, 0, NULL);

		Hooks::Install();

	}

	return TRUE;
}
