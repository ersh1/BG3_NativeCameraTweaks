#include "CameraTweaksManager.h"

#include "Hooks.h"
#include "Settings.h"

void CameraTweaks::SetCameraSettings()
{
	if (_bValuesSet) {
	    return;
	}

	auto settings = Settings::Main::GetSingleton();

	// Exploration camera
	{
		RE::CameraDefinition* camera = reinterpret_cast<RE::CameraDefinition*>(reinterpret_cast<uintptr_t>(*Hooks::Offsets::UnkCameraSingletonPtr) + 0x78C);
		
		if (!*settings->ExplorationUnlockPitch && *settings->ExplorationOverrideLockedPitch) {
		    camera->pitchClose_164 = *settings->ExplorationLockedPitchClose;
		    camera->pitchFar_160 = *settings->ExplorationLockedPitchFar;
			camera->tacticalPitchClose_174 = *settings->ExplorationLockedTacticalPitchClose;
			camera->tacticalPitchFar_170 = *settings->ExplorationLockedTacticalPitchFar;
			camera->pitchCloseAlt_17C = *settings->ExplorationLockedAltPitchClose;
			camera->pitchFarAlt_178 = *settings->ExplorationLockedAltPitchFar;
		}

		if (*settings->ExplorationOverrideZoom) {
			camera->minZoom_2C = *settings->ExplorationZoomMin;
			camera->maxZoom_28 = *settings->ExplorationZoomMax;
			camera->tactMinZoom_C8 = *settings->ExplorationTacticalZoomMin;
			camera->tactMaxZoom_CC = *settings->ExplorationTacticalZoomMax;
			camera->minZoomAlt_34 = *settings->ExplorationAltZoomMin;
			camera->maxZoomAlt_30 = *settings->ExplorationAltZoomMax;
		}

		if (*settings->ExplorationOverrideFOV) {
			camera->fovClose_84 = *settings->ExplorationFOVClose;
			camera->fovFar_88 = *settings->ExplorationFOVFar;
			camera->tacticalFov_D0 = *settings->ExplorationTacticalFOV;
			camera->fovCloseAlt_8C = *settings->ExplorationAltFOVClose;
			camera->fovFarAlt_90 = *settings->ExplorationAltFOVFar;
		}

	    if (*settings->ExplorationOverrideOffset) {
			camera->camVerticalOffsetMult_68 = *settings->ExplorationVerticalOffsetMult;
			camera->camHorizontalOffsetMult_64 = *settings->ExplorationHorizontalOffsetMult;
		}
	}

	// Combat camera
	{
		RE::CameraDefinition* camera = reinterpret_cast<RE::CameraDefinition*>(reinterpret_cast<uintptr_t>(*Hooks::Offsets::UnkCameraSingletonPtr) + 0x920);

		if (!*settings->CombatUnlockPitch && *settings->CombatOverrideLockedPitch) {
			camera->pitchCombatClose_16C = *settings->CombatLockedPitchClose;
			camera->pitchCombatFar_168 = *settings->CombatLockedPitchFar;
			camera->tacticalPitchClose_174 = *settings->CombatLockedTacticalPitchClose;
			camera->tacticalPitchFar_170 = *settings->CombatLockedTacticalPitchFar;
			camera->pitchCloseAlt_17C = *settings->CombatLockedAltPitchClose;
			camera->pitchFarAlt_178 = *settings->CombatLockedAltPitchFar;
		}

		if (*settings->CombatOverrideZoom) {
			camera->minZoom_2C = *settings->CombatZoomMin;
			camera->maxZoom_28 = *settings->CombatZoomMax;
			camera->tactMinZoom_C8 = *settings->CombatTacticalZoomMin;
			camera->tactMaxZoom_CC = *settings->CombatTacticalZoomMax;
			camera->minZoomAlt_34 = *settings->CombatAltZoomMin;
			camera->maxZoomAlt_30 = *settings->CombatAltZoomMax;
		}

		if (*settings->CombatOverrideFOV) {
			camera->fovClose_84 = *settings->CombatFOVClose;
			camera->fovFar_88 = *settings->CombatFOVFar;
			camera->tacticalFov_D0 = *settings->CombatTacticalFOV;
			camera->fovCloseAlt_8C = *settings->CombatAltFOVClose;
			camera->fovFarAlt_90 = *settings->CombatAltFOVFar;
		}

		if (*settings->CombatOverrideOffset) {
			camera->camVerticalOffsetMult_68 = *settings->CombatVerticalOffsetMult;
			camera->camHorizontalOffsetMult_64 = *settings->CombatHorizontalOffsetMult;
		}
	}

	_bValuesSet = true;
}

