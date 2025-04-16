#pragma once
#include "RE/Camera.h"

namespace Hooks
{
	using namespace DKUtil::Alias;

	class Offsets
	{
	public:
		static bool Init()
		{
			bool bSuccess = true;

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"48 8B 05 ?? ?? ?? ?? 80 B8 32 13 00 00 00 74 07">());
				if (scan) {
					auto singletonOffset = *reinterpret_cast<int32_t*>(scan + 3);
					UnkCameraSingletonPtr = reinterpret_cast<void**>(scan + 7 + singletonOffset);
					GetCurrentCameraDefinition = reinterpret_cast<tGetCurrentCameraDefinition>(scan);
					INFO("GetCurrentCameraDefinition found: {:X}", AsAddress(GetCurrentCameraDefinition) - dku::Hook::Module::get().base())
				} else {
					ERROR("GetCurrentCameraDefinition not found!")
					bSuccess = false;
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"48 8B 0D ?? ?? ?? ?? 0F B7 D0 E8 ?? ?? ?? ?? 84 C0 75 1B">());
				if (scan) {
					auto unkSingletonOffset = *reinterpret_cast<int32_t*>(scan + 3);
					auto funcOffset = *reinterpret_cast<int32_t*>(scan + 0xB);
					UnkSingletonPtr = reinterpret_cast<void**>(scan + 7 + unkSingletonOffset);
					ShouldShowSneakCones = reinterpret_cast<tShouldShowSneakCones>(scan + 0xF + funcOffset);

					auto inputSingletonOffset = *reinterpret_cast<int32_t*>(AsAddress(ShouldShowSneakCones) + 0x82 + 3);
					UnkInputSingletonPtr = reinterpret_cast<void**>(AsAddress(ShouldShowSneakCones) + 0x82 + 7 + inputSingletonOffset);

					auto getInputValueCallsite = AsAddress(ShouldShowSneakCones) + 0x9B;
					auto getInputValueOffset = *reinterpret_cast<int32_t*>(getInputValueCallsite + 1);
					GetInputValue = reinterpret_cast<tGetInputValue>(getInputValueCallsite + 5 + getInputValueOffset);

					INFO("Input related functions found: {:X}", AsAddress(ShouldShowSneakCones) - dku::Hook::Module::get().base())
				} else {
					ERROR("Input related functions not found!")
					bSuccess = false;
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"48 8B 0D ?? ?? ?? ?? 0F B7 D7 E8 ?? ?? ?? ?? 3C FF">());
				if (scan) {
					auto playerSingletonOffset = *reinterpret_cast<int32_t*>(scan + 3);
					UnkPlayerSingletonPtr = reinterpret_cast<void**>(scan + 7 + playerSingletonOffset);

					auto getPlayerControllerCallsite = scan + 0xA;
					auto getPlayerControllerOffset = *reinterpret_cast<int32_t*>(getPlayerControllerCallsite + 1);
					GetPlayerController = reinterpret_cast<tGetPlayerController>(getPlayerControllerCallsite + 5 + getPlayerControllerOffset);

					INFO("Player controller related functions found: {:X}", AsAddress(GetPlayerController) - dku::Hook::Module::get().base())
				} else {
					ERROR("Player controller functions not found!")
					bSuccess = false;
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"80 3D ?? ?? ?? ?? ?? 74 22">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 2);
					bIsInControllerMode = reinterpret_cast<bool*>(scan + 7 + offset);
					INFO("bIsInControllerMode found: {:X}", AsAddress(bIsInControllerMode) - dku::Hook::Module::get().base())
				} else {
					ERROR("bIsInControllerMode not found!")
					bSuccess = false;
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 41 0F 28 45 40">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 1);
					GetCharacter = reinterpret_cast<tGetCharacter>(scan + 5 + offset);
					INFO("GetCharacter found: {:X}", AsAddress(GetCharacter) - dku::Hook::Module::get().base())
				} else {
					ERROR("GetCharacter not found!")
					bSuccess = false;
				}
			}

			// unused
			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"48 89 5C 24 08 57 48 83 EC 30 48 8B 41 10 48 8B D9">());
				if (scan) {
					GetCharacterHeight = reinterpret_cast<tGetCharacterHeight>(scan);
					INFO("GetCharacterHeight found: {:X}", AsAddress(GetCharacterHeight) - dku::Hook::Module::get().base())
				} else {
					ERROR("GetCharacterHeight not found!")
					bSuccess = false;
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? EB 11 33 C9">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 1);
					GetFloorLevel = reinterpret_cast<tGetFloorLevel>(scan + 5 + offset);
					INFO("GetFloorLevel found: {:X}", AsAddress(GetFloorLevel) - dku::Hook::Module::get().base())
				} else {
					ERROR("GetFloorLevel not found!")
					bSuccess = false;
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F 28 DA F3 0F 5C E5">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 1);
					GetCameraMinZoom = reinterpret_cast<tGetCameraMinZoom>(scan + 5 + offset);
					INFO("GetCameraMinZoom found: {:X}", AsAddress(GetCameraMinZoom) - dku::Hook::Module::get().base())
				} else {
					ERROR("GetCameraMinZoom not found!")
					bSuccess = false;
				}
			}


			return bSuccess;
		}

		enum class InputID : int32_t
		{
		    kZoomIn = 104,
			kZoomOut = 105,
			kRotateLeft = 107,
			kRotateRight = 108,
			kMouseRotateLeft = 109,
			kMouseRotateRight = 110,
			kToggleInputMode = 0xC0
		};

		constexpr static inline uint32_t cameraBoolOffset = 0x1332;
		constexpr static inline uint32_t unkCameraOffset = 0xC58;
		constexpr static inline uint32_t explorationCameraOffset = 0x79C;
		constexpr static inline uint32_t combatCameraOffset = 0x930;

		using tGetCurrentCameraDefinition = RE::CameraDefinition* (*)(RE::CameraObject* a1);
		using tShouldShowSneakCones = bool (*)(void* a1, int16_t a_playerId);
		using tGetCharacter = uintptr_t (*)(uintptr_t a1, int16_t a_playerId);
		using tGetCharacterHeight = float (*)(uintptr_t a_character);
		using tGetPlayerController = void* (*)(void* a1, int16_t a_playerId);
		using tGetInputValue = RE::InputValue* (*)(void* a1, RE::InputValue& a_outValue, InputID& a_inputId, void* a4);
		using tGetCurrentPlayerInternal = RE::Player* (*)(uint64_t a1, uint64_t a2);
		using tGetFloorLevel = RE::FloorLevelStruct* (*)(RE::FloorLevelStruct& a_outFloorLevelStruct, uint64_t a2, bool a3, RE::CameraDefinition* a_cameraDefinition, void* a5, RE::Vector3& a_cameraPos, uint64_t a7);
		using tGetCameraMinZoom = float (*)(RE::CameraModeFlags a_flags, bool a2);

		static inline tGetCurrentCameraDefinition GetCurrentCameraDefinition;
		static inline tShouldShowSneakCones ShouldShowSneakCones;
		static inline tGetCharacter GetCharacter;
		static inline tGetCharacterHeight GetCharacterHeight;
		static inline tGetPlayerController GetPlayerController;
		static inline tGetInputValue GetInputValue;
		static inline tGetCurrentPlayerInternal GetCurrentPlayerInternal;
		static inline tGetCameraMinZoom GetCameraMinZoom;
		static inline tGetFloorLevel GetFloorLevel;

		static inline void** UnkSingletonPtr = nullptr;
		static inline void** UnkCameraSingletonPtr = nullptr;
		static inline void** UnkPlayerSingletonPtr = nullptr;
		static inline void** UnkInputSingletonPtr = nullptr;
		static inline bool* bIsInControllerMode = nullptr;
	};

    class Hooks
    {
    public:
		static bool Hook()
		{
			bool bSuccess = true;

			dku::Hook::Trampoline::AllocTrampoline(1 << 7);

			const auto UpdateCameraCallAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 48 8D 8D F8 04 00 00 E8 ?? ?? ?? ?? E9 AF FD FF FF">());
			if (UpdateCameraCallAddress) {
				_UpdateCamera = dku::Hook::write_call<5>(UpdateCameraCallAddress, Hook_UpdateCamera);
				INFO("Hooked UpdateCamera: {:X}", AsAddress(UpdateCameraCallAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("UpdateCamera not found!")
				bSuccess = false;
			}

			const auto HandleCameraInputAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 08 66 89 0B 80 3B 00">());
			if (HandleCameraInputAddress) {
				_HandleCameraInput = dku::Hook::write_call<5>(HandleCameraInputAddress, Hook_HandleCameraInput);
				INFO("Hooked HandleCameraInput: {:X}", AsAddress(HandleCameraInputAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("HandleCameraInput not found!")
			    bSuccess = false;
			}

			const auto CalculateCameraPitchAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 80 BF 4C 01 00 00 00">());
			if (CalculateCameraPitchAddress) {
				_CalculateCameraPitch = dku::Hook::write_call<5>(CalculateCameraPitchAddress, Hook_CalculateCameraPitch);
				INFO("Hooked CalculateCameraPitch: {:X}", AsAddress(CalculateCameraPitchAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("CalculateCameraPitch not found!")
				bSuccess = false;
			}

			const auto UpdateCameraPitchAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 48 8B 46 70 4C 8D 45 90 0F 28 46 30">());
			if (UpdateCameraPitchAddress) {
				_UpdateCameraPitch = dku::Hook::write_call<5>(UpdateCameraPitchAddress, Hook_UpdateCameraPitch);
				INFO("Hooked UpdateCameraPitch: {:X}", AsAddress(UpdateCameraPitchAddress) - dku::Hook::Module::get().base())
            } else {
				ERROR("UpdateCameraPitch not found!")
				bSuccess = false;
            }
			
			const auto AfterUpdateCameraZoomAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 80 BF 54 02 00 00 00">());
			if (AfterUpdateCameraZoomAddress) {
				_AfterUpdateCameraZoom = dku::Hook::write_call<5>(AfterUpdateCameraZoomAddress, Hook_AfterUpdateCameraZoom);
				INFO("Hooked AfterUpdateCameraZoom: {:X}", AsAddress(AfterUpdateCameraZoomAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("AfterUpdateCameraZoom not found!")
				bSuccess = false;
			}

			const auto HandleToggleInputModeCallAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 00 84 C0">());
			if (HandleToggleInputModeCallAddress) {
				_HandleToggleInputMode = dku::Hook::write_call<5>(HandleToggleInputModeCallAddress, Hook_HandleToggleInputMode);
				INFO("Hooked HandleToggleInputModeCall: {:X}", AsAddress(HandleToggleInputModeCallAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("HandleToggleInputModeCall not found!")
				bSuccess = false;
			}

			const auto SetDefaultZoomCallAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 48 8B CF E8 ?? ?? ?? ?? E9 81 02 00 00">());
			if (SetDefaultZoomCallAddress) {
				_SetDefaultZoom = dku::Hook::write_call<5>(SetDefaultZoomCallAddress, Hook_SetDefaultZoom);
				INFO("Hooked SetDefaultZoom: {:X}", AsAddress(SetDefaultZoomCallAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("SetDefaultZoom not found!")
				bSuccess = false;
			}

			const auto SDLMouseYHookAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F 28 B4 24 30 02 00 00 0F 28 BC 24 20 02 00 00 44 0F 28 84 24 10 02 00 00">());
			if (SDLMouseYHookAddress) {
				struct Stub : Xbyak::CodeGenerator
				{
					Stub()
					{
						mov(r9d, r12d);
						mov(rax, (uintptr_t)&Hook_SDLMouseYHook);
						jmp(rax);
					}
				};
			    static Stub stub;

				_SDLMouseYHook = dku::Hook::write_call<5>(SDLMouseYHookAddress, (bool (*)(uint64_t, uint64_t, bool, int))stub.getCode());

				INFO("Hooked SDLMouseYHook: {:X}", AsAddress(SDLMouseYHookAddress) - dku::Hook::Module::get().base())
			} else {
				ERROR("SDLMouseYHook not found!")
				bSuccess = false;
			}

			return bSuccess;
		}

    private:
		static void Hook_UpdateCamera(uint64_t a1, uint64_t a2, uint64_t a3, RE::UnkObject* a4);
		static void* Hook_HandleCameraInput(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uintptr_t a4);
		static float Hook_CalculateCameraPitch(RE::CameraObject* a_cameraObject, uint8_t a2, uint8_t a3);
		static void Hook_UpdateCameraPitch(uint64_t a1, uint64_t a2, RE::CameraObject* a_cameraObject, uint64_t a4);
		static void Hook_AfterUpdateCameraZoom(uint64_t a1, uint64_t a2, RE::UnkObject* a3, uint64_t a4);
		static int16_t* Hook_HandleToggleInputMode(uint64_t a1, int16_t& a_outResult, Offsets::InputID* a_inputId);
		static void Hook_SetDefaultZoom(RE::CameraObject* a_cameraObject);
		static bool Hook_SDLMouseYHook(uint64_t a1, uint64_t a2, bool a3, int a_deltaY); 

		static inline std::add_pointer_t<decltype(Hook_UpdateCamera)> _UpdateCamera;
		static inline std::add_pointer_t<decltype(Hook_HandleCameraInput)> _HandleCameraInput;
		static inline std::add_pointer_t<decltype(Hook_CalculateCameraPitch)> _CalculateCameraPitch;
		static inline std::add_pointer_t<decltype(Hook_UpdateCameraPitch)> _UpdateCameraPitch;
		static inline std::add_pointer_t<decltype(Hook_AfterUpdateCameraZoom)> _AfterUpdateCameraZoom;
		static inline std::add_pointer_t<decltype(Hook_HandleToggleInputMode)> _HandleToggleInputMode;
		static inline std::add_pointer_t<decltype(Hook_SetDefaultZoom)> _SetDefaultZoom;
		static inline std::add_pointer_t<decltype(Hook_SDLMouseYHook)> _SDLMouseYHook;
    };

	void Install();
}