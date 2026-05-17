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

		// Camera singleton must be valid
		if (!Hooks::Offsets::UnkCameraSingletonPtr || !*Hooks::Offsets::UnkCameraSingletonPtr) {
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
				constexpr float kMargin = 0.1f;
				float zoomMin = static_cast<float>(std::min(*settings->ExplorationZoomMin, *settings->ExplorationZoomMax));
				float zoomMax = static_cast<float>(std::max(*settings->ExplorationZoomMin, *settings->ExplorationZoomMax));
				if (zoomMax - zoomMin < kMargin) zoomMax = zoomMin + kMargin;
				camera->minZoom_2C = zoomMin;
				camera->maxZoom_28 = zoomMax;
				float tactMin = static_cast<float>(std::min(*settings->ExplorationTacticalZoomMin, *settings->ExplorationTacticalZoomMax));
				float tactMax = static_cast<float>(std::max(*settings->ExplorationTacticalZoomMin, *settings->ExplorationTacticalZoomMax));
				if (tactMax - tactMin < kMargin) tactMax = tactMin + kMargin;
				camera->tactMinZoom_C8 = tactMin;
				camera->tactMaxZoom_CC = tactMax;
				float altMin = static_cast<float>(std::min(*settings->ExplorationAltZoomMin, *settings->ExplorationAltZoomMax));
				float altMax = static_cast<float>(std::max(*settings->ExplorationAltZoomMin, *settings->ExplorationAltZoomMax));
				if (altMax - altMin < kMargin) altMax = altMin + kMargin;
				camera->altMinZoomController_34 = altMin;
				camera->altMaxZoomController_30 = altMax;
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
				constexpr float kMargin = 0.1f;
				float zoomMin = static_cast<float>(std::min(*settings->CombatZoomMin, *settings->CombatZoomMax));
				float zoomMax = static_cast<float>(std::max(*settings->CombatZoomMin, *settings->CombatZoomMax));
				if (zoomMax - zoomMin < kMargin) zoomMax = zoomMin + kMargin;
				camera->minZoom_2C = zoomMin;
				camera->maxZoom_28 = zoomMax;
				float tactMin = static_cast<float>(std::min(*settings->CombatTacticalZoomMin, *settings->CombatTacticalZoomMax));
				float tactMax = static_cast<float>(std::max(*settings->CombatTacticalZoomMin, *settings->CombatTacticalZoomMax));
				if (tactMax - tactMin < kMargin) tactMax = tactMin + kMargin;
				camera->tactMinZoom_C8 = tactMin;
				camera->tactMaxZoom_CC = tactMax;
				float altMin = static_cast<float>(std::min(*settings->CombatAltZoomMin, *settings->CombatAltZoomMax));
				float altMax = static_cast<float>(std::max(*settings->CombatAltZoomMin, *settings->CombatAltZoomMax));
				if (altMax - altMin < kMargin) altMax = altMin + kMargin;
				camera->altMinZoomController_34 = altMin;
				camera->altMaxZoomController_30 = altMax;
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

void CameraTweaks::AdjustCameraZoomForPitch(uint64_t a1, uint64_t a2, RE::CameraObject* a_cameraObject)
{
	if (!a_cameraObject) {
		return;
	}

	const auto cameraObject = a_cameraObject;
	const float minZoom = std::max(Hooks::Offsets::GetCameraMinZoom(cameraObject->cameraModeFlags, cameraObject->unkZoom_13C > 1), 0.f);

	const auto clampZoomToMin = [cameraObject, minZoom]() {
		cameraObject->desiredZoom = std::max(cameraObject->desiredZoom, minZoom);
		cameraObject->currentZoomA = std::max(cameraObject->currentZoomA, minZoom);
		cameraObject->currentZoomB = std::max(cameraObject->currentZoomB, minZoom);
		cameraObject->prevZoom_15C = std::max(cameraObject->prevZoom_15C, minZoom);
		cameraObject->currentZoom_160 = std::max(cameraObject->currentZoom_160, minZoom);
	};

	if (cameraObject->currentZoomB <= minZoom) {
		clampZoomToMin();
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

		//bool a3 = cameraObject->unkZoom_13C || (cameraObject->cameraModeFlags & 0x200) != 0;
		bool a3 = false;
		RE::CameraDefinition* cameraDefinition = Hooks::Offsets::GetCurrentCameraDefinition(cameraObject);
		Hooks::Offsets::GetFloorLevel(floorLevelStruct, a2, a3, cameraDefinition, nullptr, finalCameraPos, *reinterpret_cast<uint64_t*>(a1 + 0x118));

		if (floorLevelStruct.unk08) {
			bIsUnderFloorLevel = false;
		} else {
			bIsUnderFloorLevel = finalCameraPos.y < floorLevelStruct.floorLevel + floorOffset;

			if (bIsUnderFloorLevel) {
				finalZoom = std::max(finalZoom - ZOOM_ADJUST_STEP, minZoom);
				cameraObject->currentZoomB = finalZoom;
				cameraObject->currentZoomA = finalZoom;
				cameraObject->prevZoom_15C = finalZoom;
				cameraObject->currentZoom_160 = finalZoom;

				if (finalZoom <= minZoom) {
					clampZoomToMin();
				    return;
				}
			}
		}
	} while (bIsUnderFloorLevel);

	clampZoomToMin();
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

bool CameraTweaks::IsAlignCameraBehindOnSwitchEnabled(uint32_t a_cameraModeFlags)
{
	const auto settings = Settings::Main::GetSingleton();
	ReadLocker locker(settings->Lock);
	if (a_cameraModeFlags & RE::CameraModeFlags::kCombat) {
		return *settings->CombatAlignBehindOnSwitch;
	}
	return *settings->ExplorationAlignBehindOnSwitch;
}

bool CameraTweaks::IsAlignCameraBehindNPCEnabled(uint32_t a_cameraModeFlags)
{
	const auto settings = Settings::Main::GetSingleton();
	ReadLocker locker(settings->Lock);
	if (a_cameraModeFlags & RE::CameraModeFlags::kCombat) {
		return *settings->CombatAlignBehindNPC;
	}
	return *settings->ExplorationAlignBehindNPC;
}

void CameraTweaks::GetDebugAndAlignFlags(bool& a_outDebug, bool& a_outNeedEntityData)
{
	const auto settings = Settings::Main::GetSingleton();
	ReadLocker locker(settings->Lock);
	a_outDebug = *settings->EnableDebugMode;
	a_outNeedEntityData = a_outDebug
		|| *settings->ExplorationAlignBehindOnSwitch
		|| *settings->ExplorationAlignBehindNPC
		|| *settings->CombatAlignBehindOnSwitch
		|| *settings->CombatAlignBehindNPC;
}

void CameraTweaks::UpdateDebugData()
{
	bool bDebugMode = false;
	bool bNeedEntityData = false;
	GetDebugAndAlignFlags(bDebugMode, bNeedEntityData);

	if (bDebugMode && _memoryScanRequested.exchange(false) && _currentCamera) {
		RunMemoryScan(_currentCamera->cameraRootPos);
	}

	DebugData data{};

	data.deltaTime = _deltaTime;

	if (bDebugMode) {
		data.isControllerMode = Hooks::Offsets::bIsInControllerMode ? *Hooks::Offsets::bIsInControllerMode : false;
		data.cameraSingletonPtr = Hooks::Offsets::UnkCameraSingletonPtr ? *Hooks::Offsets::UnkCameraSingletonPtr : nullptr;
		data.unkSingletonPtr = Hooks::Offsets::UnkSingletonPtr ? *Hooks::Offsets::UnkSingletonPtr : nullptr;
		data.getCharacterSingletonPtr = Hooks::Offsets::GetCharacterSingletonPtr ? *Hooks::Offsets::GetCharacterSingletonPtr : nullptr;
	}

	if (_currentPlayer) {
		data.playerId = _currentPlayer->playerId_38;
		if (bDebugMode) {
			data.playerPtr = _currentPlayer;
		}
	}

	if (bNeedEntityData && _currentPlayer && Hooks::Offsets::GetCharacter) {
		const auto playerId = _currentPlayer->playerId_38;
		if (playerId > 0) {
			// Try calling GetCharacter with a given singleton
			auto tryGetCharacter = [&](void** singletonPtr) -> uintptr_t {
				if (!singletonPtr || !*singletonPtr) return 0;
				uintptr_t result = 0;
				__try {
					result = Hooks::Offsets::GetCharacter(reinterpret_cast<uintptr_t>(*singletonPtr), playerId);
				} __except (EXCEPTION_EXECUTE_HANDLER) {
					result = 0;
				}
				return result;
			};

			// Try the dedicated singleton found from GetCharacter's call site first
			if (Hooks::Offsets::GetCharacterSingletonPtr) {
				data.characterPtr = tryGetCharacter(Hooks::Offsets::GetCharacterSingletonPtr);
			}

			// Try all other singletons
			if (data.characterPtr == 0) {
				void** singletons[] = {
					Hooks::Offsets::UnkSingletonPtr,
					Hooks::Offsets::UnkPlayerSingletonPtr,
					Hooks::Offsets::UnkCameraSingletonPtr,
				};
				for (auto* sp : singletons) {
					if (sp == Hooks::Offsets::GetCharacterSingletonPtr) continue;
					data.characterPtr = tryGetCharacter(sp);
					if (data.characterPtr != 0) break;
				}
			}
		}
	}

	// Snapshot camera object (SEH-guarded)
	if (_currentCamera) {
		auto& cam = data.camera;
		__try {
			// Functional fields
			cam.cameraRootPos = _currentCamera->cameraRootPos;
			cam.desiredCameraRootPos = _currentCamera->desiredCameraRootPos;
			cam.cameraRotation = _currentCamera->cameraRotation;
			cam.cameraAngle = _currentCamera->angle;
			cam.cameraModeFlags = _currentCamera->cameraModeFlags;
			cam.valid = true;
			cam.ptr = _currentCamera;

			// Debug-only fields
			if (bDebugMode) {
				cam.cameraPos = _currentCamera->cameraPos;
				cam.cameraPos_150 = _currentCamera->cameraPos_150;
				cam.unkCameraRootPosA = _currentCamera->unkCameraRootPosA;
				cam.unkCameraRootPosB = _currentCamera->unkCameraRootPosB;
				cam.currentZoomA = _currentCamera->currentZoomA;
				cam.currentZoomB = _currentCamera->currentZoomB;
				cam.desiredZoom = _currentCamera->desiredZoom;
				cam.currentPitch = _currentCamera->currentPitch_164;
				cam.horizontalPanDelta = _currentCamera->horizontalPanDelta;
				cam.verticalPanDelta = _currentCamera->verticalPanDelta;
				cam.currentAngleDelta = _currentCamera->currentAngleDelta;
				cam.mouseRotationDelta = _currentCamera->mouseRotationDelta;
				cam.zoomDelta = _currentCamera->zoomDelta;
				cam.rotationSpeed = _currentCamera->rotationSpeed;
				cam.verticalOffset = _currentCamera->verticalOffset;
				cam.prevZoom_15C = _currentCamera->prevZoom_15C;
				cam.timer_140 = _currentCamera->timer_140;
			}
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			cam = {}; // reset
		}

		// Camera definition snapshot
		if (bDebugMode && Hooks::Offsets::GetCurrentCameraDefinition) {
			RE::CameraDefinition* camDef = nullptr;
			__try {
				camDef = Hooks::Offsets::GetCurrentCameraDefinition(_currentCamera);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				camDef = nullptr;
			}
			if (camDef) {
				auto& cd = data.cameraDef;
				cd.valid = true;
				cd.ptr = camDef;
				cd.maxZoom_28 = camDef->maxZoom_28;
				cd.minZoom_2C = camDef->minZoom_2C;
				cd.altMaxZoomController_30 = camDef->altMaxZoomController_30;
				cd.altMinZoomController_34 = camDef->altMinZoomController_34;
				cd.zoomSpeed_AC = camDef->zoomSpeed_AC;
				cd.scrollSpeed_B0 = camDef->scrollSpeed_B0;
				cd.unkDefaultZoom_24 = camDef->unkDefaultZoom_24;
				cd.tactDefaultZoom_C4 = camDef->tactDefaultZoom_C4;
				cd.tactMinZoom_C8 = camDef->tactMinZoom_C8;
				cd.tactMaxZoom_CC = camDef->tactMaxZoom_CC;
				cd.fovClose_84 = camDef->fovClose_84;
				cd.fovFar_88 = camDef->fovFar_88;
				cd.fovCloseAlt_8C = camDef->fovCloseAlt_8C;
				cd.fovFarAlt_90 = camDef->fovFarAlt_90;
				cd.tacticalFov_D0 = camDef->tacticalFov_D0;
				cd.pitchFar_160 = camDef->pitchFar_160;
				cd.pitchClose_164 = camDef->pitchClose_164;
				cd.pitchCombatFar_168 = camDef->pitchCombatFar_168;
				cd.pitchCombatClose_16C = camDef->pitchCombatClose_16C;
				cd.tacticalPitchFar_170 = camDef->tacticalPitchFar_170;
				cd.tacticalPitchClose_174 = camDef->tacticalPitchClose_174;
				cd.pitchFarAlt_178 = camDef->pitchFarAlt_178;
				cd.pitchCloseAlt_17C = camDef->pitchCloseAlt_17C;
				cd.pitchAdjustSpeedA_48 = camDef->pitchAdjustSpeedA_48;
				cd.pitchAdjustSpeedB_F0 = camDef->pitchAdjustSpeedB_F0;
				cd.pitchAdjustSpeedC_F4 = camDef->pitchAdjustSpeedC_F4;
				cd.camHorizontalOffsetMult_64 = camDef->camHorizontalOffsetMult_64;
				cd.camVerticalOffsetMult_68 = camDef->camVerticalOffsetMult_68;
				cd.altCamHorizontalOffsetMultController_70 = camDef->altCamHorizontalOffsetMultController_70;
				cd.altCamVerticalOffsetMultController_74 = camDef->altCamVerticalOffsetMultController_74;
				cd.maxCamDistanceFromRoot_D4 = camDef->maxCamDistanceFromRoot_D4;
			}
		}
	}

	// Detect character switch via desiredCameraRootPos
	bool bCharacterSwitched = false;
	if (data.camera.valid) {
		const auto& desiredPos = data.camera.desiredCameraRootPos;
		if (_hasPrevDesiredPos) {
			const float jdx = desiredPos.x - _prevDesiredRootPos.x;
			const float jdz = desiredPos.z - _prevDesiredRootPos.z;
			const float jumpDist = std::sqrtf(jdx * jdx + jdz * jdz);
			if (jumpDist > CHAR_SWITCH_DIST) {
				bCharacterSwitched = true;
				// Reset transform scan cache&cooldown
				_cachedTransformAddr = 0;
				_transformScanCooldown = 0.f;
			}
		}
		_prevDesiredRootPos = desiredPos;
		_hasPrevDesiredPos = true;
	}

	// Scan for entity Transform via pointer indirection from the character object
	if (data.characterPtr != 0 && data.camera.valid) {
		const auto& rootPos = data.camera.desiredCameraRootPos;
		const float matchThreshold = 5.0f;

		// Invalidate cache (e.g. on character switch)
		if (data.characterPtr != _cachedTransformCharPtr) {
			_cachedTransformAddr = 0;
			_cachedTransformCharPtr = data.characterPtr;
			_transformScanCooldown = 0.f;
		}

		// Validate a Transform at a given absolute address
		auto tryReadTransformAt = [&](uintptr_t addr) -> bool {
			RE::Quaternion quat{};
			RE::Vector3 translate{};

			if (!SafeReadQuat(addr, quat)) return false;
			if (!SafeReadVec3(addr + 0x10, translate)) return false;

			if (!std::isfinite(quat.x) || !std::isfinite(quat.y) || !std::isfinite(quat.z) || !std::isfinite(quat.w)) return false;
			if (!std::isfinite(translate.x) || !std::isfinite(translate.y) || !std::isfinite(translate.z)) return false;

			float dx = translate.x - rootPos.x;
			float dy = translate.y - rootPos.y;
			float dz = translate.z - rootPos.z;
			float distSq = dx * dx + dy * dy + dz * dz;
			if (!(distSq <= matchThreshold * matchThreshold)) return false;

			float qLen = quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w;
			if (!(qLen >= 0.9f && qLen <= 1.1f)) return false;

			// negate the forward vector
			float fwdX = 2.0f * (quat.x * quat.z + quat.w * quat.y);
			float fwdZ = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);
			float heading = std::atan2f(-fwdX, -fwdZ);
			if (!std::isfinite(heading)) return false;

			data.hasEntityRotation = true;
			data.entityRotationQuat = quat;
			data.entityTranslate = translate;
			data.entityHeadingRad = heading;
			data.entityHeadingDeg = heading * RAD_TO_DEG;
			_entityHeadingRad = heading;
			_hasEntityHeading = true;
			return true;
		};

		bool found = false;

		// try cache first
		if (_cachedTransformAddr != 0) {
			found = tryReadTransformAt(_cachedTransformAddr);
			if (!found) {
				_cachedTransformAddr = 0;  // stale
			}
		}

		// Full scan
		if (!found) do {
			_transformScanCooldown -= data.deltaTime;
			if (_transformScanCooldown > 0.f) {
				break;
			}

			// scan a memory region for a Transform where translate matches rootPos
			auto scanForTransform = [&](uintptr_t baseAddr, int maxOffset) -> bool {
				for (int offset = 0; offset <= maxOffset - static_cast<int>(sizeof(RE::Transform)); offset += 4) {
					if (tryReadTransformAt(baseAddr + offset)) {
						data.entityTransformOffset = offset;
						_cachedTransformAddr = baseAddr + offset;
						return true;
					}
				}
				return false;
			};

			// Scan the character object directly
			found = scanForTransform(data.characterPtr, 0x800);

			// follow each pointer-sized value in the character object and scan sub-objects
			if (!found) {
				for (int ptrOff = 0; ptrOff <= 0x200; ptrOff += 8) {
					uintptr_t subPtr = 0;
					__try {
						subPtr = *reinterpret_cast<uintptr_t*>(data.characterPtr + ptrOff);
					} __except (EXCEPTION_EXECUTE_HANDLER) {
						continue;
					}
					// Basic pointer validity check
					if (subPtr < 0x10000 || subPtr == data.characterPtr) continue;
					if ((subPtr & 0x3) != 0) continue;

					found = scanForTransform(subPtr, 0x800);
					if (found) {
						data.entityTransformOffset += (ptrOff << 16);  // high 16 bits = ptr offset, low 16 = transform offset
						break;
					}
				}
			}

			// follow the pointer at characterPtr+0x10 (used by GetCharacterHeight)
			// then follow another pointer
			if (!found) {
				uintptr_t level1Ptr = 0;
				__try {
					level1Ptr = *reinterpret_cast<uintptr_t*>(data.characterPtr + 0x10);
				} __except (EXCEPTION_EXECUTE_HANDLER) {
					level1Ptr = 0;
				}
				if (level1Ptr > 0x10000 && level1Ptr != data.characterPtr) {
					for (int ptrOff = 0; ptrOff <= 0x200; ptrOff += 8) {
						uintptr_t subPtr = 0;
						__try {
							subPtr = *reinterpret_cast<uintptr_t*>(level1Ptr + ptrOff);
						} __except (EXCEPTION_EXECUTE_HANDLER) {
							continue;
						}
						if (subPtr < 0x10000 || subPtr == level1Ptr || subPtr == data.characterPtr) continue;
						if ((subPtr & 0x3) != 0) continue;

						found = scanForTransform(subPtr, 0x400);
						if (found) break;
					}
				}
			}

			// avoid re-scanning
			if (!found) {
				_transformScanCooldown = TRANSFORM_SCAN_RETRY_SEC;
			}
		} while (false);
	}

	// Handle character switch
	// Determine NPC vs player, seed heading, reset heading window.
	if (bCharacterSwitched) {
		_isTrackingNPC = !data.hasEntityRotation;
		if (data.hasEntityRotation) {
			_derivedHeadingRad = data.entityHeadingRad;
			_hasHeading = true;
		} else {
			// NPC: don't seed heading; requires movement
			// TODO: Maybe find NPC entity someday
			_hasHeading = false;
		}
		// Reset heading window
		_windowStartPos = data.camera.desiredCameraRootPos;
		_windowElapsed = 0.f;
		_hasWindowStart = true;
	}

	// Derive heading
	if (data.camera.valid && data.deltaTime > 0.f) {
		const auto& pos = data.camera.desiredCameraRootPos;

		if (!_hasWindowStart) {
			_windowStartPos = pos;
			_windowElapsed = 0.f;
			_hasWindowStart = true;
			// Seed entity heading
			if (data.hasEntityRotation) {
				_derivedHeadingRad = data.entityHeadingRad;
				_hasHeading = true;
			}
		}

		_windowElapsed += data.deltaTime;

		// Evaluate heading
		if (_windowElapsed >= HEADING_WINDOW_SEC) {
			const float dx = pos.x - _windowStartPos.x;
			const float dz = pos.z - _windowStartPos.z;
			const float distXZ = std::sqrtf(dx * dx + dz * dz);
			const float speed = distXZ / _windowElapsed;

			data.movementDelta = { dx, pos.y - _windowStartPos.y, dz };
			data.movementSpeed = speed;

			// filter camera orbit jitter
			if (speed > HEADING_MIN_SPEED) {
				_derivedHeadingRad = std::atan2f(dx, dz);
				_hasHeading = true;
			}
			_windowStartPos = pos;
			_windowElapsed = 0.f;
		}

		data.derivedHeadingRad = _derivedHeadingRad;
		data.derivedHeadingDeg = _derivedHeadingRad * RAD_TO_DEG;
		data.hasHeading = _hasHeading;

		// Compute camera angle (Player tracking)
		if (data.hasEntityRotation && _currentCamera) {
			const auto& camRot = data.camera.cameraRotation;
			float currentCameraWorldHeading = std::atan2f(camRot.x, camRot.z);
			float desiredCameraWorldHeading = data.entityHeadingRad + PI;
			float deltaRad = desiredCameraWorldHeading - currentCameraWorldHeading;
			while (deltaRad > PI)  deltaRad -= TWO_PI;
			while (deltaRad < -PI) deltaRad += TWO_PI;
			data.behindEntityAngleDeg = data.camera.cameraAngle + deltaRad * RAD_TO_DEG;
			data.hasBehindEntityAngle = true;
		}

		// Compute camera angle (NPC tracking)
		if (_hasHeading && _currentCamera) {
			const auto& camRot = data.camera.cameraRotation;
			float currentCameraWorldHeading = std::atan2f(camRot.x, camRot.z);
			float desiredCameraWorldHeading = _derivedHeadingRad + PI;
			float deltaRad = desiredCameraWorldHeading - currentCameraWorldHeading;
			while (deltaRad > PI)  deltaRad -= TWO_PI;
			while (deltaRad < -PI) deltaRad += TWO_PI;
			data.behindMovementAngleDeg = data.camera.cameraAngle + deltaRad * RAD_TO_DEG;
			data.hasBehindMovementAngle = true;
		}

		data.isTrackingNPC = _isTrackingNPC;

		// align cam behind entity on switch
		if (bCharacterSwitched && !_isTrackingNPC && data.hasBehindEntityAngle && _currentCamera) {
			if (IsAlignCameraBehindOnSwitchEnabled(data.camera.cameraModeFlags)) {
				_currentCamera->angle = data.behindEntityAngleDeg;
			}
		}

		// align cam behind NPC
		if (_isTrackingNPC && data.hasBehindMovementAngle && _currentCamera) {
			if (IsAlignCameraBehindNPCEnabled(data.camera.cameraModeFlags)) {
				float angleDelta = data.behindMovementAngleDeg - _currentCamera->angle;
				while (angleDelta > 180.f)  angleDelta -= 360.f;
				while (angleDelta < -180.f) angleDelta += 360.f;
				float interpFactor = std::min(data.deltaTime * NPC_TRACK_INTERP_SPEED, 1.f);
				_currentCamera->angle += angleDelta * interpFactor;
			}
		}
	}

	_cachedDebugData = data;
}

bool CameraTweaks::SafeReadFloat(uintptr_t addr, float& outVal)
{
	__try {
		outVal = *reinterpret_cast<const float*>(addr);
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool CameraTweaks::SafeReadVec3(uintptr_t addr, RE::Vector3& outVal)
{
	__try {
		outVal = *reinterpret_cast<const RE::Vector3*>(addr);
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool CameraTweaks::SafeReadQuat(uintptr_t addr, RE::Quaternion& outVal)
{
	__try {
		outVal = *reinterpret_cast<const RE::Quaternion*>(addr);
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool CameraTweaks::SafeReadPtr(uintptr_t addr, uintptr_t& outVal)
{
	__try {
		outVal = *reinterpret_cast<const uintptr_t*>(addr);
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

// Helper: check a single 0x28-byte Transform at addr against searchPos
static bool CheckTransformAt(uintptr_t addr, const RE::Vector3& searchPos, float threshold,
	RE::Quaternion& outQuat, RE::Vector3& outTranslate, RE::Vector3& outScale)
{
	__try {
		const auto* quat = reinterpret_cast<const RE::Quaternion*>(addr);
		const auto* translate = reinterpret_cast<const RE::Vector3*>(addr + 0x10);
		const auto* scale = reinterpret_cast<const RE::Vector3*>(addr + 0x1C);

		// All components must be finite
		if (!std::isfinite(quat->x) || !std::isfinite(quat->y) || !std::isfinite(quat->z) || !std::isfinite(quat->w)) return false;
		if (!std::isfinite(translate->x) || !std::isfinite(translate->y) || !std::isfinite(translate->z)) return false;

		float qLen = quat->x * quat->x + quat->y * quat->y + quat->z * quat->z + quat->w * quat->w;
		if (!(qLen >= 0.9f && qLen <= 1.1f)) return false;

		float dx = translate->x - searchPos.x;
		float dy = translate->y - searchPos.y;
		float dz = translate->z - searchPos.z;
		float distSq = dx * dx + dy * dy + dz * dz;
		if (!(distSq <= threshold * threshold)) return false;

		// Scale should be reasonable (positive, finite, not zero)
		if (!std::isfinite(scale->x) || !std::isfinite(scale->y) || !std::isfinite(scale->z)) return false;
		if (scale->x <= 0.001f || scale->y <= 0.001f || scale->z <= 0.001f) return false;

		outQuat = *quat;
		outTranslate = *translate;
		outScale = *scale;
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

void CameraTweaks::RunMemoryScan(const RE::Vector3& searchPos)
{
	_memoryScanRunning.store(true);
	_memoryScanSearchPos = searchPos;
	_memoryScanResults.clear();

	const float threshold = 2.0f;

	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);
	uintptr_t regionStart = reinterpret_cast<uintptr_t>(sysInfo.lpMinimumApplicationAddress);
	uintptr_t regionEnd = reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress);

	MEMORY_BASIC_INFORMATION mbi{};
	uintptr_t addr = regionStart;

	while (addr < regionEnd && VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
		uintptr_t regionBase = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
		uintptr_t regionSize = mbi.RegionSize;
		uintptr_t nextAddr = regionBase + regionSize;

		// Only scan reasonable scopes
		bool isReadable = (mbi.State == MEM_COMMIT) &&
			(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) &&
			!(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS));

		// Skip image sections
		bool isPrivateOrMapped = (mbi.Type == MEM_PRIVATE || mbi.Type == MEM_MAPPED);

		if (isReadable && isPrivateOrMapped && regionSize >= sizeof(RE::Transform)) {
			// Scan for Transform structs
			uintptr_t scanEnd = regionBase + regionSize - sizeof(RE::Transform);

			for (uintptr_t scanAddr = regionBase; scanAddr <= scanEnd; scanAddr += 4) {
				RE::Quaternion quat{};
				RE::Vector3 translate{};
				RE::Vector3 scale{};

				if (CheckTransformAt(scanAddr, searchPos, threshold, quat, translate, scale)) {
					float qLen = quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w;
					float dx = translate.x - searchPos.x;
					float dy = translate.y - searchPos.y;
					float dz = translate.z - searchPos.z;
					float dist = sqrtf(dx * dx + dy * dy + dz * dz);

					// negate forward vector
					float fwdX = 2.0f * (quat.x * quat.z + quat.w * quat.y);
					float fwdZ = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);
					float headingRad = std::atan2f(-fwdX, -fwdZ);
					float headingDeg = headingRad * RAD_TO_DEG;

					TransformScanResult result;
					result.address = scanAddr;
					result.rotationQuat = quat;
					result.translate = translate;
					result.scale = scale;
					result.quatLength = sqrtf(qLen);
					result.posDistance = dist;
					result.headingDeg = headingDeg;

					_memoryScanResults.push_back(result);

					if (_memoryScanResults.size() >= 200) goto scanDone;
				}
			}
		}

		addr = nextAddr;
	}

scanDone:
	INFO("Memory scan complete: {} transform(s) found matching position ({:.1f}, {:.1f}, {:.1f})",
		_memoryScanResults.size(), searchPos.x, searchPos.y, searchPos.z);
	_memoryScanRunning.store(false);
}
