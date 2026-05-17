#include "Hooks.h"

#include "CameraTweaksManager.h"
#include "Settings.h"
#include "Utils.h"

namespace
{
	// TODO: Untested for controller. May be too aggressive.
	bool IsPanDetachInput(Hooks::Offsets::InputID a_inputId)
	{
		switch (a_inputId) {
		//case Hooks::Offsets::InputID::kPanInput2:
		case Hooks::Offsets::InputID::kPanInput17:
		case Hooks::Offsets::InputID::kPanInput18:
		case Hooks::Offsets::InputID::kPanInput19:
		case Hooks::Offsets::InputID::kPanInput20:
		case Hooks::Offsets::InputID::kPanInput21:
		case Hooks::Offsets::InputID::kPanInput22:
		//case Hooks::Offsets::InputID::kPanInput29:
		//case Hooks::Offsets::InputID::kPanInput30:
		//case Hooks::Offsets::InputID::kPanInput31:
		//case Hooks::Offsets::InputID::kPanInput32:
		//case Hooks::Offsets::InputID::kPanInput98:
		case Hooks::Offsets::InputID::kPanInput99:
		case Hooks::Offsets::InputID::kPanInput100:
		case Hooks::Offsets::InputID::kPanInput101:
		case Hooks::Offsets::InputID::kPanInput102:
		case Hooks::Offsets::InputID::kPanInput217:
			return true;
		default:
			return false;
		}
	}

	void SuppressCameraPan(RE::CameraObject* a_cameraObject)
	{
		if (!a_cameraObject) {
			return;
		}

		a_cameraObject->horizontalPanDelta = 0.f;
		a_cameraObject->verticalPanDelta = 0.f;
		a_cameraObject->desiredCameraRootPos = a_cameraObject->cameraRootPos;
	}

	void SuppressCameraPan(RE::UnkObject* a_cameraState)
	{
		if (!a_cameraState) {
			return;
		}

		SuppressCameraPan(a_cameraState->currentCameraObject);
		if (a_cameraState->currentCameraObject2 != a_cameraState->currentCameraObject) {
			SuppressCameraPan(a_cameraState->currentCameraObject2);
		}
	}

	void LogPotentialPanInput(Hooks::Offsets::InputID a_inputId, RE::UnkObject* a_cameraState)
	{
		static std::mutex s_loggedInputsLock;
		static std::unordered_set<int32_t> s_loggedInputs;

		const auto maybeLog = [&](RE::CameraObject* a_cameraObject) {
			if (!a_cameraObject) {
				return;
			}

			const bool bHasPanDelta = std::fabs(a_cameraObject->horizontalPanDelta) > 0.0001f || std::fabs(a_cameraObject->verticalPanDelta) > 0.0001f;
			const bool bHasDetachedRoot = std::fabs(a_cameraObject->desiredCameraRootPos.x - a_cameraObject->cameraRootPos.x) > 0.0001f ||
				std::fabs(a_cameraObject->desiredCameraRootPos.y - a_cameraObject->cameraRootPos.y) > 0.0001f ||
				std::fabs(a_cameraObject->desiredCameraRootPos.z - a_cameraObject->cameraRootPos.z) > 0.0001f;

			if (!bHasPanDelta && !bHasDetachedRoot) {
				return;
			}

			const auto rawInputId = static_cast<int32_t>(a_inputId);
			std::lock_guard lock(s_loggedInputsLock);
			if (s_loggedInputs.emplace(rawInputId).second) {
				INFO("Potential pan/detach input observed: id={} pan=({}, {}) rootDelta=({}, {}, {})",
					rawInputId,
					a_cameraObject->horizontalPanDelta,
					a_cameraObject->verticalPanDelta,
					a_cameraObject->desiredCameraRootPos.x - a_cameraObject->cameraRootPos.x,
					a_cameraObject->desiredCameraRootPos.y - a_cameraObject->cameraRootPos.y,
					a_cameraObject->desiredCameraRootPos.z - a_cameraObject->cameraRootPos.z);
			}
		};

		if (!a_cameraState) {
			return;
		}

		maybeLog(a_cameraState->currentCameraObject);
		if (a_cameraState->currentCameraObject2 != a_cameraState->currentCameraObject) {
			maybeLog(a_cameraState->currentCameraObject2);
		}
	}

