#pragma once
#include "RE/Camera.h"

namespace Hooks
{
	using namespace DKUtil::Alias;

	class Offsets
	{
	public:
		static void Init()
		{
			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 41 83 BD ?? ?? ?? ?? ?? 76 1C">());
				if (!scan) {
					FATAL("GetCameraObject not found!")
				}
				auto callsiteOffset = *reinterpret_cast<int32_t*>(scan + 1);
				auto callsite = scan + 5 + callsiteOffset + 0xAA;
				auto offset = *reinterpret_cast<int32_t*>(callsite + 1);
				GetCameraObject = reinterpret_cast<tGetCameraObject>(callsite + 5 + offset);
				INFO("GetCameraObject found: {:X}", AsAddress(GetCameraObject))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? ?? 74 1D">());
				if (!scan) {
					FATAL("GetCurrentCameraDefinition not found!")
				}
				auto offset = *reinterpret_cast<int32_t*>(scan + 1);
				GetCurrentCameraDefinition = reinterpret_cast<tGetCurrentCameraDefinition>(scan + 5 + offset);
				INFO("GetCurrentCameraDefinition found: {:X}", AsAddress(GetCurrentCameraDefinition))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 44 0F B6 F0 4D 85 ED">());
				if (!scan) {
					FATAL("Input related functions not found!")
				}
				auto unkSingletonOffset = *reinterpret_cast<int32_t*>(scan + 3);
				auto funcOffset = *reinterpret_cast<int32_t*>(scan + 8);
				UnkSingletonPtr = reinterpret_cast<void**>(scan + 7 + unkSingletonOffset);
				ShouldShowSneakCones = reinterpret_cast<tShouldShowSneakCones>(scan + 0xC + funcOffset);

				auto playerSingletonOffset = *reinterpret_cast<int32_t*>(AsAddress(ShouldShowSneakCones) + 0x46 + 3);
				UnkPlayerSingletonPtr = reinterpret_cast<void**>(AsAddress(ShouldShowSneakCones) + 0x46 + 7 + playerSingletonOffset);

				auto getPlayerControllerCallsite = AsAddress(ShouldShowSneakCones) + 0x51;
				auto getPlayerControllerOffset = *reinterpret_cast<int32_t*>(getPlayerControllerCallsite + 1);
				GetPlayerController = reinterpret_cast<tGetPlayerController>(getPlayerControllerCallsite + 5 + getPlayerControllerOffset);

				auto inputSingletonOffset = *reinterpret_cast<int32_t*>(AsAddress(ShouldShowSneakCones) + 0x56 + 3);
				UnkInputSingletonPtr = reinterpret_cast<void**>(AsAddress(ShouldShowSneakCones) + 0x56 + 7 + inputSingletonOffset);

				auto getInputPressedCallsite = AsAddress(ShouldShowSneakCones) + 0x6E;
				auto getInputPressedOffset = *reinterpret_cast<int32_t*>(getInputPressedCallsite + 1);
				GetInputPressed = reinterpret_cast<tGetInputPressed>(getInputPressedCallsite + 5 + getInputPressedOffset);

				auto getInputValueCallsite = AsAddress(GetInputPressed) + 0x10;
				auto getInputValueOffset = *reinterpret_cast<int32_t*>(getInputValueCallsite + 1);
				GetInputValue = reinterpret_cast<tGetInputValue>(getInputValueCallsite + 5 + getInputValueOffset);

				INFO("Input related functions found: {:X}", AsAddress(ShouldShowSneakCones))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? ?? 74 1D">());
				if (!scan) {
					FATAL("CameraSingleton not found!")
				}
				auto offset = *reinterpret_cast<int32_t*>(scan + 1);
				auto function = scan + 5 + offset;
				auto singletonOffset = *reinterpret_cast<int32_t*>(function + 3);
				UnkCameraSingletonPtr = reinterpret_cast<void**>(function + 7 + singletonOffset);
				INFO("CameraSingleton found: {:X}", AsAddress(UnkCameraSingletonPtr))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"80 3D ?? ?? ?? ?? ?? 74 22">());
				if (!scan) {
					FATAL("bIsInControllerMode not found!")
				}
				auto offset = *reinterpret_cast<int32_t*>(scan + 2);
				bIsInControllerMode = reinterpret_cast<bool*>(scan + 7 + offset);
				INFO("bIsInControllerMode found: {:X}", AsAddress(bIsInControllerMode))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 50 38">());
				if (!scan) {
					FATAL("GetUnkPlayerObject not found!")
				}
				auto offset = *reinterpret_cast<int32_t*>(scan + 1);
				GetUnkPlayerObject = reinterpret_cast<tGetUnkPlayerObject>(scan + 5 + offset);
				INFO("GetUnkPlayerObject found: {:X}", AsAddress(GetUnkPlayerObject))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 80 7B 4D 00">());
				if (!scan) {
					FATAL("GetCharacter not found!")
				}
				auto offset = *reinterpret_cast<int32_t*>(scan + 1);
				GetCharacter = reinterpret_cast<tGetCharacter>(scan + 5 + offset);
				INFO("GetCharacter found: {:X}", AsAddress(GetCharacter))
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 49 8B 1F F3 0F 10 4F ??">());
				if (!scan) {
					FATAL("GetCharacterHeight not found!")
				}
				auto offset = *reinterpret_cast<int32_t*>(scan + 1);
				GetCharacterHeight = reinterpret_cast<tGetCharacterHeight>(scan + 5 + offset);
				INFO("GetCharacterHeight found: {:X}", AsAddress(GetCharacterHeight))
			}
		}

		using tGetCameraObject = RE::CameraObject* (*)(RE::UnkObject* a1);
		using tGetCurrentCameraDefinition = RE::CameraDefinition* (*)(uint32_t a_cameraModeFlags);
		using tShouldShowSneakCones = bool (*)(void* a1, int16_t a_playerId);
		using tGetUnkPlayerObject = RE::Player* (*)(RE::UnkObject* a1);
		using tGetCharacter = uintptr_t (*)(uintptr_t a1, int16_t a_playerId);
		using tGetCharacterHeight = float (*)(uintptr_t a_character);
		using tGetPlayerController = void* (*)(void* a1, int16_t a_playerId);
		using tGetInputPressed = bool (*)(void* a1, int32_t& a_inputId, void* a3);
		using tGetInputValue = RE::InputValue& (*)(void* a1, RE::InputValue& a_outValue, int32_t& a_inputId, void* a3);

		static inline tGetCameraObject GetCameraObject;
		static inline tGetCurrentCameraDefinition GetCurrentCameraDefinition;
		static inline tShouldShowSneakCones ShouldShowSneakCones;
		static inline tGetUnkPlayerObject GetUnkPlayerObject;
		static inline tGetCharacter GetCharacter;
		static inline tGetCharacterHeight GetCharacterHeight;
		static inline tGetPlayerController GetPlayerController;
		static inline tGetInputPressed GetInputPressed;
		static inline tGetInputValue GetInputValue;

		static inline void** UnkSingletonPtr = nullptr;
		static inline void** UnkCameraSingletonPtr = nullptr;
		static inline void** UnkPlayerSingletonPtr = nullptr;
		static inline void** UnkInputSingletonPtr = nullptr;
		static inline bool* bIsInControllerMode = nullptr;
	};

    class Hooks
    {
    public:
		static void Hook()
		{
			dku::Hook::Trampoline::AllocTrampoline(1 << 7);

			const auto UpdateCameraCallAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? E9 ?? ?? ?? ?? 44 3B 2D ?? ?? ?? ?? 75 1B">());
			if (!UpdateCameraCallAddress) {
				FATAL("UpdateCamera not found!")
			}
			_UpdateCamera = dku::Hook::write_call<5>(UpdateCameraCallAddress, Hook_UpdateCamera);
			INFO("Hooked UpdateCamera: {:X}", AsAddress(UpdateCameraCallAddress))

			const auto SetDesiredCameraValueAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 08 66 89 0B 80 3B 00">());
			if (!SetDesiredCameraValueAddress) {
				FATAL("SetDesiredCameraValue not found!")
			}
			_SetDesiredCameraValue = dku::Hook::write_call<5>(SetDesiredCameraValueAddress, Hook_SetDesiredCameraValue);
			INFO("Hooked SetDesiredCameraValue: {:X}", AsAddress(SetDesiredCameraValueAddress))

			const auto CalculateCameraPitchAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? F3 0F 10 9B ?? ?? ?? ?? 0F 28 C8">());
			if (!CalculateCameraPitchAddress) {
				FATAL("CalculateCameraPitch not found!")
			}
			_CalculateCameraPitch = dku::Hook::write_call<5>(CalculateCameraPitchAddress, Hook_CalculateCameraPitch);
			INFO("Hooked CalculateCameraPitch: {:X}", AsAddress(CalculateCameraPitchAddress))

			const auto UpdateCameraPitchAddress = AsAddress(dku::Hook::Assembly::search_pattern<"49 8B CF E8 ?? ?? ?? ?? 4D 8B CD 4C 89 64 24 ??">()) + 3;
			if (!UpdateCameraPitchAddress) {
				FATAL("UpdateCameraPitch not found!")
            }
			_UpdateCameraPitch = dku::Hook::write_call<5>(UpdateCameraPitchAddress, Hook_UpdateCameraPitch);
			INFO("Hooked UpdateCameraPitch: {:X}", AsAddress(UpdateCameraPitchAddress))

			const auto UpdateCameraZoomAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 41 83 BD ?? ?? ?? ?? ?? 76 1C">());
			if (!UpdateCameraZoomAddress) {
				FATAL("UpdateCameraZoom not found!")
			}
			_UpdateCameraZoom = dku::Hook::write_call<5>(UpdateCameraZoomAddress, Hook_UpdateCameraZoom);
			INFO("Hooked UpdateCameraZoom: {:X}", AsAddress(UpdateCameraZoomAddress))

			const auto HandleToggleInputModeCallAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 00 84 C0">());
			if (!HandleToggleInputModeCallAddress) {
				FATAL("HandleToggleInputModeCall not found!")
			}
			_HandleToggleInputMode = dku::Hook::write_call<5>(HandleToggleInputModeCallAddress, Hook_HandleToggleInputMode);
			INFO("Hooked HandleToggleInputModeCall: {:X}", AsAddress(HandleToggleInputModeCallAddress))

			
		}

    private:
		static void Hook_UpdateCamera(uint64_t a1, uint64_t a2, uint64_t a3, RE::UnkObject* a4);
		static void* Hook_SetDesiredCameraValue(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uintptr_t a4);
		static float Hook_CalculateCameraPitch(RE::CameraObject* a_cameraObject, uint8_t a2, uint8_t a3);
		static void Hook_UpdateCameraPitch(uint64_t a1, RE::UnkObject* a2, RE::CameraObject* a_cameraObject, uint64_t a4);
		static void Hook_UpdateCameraZoom(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uint64_t a4);
		static int16_t* Hook_HandleToggleInputMode(uint64_t a1, int16_t& a_outResult, int32_t* a_inputId);

		static inline std::add_pointer_t<decltype(Hook_UpdateCamera)> _UpdateCamera;
		static inline std::add_pointer_t<decltype(Hook_SetDesiredCameraValue)> _SetDesiredCameraValue;
		static inline std::add_pointer_t<decltype(Hook_CalculateCameraPitch)> _CalculateCameraPitch;
		static inline std::add_pointer_t<decltype(Hook_UpdateCameraPitch)> _UpdateCameraPitch;
		static inline std::add_pointer_t<decltype(Hook_UpdateCameraZoom)> _UpdateCameraZoom;
		static inline std::add_pointer_t<decltype(Hook_HandleToggleInputMode)> _HandleToggleInputMode;
    };

	void Install();
}