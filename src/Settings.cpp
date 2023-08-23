#include "Settings.h"

namespace Settings
{
	void Main::Load() noexcept
	{
	    static std::once_flag ConfigInit;
		std::call_once(ConfigInit, [&]() {
			config.Bind(UnlockedPitchInitialValue, 25.f);
			config.Bind(UnlockedPitchClampSpeed, 5.f);
			config.Bind(UnlockedPitchLimitClipping, true);
			config.Bind(UnlockedPitchFloorOffset, 0.1f);


			config.Bind(ExplorationUnlockPitch, true);
			config.Bind(ExplorationKeepTacticalPitchLocked, false);
			config.Bind<-89, 89>(ExplorationUnlockedPitchMin, -85.f);
			config.Bind<-89, 89>(ExplorationUnlockedPitchMax, 85.f);
			config.Bind(ExplorationOverrideLockedPitch, false);
			config.Bind<-89, 89>(ExplorationLockedPitchClose, 19.05f);          // 19.05
			config.Bind<-89, 89>(ExplorationLockedPitchFar, 40.71f);            // 40.71
			config.Bind<-89, 89>(ExplorationLockedTacticalPitchClose, 85.55f);  // 85.55
			config.Bind<-89, 89>(ExplorationLockedTacticalPitchFar, 85.55f);    // 85.55
			config.Bind<-89, 89>(ExplorationLockedAltPitchClose, 32.69f);       // 32.69
			config.Bind<-89, 89>(ExplorationLockedAltPitchFar, 39.7f);          // 39.7

			config.Bind(ExplorationOverrideZoom, true);
			config.Bind(ExplorationZoomMin, 0.5f);          // 3.5
			config.Bind(ExplorationZoomMax, 20.f);          // 12
			config.Bind(ExplorationTacticalZoomMin, 10.f);  // 10
			config.Bind(ExplorationTacticalZoomMax, 50.f);  // 50
			config.Bind(ExplorationAltZoomMin, 10.f);       // 10
			config.Bind(ExplorationAltZoomMax, 40.f);       // 40

			config.Bind(ExplorationOverrideFOV, false);
			config.Bind(ExplorationFOVClose, 55.f);     // 55
			config.Bind(ExplorationFOVFar, 55.f);       // 55
			config.Bind(ExplorationTacticalFOV, 25.f);  // 25
			config.Bind(ExplorationAltFOVClose, 45.f);  // 45
			config.Bind(ExplorationAltFOVFar, 45.f);    // 45

			config.Bind(ExplorationOverrideOffset, false);
			config.Bind(ExplorationHorizontalOffsetMult, 0.f);  // 0
			config.Bind(ExplorationVerticalOffsetMult, 0.8f);   // 0.8


			config.Bind(CombatUnlockPitch, true);
			config.Bind(CombatKeepTacticalPitchLocked, false);
			config.Bind<-89, 89>(CombatUnlockedPitchMin, -85.f);
			config.Bind<-89, 89>(CombatUnlockedPitchMax, 85.f);
			config.Bind(CombatOverrideLockedPitch, false);
			config.Bind<-89, 89>(CombatLockedPitchClose, 32.73f);          // 32.73
			config.Bind<-89, 89>(CombatLockedPitchFar, 52.42f);            // 52.42
			config.Bind<-89, 89>(CombatLockedTacticalPitchClose, 85.55f);  // 85.55
			config.Bind<-89, 89>(CombatLockedTacticalPitchFar, 85.55f);    // 85.55
			config.Bind<-89, 89>(CombatLockedAltPitchClose, 32.69f);       // 32.69
			config.Bind<-89, 89>(CombatLockedAltPitchFar, 39.7f);          // 39.7

			config.Bind(CombatOverrideZoom, true);
			config.Bind(CombatZoomMin, 0.5f);          // 4
			config.Bind(CombatZoomMax, 20.f);          // 15
			config.Bind(CombatTacticalZoomMin, 10.f);  // 10
			config.Bind(CombatTacticalZoomMax, 50.f);  // 50
			config.Bind(CombatAltZoomMin, 10.f);       // 10
			config.Bind(CombatAltZoomMax, 40.f);       // 40

			config.Bind(CombatOverrideFOV, false);
			config.Bind(CombatFOVClose, 55.f);     // 55
			config.Bind(CombatFOVFar, 55.f);       // 55
			config.Bind(CombatTacticalFOV, 25.f);  // 25
			config.Bind(CombatAltFOVClose, 45.f);  // 45
			config.Bind(CombatAltFOVFar, 45.f);    // 45

			config.Bind(CombatOverrideOffset, false);
			config.Bind(CombatHorizontalOffsetMult, 0.f);  // 0
			config.Bind(CombatVerticalOffsetMult, 0.8f);   // 0.8


			config.Bind(MouseCameraRotationMult, 1.f);
			config.Bind(MousePitchMult, 0.25f);
			config.Bind(MouseZoomMult, 0.5f);
			config.Bind(InvertMousePitch, false);

			config.Bind(KeyboardCameraRotationMult, 2.f);

			config.Bind(ControllerCameraRotationMult, 2.f);
			config.Bind(ControllerPitchMult, 0.5f);
			config.Bind(ControllerZoomMult, 0.5f);
			config.Bind(InvertControllerPitch, false);
			config.Bind(SwapZoomAndPitch, false);
			config.Bind(UseRightStickPressForZoom, false);

			config.Bind(OverrideRightStickDeadzone, true);
			config.Bind(NewDeadzone, 0.15f);
		});

		config.Load();

		INFO("Config loaded"sv)
	}
}