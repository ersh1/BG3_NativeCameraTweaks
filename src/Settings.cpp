#include "Settings.h"

namespace Settings
{
	void Main::Load() noexcept
	{
	    static std::once_flag ConfigInit;
		std::call_once(ConfigInit, [&]() {
			config.Bind(ZoomMin, 0.5f);
			config.Bind(ZoomMax, 20.f);
			config.Bind<-89, 89>(PitchMin, -30.f);
			config.Bind<-89, 89>(PitchMax, 80.f);
			config.Bind(MouseZoomMult, 0.5f);
			config.Bind(MousePitchMult, 0.25f);
			config.Bind(ControllerRightStickMult, 2.f);
			config.Bind(ControllerZoomMult, 0.5f);
			config.Bind(ControllerPitchMult, 0.5f);
			config.Bind(InvertControllerPitch, false);
			config.Bind(OverrideRightStickDeadzone, true);
			config.Bind(NewDeadzone, 0.15f);
		});

		config.Load();

		INFO("Config loaded"sv)
	}
}