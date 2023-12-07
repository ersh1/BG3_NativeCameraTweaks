#pragma once
#include "RE/Camera.h"

struct Vector2
{
	float x;
	float y;
};

class CameraTweaks : public DKUtil::model::Singleton<CameraTweaks>
{
public:
    enum class CameraMode : uint8_t
    {
		kExploration,
		kExplorationTactical,
		kCombat,
		kCombatTactical,
		kFreeCamera
    };

	struct PlayerData
	{
		std::optional<float> pitch = std::nullopt;
		float controllerPitchDelta = 0.f;
		bool bSkipToggleInputMode = false;
		RE::CameraObject* cameraObject = nullptr;
	};

	static constexpr float VANILLA_DEADZONE = 0.65f;
	static constexpr float NORMALIZE_DEADZONE = 1.f / (1.f - VANILLA_DEADZONE);
	static constexpr float ZOOM_ADJUST_STEP = 0.01f;

	void SetCameraSettings();

	int16_t GetPlayerIdFromCameraObject(RE::CameraObject* a_cameraObject) const;
	void SetCameraObjectForPlayer(int16_t a_playerId, RE::CameraObject* a_cameraObject);
	static CameraMode GetCurrentCameraMode(RE::CameraObject* a_cameraObject);
	static CameraMode GetCurrentCameraMode(RE::CameraModeFlags a_cameraModeFlags);
	bool IsCameraUnlocked(int16_t a_playerId, RE::CameraObject* a_cameraObject) const;
	bool CanAdjustPitch(RE::CameraObject* a_cameraObject) const;
	bool CanAdjustPitch(CameraTweaks::CameraMode cameraMode) const;

	bool ShouldSkipToggleInputMode(int16_t a_playerId) const { return GetPlayerData(a_playerId).bSkipToggleInputMode; }

	void SetControllerPitchDelta(int16_t a_playerId, float a_inputValue);
	void SetSkipToggleInputMode(int16_t a_playerId, bool a_bSkip) { GetPlayerData(a_playerId).bSkipToggleInputMode = a_bSkip; }

	void SetDeltaTime(float a_deltaTime) { _deltaTime = a_deltaTime; }

	bool CalculateCameraPitch(int16_t a_playerId, RE::CameraObject* a_cameraObject, float& a_outPitch);
	void AdjustCameraZoomForPitch(uint64_t a1, RE::UnkObject* a2);
	
	float AdjustInputValueForDeadzone(float a_inputValue, bool a_bApplyMult = true);

	int delta_y;

protected:
	PlayerData& GetPlayerData(int16_t a_playerId) { return _playerData[a_playerId - 1]; }
	const PlayerData& GetPlayerData(int16_t a_playerId) const { return _playerData[a_playerId - 1]; }

	float GetDeadzone();
	float NormalizeWithinRange(float a_min, float a_max, float a_value);
	float Denormalize(float a_min, float a_max, float a_value);
	float DegreesToRadians(float a_degrees);
	float InterpTo(float a_current, float a_target, float a_deltaTime, float a_interpSpeed);

	std::array<PlayerData, 2> _playerData;

	float _deltaTime = 0.f;
};
