#include "Settings.h"

#include "ImGuiRenderer.h"

namespace Settings
{
	void Main::Load() noexcept
	{
		WriteLocker locker(Lock);

	    static std::once_flag ConfigInit;
		std::call_once(ConfigInit, [&]() {
			config.Bind(UnlockedPitchInitialValue, 25.f);
			config.Bind(UnlockedPitchClampSpeed, 5.f);
			config.Bind(UnlockedPitchLimitClipping, true);
			config.Bind(UnlockedPitchFloorOffset, 0.1f);
			config.Bind(ResetZoomOnZoneChange, false);

			config.Bind(WatchForConfigChanges, true);

			config.Bind(ToggleMenuKey, "Delete"s);

			config.Bind(EnableDebugMode, false);

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

			config.Bind(ExplorationAlignBehindOnSwitch, false);
			config.Bind(ExplorationAlignBehindNPC, false);
			config.Bind(ExplorationDisableCameraPan, false);


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

			config.Bind(CombatAlignBehindOnSwitch, false);
			config.Bind(CombatAlignBehindNPC, false);
			config.Bind(CombatDisableCameraPan, false);

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

		Validate();

		INFO("Config loaded."sv)

		bChanged = true;
	}

	void Main::Validate() noexcept
	{
		const auto sanitizeDouble = [](Double& a_setting, double a_default, double a_min, double a_max) {
			double val = *a_setting;
			if (!std::isfinite(val) || val < a_min || val > a_max) {
				WARN("Settings: sanitizing '{}' — value {} is invalid, resetting to {}", a_setting.get_key(), val, a_default)
				a_setting.set_data(a_default);
			}
		};

		// [General]
		sanitizeDouble(UnlockedPitchInitialValue, 25.0, -89.0, 89.0);
		sanitizeDouble(UnlockedPitchClampSpeed, 5.0, 0.0, 100.0);
		sanitizeDouble(UnlockedPitchFloorOffset, 0.1, -10.0, 50.0);

		// [ExplorationPitch]
		sanitizeDouble(ExplorationUnlockedPitchMin, -85.0, -89.0, 89.0);
		sanitizeDouble(ExplorationUnlockedPitchMax, 85.0, -89.0, 89.0);
		sanitizeDouble(ExplorationLockedPitchClose, 19.05, -89.0, 89.0);
		sanitizeDouble(ExplorationLockedPitchFar, 40.71, -89.0, 89.0);
		sanitizeDouble(ExplorationLockedTacticalPitchClose, 85.55, -89.0, 89.0);
		sanitizeDouble(ExplorationLockedTacticalPitchFar, 85.55, -89.0, 89.0);
		sanitizeDouble(ExplorationLockedAltPitchClose, 32.69, -89.0, 89.0);
		sanitizeDouble(ExplorationLockedAltPitchFar, 39.7, -89.0, 89.0);

		// [ExplorationZoom]
		sanitizeDouble(ExplorationZoomMin, 0.5, 0.0, 200.0);
		sanitizeDouble(ExplorationZoomMax, 20.0, 0.0, 200.0);
		sanitizeDouble(ExplorationTacticalZoomMin, 10.0, 0.0, 200.0);
		sanitizeDouble(ExplorationTacticalZoomMax, 50.0, 0.0, 200.0);
		sanitizeDouble(ExplorationAltZoomMin, 10.0, 0.0, 200.0);
		sanitizeDouble(ExplorationAltZoomMax, 40.0, 0.0, 200.0);

		// [ExplorationFOV]
		sanitizeDouble(ExplorationFOVClose, 55.0, 1.0, 170.0);
		sanitizeDouble(ExplorationFOVFar, 55.0, 1.0, 170.0);
		sanitizeDouble(ExplorationTacticalFOV, 25.0, 1.0, 170.0);
		sanitizeDouble(ExplorationAltFOVClose, 45.0, 1.0, 170.0);
		sanitizeDouble(ExplorationAltFOVFar, 45.0, 1.0, 170.0);

		// [ExplorationOffset]
		sanitizeDouble(ExplorationHorizontalOffsetMult, 0.0, -10.0, 10.0);
		sanitizeDouble(ExplorationVerticalOffsetMult, 0.8, -10.0, 10.0);

		// [CombatPitch]
		sanitizeDouble(CombatUnlockedPitchMin, -85.0, -89.0, 89.0);
		sanitizeDouble(CombatUnlockedPitchMax, 85.0, -89.0, 89.0);
		sanitizeDouble(CombatLockedPitchClose, 32.73, -89.0, 89.0);
		sanitizeDouble(CombatLockedPitchFar, 52.42, -89.0, 89.0);
		sanitizeDouble(CombatLockedTacticalPitchClose, 85.55, -89.0, 89.0);
		sanitizeDouble(CombatLockedTacticalPitchFar, 85.55, -89.0, 89.0);
		sanitizeDouble(CombatLockedAltPitchClose, 32.69, -89.0, 89.0);
		sanitizeDouble(CombatLockedAltPitchFar, 39.7, -89.0, 89.0);

		// [CombatZoom]
		sanitizeDouble(CombatZoomMin, 0.5, 0.0, 200.0);
		sanitizeDouble(CombatZoomMax, 20.0, 0.0, 200.0);
		sanitizeDouble(CombatTacticalZoomMin, 10.0, 0.0, 200.0);
		sanitizeDouble(CombatTacticalZoomMax, 50.0, 0.0, 200.0);
		sanitizeDouble(CombatAltZoomMin, 10.0, 0.0, 200.0);
		sanitizeDouble(CombatAltZoomMax, 40.0, 0.0, 200.0);

		// [CombatFOV]
		sanitizeDouble(CombatFOVClose, 55.0, 1.0, 170.0);
		sanitizeDouble(CombatFOVFar, 55.0, 1.0, 170.0);
		sanitizeDouble(CombatTacticalFOV, 25.0, 1.0, 170.0);
		sanitizeDouble(CombatAltFOVClose, 45.0, 1.0, 170.0);
		sanitizeDouble(CombatAltFOVFar, 45.0, 1.0, 170.0);

		// [CombatOffset]
		sanitizeDouble(CombatHorizontalOffsetMult, 0.0, -10.0, 10.0);
		sanitizeDouble(CombatVerticalOffsetMult, 0.8, -10.0, 10.0);

		// [Mouse]
		sanitizeDouble(MouseCameraRotationMult, 1.0, 0.001, 100.0);
		sanitizeDouble(MousePitchMult, 0.25, 0.001, 100.0);
		sanitizeDouble(MouseZoomMult, 0.5, 0.001, 100.0);

		// [Keyboard]
		sanitizeDouble(KeyboardCameraRotationMult, 2.0, 0.001, 100.0);

		// [Controller]
		sanitizeDouble(ControllerCameraRotationMult, 2.0, 0.001, 100.0);
		sanitizeDouble(ControllerPitchMult, 0.5, 0.001, 100.0);
		sanitizeDouble(ControllerZoomMult, 0.5, 0.001, 100.0);

		// [ControllerDeadzone]
		sanitizeDouble(NewDeadzone, 0.15, 0.0, 0.99);
	}

	// from https://github.com/emoose/DLSSTweaks
    void Main::WatchForChanges()
	{
		const auto path = std::filesystem::current_path() / CONFIG_PATH;

		const auto cfgFilename = path.filename().wstring();
		const auto cfgFolder = path.parent_path().wstring();

		HANDLE file = CreateFileW(cfgFolder.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if (!file) {
			DWORD err = GetLastError();
			WARN("Config monitoring: CreateFileW \"{}\" failed with error code {}", path.parent_path().string(), err);
			return;
		}

		OVERLAPPED overlapped;
		overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
		if (!overlapped.hEvent) {
			DWORD err = GetLastError();
			WARN("Config monitoring: CreateEvent failed with error code {}", err);
			CloseHandle(file);
		}

		uint8_t change_buf[1024];
		BOOL success = ReadDirectoryChangesW(
			file, change_buf, 1024, TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_LAST_WRITE,
			NULL, &overlapped, NULL);

	    if (!success) {
			DWORD err = GetLastError();
			WARN("Config monitoring: ReadDirectoryChangesW failed with error code {}", err);
			return;
		}

		INFO("Config monitoring: watching for config updates...");

		while (success) {
			DWORD result = WaitForSingleObject(overlapped.hEvent, INFINITE);
			if (result == WAIT_OBJECT_0) {
				DWORD bytes_transferred;
				GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

				auto* evt = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(change_buf);

				for (;;) {
					if (evt->Action == FILE_ACTION_MODIFIED) {
						// evt->FileName isn't null-terminated, so construct wstring for it based on FileNameLength
						std::wstring name = std::wstring(evt->FileName, evt->FileNameLength / sizeof(WCHAR));
						if (!_wcsicmp(name.c_str(), cfgFilename.c_str())) {
							// Config read might fail if it's still being updated by a text editor etc
							// so try attempting a few times, Sleep(1000) between attempts should hopefully let us read it fine
							int attempts = 3;
							while (attempts--) {
								INFO("Config monitoring: Change detected! Updating config...")
								Load();
								ImGuiRenderer::RefreshToggleKey();
								if (attempts > 0) {
									Sleep(1000);
								}
							}
						}
					}

					// Any more events to handle?
					if (evt->NextEntryOffset)
						*(uint8_t**)&evt += evt->NextEntryOffset;
					else
						break;
				}

				// Queue the next event
				success = ReadDirectoryChangesW(
					file, change_buf, 1024, TRUE,
					FILE_NOTIFY_CHANGE_FILE_NAME |
						FILE_NOTIFY_CHANGE_DIR_NAME |
						FILE_NOTIFY_CHANGE_LAST_WRITE,
					NULL, &overlapped, NULL);
			}
		}

		CloseHandle(file);
	}
}