	void* BlockPanDetachInput(RE::UnkObject* a_cameraState, uintptr_t a_inputData, Hooks::Offsets::InputID a_inputId)
	{
		static std::mutex s_loggedBlockedInputsLock;
		static std::unordered_set<int32_t> s_loggedBlockedInputs;
		static thread_local int16_t s_suppressedResult = 0;

		if (a_inputData != 0) {
			*reinterpret_cast<float*>(a_inputData + 0x14) = 0.f;
			*reinterpret_cast<float*>(a_inputData + 0x18) = 0.f;
		}

		SuppressCameraPan(a_cameraState);

		const auto rawInputId = static_cast<int32_t>(a_inputId);
		std::lock_guard lock(s_loggedBlockedInputsLock);
		if (s_loggedBlockedInputs.emplace(rawInputId).second) {
			INFO("Blocking pan input id {} before detach handling", rawInputId);
		}

		s_suppressedResult = 0;
		return &s_suppressedResult;
	}
}

namespace Hooks
{
	void Install()
	{
		Offsets::Init();
		Hooks::Hook();
	}

    void Hooks::Hook_UpdateCamera(uint64_t a1, uint64_t a2, uint64_t a3, RE::UnkObject* a4)
	{
		CameraTweaks::GetSingleton()->SetCameraSettings();

		const auto cameraTweaks = CameraTweaks::GetSingleton();
		cameraTweaks->SetCurrentPlayer(a4->currentPlayer);
		cameraTweaks->SetCurrentCamera(a4->currentCameraObject);

		const auto settings = Settings::Main::GetSingleton();
		const bool bInCombat = a4->currentCameraObject && (a4->currentCameraObject->cameraModeFlags & RE::CameraModeFlags::kCombat);
		const bool bDisablePan = bInCombat ? *settings->CombatDisableCameraPan : *settings->ExplorationDisableCameraPan;
		if (bDisablePan) {
			SuppressCameraPan(a4);
		}

		_UpdateCamera(a1, a2, a3, a4);
		if (bDisablePan) {
			SuppressCameraPan(a4);
		}

		cameraTweaks->UpdateDebugData();
	}

