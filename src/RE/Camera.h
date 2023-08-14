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

    struct CameraDefinition
    {
		uint64_t unk00[5];
		float maxFloat_28;
		float minFloat_2C;
		float maxFloat_30;
		float minFloat_34;
		uint64_t unk38;
		uint64_t unk40;
		float pitchAdjustSpeedA_48;
		uint32_t unk4C;
		uint32_t unk50;
		float float_54;
		float maxFloat_58;
		float minFloat_5C;
		uint64_t unk60[13];
		float minFloat_C8;
		float maxFloat_CC;
		uint64_t unkD0;
		uint64_t unkD8;
		uint64_t unkE0;
		uint64_t unkE8;
		float pitchAdjustSpeedB_F0;
		float pitchAdjustSpeedC_F4;
		uint64_t unkF8;
		uint64_t unk100;
		float timer_108;
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
		float pitchZoomOut_160;
		float pitchZoomIn_164;
		float float168;
		float float16C;
		float float170;
		float float174;
		float float178;
		float float17C;
		uint64_t unk180;
		uint64_t unk188;
    };

    struct CameraObject
    {
		uint64_t unk00;
		uint64_t unk08;
		uint64_t unk10;
		Vector3 cameraPosA_18;
		Vector3 desiredCameraPos_24;
		uint64_t unk30;
		uint32_t unk38;
		Vector3 cameraPosB_3C;
		Vector3 cameraPosC_48;
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
		float float_94;
		float float_98;
		float currentAngleDelta_9C;
		float mouseRotationDelta_A0;
		float zoomDelta_A4;
		uint32_t cameraModeFlags_A8;
		float angle_AC;
		uint64_t unkB0;
		uint64_t unkB8;
		float rotationSpeed_C0;
		uint32_t unkC4;
		uint64_t unkC8;
		float float_D0;
		float float_D4;
		float float_D8;
		float float_DC;
		float float_E0;
		float timer_E4;
		uint64_t unkE8[10];
		uint32_t unk138;
		uint32_t currentZoomPercent_13C;
		float timer_140;
		uint32_t unk144;
		uint64_t unk148;
		Vector3 vector_150;
		float prevZoom_15C;
		float currentZoom_160;
		float currentPitch_164;
		uint64_t unk168[15];
		float pitch_1E0;
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
    };
	static_assert(offsetof(CameraObject, cameraModeFlags_A8) == 0xA8);
}
