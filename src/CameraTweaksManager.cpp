#include "CameraTweaksManager.h"

#include "Hooks.h"
#include "Settings.h"
#include "Utils.h"

void CameraTweaks::SetCameraSettings()
{
	const auto settings = Settings::Main::GetSingleton();

	{
		ReadLocker locker(settings->Lock);

		if (!settings->bChanged) {
			return;
		}

		// Exploration camera
		{
			RE::CameraDefinition* camera = reinterpret_cast<RE::CameraDefinition*>(reinterpret_cast<uintptr_t>(*Hooks::Offsets::UnkCameraSingletonPtr) + Hooks::Offsets::explorationCameraOffset);

			if (*settings->ExplorationOverrideLockedPitch) {
				if (!*settings->ExplorationUnlockPitch) {
					camera->pitchClose_164 = *settings->ExplorationLockedPitchClose;
					camera->pitchFar_160 = *settings->ExplorationLockedPitchFar;
					camera->tacticalPitchClose_174 = *settings->ExplorationLockedTacticalPitchClose;
					camera->tacticalPitchFar_170 = *settings->ExplorationLockedTacticalPitchFar;
					camera->pitchCloseAlt_17C = *settings->ExplorationLockedAltPitchClose;
					camera->pitchFarAlt_178 = *settings->ExplorationLockedAltPitchFar;
				} else if (*settings->ExplorationKeepTacticalPitchLocked) {
					camera->tacticalPitchClose_174 = *settings->ExplorationLockedTacticalPitchClose;
					camera->tacticalPitchFar_170 = *settings->ExplorationLockedTacticalPitchFar;
				}
			}

			if (*settings->ExplorationOverrideZoom) {
				camera->minZoom_2C = *settings->ExplorationZoomMin;
				camera->maxZoom_28 = *settings->ExplorationZoomMax;
				camera->tactMinZoom_C8 = *settings->ExplorationTacticalZoomMin;
				camera->tactMaxZoom_CC = *settings->ExplorationTacticalZoomMax;
				camera->altMinZoomController_34 = *settings->ExplorationAltZoomMin;
				camera->altMaxZoomController_30 = *settings->ExplorationAltZoomMax;
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
			RE::CameraDefinition* camera = reinterpret_cast<RE::CameraDefinition*>(reinterpret_cast<uintptr_t>(*Hooks::Offsets::UnkCameraSingletonPtr) + Hooks::Offsets::combatCameraOffset);

			if (*settings->CombatOverrideLockedPitch) {
				if (!*settings->CombatUnlockPitch) {
					camera->pitchCombatClose_16C = *settings->CombatLockedPitchClose;
					camera->pitchCombatFar_168 = *settings->CombatLockedPitchFar;
					camera->tacticalPitchClose_174 = *settings->CombatLockedTacticalPitchClose;
					camera->tacticalPitchFar_170 = *settings->CombatLockedTacticalPitchFar;
					camera->pitchCloseAlt_17C = *settings->CombatLockedAltPitchClose;
					camera->pitchFarAlt_178 = *settings->CombatLockedAltPitchFar;
				} else if (*settings->CombatKeepTacticalPitchLocked) {
					camera->tacticalPitchClose_174 = *settings->CombatLockedTacticalPitchClose;
					camera->tacticalPitchFar_170 = *settings->CombatLockedTacticalPitchFar;
				}
			}

			if (*settings->CombatOverrideZoom) {
				camera->minZoom_2C = *settings->CombatZoomMin;
				camera->maxZoom_28 = *settings->CombatZoomMax;
				camera->tactMinZoom_C8 = *settings->CombatTacticalZoomMin;
				camera->tactMaxZoom_CC = *settings->CombatTacticalZoomMax;
				camera->altMinZoomController_34 = *settings->CombatAltZoomMin;
				camera->altMaxZoomController_30 = *settings->CombatAltZoomMax;
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
	}
	
	WriteLocker locker(settings->Lock);
	settings->bChanged = false;
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
	    return GetCurrentCameraMode(a_cameraObject->cameraModeFlags);
	}

	return CameraMode::kExploration;
}

CameraTweaks::CameraMode CameraTweaks::GetCurrentCameraMode(RE::CameraModeFlags a_cameraModeFlags)
{
	if (a_cameraModeFlags & RE::CameraModeFlags::kCombat) {
		if (a_cameraModeFlags & RE::CameraModeFlags::kTactical) {
			return CameraMode::kCombatTactical;
		}
		return CameraMode::kCombat;
	}

	if (a_cameraModeFlags & RE::CameraModeFlags::kTactical) {
	    return CameraMode::kExplorationTactical;
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
	
	return CanAdjustPitch(cameraMode);
}

bool CameraTweaks::CanAdjustPitch(CameraTweaks::CameraMode cameraMode) const
{
	const auto settings = Settings::Main::GetSingleton();

	ReadLocker locker(settings->Lock);

	switch (cameraMode) {
	case CameraMode::kExploration:
		return *settings->ExplorationUnlockPitch;
	case CameraMode::kCombat:
		return *settings->CombatUnlockPitch;
	case CameraMode::kExplorationTactical:
		return *settings->ExplorationUnlockPitch && !*settings->ExplorationKeepTacticalPitchLocked;
	case CameraMode::kCombatTactical:
		return *settings->CombatUnlockPitch && !*settings->CombatKeepTacticalPitchLocked;
	}

	return false;
}

void CameraTweaks::SetControllerPitchDelta(int16_t a_playerId, float a_inputValue)
{
	const auto settings = Settings::Main::GetSingleton();
	ReadLocker locker(settings->Lock);

	const float deadzone = GetDeadzone();
	const float normalizeDeadzone = *settings->OverrideRightStickDeadzone ? 1.f / (1.f - *settings->NewDeadzone) : NORMALIZE_DEADZONE;

	if (fabs(a_inputValue) <= deadzone) {  // deadzone
		GetPlayerData(a_playerId).controllerPitchDelta = 0.f;
	} else {
		float sign = a_inputValue < 0.f ? -1.f : 1.f;
		float value = sign * (fabs(a_inputValue) - deadzone) * normalizeDeadzone;  // normalize outside of deadzone

		value *= *settings->InvertControllerPitch ? -1.f : 1.f;
		GetPlayerData(a_playerId).controllerPitchDelta = value * *settings->ControllerCameraRotationMult;
	}
}

bool CameraTweaks::CalculateCameraPitch(int16_t a_playerId, RE::CameraObject* a_cameraObject, float& a_outPitch)
{
	const auto settings = Settings::Main::GetSingleton();

    const auto cameraMode = GetCurrentCameraMode(a_cameraObject);
	bool bIsPitchUnlocked = CanAdjustPitch(cameraMode);

	auto& playerData = GetPlayerData(a_playerId);

	ReadLocker locker(settings->Lock);

	if (bIsPitchUnlocked) {
		float pitchMin, pitchMax; 
		switch (cameraMode) {
		case CameraMode::kExploration:
		case CameraMode::kExplorationTactical:
			pitchMin = *settings->ExplorationUnlockedPitchMin;
			pitchMax = *settings->ExplorationUnlockedPitchMax;
			break;
		case CameraMode::kCombat:
		case CameraMode::kCombatTactical:
			pitchMin = *settings->CombatUnlockedPitchMin;
			pitchMax = *settings->CombatUnlockedPitchMax;
			break;
		}
				
		int32_t deltaY = 0;
		if (a_cameraObject->cameraModeFlags & RE::CameraModeFlags::kMouseRotation) {  // mouse rotation mode
			const float sign = *settings->InvertMousePitch ? -1.f : 1.f;
			deltaY = delta_y * sign;
			delta_y = 0;
		} else {
		    delta_y = 0;
		}

		float pitchDelta = 0.f;
		pitchDelta += deltaY * *settings->MousePitchMult * *settings->MouseCameraRotationMult;  // mouse
		pitchDelta += playerData.controllerPitchDelta * _deltaTime * a_cameraObject->rotationSpeed * *settings->ControllerPitchMult;  // controller

	    if (!playerData.pitch.has_value()) {
			playerData.pitch = std::clamp(static_cast<float>(*settings->UnlockedPitchInitialValue), pitchMin, pitchMax);  // initialize pitch
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

void CameraTweaks::AdjustCameraZoomForPitch(uint64_t a1, RE::CameraObject* a2)
{
    const auto cameraObject = a2;
	//const float minZoom = Hooks::Offsets::GetCameraMinZoom(cameraObject->cameraModeFlags, cameraObject->unkZoom_13C > 1);
	constexpr float minZoom = 0.5f;

	if (cameraObject->currentZoomB <= minZoom) {
	    return;
	}

	float floorOffset;
	{
        const auto settings = Settings::Main::GetSingleton();
		ReadLocker locker(settings->Lock);

		floorOffset = *settings->UnlockedPitchFloorOffset;
	}

	bool bIsUnderFloorLevel;

	const auto skipSteps = std::truncf((cameraObject->desiredZoom - cameraObject->currentZoomB) / ZOOM_ADJUST_STEP);
	float finalZoom = cameraObject->desiredZoom - (skipSteps * ZOOM_ADJUST_STEP);
	do {
		RE::Vector3 finalCameraPos;
		finalCameraPos.x = cameraObject->desiredCameraRootPos.x + cameraObject->cameraRotation.x * finalZoom;
		finalCameraPos.y = cameraObject->desiredCameraRootPos.y + cameraObject->cameraRotation.y * finalZoom;
		finalCameraPos.z = cameraObject->desiredCameraRootPos.z + cameraObject->cameraRotation.z * finalZoom;

		RE::FloorLevelStruct floorLevelStruct;
		Hooks::Offsets::GetFloorLevel(floorLevelStruct, a1, cameraObject, finalCameraPos);

		if (floorLevelStruct.unk08) {
			bIsUnderFloorLevel = false;
		} else {
			bIsUnderFloorLevel = finalCameraPos.y < floorLevelStruct.floorLevel + floorOffset;

			if (bIsUnderFloorLevel) {
				finalZoom = std::max(finalZoom - ZOOM_ADJUST_STEP, minZoom);
				cameraObject->currentZoomB = finalZoom;
				cameraObject->currentZoomA = finalZoom;

				if (finalZoom <= minZoom) {
				    return;
				}
			}
		}
	} while (bIsUnderFloorLevel);
}

float CameraTweaks::AdjustInputValueForDeadzone(float a_inputValue, bool a_bApplyMult)
{
	// adjust deadzone
	const float deadzone = GetDeadzone();

	if (a_inputValue > deadzone) {
		float normalizedValue = (a_inputValue - deadzone) * (1.f / (1.f - deadzone));
		if (a_bApplyMult) {
			const auto settings = Settings::Main::GetSingleton();
			ReadLocker locker(settings->Lock);
			normalizedValue *= *settings->ControllerCameraRotationMult;  // apply multiplier from settings
		}
		return Denormalize(VANILLA_DEADZONE, 1.f, normalizedValue);
	}

	return 0.f;
}

float CameraTweaks::GetDeadzone()
{
	const auto settings = Settings::Main::GetSingleton();
	ReadLocker locker(settings->Lock);

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
