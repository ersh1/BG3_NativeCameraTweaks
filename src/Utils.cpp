#include "Utils.h"

#include "Hooks.h"

#pragma comment(lib, "version")

namespace Utils
{
    bool GetProductVersion(std::string_view a_processPath, std::string& a_outProductVersion)
    {
		const auto file = fmt::detail::utf8_to_utf16(a_processPath);
		DWORD dwHandle;
		const DWORD size = GetFileVersionInfoSizeW(file.c_str(), &dwHandle);
		if (size != 0) {
			std::vector<BYTE> versionInfoBuffer(size);

			if (GetFileVersionInfoW(file.c_str(), 0, size, versionInfoBuffer.data())) {
				LPVOID versionInfo;
				UINT versionInfoSize;
				if (VerQueryValueW(versionInfoBuffer.data(), L"\\StringFileInfo\\000004B0\\ProductVersion", &versionInfo, &versionInfoSize)) {
					const auto productVersion = fmt::detail::to_utf8<wchar_t>(static_cast<const wchar_t*>(versionInfo));
					a_outProductVersion = productVersion.c_str();

					return true;
				}
			}
		}

		return false;
    }

    int16_t GetPlayerID(RE::UnkObject* a1)
    {
		auto currentPlayer = a1->currentPlayer_70;
		if (!currentPlayer) {
			if (a1->unk120) {
				currentPlayer = Hooks::Offsets::GetCurrentPlayerInternal(a1->unk08, a1->unk00);
			}
		}

		return currentPlayer->playerId_38;
    }
}
