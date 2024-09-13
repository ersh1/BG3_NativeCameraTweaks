#pragma once
#include "RE/Camera.h"

namespace Utils
{
	bool GetProductVersion(std::string_view a_processPath, std::string& a_outProductVersion);
	int16_t GetPlayerID(RE::UnkObject* a1);
	RE::Player* GetCurrentPlayer(RE::UnkObject* a1);
	RE::CameraDefinition* GetCurrentCameraDefinition(RE::CameraModeFlags a_cameraModeFlags);
}
