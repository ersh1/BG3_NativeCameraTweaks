#include "Hooks.h"

#include "CameraTweaksManager.h"
#include "Settings.h"

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

		_UpdateCamera(a1, a2, a3, a4);
	}

	void* Hooks::Hook_SetDesiredCameraValue(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uintptr_t a4)
    {
		const int32_t inputId = *reinterpret_cast<int32_t*>(a4);
		const bool bIsInControllerMode = *Offsets::bIsInControllerMode;

		auto* settings = Settings::Main::GetSingleton();
		
		if (inputId == 107 || inputId == 108) {  // zoom in and out
			const auto cameraObject = Offsets::GetCameraObject(a3);
			if (bIsInControllerMode) {
			    const auto playerId = a3->currentPlayer_60->playerId_38;

				const auto cameraTweaks = CameraTweaks::GetSingleton();

				const auto playerController = Offsets::GetPlayerController(*Offsets::UnkPlayerSingletonPtr, playerId);
				int32_t toggleInputId = 0xB2;  // left stick click
				const bool bIsToggleInputModePressed = Offsets::GetInputPressed(*Offsets::UnkInputSingletonPtr, toggleInputId, playerController);
				if (bIsToggleInputModePressed) {
					// let it do the original function (zoom)
					
					cameraTweaks->SetControllerPitchDelta(playerId, 0.f);  // clear the delta

					float* pInputValue = reinterpret_cast<float*>(a4 + 0x18);

					if (*pInputValue > CameraTweaks::VANILLA_DEADZONE) {
						// if we're actually zooming, skip the next toggle input mode press
						cameraTweaks->SetSkipToggleInputMode(playerId, true);
					}
				} else {
					// do pitch instead of zoom
					cameraObject->zoomDelta_A4 = 0.f;  // set zoom delta to 0 in case we were just zooming with the controller and then stopped pressing the stick

					float* pInputValue = reinterpret_cast<float*>(a4 + 0x18);
					const float sign = inputId == 107 ? -1.f : 1.f;

					cameraTweaks->SetControllerPitchDelta(playerId, *pInputValue * sign);

					*pInputValue = 0.f;                             // set input value to 0 so that the game doesn't do anything with it
					return _SetDesiredCameraValue(a1, a2, a3, a4);  // call original to preserve hooking compatibility
				}
			}

			// slower zoom
			auto ret = _SetDesiredCameraValue(a1, a2, a3, a4);
			cameraObject->zoomDelta_A4 *= bIsInControllerMode ? *settings->ControllerZoomMult : *settings->MouseZoomMult;
			return ret;
		}

		if (inputId == 110 || inputId == 111) {  // rotate left and right
			float* pInputValue = reinterpret_cast<float*>(a4 + 0x18);
			if (bIsInControllerMode) {
				// adjust deadzone + add mult from settings
				*pInputValue = CameraTweaks::GetSingleton()->AdjustInputValueForDeadzone(*pInputValue);
			} else {
				// add mult from settings for keyboard rotation
				const auto cameraObject = Offsets::GetCameraObject(a3);
				auto ret = _SetDesiredCameraValue(a1, a2, a3, a4);
				cameraObject->currentAngleDelta_9C  *= *settings->KeyboardCameraRotationMult;  // apply mult
				return ret;
			}
		}

		if (inputId == 112 || inputId == 113) {  // mouse rotate left and right
			float* pInputValue = reinterpret_cast<float*>(a4 + 0x14);
			*pInputValue *= *settings->MouseCameraRotationMult;
		}

		// call original
		return _SetDesiredCameraValue(a1, a2, a3, a4);
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

    void Hooks::Hook_UpdateCameraPitch(uint64_t a1, RE::UnkObject* a2, RE::CameraObject* a_cameraObject, uint64_t a4)
	{
		const float deltaTime = *reinterpret_cast<float*>(a4 + 0x8);
		auto cameraTweaks = CameraTweaks::GetSingleton();
		cameraTweaks->SetDeltaTime(deltaTime);

		const auto playerId = a2->currentPlayer_60->playerId_38;

		if (cameraTweaks->IsCameraUnlocked(playerId, a_cameraObject)) {
			const auto cameraDefinition = Offsets::GetCurrentCameraDefinition(a_cameraObject->cameraModeFlags_A8);
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

    void Hooks::Hook_UpdateCameraZoom(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uint64_t a4)
	{
	    _UpdateCameraZoom(a1, a2, a3, a4);

		if (*Settings::Main::GetSingleton()->UnlockedPitchLimitClipping) {
			const auto pUnkObject = Offsets::GetUnkPlayerObject(a3);
			const auto character = Offsets::GetCharacter(*reinterpret_cast<uintptr_t*>(a1 + 0x130), pUnkObject->playerId_38);
			if (character) {
				const float characterHeight = Offsets::GetCharacterHeight(character);

				CameraTweaks::GetSingleton()->AdjustCameraZoomForPitch(Offsets::GetCameraObject(a3), characterHeight);
			}
		}
	}

    int16_t* Hooks::Hook_HandleToggleInputMode(uint64_t a1, int16_t& a_outResult, int32_t* a_inputId)
	{
		if (*a_inputId == 0xB2) {  // left stick click "ToggleInputMode"
			const auto cameraTweaks = CameraTweaks::GetSingleton();
			const auto playerId = *reinterpret_cast<int16_t*>(a1 + 0x168);
			if (cameraTweaks->ShouldSkipToggleInputMode(playerId)) {
				cameraTweaks->SetSkipToggleInputMode(playerId, false);
				a_outResult = 0;
				return &a_outResult;  // do not call original - block input
			}
		}

		return _HandleToggleInputMode(a1, a_outResult, a_inputId);
	}
}
