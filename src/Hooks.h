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
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 48 85 FF 74 6A">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 1);
					GetCameraObject = reinterpret_cast<tGetCameraObject>(scan + 5 + offset);
					INFO("GetCameraObject found: {:X}", AsAddress(GetCameraObject))
				} else {
					FATAL("GetCameraObject not found")
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 55 38">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 1);
				    GetCurrentCameraDefinition = reinterpret_cast<tGetCurrentCameraDefinition>(scan + 5 + offset);
					INFO("GetCurrentCameraDefinition found: {:X}", AsAddress(GetCurrentCameraDefinition))
				} else {
					FATAL("GetCurrentCameraDefinition not found")
				}
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 44 0F B6 F0 4D 85 ED">());
				if (scan) {
					auto singletonOffset = *reinterpret_cast<int32_t*>(scan + 3);
					auto funcOffset = *reinterpret_cast<int32_t*>(scan + 8);
					UnkSingletonPtr = reinterpret_cast<void**>(scan + 7 + singletonOffset);
					ShouldShowSneakCones = reinterpret_cast<tShouldShowSneakCones>(scan + 0xC + funcOffset);
					INFO("ShouldShowSneakCones found: {:X}", AsAddress(ShouldShowSneakCones))
				} else {
                    FATAL("ShouldShowSneakCones not found")
                }
			}

			{
				auto scan = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"80 3D ?? ?? ?? ?? ?? 74 22">());
				if (scan) {
					auto offset = *reinterpret_cast<int32_t*>(scan + 2);
					bIsInControllerMode = reinterpret_cast<bool*>(scan + 7 + offset);
					INFO("bIsInControllerMode found: {:X}", AsAddress(bIsInControllerMode))
				} else {
				    FATAL("bIsInControllerMode not found")
				}
				
			}
		}

		using tGetCameraObject = RE::CameraObject* (*)(uint64_t a1);
		using tGetCurrentCameraDefinition = RE::CameraDefinition* (*)(RE::CameraObject* a_cameraObject);
		using tShouldShowSneakCones = bool (*)(void* a1, uint16_t a2);

		static inline tGetCameraObject GetCameraObject;
		static inline tGetCurrentCameraDefinition GetCurrentCameraDefinition;
		static inline tShouldShowSneakCones ShouldShowSneakCones;

		static inline void** UnkSingletonPtr = nullptr;
		static inline bool* bIsInControllerMode = nullptr;
	};

    class Hooks
    {
    public:
		static void Hook()
		{
			dku::Hook::Trampoline::AllocTrampoline(1 << 7);

			const auto SetDesiredCameraValueAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F B7 08 66 89 0B 80 3B 00">());
			if (SetDesiredCameraValueAddress) {
				_SetDesiredCameraValue = dku::Hook::write_call<5>(SetDesiredCameraValueAddress, Hook_SetDesiredCameraValue);
				INFO("Hooked SetDesiredCameraValue: {:X}", AsAddress(SetDesiredCameraValueAddress))
			} else {
				FATAL("SetDesiredCameraValue not found")
			}

			const auto CalculateCameraPitchAddress = AsAddress(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? F3 0F 10 9B ?? ?? ?? ?? 0F 28 C8">());
			if (CalculateCameraPitchAddress) {
				_CalculateCameraPitch = dku::Hook::write_call<5>(CalculateCameraPitchAddress, Hook_CalculateCameraPitch);
				INFO("Hooked CalculateCameraPitch: {:X}", AsAddress(CalculateCameraPitchAddress))
			} else {
			    FATAL("CalculateCameraPitch not found")
			}

			const auto UpdateCameraPitchAddress = AsAddress(dku::Hook::Assembly::search_pattern<"49 8B CF E8 ?? ?? ?? ?? 4D 8B CD 4C 89 64 24 ??">()) + 3;
			if (UpdateCameraPitchAddress) {
				_UpdateCameraPitch = dku::Hook::write_call<5>(UpdateCameraPitchAddress, Hook_UpdateCameraPitch);
				INFO("Hooked UpdateCameraPitch: {:X}", AsAddress(UpdateCameraPitchAddress))
            } else {
                FATAL("UpdateCameraPitch not found")
			}

			constexpr dku::Hook::Patch epilog = {
				"\xC3",  // retn
				0x1
			};

			{
				auto callAddr = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? 0F 28 F0 83 BB ?? ?? ?? ?? ??">());
				if (callAddr) {
					auto offset = *reinterpret_cast<int32_t*>(callAddr + 1);
					const auto GetCameraMinZoomAddress = reinterpret_cast<uintptr_t>(callAddr + 5 + offset);

					auto prolog = static_cast<Patch*>(nullptr);
					auto hook = dku::Hook::AddCaveHook(GetCameraMinZoomAddress, { 0, 5 }, FUNC_INFO(OverrideCameraMinZoom), prolog, &epilog);
					hook->Enable();
					INFO("Hooked GetCameraMinZoom: {:X}", AsAddress(GetCameraMinZoomAddress))
				} else {
				    FATAL("GetCameraMinZoom not found")
				}
			}

			{
				auto callAddr = static_cast<uint8_t*>(dku::Hook::Assembly::search_pattern<"E8 ?? ?? ?? ?? B2 01 0F 28 F8">());
				if (callAddr) {
					auto offset = *reinterpret_cast<int32_t*>(callAddr + 1);
					const auto GetCameraMaxZoomAddress = reinterpret_cast<uintptr_t>(callAddr + 5 + offset);

					auto prolog = static_cast<Patch*>(nullptr);
					auto hook = dku::Hook::AddCaveHook(GetCameraMaxZoomAddress, { 0, 5 }, FUNC_INFO(OverrideCameraMaxZoom), prolog, &epilog);
					hook->Enable();
					INFO("Hooked GetCameraMaxZoom: {:X}", AsAddress(GetCameraMaxZoomAddress))
				} else {
				    FATAL("GetCameraMaxZoom not found")
				}
			}
		}

    private:
		static void* Hook_SetDesiredCameraValue(uint64_t a1, uint64_t a2, uint64_t a3, uintptr_t a4);
		static float Hook_CalculateCameraPitch(RE::CameraObject* a_cameraObject, uint8_t a2, uint8_t a3);
		static void Hook_UpdateCameraPitch(uint64_t a1, uint64_t a2, RE::CameraObject* a_cameraObject, uint64_t a4);

		static float OverrideCameraMinZoom();
		static float OverrideCameraMaxZoom();


		static inline std::add_pointer_t<decltype(Hook_SetDesiredCameraValue)> _SetDesiredCameraValue;
		static inline std::add_pointer_t<decltype(Hook_CalculateCameraPitch)> _CalculateCameraPitch;
		static inline std::add_pointer_t<decltype(Hook_UpdateCameraPitch)> _UpdateCameraPitch;
    };

	void Install();
}