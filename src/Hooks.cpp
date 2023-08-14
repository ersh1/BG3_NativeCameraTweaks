#include "Hooks.h"
#include "Settings.h"

namespace Hooks
{
	void Install()
	{
		Offsets::Init();
		Hooks::Hook();
	}

	static constexpr float VANILLA_DEADZONE = 0.65f;
	static constexpr float NORMALIZE_DEADZONE = 1.f / (1.f - VANILLA_DEADZONE);

	static POINT lastCursorPos = { 0, 0 };
	static float pitchOffset = 25.f;  // initial value
	static float controllerPitchDelta = 0.f;
	static float lastDeltaTime = 0.f;

	float Denormalize(float a_min, float a_max, float a_value)
	{
	    float normalizedRange = a_max - a_min;
		float scaledValue = a_value * normalizedRange;
		return scaledValue + a_min;
	}

	float GetDeadzone()
	{
		auto* settings = Settings::Main::GetSingleton();

		return *settings->OverrideRightStickDeadzone ? *settings->NewDeadzone : VANILLA_DEADZONE;
	}

	void* Hooks::Hook_SetDesiredCameraValue(uint64_t a1, uint64_t a2, uint64_t a3, uintptr_t a4)
    {
		const int32_t inputId = *reinterpret_cast<int32_t*>(a4);
		const bool bIsInControllerMode = *Offsets::bIsInControllerMode;

		auto* settings = Settings::Main::GetSingleton();

		if (bIsInControllerMode && (inputId == 110 || inputId == 111)) {  // rotate left and right
		    // adjust deadzone
			float* pInputValue = reinterpret_cast<float*>(a4 + 0x18);

			const float deadzone = GetDeadzone();

			if (*pInputValue > deadzone) {
				float normalizedValue = (*pInputValue - deadzone) * (1.f / (1.f - deadzone));
				normalizedValue *= *settings->ControllerRightStickMult;  // apply multiplier from settings
				const float adjustedValue = Denormalize(VANILLA_DEADZONE, 1.f, normalizedValue);
				*pInputValue = adjustedValue;
			}
		}

		if (inputId == 107 || inputId == 108) {
			const auto cameraObject = Offsets::GetCameraObject(a3);
			if (bIsInControllerMode) {
				if (!Offsets::ShouldShowSneakCones(*Offsets::UnkSingletonPtr, 1)) {
				    cameraObject->zoomDelta_A4 = 0.f;  // set zoom delta to 0 in case we were zooming with controller and then stopped pressing the stick

					// do pitch instead
					const float inputValue = *reinterpret_cast<float*>(a4 + 0x18);

					const float deadzone = GetDeadzone();
					const float normalizeDeadzone = *settings->OverrideRightStickDeadzone ? 1.f / (1.f - *settings->NewDeadzone) : NORMALIZE_DEADZONE;

					if (inputValue <= deadzone) {  // deadzone
						controllerPitchDelta = 0.f;
					} else {
						const float value = (inputValue - deadzone) * normalizeDeadzone;  // normalize outside of deadzone
						float sign = inputId == 107 ? -1.f : 1.f;
						if (*settings->InvertControllerPitch) {
						    sign *= -1.f;
						}
						controllerPitchDelta = value * sign * *settings->ControllerRightStickMult;
					}

					return reinterpret_cast<void*>(a2);
				} else {
					controllerPitchDelta = 0.f;
				}
			}

			// slower zoom
			auto ret = _SetDesiredCameraValue(a1, a2, a3, a4);

			cameraObject->zoomDelta_A4 *= bIsInControllerMode ? *settings->ControllerZoomMult : *settings->MouseZoomMult;

			return ret;
		}

		// call original
		return _SetDesiredCameraValue(a1, a2, a3, a4);
    }

    float Hooks::Hook_CalculateCameraPitch(RE::CameraObject* a_cameraObject, uint8_t a2, uint8_t a3)
	{
		// completely replace the original function
		//return _CalculateCameraPitch(a_cameraObject, a2, a3);

		int32_t deltaY = 0;
		POINT cursorPos = { 0, 0 };
		if (GetCursorPos(&cursorPos)) {
			if ((a_cameraObject->cameraModeFlags_A8 & 0x100) != 0) {  // mouse rotation mode
				deltaY = cursorPos.y - lastCursorPos.y;
			}

			lastCursorPos = cursorPos;
		}

		auto* settings = Settings::Main::GetSingleton();
		
		pitchOffset += deltaY * *settings->MousePitchMult;
		pitchOffset += controllerPitchDelta * lastDeltaTime * a_cameraObject->rotationSpeed_C0 * *settings->ControllerPitchMult;
		pitchOffset = std::clamp(pitchOffset, static_cast<float>(*settings->PitchMin), static_cast<float>(*settings->PitchMax));

		return pitchOffset;
	}

    void Hooks::Hook_UpdateCameraPitch(uint64_t a1, uint64_t a2, RE::CameraObject* a_cameraObject, uint64_t a4)
	{
		lastDeltaTime = *reinterpret_cast<float*>(a4 + 0x8);

		const auto cameraDefinition = Offsets::GetCurrentCameraDefinition(a_cameraObject);
		const float originalPitchAdjustSpeedA = cameraDefinition->pitchAdjustSpeedA_48;
		const float originalPitchAdjustSpeedB = cameraDefinition->pitchAdjustSpeedB_F0;
		const float originalPitchAdjustSpeedC = cameraDefinition->pitchAdjustSpeedC_F4;

		cameraDefinition->pitchAdjustSpeedA_48 = 100000.f;
		cameraDefinition->pitchAdjustSpeedB_F0 = 100000.f;
		cameraDefinition->pitchAdjustSpeedC_F4 = 100000.f;

		_UpdateCameraPitch(a1, a2, a_cameraObject, a4);

		cameraDefinition->pitchAdjustSpeedA_48 = originalPitchAdjustSpeedA;
		cameraDefinition->pitchAdjustSpeedB_F0 = originalPitchAdjustSpeedB;
		cameraDefinition->pitchAdjustSpeedC_F4 = originalPitchAdjustSpeedC;
	}

    float Hooks::OverrideCameraMinZoom()
	{
		auto* settings = Settings::Main::GetSingleton();
	    return *settings->ZoomMin;
	}

    float Hooks::OverrideCameraMaxZoom()
	{
		auto* settings = Settings::Main::GetSingleton();
		return *settings->ZoomMax;
	}
}