int16_t CameraTweaks::GetPlayerIdFromCameraObject(RE::CameraObject* a_cameraObject) const
{
	for (int i = 0; i < _playerData.size(); i++) {
		if (_playerData[i].cameraObject == a_cameraObject) {
			return i + 1;
		}
	}

	return 0;
}

void CameraTweaks::SetCameraObjectForPlayer(int16_t a_playerId, RE::CameraObject* a_cameraObject)
{
    if (a_playerId > 0 && a_playerId <= _playerData.size()) {
        _playerData[a_playerId - 1].cameraObject = a_cameraObject;
    }
}

CameraTweaks::CameraMode CameraTweaks::GetCurrentCameraMode(RE::CameraObject* a_cameraObject)
{
	if (a_cameraObject) {
	    return GetCurrentCameraMode(a_cameraObject->cameraModeFlags_A8);
	}

	return CameraMode::kExploration;
}

CameraTweaks::CameraMode CameraTweaks::GetCurrentCameraMode(uint32_t a_cameraModeFlags)
{
	/*const bool bUnkCamera = *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(*Hooks::Offsets::UnkCameraSingletonPtr) + 0x1311);
	if (bUnkCamera) {
	    return CameraMode::kFreeCamera;
	}*/

	if ((a_cameraModeFlags & 1) != 0) {
		return CameraMode::kCombat;
	}

	return CameraMode::kExploration;
}

bool CameraTweaks::IsCameraUnlocked(int16_t a_playerId, RE::CameraObject* a_cameraObject) const
{
	if (GetPlayerData(a_playerId).pitch.has_value()) {
	    return true;
	}

	return CanAdjustPitch(a_cameraObject);
}

bool CameraTweaks::CanAdjustPitch(RE::CameraObject* a_cameraObject) const
{
	const auto cameraMode = GetCurrentCameraMode(a_cameraObject);
	const auto settings = Settings::Main::GetSingleton();

	switch (cameraMode) {
	case CameraMode::kExploration:
		return *settings->ExplorationUnlockPitch;
	case CameraMode::kCombat:
		return *settings->CombatUnlockPitch;
	}

	return false;
}

void CameraTweaks::SetControllerPitchDelta(int16_t a_playerId, float a_inputValue)
{
	const auto settings = Settings::Main::GetSingleton();

	const float deadzone = GetDeadzone();
	const float normalizeDeadzone = *settings->OverrideRightStickDeadzone ? 1.f / (1.f - *settings->NewDeadzone) : NORMALIZE_DEADZONE;

	if (fabs(a_inputValue) <= deadzone) {  // deadzone
		GetPlayerData(a_playerId).controllerPitchDelta = 0.f;
	} else {
		const float value = (a_inputValue - deadzone) * normalizeDeadzone;  // normalize outside of deadzone

		float sign = *settings->InvertControllerPitch ? -1.f : 1.f;
		GetPlayerData(a_playerId).controllerPitchDelta = value * sign * *settings->ControllerCameraRotationMult;
	}
}

bool CameraTweaks::CalculateCameraPitch(int16_t a_playerId, RE::CameraObject* a_cameraObject, float& a_outPitch)
{
	const auto settings = Settings::Main::GetSingleton();

    const auto cameraMode = GetCurrentCameraMode(a_cameraObject);
	bool bIsPitchUnlocked = false;

	switch (cameraMode) {
	case CameraMode::kExploration:
		bIsPitchUnlocked = *settings->ExplorationUnlockPitch;
		break;
	case CameraMode::kCombat:
		bIsPitchUnlocked = *settings->CombatUnlockPitch;
		break;
	}

	auto& playerData = GetPlayerData(a_playerId);

	if (bIsPitchUnlocked) {
		float pitchMin, pitchMax; 
		switch (cameraMode) {
		case CameraMode::kExploration:
			pitchMin = *settings->ExplorationUnlockedPitchMin;
			pitchMax = *settings->ExplorationUnlockedPitchMax;
			break;
		case CameraMode::kCombat:
			pitchMin = *settings->CombatUnlockedPitchMin;
			pitchMax = *settings->CombatUnlockedPitchMax;
			break;
		}
				
		int32_t deltaY = 0;
		if ((a_cameraObject->cameraModeFlags_A8 & 0x100) != 0) {  // mouse rotation mode
			const float sign = *settings->InvertMousePitch ? -1.f : 1.f;
			deltaY = delta_y * sign;
			delta_y = 0;

		}

		float pitchDelta = 0.f;
		pitchDelta += deltaY * *settings->MousePitchMult * *settings->MouseCameraRotationMult;  // mouse
		pitchDelta += playerData.controllerPitchDelta * _deltaTime * a_cameraObject->rotationSpeed_C0 * *settings->ControllerPitchMult;  // controller

	    if (!playerData.pitch.has_value()) {
			playerData.pitch = std::clamp(static_cast<float>(*Settings::Main::GetSingleton()->UnlockedPitchInitialValue), pitchMin, pitchMax);  // initialize pitch
		} else if (*playerData.pitch + pitchDelta > pitchMax) {
			playerData.pitch = std::min(*playerData.pitch, pitchMax);
		} else if (*playerData.pitch + pitchDelta < pitchMin) {
			playerData.pitch = std::max(*playerData.pitch, pitchMin);
        } else {
			playerData.pitch = *playerData.pitch + pitchDelta;
        }

		// smoothly clamp pitch if the limits have changed since the last frame
		if (playerData.pitch > pitchMax) {
			playerData.pitch = InterpTo(*playerData.pitch, pitchMax, _deltaTime, *settings->UnlockedPitchClampSpeed);
		} else if (playerData.pitch < pitchMin) {
			playerData.pitch = InterpTo(*playerData.pitch, pitchMin, _deltaTime, *settings->UnlockedPitchClampSpeed);
        }

		a_outPitch = *playerData.pitch;
		return true;
	} else {  // pitch is locked in this mode
		if (playerData.pitch.has_value()) {  // smoothly adjust pitch if our pitch didn't match the game pitch
			if (playerData.pitch != a_outPitch) {
				playerData.pitch = InterpTo(*playerData.pitch, a_outPitch, _deltaTime, *settings->UnlockedPitchClampSpeed);
			} else {
				playerData.pitch.reset();
			}
		}

		if (playerData.pitch.has_value()) {
			a_outPitch = *playerData.pitch;
		}
	    return false;
	}
}

