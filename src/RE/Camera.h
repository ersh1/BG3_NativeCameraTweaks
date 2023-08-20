#pragma once

namespace RE
{
	struct Vector3
	{
	    float x;
		float y;
		float z;
	};
	static_assert(sizeof(Vector3) == 0xC);

	struct InputValue
	{
	    float valueA;
		float valueB;
		bool bIsPressed;
	};

	struct Player
	{
	    uint64_t unk00;
	    uint64_t unk08;
	    uint64_t unk10;
	    uint64_t unk18;
	    uint64_t unk20;
	    uint64_t unk28;
	    uint64_t unk30;
		int16_t playerId_38;
	};

	struct UnkObject
	{
		uint64_t unk00;
		uint64_t unk08;
		uint64_t unk10;
		uint64_t unk18;
		uint64_t unk20;
		uint64_t unk28;
		uint64_t unk30;
		uint64_t unk38;
		uint64_t unk40;
		uint64_t unk48;
		uint64_t unk50;
		uint64_t unk58;
		Player* currentPlayer_60;
	};

    struct CameraDefinition
    {
		uint64_t unk00[5];
		float maxZoom_28;
		float minZoom_2C;
		float maxZoomAlt_30;
		float minZoomAlt_34;
		uint64_t unk38;
		uint64_t unk40;
		float pitchAdjustSpeedA_48;
		uint32_t unk4C;
		uint32_t unk50;
		float float_54;
		float maxZoomUnk_58;
		float minZoomUnk_5C;
		uint32_t unk60;
		float camHorizontalOffsetMult_64;
		float camVerticalOffsetMult_68;
		uint32_t unk6C;
		uint64_t unk70;
		uint64_t unk78;
		uint32_t unk80;
		float fovClose_84;
		float fovFar_88;
		float fovCloseAlt_8C;
		float fovFarAlt_90;
		float unk94;
		uint64_t unk98;
		uint64_t unkA0;
		uint32_t unkA8;
		float zoomSpeed_AC;
		float scrollSpeed_B0;
		uint32_t unkB4;
		uint64_t unkB8;
		uint64_t unkC0;
		float tactMinZoom_C8;
		float tactMaxZoom_CC;
		float tacticalFov_D0;
		float maxCamDistanceFromRoot_D4;
		uint64_t unkD8;
		uint64_t unkE0;
		uint64_t unkE8;
		float pitchAdjustSpeedB_F0;
		float pitchAdjustSpeedC_F4;
		uint64_t unkF8;
		uint64_t unk100;
		float unk108;
		uint32_t unk10C;
		uint64_t unk110;
		uint64_t unk118;
		float float_120;
		uint32_t unk124;
		float float_128;
		float float_12C;
		float float_130;
		uint32_t uint32_134;
		uint64_t unk138;
		uint64_t unk140;
		uint64_t unk148;
		uint64_t unk150;
		uint64_t unk158;
		float pitchFar_160;
		float pitchClose_164;
		float pitchCombatFar_168;
		float pitchCombatClose_16C;
		float tacticalPitchFar_170;
		float tacticalPitchClose_174;
		float pitchFarAlt_178;
		float pitchCloseAlt_17C;
		uint64_t unk180;
		uint64_t unk188;
    };
	static_assert(offsetof(CameraDefinition, fovClose_84) == 0x84);
	static_assert(offsetof(CameraDefinition, pitchAdjustSpeedB_F0) == 0xF0);

	// a lot of this is wrong
    struct CameraObject
    {
		uint64_t unk00;
		uint64_t unk08;
		uint64_t unk10;
		Vector3 desiredCameraRootPosA_18;
		Vector3 desiredCameraRootPosB_24;
		uint64_t unk30;
		uint32_t unk38;
		Vector3 cameraRootPosA_3C;
		Vector3 cameraRootPosB_48;
		float currentZoomA_54;
		float currentZoomB_58;
		float desiredZoom_5C;
		float float_60;
		float float_64;
		float float_68;
		float float_6C;
		Vector3 cameraRotation_70;
		Vector3 prevCameraRotation_7C;
		uint64_t unk88;
		uint32_t unk90;
		float horizontalPanDelta_94;
		float verticalPanDelta_98;
		float currentAngleDelta_9C;
		float mouseRotationDelta_A0;
		float zoomDelta_A4;
		uint32_t cameraModeFlags_A8;
		float angle_AC;
		uint64_t unkB0;
		uint64_t unkB8;
		float rotationSpeed_C0;
		Vector3 cameraRoot_C4;
		float verticalOffset_D0;
		float float_D4;
		float float_D8;
		float verticalOffsetNegated_DC;
		float float_E0;
		float timer_E4;
		float verticalOffset_E8;
		uint32_t unkEC;
		uint64_t unkF0;
		uint64_t unkF8[8];
		uint32_t unk138;
		uint32_t currentZoomPercent_13C;
		float timer_140;
		uint32_t unk144;
		uint64_t unk148;
		Vector3 cameraPos_150;
		float prevZoom_15C;
		float currentZoom_160;
		float currentPitch_164;
		uint64_t unk168[15];
		float unk1E0;
		bool bool_1E4;
		uint8_t unk1E5;
		uint16_t unk1E6;
		uint64_t unk1E8;
		uint8_t unk1F0;
		uint8_t unk1F1;
		uint16_t unk1F2;
		uint32_t unk1F4;
		uint64_t unk1F8[11];
		uint32_t unk250;
		uint32_t unk254;

		// 0x35C = char angle ? 
    };
	static_assert(offsetof(CameraObject, cameraModeFlags_A8) == 0xA8);
}
