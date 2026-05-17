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

	static constexpr float PI = 3.14159265f;
	static constexpr float TWO_PI = 2.f * PI;
	static constexpr float RAD_TO_DEG = 180.f / PI;
	static constexpr float DEG_TO_RAD = PI / 180.f;

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
	void AdjustCameraZoomForPitch(uint64_t a1, uint64_t a2, RE::CameraObject* a_cameraObject);
	
	float AdjustInputValueForDeadzone(float a_inputValue, bool a_bApplyMult = true);

	int delta_y;

	RE::Player* GetCurrentPlayer() { return _currentPlayer; }
	void SetCurrentPlayer(RE::Player* a_player) { _currentPlayer = a_player; }
	RE::CameraObject* GetCurrentCamera() { return _currentCamera; }
	void SetCurrentCamera(RE::CameraObject* a_camera) { _currentCamera = a_camera; }

	// Debug data
	struct CameraObjectSnapshot
	{
		bool valid = false;
		void* ptr = nullptr;
		RE::Vector3 cameraRootPos{};
		RE::Vector3 desiredCameraRootPos{};
		RE::Vector3 cameraPos{};
		RE::Vector3 cameraRotation{};
		RE::Vector3 cameraPos_150{};
		RE::Vector3 unkCameraRootPosA{};
		RE::Vector3 unkCameraRootPosB{};
		float currentZoomA = 0.f;
		float currentZoomB = 0.f;
		float desiredZoom = 0.f;
		float currentPitch = 0.f;
		float cameraAngle = 0.f;
		float horizontalPanDelta = 0.f;
		float verticalPanDelta = 0.f;
		float currentAngleDelta = 0.f;
		float mouseRotationDelta = 0.f;
		float zoomDelta = 0.f;
		float rotationSpeed = 0.f;
		float verticalOffset = 0.f;
		float prevZoom_15C = 0.f;
		float timer_140 = 0.f;
		uint32_t cameraModeFlags = 0;
	};

	struct CameraDefSnapshot
	{
		bool valid = false;
		void* ptr = nullptr;
		float maxZoom_28 = 0.f;
		float minZoom_2C = 0.f;
		float altMaxZoomController_30 = 0.f;
		float altMinZoomController_34 = 0.f;
		float zoomSpeed_AC = 0.f;
		float scrollSpeed_B0 = 0.f;
		float unkDefaultZoom_24 = 0.f;
		float tactDefaultZoom_C4 = 0.f;
		float tactMinZoom_C8 = 0.f;
		float tactMaxZoom_CC = 0.f;
		float fovClose_84 = 0.f;
		float fovFar_88 = 0.f;
		float fovCloseAlt_8C = 0.f;
		float fovFarAlt_90 = 0.f;
		float tacticalFov_D0 = 0.f;
		float pitchFar_160 = 0.f;
		float pitchClose_164 = 0.f;
		float pitchCombatFar_168 = 0.f;
		float pitchCombatClose_16C = 0.f;
		float tacticalPitchFar_170 = 0.f;
		float tacticalPitchClose_174 = 0.f;
		float pitchFarAlt_178 = 0.f;
		float pitchCloseAlt_17C = 0.f;
		float pitchAdjustSpeedA_48 = 0.f;
		float pitchAdjustSpeedB_F0 = 0.f;
		float pitchAdjustSpeedC_F4 = 0.f;
		float camHorizontalOffsetMult_64 = 0.f;
		float camVerticalOffsetMult_68 = 0.f;
		float altCamHorizontalOffsetMultController_70 = 0.f;
		float altCamVerticalOffsetMultController_74 = 0.f;
		float maxCamDistanceFromRoot_D4 = 0.f;
	};

	struct DebugData
	{
		uintptr_t characterPtr = 0;
		int16_t playerId = 0;
		bool isControllerMode = false;
		float deltaTime = 0.f;
		void* playerPtr = nullptr;
		void* cameraSingletonPtr = nullptr;
		void* unkSingletonPtr = nullptr;
		void* getCharacterSingletonPtr = nullptr;
		CameraObjectSnapshot camera;
		CameraDefSnapshot cameraDef;

		// Entity rotation
		bool hasEntityRotation = false;
		RE::Quaternion entityRotationQuat{};
		RE::Vector3 entityTranslate{};
		float entityHeadingRad = 0.f;
		float entityHeadingDeg = 0.f;
		int entityTransformOffset = -1;

		float derivedHeadingRad = 0.f;
		float derivedHeadingDeg = 0.f;
		bool hasHeading = false;          // true once we've seen movement
		RE::Vector3 movementDelta{};
		float movementSpeed = 0.f;

		float behindEntityAngleDeg = 0.f;
		bool hasBehindEntityAngle = false;

		bool isTrackingNPC = false;         // true when camera follows an entity we don't have rotation for
		float behindMovementAngleDeg = 0.f;
		bool hasBehindMovementAngle = false;
	};

	void UpdateDebugData();

	// Helpers to read settings outside of SEH (__try) context
	bool IsAlignCameraBehindOnSwitchEnabled(uint32_t a_cameraModeFlags);
	bool IsAlignCameraBehindNPCEnabled(uint32_t a_cameraModeFlags);
	void GetDebugAndAlignFlags(bool& a_outDebug, bool& a_outNeedEntityData);

	DebugData GetDebugData() const { return _cachedDebugData; }

	static bool SafeReadFloat(uintptr_t addr, float& outVal);
	static bool SafeReadVec3(uintptr_t addr, RE::Vector3& outVal);
	static bool SafeReadQuat(uintptr_t addr, RE::Quaternion& outVal);
	static bool SafeReadPtr(uintptr_t addr, uintptr_t& outVal);

	struct TransformScanResult
	{
		uintptr_t address = 0;          // address where Transform was found
		RE::Quaternion rotationQuat{};
		RE::Vector3 translate{};
		RE::Vector3 scale{};
		float quatLength = 0.f;
		float posDistance = 0.f;         // distance from search position
		float headingDeg = 0.f;
	};

	void RequestMemoryScan() { _memoryScanRequested.store(true); }
	bool IsMemoryScanRunning() const { return _memoryScanRunning.load(); }
	const std::vector<TransformScanResult>& GetMemoryScanResults() const { return _memoryScanResults; }
	RE::Vector3 GetMemoryScanSearchPos() const { return _memoryScanSearchPos; }
	
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
	RE::Player* _currentPlayer = nullptr;
	RE::CameraObject* _currentCamera = nullptr;
	DebugData _cachedDebugData{};

	// Heading derivation constants & tuning
	static constexpr float HEADING_WINDOW_SEC = 0.15f;
	static constexpr float HEADING_MIN_SPEED = 1.0f;
	static constexpr float CHAR_SWITCH_DIST = 1.0f;
	static constexpr float NPC_TRACK_INTERP_SPEED = 3.5f;
	RE::Vector3 _windowStartPos{};    // position at start of current window
	float _windowElapsed = 0.f;        // time accumulated in current window
	bool _hasWindowStart = false;
	float _derivedHeadingRad = 0.f;
	bool _hasHeading = false;

	// Switch detection state
	RE::Vector3 _prevDesiredRootPos{};
	bool _hasPrevDesiredPos = false;

	// Entity/NPC rotation state
	float _entityHeadingRad = 0.f;
	bool _hasEntityHeading = false;

	bool _isTrackingNPC = false;

	// Scan cache
	uintptr_t _cachedTransformAddr = 0;
	uintptr_t _cachedTransformCharPtr = 0;
	float _transformScanCooldown = 0.f;
	static constexpr float TRANSFORM_SCAN_RETRY_SEC = 0.5f;

	// Memory scan state
	std::atomic<bool> _memoryScanRequested = false;
	std::atomic<bool> _memoryScanRunning = false;
	std::vector<TransformScanResult> _memoryScanResults;
	RE::Vector3 _memoryScanSearchPos{};

	void RunMemoryScan(const RE::Vector3& searchPos);
};
