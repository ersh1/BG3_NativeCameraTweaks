#include "CameraTweaksAPI.h"
#include "CameraTweaksManager.h"

namespace
{
	void SetDesiredPitch(int16_t a_playerId, float a_pitch, float a_duration)
	{
		CameraTweaks::GetSingleton()->SetDesiredPitch(a_playerId, a_pitch, a_duration);
	}

	CameraTweaksApi api{
		.version = 1,
		.SetDesiredPitch = &SetDesiredPitch,
	};
}

extern "C" __declspec(dllexport)
const CameraTweaksApi*
	GetCameraTweaksApi()
{
	return &api;
}