	void* Hooks::Hook_HandleCameraInput(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uintptr_t a4)
    {
		const Offsets::InputID inputId = *reinterpret_cast<Offsets::InputID*>(a4);
		const bool bIsInControllerMode = *Offsets::bIsInControllerMode;

		const bool bInCombatInput = a3->currentCameraObject2 && (a3->currentCameraObject2->cameraModeFlags & RE::CameraModeFlags::kCombat);
		const bool bDisablePanInput = bInCombatInput ? *Settings::Main::GetSingleton()->CombatDisableCameraPan : *Settings::Main::GetSingleton()->ExplorationDisableCameraPan;
		if (bDisablePanInput && IsPanDetachInput(inputId)) {
			return BlockPanDetachInput(a3, a4, inputId);
		}

		auto* settings = Settings::Main::GetSingleton();

		switch (inputId) {
		case Offsets::InputID::kZoomIn:
		case Offsets::InputID::kZoomOut:  // zoom in and out
			{
				ReadLocker locker(settings->Lock);

				const auto cameraObject = a3->currentCameraObject2;
				if (bIsInControllerMode) {
					const auto playerId = Utils::GetPlayerID(a3);
					float* pInputValue = reinterpret_cast<float*>(a4 + 0x18);  // RAWOFFSET

					const auto cameraTweaks = CameraTweaks::GetSingleton();

					bool bDoZoom;
					bool bCanAdjustPitch = cameraTweaks->CanAdjustPitch(cameraObject);

					if (!bCanAdjustPitch) {
						bDoZoom = true;
					} else if (*settings->UseRightStickPressForZoom) {
						bDoZoom = Offsets::ShouldShowSneakCones(*Offsets::UnkSingletonPtr, playerId);
					} else {
						const auto playerController = Offsets::GetPlayerController(*Offsets::UnkPlayerSingletonPtr, playerId);
						auto toggleInputId = Offsets::InputID::kToggleInputMode;  // (ToggleInputMode - by default left stick click)
						RE::InputValue inputValue;
						Offsets::GetInputValue(*Offsets::UnkInputSingletonPtr, inputValue, toggleInputId, playerController);
						bDoZoom = inputValue.bIsPressed;
					}

					if (bCanAdjustPitch && *settings->SwapZoomAndPitch) {
					    bDoZoom = !bDoZoom;
					}

					const bool bShouldSkipToggleInputMode = *settings->SwapZoomAndPitch ? !bDoZoom : bDoZoom;
					if (bCanAdjustPitch && bShouldSkipToggleInputMode && !*settings->UseRightStickPressForZoom && *pInputValue > CameraTweaks::VANILLA_DEADZONE) {
						// if we're actually zooming, and doing this with the ToggleInputMode held, skip the next ToggleInputMode press
						cameraTweaks->SetSkipToggleInputMode(playerId, true);
					}

					if (bDoZoom) {
						// let it do the original function (zoom)
						cameraTweaks->SetControllerPitchDelta(playerId, 0.f);  // clear the delta
					} else {
						// do pitch instead of zoom
						cameraObject->zoomDelta = 0.f;  // set zoom delta to 0 in case we were just zooming with the controller and then stopped pressing the stick

						const float sign = inputId == Offsets::InputID::kZoomIn ? -1.f : 1.f;

						cameraTweaks->SetControllerPitchDelta(playerId, *pInputValue * sign);

						*pInputValue = 0.f;                         // set input value to 0 so that the game doesn't do anything with it
						return _HandleCameraInput(a1, a2, a3, a4);  // call original to preserve hooking compatibility
					}
				}

				// slower zoom
				auto ret = _HandleCameraInput(a1, a2, a3, a4);
				cameraObject->zoomDelta *= bIsInControllerMode ? *settings->ControllerZoomMult : *settings->MouseZoomMult;
				return ret;
			}
		case Offsets::InputID::kRotateLeft:
		case Offsets::InputID::kRotateRight:  // rotate left and right
	        {
				float* pInputValue = reinterpret_cast<float*>(a4 + 0x18);  // RAWOFFSET
				if (bIsInControllerMode) {
					// adjust deadzone + add mult from settings
					*pInputValue = CameraTweaks::GetSingleton()->AdjustInputValueForDeadzone(*pInputValue);
				} else {
					// add mult from settings for keyboard rotation
					const auto cameraObject = a3->currentCameraObject2;
					auto ret = _HandleCameraInput(a1, a2, a3, a4);
					ReadLocker locker(settings->Lock);
					cameraObject->currentAngleDelta *= *settings->KeyboardCameraRotationMult;  // apply mult
					return ret;
				}
			    break;
	        }
		case Offsets::InputID::kMouseRotateLeft:
		case Offsets::InputID::kMouseRotateRight:  // mouse rotate left and right
		    {
				ReadLocker locker(settings->Lock);
				float* pInputValue = reinterpret_cast<float*>(a4 + 0x14);  // RAWOFFSET
				*pInputValue *= *settings->MouseCameraRotationMult;
			    break;
		    }
		}

		// call original
		auto* result = _HandleCameraInput(a1, a2, a3, a4);
		if (bDisablePanInput) {
			LogPotentialPanInput(inputId, a3);
			SuppressCameraPan(a3);
		}
		return result;
    }

    float Hooks::Hook_CalculateCameraPitch(RE::CameraObject* a_cameraObject, uint8_t a2, uint8_t a3)
	{
		float pitch = _CalculateCameraPitch(a_cameraObject, a2, a3);

		auto cameraTweaks = CameraTweaks::GetSingleton();
		auto playerId = cameraTweaks->GetPlayerIdFromCameraObject(a_cameraObject);
		if (playerId == 0) {  // should never happen
		    return pitch;
		}

		cameraTweaks->CalculateCameraPitch(playerId, a_cameraObject, pitch);

		return pitch;
	}

