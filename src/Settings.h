#pragma once
#include "DKUtil/Config.hpp"

namespace Settings
{
	using namespace DKUtil::Alias;

	class Main : public DKUtil::model::Singleton<Main>
	{
	public:
		Double ZoomMin{ "ZoomMin", "General" };
		Double ZoomMax { "ZoomMax", "General" };
		Double PitchMin { "PitchMin", "General" };
		Double PitchMax { "PitchMax", "General" };
		Double MouseZoomMult{ "MouseZoomMult", "Mouse" };
		Double MousePitchMult{ "MousePitchMult", "Mouse" };
		Double ControllerRightStickMult{ "ControllerRightStickMult", "Controller" };
		Double ControllerZoomMult{ "ControllerZoomMult", "Controller" };
		Double ControllerPitchMult{ "ControllerPitchMult", "Controller" };
		Boolean InvertControllerPitch{ "InvertControllerPitch", "Controller" };
		Boolean OverrideRightStickDeadzone{ "OverrideRightStickDeadzone", "Deadzone" };
		Double NewDeadzone{ "NewDeadzone", "Deadzone" };

		void Load() noexcept;

	private:
		TomlConfig config = COMPILE_PROXY("NativeMods/BG3NativeCameraTweaks.toml"sv);
	};


}