void CameraTweaks::AdjustCameraZoomForPitch(RE::CameraObject* a_cameraObject, float a_characterHeight)
{
	const auto cameraDefinition = Hooks::Offsets::GetCurrentCameraDefinition(a_cameraObject->cameraModeFlags_A8);

	const float cameraHeight = (a_characterHeight * cameraDefinition->camVerticalOffsetMult_68) - *Settings::Main::GetSingleton()->UnlockedPitchFloorOffset;
	
	if (a_cameraObject->currentPitch_164 >= 0 || a_cameraObject->currentZoomB_58 <= cameraHeight) {
		return;
	}

	const float cosPitch = std::cos(DegreesToRadians(90.f + a_cameraObject->currentPitch_164));
	if (cosPitch == 0) {
	    return;
	}

	const float finalZoom = std::min(a_cameraObject->currentZoomB_58, cameraHeight / cosPitch);

	a_cameraObject->currentZoomA_54 = finalZoom;
	a_cameraObject->currentZoomB_58 = finalZoom;
}

float CameraTweaks::AdjustInputValueForDeadzone(float a_inputValue, bool a_bApplyMult)
{
	// adjust deadzone
	const float deadzone = GetDeadzone();

	if (a_inputValue > deadzone) {
		float normalizedValue = (a_inputValue - deadzone) * (1.f / (1.f - deadzone));
		if (a_bApplyMult) {
			normalizedValue *= *Settings::Main::GetSingleton()->ControllerCameraRotationMult;  // apply multiplier from settings
		}
		return Denormalize(VANILLA_DEADZONE, 1.f, normalizedValue);
	}

	return a_inputValue;
}

float CameraTweaks::GetDeadzone()
{
	auto* settings = Settings::Main::GetSingleton();

	return *settings->OverrideRightStickDeadzone ? *settings->NewDeadzone : VANILLA_DEADZONE;
}

float CameraTweaks::NormalizeWithinRange(float a_min, float a_max, float a_value)
{
	return fminf(a_max - a_min, fmaxf(a_min, fminf(a_max, a_value)) - a_min) / (a_max - a_min);
}

float CameraTweaks::Denormalize(float a_min, float a_max, float a_value)
{
	float normalizedRange = a_max - a_min;
	float scaledValue = a_value * normalizedRange;
	return scaledValue + a_min;
}

float CameraTweaks::DegreesToRadians(float a_degrees)
{
	static constexpr float PI = 3.1415926535897932;
	static constexpr float DEG_TO_RADIAN = PI / 180.f;

    return a_degrees * DEG_TO_RADIAN;
}

float CameraTweaks::InterpTo(float a_current, float a_target, float a_deltaTime, float a_interpSpeed)
{
	if (a_interpSpeed <= 0.f) {
	    return a_target;
	}

	const float distance = a_target - a_current;

	if (distance < FLT_EPSILON) {
	    return a_target;
	}

	const float delta = distance * std::clamp(a_deltaTime * a_interpSpeed, 0.f, 1.f);

	return a_current + delta;
}