    void Hooks::Hook_UpdateCameraPitch(uint64_t a1, uint64_t a2, RE::CameraObject* a_cameraObject, uint64_t a4)
	{
		const float deltaTime = *reinterpret_cast<float*>(a4 + 0x8);  // RAWOFFSET
		auto cameraTweaks = CameraTweaks::GetSingleton();
		cameraTweaks->SetDeltaTime(deltaTime);

		auto currentPlayer = cameraTweaks->GetCurrentPlayer();
		const auto playerId = currentPlayer ? currentPlayer->playerId_38 : 1;

		if (cameraTweaks->IsCameraUnlocked(playerId, a_cameraObject)) {
			const auto cameraDefinition = Offsets::GetCurrentCameraDefinition(a_cameraObject);
			const float originalPitchAdjustSpeedA = cameraDefinition->pitchAdjustSpeedA_48;
			const float originalPitchAdjustSpeedB = cameraDefinition->pitchAdjustSpeedB_F0;
			const float originalPitchAdjustSpeedC = cameraDefinition->pitchAdjustSpeedC_F4;

			cameraDefinition->pitchAdjustSpeedA_48 = 100000.f;
			cameraDefinition->pitchAdjustSpeedB_F0 = 100000.f;
			cameraDefinition->pitchAdjustSpeedC_F4 = 100000.f;

			cameraTweaks->SetCameraObjectForPlayer(playerId, a_cameraObject);
			_UpdateCameraPitch(a1, a2, a_cameraObject, a4);

			cameraDefinition->pitchAdjustSpeedA_48 = originalPitchAdjustSpeedA;
			cameraDefinition->pitchAdjustSpeedB_F0 = originalPitchAdjustSpeedB;
			cameraDefinition->pitchAdjustSpeedC_F4 = originalPitchAdjustSpeedC;
		} else {
			_UpdateCameraPitch(a1, a2, a_cameraObject, a4);
		}
	}

	void Hooks::Hook_AfterUpdateCameraZoom(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uint64_t a4)
	{
		_AfterUpdateCameraZoom(a1, a2, a3, a4);

		const auto settings = Settings::Main::GetSingleton();
		ReadLocker locker(settings->Lock);
		if (*settings->UnlockedPitchLimitClipping) {
			const auto cameraTweaks = CameraTweaks::GetSingleton();
			CameraTweaks::GetSingleton()->AdjustCameraZoomForPitch(a1, a2, cameraTweaks->GetCurrentCamera());
		}
	}

    int16_t* Hooks::Hook_HandleToggleInputMode(uint64_t a1, int16_t& a_outResult, Offsets::InputID* a_inputId)
	{
		if (*a_inputId == Offsets::InputID::kToggleInputMode) {  // left stick click "ToggleInputMode"
			const auto cameraTweaks = CameraTweaks::GetSingleton();
			const auto playerId = *reinterpret_cast<int16_t*>(a1 + 0x168);  // RAWOFFSET
			if (cameraTweaks->ShouldSkipToggleInputMode(playerId)) {
				cameraTweaks->SetSkipToggleInputMode(playerId, false);
				a_outResult = 0;
				return &a_outResult;  // do not call original - block input
			}
		}

		return _HandleToggleInputMode(a1, a_outResult, a_inputId);
	}

    void Hooks::Hook_SetDefaultZoom(RE::CameraObject* a_cameraObject)
	{
		float desiredZoom = a_cameraObject->desiredZoom;

		_SetDefaultZoom(a_cameraObject);

		const auto settings = Settings::Main::GetSingleton();
		ReadLocker locker(settings->Lock);
		if (!*settings->ResetZoomOnZoneChange) {
			a_cameraObject->desiredZoom = desiredZoom;
			a_cameraObject->currentZoom_160 = desiredZoom;
		}
	}

    bool Hooks::Hook_SDLMouseYHook(uint64_t a1, uint64_t a2, bool a3, int a_deltaY)
	{
		CameraTweaks::GetSingleton()->delta_y = a_deltaY;

		return _SDLMouseYHook(a1, a2, a3, a_deltaY);
	}
}
