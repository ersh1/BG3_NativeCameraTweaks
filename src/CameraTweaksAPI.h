#pragma once

#include <cstdint>

struct CameraTweaksApi
{
	uint32_t version;

	void (*SetDesiredPitch)(int16_t a_playerId, float a_pitch, float a_duration);
};
