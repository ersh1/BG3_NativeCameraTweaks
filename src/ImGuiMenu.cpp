#include "ImGuiMenu.h"

#include "CameraTweaksManager.h"
#include "Hooks.h"
#include "ImGuiRenderer.h"
#include "Settings.h"
#include "Utils.h"

#include <imgui.h>

#include <format>
#include <map>
#include <cerrno>

namespace ImGuiMenu
{
	static const ImVec4 kGoldHeader{ 0.78f, 0.62f, 0.30f, 1.0f };

	 // Helper wrappers 

	static bool SliderSetting(const char* a_label, DKUtil::Alias::Double& a_setting,
		float a_min, float a_max, bool& a_anyChanged, const char* a_fmt = "%.2f")
	{
		float val = static_cast<float>(*a_setting);
		if (ImGui::SliderFloat(a_label, &val, a_min, a_max, a_fmt)) {
			a_setting.set_data(static_cast<double>(val));
			a_anyChanged = true;
			return true;
		}
		return false;
	}

	static bool DragSetting(const char* a_label, DKUtil::Alias::Double& a_setting,
		float a_speed, float a_min, float a_max, bool& a_anyChanged, const char* a_fmt = "%.2f")
	{
		float val = static_cast<float>(*a_setting);
		if (ImGui::DragFloat(a_label, &val, a_speed, a_min, a_max, a_fmt)) {
			a_setting.set_data(static_cast<double>(val));
			a_anyChanged = true;
			return true;
		}
		return false;
	}

	static bool CheckboxSetting(const char* a_label, DKUtil::Alias::Boolean& a_setting, bool& a_anyChanged)
	{
		bool val = *a_setting;
		if (ImGui::Checkbox(a_label, &val)) {
			a_setting.set_data(val);
			a_anyChanged = true;
			return true;
		}
		return false;
	}

	static void HelpMarker(const char* a_desc)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
			ImGui::TextUnformatted(a_desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static void SectionSeparator()
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	 // Section drawing functions 

	static void DrawGeneralSection(Settings::Main* a_settings, bool& a_anyChanged)
	{
		ImGui::Spacing();

		SliderSetting("Unlocked Pitch Initial Value", a_settings->UnlockedPitchInitialValue, -89.0f, 89.0f, a_anyChanged);
		HelpMarker("The initial pitch angle when unlocked pitch mode first activates.");

		SliderSetting("Unlocked Pitch Clamp Speed", a_settings->UnlockedPitchClampSpeed, 0.0f, 50.0f, a_anyChanged);
		HelpMarker("How fast the pitch smoothly clamps to limits when they change.");

		CheckboxSetting("Unlocked Pitch Limit Clipping", a_settings->UnlockedPitchLimitClipping, a_anyChanged);
		HelpMarker("Prevent the camera from going below the floor level.");

		SliderSetting("Unlocked Pitch Floor Offset", a_settings->UnlockedPitchFloorOffset, -2.0f, 5.0f, a_anyChanged);
		HelpMarker("Offset above the floor level for the clipping limit.");

		CheckboxSetting("Reset Zoom On Zone Change", a_settings->ResetZoomOnZoneChange, a_anyChanged);
		HelpMarker("Whether the zoom resets to default when entering a new zone.");

		SectionSeparator();

		CheckboxSetting("Watch For Config Changes", a_settings->WatchForConfigChanges, a_anyChanged);
		HelpMarker("Automatically reload the TOML config when it changes on disk.\nRequires restart to take effect.");

		SectionSeparator();

		CheckboxSetting("Enable Debug Mode", a_settings->EnableDebugMode, a_anyChanged);
		HelpMarker("Enable the Debug tab and detailed per-frame data collection.\nCosts some performance.");

		SectionSeparator();

		// Toggle key selector
		{
			static const char* keyOptions[] = {
				"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
				"Insert", "Delete", "Home", "End", "PageUp", "PageDown", "Pause", "ScrollLock",
				"Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5",
				"Numpad6", "Numpad7", "Numpad8", "Numpad9"
			};
			static constexpr int keyCount = sizeof(keyOptions) / sizeof(keyOptions[0]);

			const std::string& currentKey = *a_settings->ToggleMenuKey;
			int selectedIdx = 13;  // Del
			for (int i = 0; i < keyCount; ++i) {
				std::string opt = keyOptions[i];
				std::string cur = currentKey;
				std::transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
				std::transform(cur.begin(), cur.end(), cur.begin(), ::tolower);
				if (opt == cur) {
					selectedIdx = i;
					break;
				}
			}

			if (ImGui::Combo("Menu Toggle Key", &selectedIdx, keyOptions, keyCount)) {
				a_settings->ToggleMenuKey.set_data(std::string(keyOptions[selectedIdx]));
				a_anyChanged = true;
				ImGuiRenderer::RefreshToggleKey();
			}
			HelpMarker("Key to open/close this menu.");
		}
	}

	static void DrawPitchSection(
		const char* a_prefix, bool a_isExploration,
		DKUtil::Alias::Boolean& a_unlockPitch,
		DKUtil::Alias::Boolean& a_keepTacticalLocked,
		DKUtil::Alias::Double& a_unlockedPitchMin,
		DKUtil::Alias::Double& a_unlockedPitchMax,
		DKUtil::Alias::Boolean& a_overrideLockedPitch,
		DKUtil::Alias::Double& a_lockedPitchClose,
		DKUtil::Alias::Double& a_lockedPitchFar,
		DKUtil::Alias::Double& a_lockedTacticalPitchClose,
		DKUtil::Alias::Double& a_lockedTacticalPitchFar,
		DKUtil::Alias::Double& a_lockedAltPitchClose,
		DKUtil::Alias::Double& a_lockedAltPitchFar,
		bool& a_anyChanged)
	{
		CheckboxSetting("Unlock Pitch", a_unlockPitch, a_anyChanged);
		HelpMarker("Allow free pitch rotation with mouse/controller.");

		if (*a_unlockPitch) {
			CheckboxSetting("Keep Tactical Pitch Locked", a_keepTacticalLocked, a_anyChanged);
			HelpMarker("Lock pitch in tactical camera mode even when pitch is unlocked.");

			SliderSetting("Unlocked Pitch Min", a_unlockedPitchMin, -89.0f, 89.0f, a_anyChanged);
			SliderSetting("Unlocked Pitch Max", a_unlockedPitchMax, -89.0f, 89.0f, a_anyChanged);
		}

		SectionSeparator();

		CheckboxSetting("Override Locked Pitch", a_overrideLockedPitch, a_anyChanged);
		HelpMarker("Override the game's default locked pitch angles.");

		if (*a_overrideLockedPitch) {
			ImGui::Indent(10.0f);
			ImGui::TextColored(kGoldHeader, "Standard");
			SliderSetting("Locked Pitch Close", a_lockedPitchClose, -89.0f, 89.0f, a_anyChanged);
			SliderSetting("Locked Pitch Far", a_lockedPitchFar, -89.0f, 89.0f, a_anyChanged);

			ImGui::Spacing();
			ImGui::TextColored(kGoldHeader, "Tactical");
			SliderSetting("Tactical Pitch Close", a_lockedTacticalPitchClose, -89.0f, 89.0f, a_anyChanged);
			SliderSetting("Tactical Pitch Far", a_lockedTacticalPitchFar, -89.0f, 89.0f, a_anyChanged);

			ImGui::Spacing();
			ImGui::TextColored(kGoldHeader, "Alt (Controller)");
			SliderSetting("Alt Pitch Close", a_lockedAltPitchClose, -89.0f, 89.0f, a_anyChanged);
			SliderSetting("Alt Pitch Far", a_lockedAltPitchFar, -89.0f, 89.0f, a_anyChanged);
			ImGui::Unindent(10.0f);
		}
	}

	// Clamp min + margin <= max for a zoom pair 
	// Game freaks out otherwise...
	static constexpr double kZoomMargin = 0.1;

	static void ClampZoomPair(DKUtil::Alias::Double& a_min, DKUtil::Alias::Double& a_max, bool& a_anyChanged)
	{
		if (*a_max - *a_min < kZoomMargin) {
			a_max.set_data(*a_min + kZoomMargin);
			a_anyChanged = true;
		}
	}

	static void DrawZoomSection(
		DKUtil::Alias::Boolean& a_overrideZoom,
		DKUtil::Alias::Double& a_zoomMin,
		DKUtil::Alias::Double& a_zoomMax,
		DKUtil::Alias::Double& a_tactZoomMin,
		DKUtil::Alias::Double& a_tactZoomMax,
		DKUtil::Alias::Double& a_altZoomMin,
		DKUtil::Alias::Double& a_altZoomMax,
		bool& a_anyChanged)
	{
		CheckboxSetting("Override Zoom", a_overrideZoom, a_anyChanged);
		HelpMarker("Override the game's default zoom limits.");

		if (*a_overrideZoom) {
			ImGui::Indent(10.0f);
			ImGui::TextColored(kGoldHeader, "Standard");
			DragSetting("Zoom Min", a_zoomMin, 0.1f, 0.0f, 100.0f, a_anyChanged);
			DragSetting("Zoom Max", a_zoomMax, 0.1f, 0.0f, 100.0f, a_anyChanged);
			ClampZoomPair(a_zoomMin, a_zoomMax, a_anyChanged);

			ImGui::Spacing();
			ImGui::TextColored(kGoldHeader, "Tactical");
			DragSetting("Tactical Zoom Min", a_tactZoomMin, 0.1f, 0.0f, 100.0f, a_anyChanged);
			DragSetting("Tactical Zoom Max", a_tactZoomMax, 0.1f, 0.0f, 100.0f, a_anyChanged);
			ClampZoomPair(a_tactZoomMin, a_tactZoomMax, a_anyChanged);

			ImGui::Spacing();
			ImGui::TextColored(kGoldHeader, "Alt (Controller)");
			DragSetting("Alt Zoom Min", a_altZoomMin, 0.1f, 0.0f, 100.0f, a_anyChanged);
			DragSetting("Alt Zoom Max", a_altZoomMax, 0.1f, 0.0f, 100.0f, a_anyChanged);
			ClampZoomPair(a_altZoomMin, a_altZoomMax, a_anyChanged);
			ImGui::Unindent(10.0f);
		}
	}

	static void DrawFOVSection(
		DKUtil::Alias::Boolean& a_overrideFOV,
		DKUtil::Alias::Double& a_fovClose,
		DKUtil::Alias::Double& a_fovFar,
		DKUtil::Alias::Double& a_tacticalFOV,
		DKUtil::Alias::Double& a_altFOVClose,
		DKUtil::Alias::Double& a_altFOVFar,
		bool& a_anyChanged)
	{
		CheckboxSetting("Override FOV", a_overrideFOV, a_anyChanged);
		HelpMarker("Override the game's field of view settings.");

		if (*a_overrideFOV) {
			ImGui::Indent(10.0f);
			ImGui::TextColored(kGoldHeader, "Standard");
			SliderSetting("FOV Close", a_fovClose, 10.0f, 120.0f, a_anyChanged);
			SliderSetting("FOV Far", a_fovFar, 10.0f, 120.0f, a_anyChanged);

			ImGui::Spacing();
			ImGui::TextColored(kGoldHeader, "Tactical");
			SliderSetting("Tactical FOV", a_tacticalFOV, 10.0f, 120.0f, a_anyChanged);

			ImGui::Spacing();
			ImGui::TextColored(kGoldHeader, "Alt (Controller)");
			SliderSetting("Alt FOV Close", a_altFOVClose, 10.0f, 120.0f, a_anyChanged);
			SliderSetting("Alt FOV Far", a_altFOVFar, 10.0f, 120.0f, a_anyChanged);
			ImGui::Unindent(10.0f);
		}
	}

	static void DrawOffsetSection(
		DKUtil::Alias::Boolean& a_overrideOffset,
		DKUtil::Alias::Double& a_horizontalMult,
		DKUtil::Alias::Double& a_verticalMult,
		bool& a_anyChanged)
	{
		CheckboxSetting("Override Offset", a_overrideOffset, a_anyChanged);
		HelpMarker("Override the camera offset multipliers.");

		if (*a_overrideOffset) {
			ImGui::Indent(10.0f);
			DragSetting("Horizontal Offset Mult", a_horizontalMult, 0.01f, -5.0f, 5.0f, a_anyChanged);
			DragSetting("Vertical Offset Mult", a_verticalMult, 0.01f, -5.0f, 5.0f, a_anyChanged);
			ImGui::Unindent(10.0f);
		}
	}

	// Forward declarations for profile serialization
	static std::string FormatDouble(double a_val);
	static std::string FormatBool(bool a_val);
	static bool SaveSettingsToFile(Settings::Main* a_settings);

	// Profile System 

	static std::map<std::string, toml::table> s_explorationProfiles;
	static std::map<std::string, toml::table> s_combatProfiles;
	static int  s_selectedExplProfileIdx  = -1;
	static int  s_selectedCombatProfileIdx = -1;
	static char s_newExplProfileName[64]  = "";
	static char s_newCombatProfileName[64] = "";
	static bool s_profilesLoaded = false;

	// Serialization helpers 

	static toml::table SerializeExplorationProfile(Settings::Main* s)
	{
		toml::table t;
		t.insert("UnlockPitch",               *s->ExplorationUnlockPitch);
		t.insert("KeepTacticalPitchLocked",   *s->ExplorationKeepTacticalPitchLocked);
		t.insert("UnlockedPitchMin",          *s->ExplorationUnlockedPitchMin);
		t.insert("UnlockedPitchMax",          *s->ExplorationUnlockedPitchMax);
		t.insert("OverrideLockedPitch",       *s->ExplorationOverrideLockedPitch);
		t.insert("LockedPitchClose",          *s->ExplorationLockedPitchClose);
		t.insert("LockedPitchFar",            *s->ExplorationLockedPitchFar);
		t.insert("LockedTacticalPitchClose",  *s->ExplorationLockedTacticalPitchClose);
		t.insert("LockedTacticalPitchFar",    *s->ExplorationLockedTacticalPitchFar);
		t.insert("LockedAltPitchClose",       *s->ExplorationLockedAltPitchClose);
		t.insert("LockedAltPitchFar",         *s->ExplorationLockedAltPitchFar);
		t.insert("OverrideZoom",              *s->ExplorationOverrideZoom);
		t.insert("ZoomMin",                   *s->ExplorationZoomMin);
		t.insert("ZoomMax",                   *s->ExplorationZoomMax);
		t.insert("TacticalZoomMin",           *s->ExplorationTacticalZoomMin);
		t.insert("TacticalZoomMax",           *s->ExplorationTacticalZoomMax);
		t.insert("AltZoomMin",                *s->ExplorationAltZoomMin);
		t.insert("AltZoomMax",                *s->ExplorationAltZoomMax);
		t.insert("OverrideFOV",               *s->ExplorationOverrideFOV);
		t.insert("FOVClose",                  *s->ExplorationFOVClose);
		t.insert("FOVFar",                    *s->ExplorationFOVFar);
		t.insert("TacticalFOV",               *s->ExplorationTacticalFOV);
		t.insert("AltFOVClose",               *s->ExplorationAltFOVClose);
		t.insert("AltFOVFar",                 *s->ExplorationAltFOVFar);
		t.insert("OverrideOffset",            *s->ExplorationOverrideOffset);
		t.insert("HorizontalOffsetMult",      *s->ExplorationHorizontalOffsetMult);
		t.insert("VerticalOffsetMult",        *s->ExplorationVerticalOffsetMult);
		t.insert("AlignBehindOnSwitch",       *s->ExplorationAlignBehindOnSwitch);
		t.insert("AlignBehindNPC",            *s->ExplorationAlignBehindNPC);
		t.insert("DisableCameraPan",          *s->ExplorationDisableCameraPan);
		return t;
	}

	static void DeserializeExplorationProfile(const toml::table& t, Settings::Main* s)
	{
		auto b = [&](const char* key) -> std::optional<bool> { return t[key].value<bool>(); };
		auto d = [&](const char* key) -> std::optional<double> { return t[key].value<double>(); };

		if (auto v = b("UnlockPitch"))              s->ExplorationUnlockPitch.set_data(*v);
		if (auto v = b("KeepTacticalPitchLocked"))  s->ExplorationKeepTacticalPitchLocked.set_data(*v);
		if (auto v = d("UnlockedPitchMin"))         s->ExplorationUnlockedPitchMin.set_data(*v);
		if (auto v = d("UnlockedPitchMax"))         s->ExplorationUnlockedPitchMax.set_data(*v);
		if (auto v = b("OverrideLockedPitch"))      s->ExplorationOverrideLockedPitch.set_data(*v);
		if (auto v = d("LockedPitchClose"))         s->ExplorationLockedPitchClose.set_data(*v);
		if (auto v = d("LockedPitchFar"))           s->ExplorationLockedPitchFar.set_data(*v);
		if (auto v = d("LockedTacticalPitchClose")) s->ExplorationLockedTacticalPitchClose.set_data(*v);
		if (auto v = d("LockedTacticalPitchFar"))   s->ExplorationLockedTacticalPitchFar.set_data(*v);
		if (auto v = d("LockedAltPitchClose"))      s->ExplorationLockedAltPitchClose.set_data(*v);
		if (auto v = d("LockedAltPitchFar"))        s->ExplorationLockedAltPitchFar.set_data(*v);
		if (auto v = b("OverrideZoom"))             s->ExplorationOverrideZoom.set_data(*v);
		if (auto v = d("ZoomMin"))                  s->ExplorationZoomMin.set_data(*v);
		if (auto v = d("ZoomMax"))                  s->ExplorationZoomMax.set_data(*v);
		if (auto v = d("TacticalZoomMin"))          s->ExplorationTacticalZoomMin.set_data(*v);
		if (auto v = d("TacticalZoomMax"))          s->ExplorationTacticalZoomMax.set_data(*v);
		if (auto v = d("AltZoomMin"))               s->ExplorationAltZoomMin.set_data(*v);
		if (auto v = d("AltZoomMax"))               s->ExplorationAltZoomMax.set_data(*v);
		if (auto v = b("OverrideFOV"))              s->ExplorationOverrideFOV.set_data(*v);
		if (auto v = d("FOVClose"))                 s->ExplorationFOVClose.set_data(*v);
		if (auto v = d("FOVFar"))                   s->ExplorationFOVFar.set_data(*v);
		if (auto v = d("TacticalFOV"))              s->ExplorationTacticalFOV.set_data(*v);
		if (auto v = d("AltFOVClose"))              s->ExplorationAltFOVClose.set_data(*v);
		if (auto v = d("AltFOVFar"))                s->ExplorationAltFOVFar.set_data(*v);
		if (auto v = b("OverrideOffset"))           s->ExplorationOverrideOffset.set_data(*v);
		if (auto v = d("HorizontalOffsetMult"))     s->ExplorationHorizontalOffsetMult.set_data(*v);
		if (auto v = d("VerticalOffsetMult"))       s->ExplorationVerticalOffsetMult.set_data(*v);
		if (auto v = b("AlignBehindOnSwitch"))      s->ExplorationAlignBehindOnSwitch.set_data(*v);
		if (auto v = b("AlignBehindNPC"))           s->ExplorationAlignBehindNPC.set_data(*v);
		if (auto v = b("DisableCameraPan"))         s->ExplorationDisableCameraPan.set_data(*v);
	}

	static toml::table SerializeCombatProfile(Settings::Main* s)
	{
		toml::table t;
		t.insert("UnlockPitch",               *s->CombatUnlockPitch);
		t.insert("KeepTacticalPitchLocked",   *s->CombatKeepTacticalPitchLocked);
		t.insert("UnlockedPitchMin",          *s->CombatUnlockedPitchMin);
		t.insert("UnlockedPitchMax",          *s->CombatUnlockedPitchMax);
		t.insert("OverrideLockedPitch",       *s->CombatOverrideLockedPitch);
		t.insert("LockedPitchClose",          *s->CombatLockedPitchClose);
		t.insert("LockedPitchFar",            *s->CombatLockedPitchFar);
		t.insert("LockedTacticalPitchClose",  *s->CombatLockedTacticalPitchClose);
		t.insert("LockedTacticalPitchFar",    *s->CombatLockedTacticalPitchFar);
		t.insert("LockedAltPitchClose",       *s->CombatLockedAltPitchClose);
		t.insert("LockedAltPitchFar",         *s->CombatLockedAltPitchFar);
		t.insert("OverrideZoom",              *s->CombatOverrideZoom);
		t.insert("ZoomMin",                   *s->CombatZoomMin);
		t.insert("ZoomMax",                   *s->CombatZoomMax);
		t.insert("TacticalZoomMin",           *s->CombatTacticalZoomMin);
		t.insert("TacticalZoomMax",           *s->CombatTacticalZoomMax);
		t.insert("AltZoomMin",                *s->CombatAltZoomMin);
		t.insert("AltZoomMax",                *s->CombatAltZoomMax);
		t.insert("OverrideFOV",               *s->CombatOverrideFOV);
		t.insert("FOVClose",                  *s->CombatFOVClose);
		t.insert("FOVFar",                    *s->CombatFOVFar);
		t.insert("TacticalFOV",               *s->CombatTacticalFOV);
		t.insert("AltFOVClose",               *s->CombatAltFOVClose);
		t.insert("AltFOVFar",                 *s->CombatAltFOVFar);
		t.insert("OverrideOffset",            *s->CombatOverrideOffset);
		t.insert("HorizontalOffsetMult",      *s->CombatHorizontalOffsetMult);
		t.insert("VerticalOffsetMult",        *s->CombatVerticalOffsetMult);
		t.insert("AlignBehindOnSwitch",       *s->CombatAlignBehindOnSwitch);
		t.insert("AlignBehindNPC",            *s->CombatAlignBehindNPC);
		t.insert("DisableCameraPan",          *s->CombatDisableCameraPan);
		return t;
	}

	static void DeserializeCombatProfile(const toml::table& t, Settings::Main* s)
	{
		auto b = [&](const char* key) -> std::optional<bool> { return t[key].value<bool>(); };
		auto d = [&](const char* key) -> std::optional<double> { return t[key].value<double>(); };

		if (auto v = b("UnlockPitch"))              s->CombatUnlockPitch.set_data(*v);
		if (auto v = b("KeepTacticalPitchLocked"))  s->CombatKeepTacticalPitchLocked.set_data(*v);
		if (auto v = d("UnlockedPitchMin"))         s->CombatUnlockedPitchMin.set_data(*v);
		if (auto v = d("UnlockedPitchMax"))         s->CombatUnlockedPitchMax.set_data(*v);
		if (auto v = b("OverrideLockedPitch"))      s->CombatOverrideLockedPitch.set_data(*v);
		if (auto v = d("LockedPitchClose"))         s->CombatLockedPitchClose.set_data(*v);
		if (auto v = d("LockedPitchFar"))           s->CombatLockedPitchFar.set_data(*v);
		if (auto v = d("LockedTacticalPitchClose")) s->CombatLockedTacticalPitchClose.set_data(*v);
		if (auto v = d("LockedTacticalPitchFar"))   s->CombatLockedTacticalPitchFar.set_data(*v);
		if (auto v = d("LockedAltPitchClose"))      s->CombatLockedAltPitchClose.set_data(*v);
		if (auto v = d("LockedAltPitchFar"))        s->CombatLockedAltPitchFar.set_data(*v);
		if (auto v = b("OverrideZoom"))             s->CombatOverrideZoom.set_data(*v);
		if (auto v = d("ZoomMin"))                  s->CombatZoomMin.set_data(*v);
		if (auto v = d("ZoomMax"))                  s->CombatZoomMax.set_data(*v);
		if (auto v = d("TacticalZoomMin"))          s->CombatTacticalZoomMin.set_data(*v);
		if (auto v = d("TacticalZoomMax"))          s->CombatTacticalZoomMax.set_data(*v);
		if (auto v = d("AltZoomMin"))               s->CombatAltZoomMin.set_data(*v);
		if (auto v = d("AltZoomMax"))               s->CombatAltZoomMax.set_data(*v);
		if (auto v = b("OverrideFOV"))              s->CombatOverrideFOV.set_data(*v);
		if (auto v = d("FOVClose"))                 s->CombatFOVClose.set_data(*v);
		if (auto v = d("FOVFar"))                   s->CombatFOVFar.set_data(*v);
		if (auto v = d("TacticalFOV"))              s->CombatTacticalFOV.set_data(*v);
		if (auto v = d("AltFOVClose"))              s->CombatAltFOVClose.set_data(*v);
		if (auto v = d("AltFOVFar"))                s->CombatAltFOVFar.set_data(*v);
		if (auto v = b("OverrideOffset"))           s->CombatOverrideOffset.set_data(*v);
		if (auto v = d("HorizontalOffsetMult"))     s->CombatHorizontalOffsetMult.set_data(*v);
		if (auto v = d("VerticalOffsetMult"))       s->CombatVerticalOffsetMult.set_data(*v);
		if (auto v = b("AlignBehindOnSwitch"))      s->CombatAlignBehindOnSwitch.set_data(*v);
		if (auto v = b("AlignBehindNPC"))           s->CombatAlignBehindNPC.set_data(*v);
		if (auto v = b("DisableCameraPan"))         s->CombatDisableCameraPan.set_data(*v);
	}

	static void LoadProfilesFromFile()
	{
		try {
			const auto path = std::filesystem::current_path() / CONFIG_PATH;
			if (!std::filesystem::exists(path)) {
				INFO("Profiles: config file does not exist yet, skipping profile load")
				s_profilesLoaded = true;
				return;
			}
			auto result = toml::parse_file(path.string());
			if (!result) {
				INFO("Profiles: could not parse TOML for profiles ({})", result.error().description())
				s_profilesLoaded = true;
				return;
			}

			toml::table tbl = std::move(result).table();

			s_explorationProfiles.clear();
			if (auto* explProfiles = tbl["ExplorationProfiles"].as_table()) {
				for (auto& [name, node] : *explProfiles) {
					if (auto* profileTbl = node.as_table()) {
						s_explorationProfiles[std::string(name.str())] = *profileTbl;
					}
				}
			}

			s_combatProfiles.clear();
			if (auto* combatProfiles = tbl["CombatProfiles"].as_table()) {
				for (auto& [name, node] : *combatProfiles) {
					if (auto* profileTbl = node.as_table()) {
						s_combatProfiles[std::string(name.str())] = *profileTbl;
					}
				}
			}

			s_selectedExplProfileIdx = s_explorationProfiles.empty() ? -1 : 0;
			s_selectedCombatProfileIdx = s_combatProfiles.empty() ? -1 : 0;
			s_profilesLoaded = true;

			INFO("Profiles: loaded {} exploration, {} combat profiles",
				s_explorationProfiles.size(), s_combatProfiles.size())
		} catch (const std::exception& e) {
			WARN("Profiles: exception while loading profiles: {}", e.what())
			s_profilesLoaded = true;
		} catch (...) {
			WARN("Profiles: unknown exception while loading profiles")
			s_profilesLoaded = true;
		}
	}

	static void WriteProfilesToFile(std::ofstream& file)
	{
		if (!s_explorationProfiles.empty()) {
			file << "#  Exploration Profiles \n\n";
			for (const auto& [name, tbl] : s_explorationProfiles) {
				file << "[ExplorationProfiles." << name << "]\n";
				for (const auto& [key, val] : tbl) {
					file << key.str() << " = ";
					if (auto* b = val.as_boolean()) {
						file << (*b ? "true" : "false");
					} else if (auto* f = val.as_floating_point()) {
						file << FormatDouble(f->get());
					} else if (auto* i = val.as_integer()) {
						file << FormatDouble(static_cast<double>(i->get()));
					}
					file << "\n";
				}
				file << "\n";
			}
		}

		if (!s_combatProfiles.empty()) {
			file << "#  Combat Profiles \n\n";
			for (const auto& [name, tbl] : s_combatProfiles) {
				file << "[CombatProfiles." << name << "]\n";
				for (const auto& [key, val] : tbl) {
					file << key.str() << " = ";
					if (auto* b = val.as_boolean()) {
						file << (*b ? "true" : "false");
					} else if (auto* f = val.as_floating_point()) {
						file << FormatDouble(f->get());
					} else if (auto* i = val.as_integer()) {
						file << FormatDouble(static_cast<double>(i->get()));
					}
					file << "\n";
				}
				file << "\n";
			}
		}
	}

	 // Profile UI Section 

	static bool DrawProfileSection(
		const char* a_mode,
		std::map<std::string, toml::table>& a_profiles,
		int& a_selectedIdx,
		char* a_newNameBuf,
		size_t a_newNameBufSize,
		Settings::Main* a_settings,
		std::function<toml::table(Settings::Main*)> a_serialize,
		std::function<void(const toml::table&, Settings::Main*)> a_deserialize)
	{
		bool profileLoaded = false;
		char headerId[64];
		snprintf(headerId, sizeof(headerId), "Profiles##%s", a_mode);

		if (ImGui::CollapsingHeader(headerId)) {
			ImGui::PushID(headerId);
			ImGui::Indent(4.0f);

			// Build name list from map keys
			std::vector<std::string> names;
			names.reserve(a_profiles.size());
			for (const auto& [name, _] : a_profiles) {
				names.push_back(name);
			}

			// Clamp selected index
			if (a_selectedIdx >= static_cast<int>(names.size())) {
				a_selectedIdx = names.empty() ? -1 : static_cast<int>(names.size()) - 1;
			}
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
				if (names.empty()) {
					int dummy = -1;
					ImGui::Combo("##ProfileSelect", &dummy, "(no profiles)\0", 1);
				} else {
					std::string combo;
					for (const auto& n : names) {
						combo += n;
						combo += '\0';
					}
					combo += '\0';
					ImGui::Combo("##ProfileSelect", &a_selectedIdx, combo.c_str());
				}

				ImGui::SameLine();
				bool hasSelection = a_selectedIdx >= 0 && a_selectedIdx < static_cast<int>(names.size());

				if (!hasSelection) ImGui::BeginDisabled();
				if (ImGui::Button("Load")) {
					const auto& profileName = names[a_selectedIdx];
					a_deserialize(a_profiles[profileName], a_settings);
					profileLoaded = true;
					INFO("Profiles: loaded {} profile '{}'", a_mode, profileName)
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
					ImGui::SetTooltip("Apply this profile's settings");
				}

				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					ImGui::OpenPopup("ConfirmDelete");
				}
				if (!hasSelection) ImGui::EndDisabled();

				if (ImGui::BeginPopup("ConfirmDelete")) {
					ImGui::Text("Delete profile \"%s\"?", hasSelection ? names[a_selectedIdx].c_str() : "");
					if (ImGui::Button("Yes", ImVec2(60, 0))) {
						if (hasSelection) {
							a_profiles.erase(names[a_selectedIdx]);
							a_selectedIdx = a_profiles.empty() ? -1 : 0;
							SaveSettingsToFile(a_settings);
							INFO("Profiles: deleted {} profile", a_mode)
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(60, 0))) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}

			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
				ImGui::InputTextWithHint("##NewProfileName", "Profile name...", a_newNameBuf, a_newNameBufSize);

				ImGui::SameLine();
				bool validName = a_newNameBuf[0] != '\0';
				// Sanitize name
				std::string candidate(a_newNameBuf);
				for (char& c : candidate) {
					if (c == ' ') c = '_';
				}
				bool nameOk = validName && candidate.find_first_of("[]#=\"'") == std::string::npos;

				if (!nameOk) ImGui::BeginDisabled();
				if (ImGui::Button("Save As")) {
					a_profiles[candidate] = a_serialize(a_settings);
					// Select the newly saved profile
					int idx = 0;
					for (const auto& [name, _] : a_profiles) {
						if (name == candidate) { a_selectedIdx = idx; break; }
						++idx;
					}
					a_newNameBuf[0] = '\0';
					SaveSettingsToFile(a_settings);
					INFO("Profiles: saved {} profile '{}'", a_mode, candidate)
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
					if (!validName)
						ImGui::SetTooltip("Enter a profile name first");
					else if (!nameOk)
						ImGui::SetTooltip("Name cannot contain [ ] # = \" '");
					else
						ImGui::SetTooltip("Save current %s settings as a new profile", a_mode);
				}
				if (!nameOk) ImGui::EndDisabled();
			}

			ImGui::Unindent(4.0f);
			ImGui::PopID();
		}

		return profileLoaded;
	}

	static void DrawExplorationTab(Settings::Main* a_settings, bool& a_anyChanged)
	{
		ImGui::Spacing();

		if (DrawProfileSection("Exploration", s_explorationProfiles,
				s_selectedExplProfileIdx, s_newExplProfileName, sizeof(s_newExplProfileName),
				a_settings, SerializeExplorationProfile, DeserializeExplorationProfile)) {
			a_anyChanged = true;
		}

		if (ImGui::CollapsingHeader("Pitch##Exploration", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("ExplPitch");
			DrawPitchSection("Exploration", true,
				a_settings->ExplorationUnlockPitch,
				a_settings->ExplorationKeepTacticalPitchLocked,
				a_settings->ExplorationUnlockedPitchMin,
				a_settings->ExplorationUnlockedPitchMax,
				a_settings->ExplorationOverrideLockedPitch,
				a_settings->ExplorationLockedPitchClose,
				a_settings->ExplorationLockedPitchFar,
				a_settings->ExplorationLockedTacticalPitchClose,
				a_settings->ExplorationLockedTacticalPitchFar,
				a_settings->ExplorationLockedAltPitchClose,
				a_settings->ExplorationLockedAltPitchFar,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Zoom##Exploration", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("ExplZoom");
			DrawZoomSection(
				a_settings->ExplorationOverrideZoom,
				a_settings->ExplorationZoomMin,
				a_settings->ExplorationZoomMax,
				a_settings->ExplorationTacticalZoomMin,
				a_settings->ExplorationTacticalZoomMax,
				a_settings->ExplorationAltZoomMin,
				a_settings->ExplorationAltZoomMax,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Field of View##Exploration")) {
			ImGui::PushID("ExplFOV");
			DrawFOVSection(
				a_settings->ExplorationOverrideFOV,
				a_settings->ExplorationFOVClose,
				a_settings->ExplorationFOVFar,
				a_settings->ExplorationTacticalFOV,
				a_settings->ExplorationAltFOVClose,
				a_settings->ExplorationAltFOVFar,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Camera Offset##Exploration")) {
			ImGui::PushID("ExplOffset");
			DrawOffsetSection(
				a_settings->ExplorationOverrideOffset,
				a_settings->ExplorationHorizontalOffsetMult,
				a_settings->ExplorationVerticalOffsetMult,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Camera Alignment##Exploration")) {
			ImGui::PushID("ExplAlign");
			CheckboxSetting("Align Behind On Character Switch", a_settings->ExplorationAlignBehindOnSwitch, a_anyChanged);
			HelpMarker("When switching to a different party member, automatically rotate the camera\n"
				"to sit behind them, facing the same direction they are.\n"
				"After the initial alignment you can freely move the camera.");
			CheckboxSetting("Align Behind NPC", a_settings->ExplorationAlignBehindNPC, a_anyChanged);
			HelpMarker("Continuously keep the camera behind NPCs by following\n"
				"their movement direction.");
			CheckboxSetting("Disable Camera Pan", a_settings->ExplorationDisableCameraPan, a_anyChanged);
			HelpMarker("Block all camera panning/detach inputs during exploration,\n"
				"keeping the camera centered on the character.");
			ImGui::PopID();
		}
	}

	static void DrawCombatTab(Settings::Main* a_settings, bool& a_anyChanged)
	{
		ImGui::Spacing();

		if (DrawProfileSection("Combat", s_combatProfiles,
				s_selectedCombatProfileIdx, s_newCombatProfileName, sizeof(s_newCombatProfileName),
				a_settings, SerializeCombatProfile, DeserializeCombatProfile)) {
			a_anyChanged = true;
		}

		if (ImGui::CollapsingHeader("Pitch##Combat", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("CombatPitch");
			DrawPitchSection("Combat", false,
				a_settings->CombatUnlockPitch,
				a_settings->CombatKeepTacticalPitchLocked,
				a_settings->CombatUnlockedPitchMin,
				a_settings->CombatUnlockedPitchMax,
				a_settings->CombatOverrideLockedPitch,
				a_settings->CombatLockedPitchClose,
				a_settings->CombatLockedPitchFar,
				a_settings->CombatLockedTacticalPitchClose,
				a_settings->CombatLockedTacticalPitchFar,
				a_settings->CombatLockedAltPitchClose,
				a_settings->CombatLockedAltPitchFar,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Zoom##Combat", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("CombatZoom");
			DrawZoomSection(
				a_settings->CombatOverrideZoom,
				a_settings->CombatZoomMin,
				a_settings->CombatZoomMax,
				a_settings->CombatTacticalZoomMin,
				a_settings->CombatTacticalZoomMax,
				a_settings->CombatAltZoomMin,
				a_settings->CombatAltZoomMax,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Field of View##Combat")) {
			ImGui::PushID("CombatFOV");
			DrawFOVSection(
				a_settings->CombatOverrideFOV,
				a_settings->CombatFOVClose,
				a_settings->CombatFOVFar,
				a_settings->CombatTacticalFOV,
				a_settings->CombatAltFOVClose,
				a_settings->CombatAltFOVFar,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Camera Offset##Combat")) {
			ImGui::PushID("CombatOffset");
			DrawOffsetSection(
				a_settings->CombatOverrideOffset,
				a_settings->CombatHorizontalOffsetMult,
				a_settings->CombatVerticalOffsetMult,
				a_anyChanged);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Camera Alignment##Combat")) {
			ImGui::PushID("CombatAlign");
			CheckboxSetting("Align Behind On Character Switch", a_settings->CombatAlignBehindOnSwitch, a_anyChanged);
			HelpMarker("When switching to a different party member, automatically rotate the camera\n"
				"to sit behind them, facing the same direction they are.\n"
				"After the initial alignment you can freely move the camera.");
			CheckboxSetting("Align Behind NPC", a_settings->CombatAlignBehindNPC, a_anyChanged);
			HelpMarker("During combat, continuously keep the camera behind NPCs during\n"
				"their turns by following their movement direction.\n"
				"Since we can't read NPC facing directly, the camera tracks\n"
				"their movement heading and stays behind them as they move.");
			CheckboxSetting("Disable Camera Pan", a_settings->CombatDisableCameraPan, a_anyChanged);
			HelpMarker("Block all camera panning/detach inputs during combat,\n"
				"keeping the camera centered on the character.");
			ImGui::PopID();
		}
	}

	static void DrawInputTab(Settings::Main* a_settings, bool& a_anyChanged)
	{
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Mouse", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("Mouse");

			DragSetting("Camera Rotation Mult", a_settings->MouseCameraRotationMult, 0.01f, 0.01f, 10.0f, a_anyChanged);
			HelpMarker("Multiplier for mouse camera rotation speed.");

			DragSetting("Pitch Mult", a_settings->MousePitchMult, 0.01f, 0.01f, 5.0f, a_anyChanged);
			HelpMarker("Multiplier for mouse pitch adjustment speed.");

			DragSetting("Zoom Mult", a_settings->MouseZoomMult, 0.01f, 0.01f, 5.0f, a_anyChanged);
			HelpMarker("Multiplier for mouse zoom speed.");

			CheckboxSetting("Invert Mouse Pitch", a_settings->InvertMousePitch, a_anyChanged);

			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Keyboard", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("Keyboard");

			DragSetting("Camera Rotation Mult", a_settings->KeyboardCameraRotationMult, 0.01f, 0.01f, 10.0f, a_anyChanged);
			HelpMarker("Multiplier for keyboard camera rotation speed.");

			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Controller", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("Controller");

			DragSetting("Camera Rotation Mult", a_settings->ControllerCameraRotationMult, 0.01f, 0.01f, 10.0f, a_anyChanged);
			HelpMarker("Multiplier for controller camera rotation speed.");

			DragSetting("Pitch Mult", a_settings->ControllerPitchMult, 0.01f, 0.01f, 5.0f, a_anyChanged);
			HelpMarker("Multiplier for controller pitch adjustment speed.");

			DragSetting("Zoom Mult", a_settings->ControllerZoomMult, 0.01f, 0.01f, 5.0f, a_anyChanged);
			HelpMarker("Multiplier for controller zoom speed.");

			CheckboxSetting("Invert Controller Pitch", a_settings->InvertControllerPitch, a_anyChanged);

			CheckboxSetting("Swap Zoom and Pitch", a_settings->SwapZoomAndPitch, a_anyChanged);
			HelpMarker("Swap the zoom and pitch controls on the right stick.");

			CheckboxSetting("Use Right Stick Press for Zoom", a_settings->UseRightStickPressForZoom, a_anyChanged);
			HelpMarker("Use right stick press (R3) to toggle zoom mode instead of left stick press.");

			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Controller Deadzone")) {
			ImGui::PushID("Deadzone");

			CheckboxSetting("Override Right Stick Deadzone", a_settings->OverrideRightStickDeadzone, a_anyChanged);
			HelpMarker("Override the default right stick deadzone.");

			if (*a_settings->OverrideRightStickDeadzone) {
				ImGui::Indent(10.0f);
				SliderSetting("Deadzone", a_settings->NewDeadzone, 0.0f, 0.95f, a_anyChanged, "%.3f");
				ImGui::Unindent(10.0f);
			}

			ImGui::PopID();
		}
	}

	 // file save status feedback
	static std::string s_saveStatusMessage;
	static bool        s_saveStatusIsError      = false;
	static float       s_saveStatusTimer        = 0.0f;
	static bool        s_saveStatusScrollNeeded = false;

	static void SetSaveStatus(std::string_view a_message, bool a_isError)
	{
		s_saveStatusMessage      = a_message;
		s_saveStatusIsError      = a_isError;
		s_saveStatusTimer        = 4.0f;
		s_saveStatusScrollNeeded = true;
	}

	 // Save to file 

	static std::string FormatDouble(double a_val)
	{
		std::string s = std::format("{}", a_val);
		if (s.find('.') == std::string::npos && s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
			s += ".0";
		}
		return s;
	}

	static std::string FormatBool(bool a_val)
	{
		return a_val ? "true" : "false";
	}

	static bool SaveSettingsToFile(Settings::Main* a_settings)
	{
		const auto basePath = std::filesystem::current_path() / CONFIG_PATH;
		const auto tmpPath  = std::filesystem::current_path() /
							  (std::string(CONFIG_PATH) + ".tmp");

		{
			std::ofstream file(tmpPath, std::ios::trunc);
			if (!file.is_open()) {
				const auto err = errno;
				ERROR("Failed to open config file for writing: {} (errno: {})", tmpPath.string(), err)
				if (err == EACCES || err == EPERM) {
					SetSaveStatus("Save failed: no write permission for the NativeMods folder. "
								  "Is the game installed in a protected directory (e.g. Program Files)?",
								  true);
				} else {
					SetSaveStatus(std::format("Save failed: could not open config file (errno: {})", err), true);
				}
				return false;
			}

			file << "# Native Camera Tweaks Configuration\n";
			file << "# Saved from in-game menu\n\n";

			file << "[General]\n";
			file << "UnlockedPitchInitialValue = " << FormatDouble(*a_settings->UnlockedPitchInitialValue) << "\n";
			file << "UnlockedPitchClampSpeed = " << FormatDouble(*a_settings->UnlockedPitchClampSpeed) << "\n";
			file << "UnlockedPitchLimitClipping = " << FormatBool(*a_settings->UnlockedPitchLimitClipping) << "\n";
			file << "UnlockedPitchFloorOffset = " << FormatDouble(*a_settings->UnlockedPitchFloorOffset) << "\n";
			file << "ResetZoomOnZoneChange = " << FormatBool(*a_settings->ResetZoomOnZoneChange) << "\n";
			file << "WatchForConfigChanges = " << FormatBool(*a_settings->WatchForConfigChanges) << "\n";
			file << "ToggleMenuKey = \"" << *a_settings->ToggleMenuKey << "\"\n";
			file << "EnableDebugMode = " << FormatBool(*a_settings->EnableDebugMode) << "\n";
			file << "\n";

			file << "[ExplorationPitch]\n";
			file << "ExplorationUnlockPitch = " << FormatBool(*a_settings->ExplorationUnlockPitch) << "\n";
			file << "ExplorationKeepTacticalPitchLocked = " << FormatBool(*a_settings->ExplorationKeepTacticalPitchLocked) << "\n";
			file << "ExplorationUnlockedPitchMin = " << FormatDouble(*a_settings->ExplorationUnlockedPitchMin) << "\n";
			file << "ExplorationUnlockedPitchMax = " << FormatDouble(*a_settings->ExplorationUnlockedPitchMax) << "\n";
			file << "ExplorationOverrideLockedPitch = " << FormatBool(*a_settings->ExplorationOverrideLockedPitch) << "\n";
			file << "ExplorationLockedPitchClose = " << FormatDouble(*a_settings->ExplorationLockedPitchClose) << "\n";
			file << "ExplorationLockedPitchFar = " << FormatDouble(*a_settings->ExplorationLockedPitchFar) << "\n";
			file << "ExplorationLockedTacticalPitchClose = " << FormatDouble(*a_settings->ExplorationLockedTacticalPitchClose) << "\n";
			file << "ExplorationLockedTacticalPitchFar = " << FormatDouble(*a_settings->ExplorationLockedTacticalPitchFar) << "\n";
			file << "ExplorationLockedAltPitchClose = " << FormatDouble(*a_settings->ExplorationLockedAltPitchClose) << "\n";
			file << "ExplorationLockedAltPitchFar = " << FormatDouble(*a_settings->ExplorationLockedAltPitchFar) << "\n";
			file << "\n";

			file << "[ExplorationZoom]\n";
			file << "ExplorationOverrideZoom = " << FormatBool(*a_settings->ExplorationOverrideZoom) << "\n";
			file << "ExplorationZoomMin = " << FormatDouble(*a_settings->ExplorationZoomMin) << "\n";
			file << "ExplorationZoomMax = " << FormatDouble(*a_settings->ExplorationZoomMax) << "\n";
			file << "ExplorationTacticalZoomMin = " << FormatDouble(*a_settings->ExplorationTacticalZoomMin) << "\n";
			file << "ExplorationTacticalZoomMax = " << FormatDouble(*a_settings->ExplorationTacticalZoomMax) << "\n";
			file << "ExplorationAltZoomMin = " << FormatDouble(*a_settings->ExplorationAltZoomMin) << "\n";
			file << "ExplorationAltZoomMax = " << FormatDouble(*a_settings->ExplorationAltZoomMax) << "\n";
			file << "\n";

			file << "[ExplorationFOV]\n";
			file << "ExplorationOverrideFOV = " << FormatBool(*a_settings->ExplorationOverrideFOV) << "\n";
			file << "ExplorationFOVClose = " << FormatDouble(*a_settings->ExplorationFOVClose) << "\n";
			file << "ExplorationFOVFar = " << FormatDouble(*a_settings->ExplorationFOVFar) << "\n";
			file << "ExplorationTacticalFOV = " << FormatDouble(*a_settings->ExplorationTacticalFOV) << "\n";
			file << "ExplorationAltFOVClose = " << FormatDouble(*a_settings->ExplorationAltFOVClose) << "\n";
			file << "ExplorationAltFOVFar = " << FormatDouble(*a_settings->ExplorationAltFOVFar) << "\n";
			file << "\n";

			file << "[ExplorationOffset]\n";
			file << "ExplorationOverrideOffset = " << FormatBool(*a_settings->ExplorationOverrideOffset) << "\n";
			file << "ExplorationHorizontalOffsetMult = " << FormatDouble(*a_settings->ExplorationHorizontalOffsetMult) << "\n";
			file << "ExplorationVerticalOffsetMult = " << FormatDouble(*a_settings->ExplorationVerticalOffsetMult) << "\n";
			file << "\n";

			file << "[ExplorationCamera]\n";
			file << "ExplorationAlignBehindOnSwitch = " << FormatBool(*a_settings->ExplorationAlignBehindOnSwitch) << "\n";
			file << "ExplorationAlignBehindNPC = " << FormatBool(*a_settings->ExplorationAlignBehindNPC) << "\n";
			file << "ExplorationDisableCameraPan = " << FormatBool(*a_settings->ExplorationDisableCameraPan) << "\n";
			file << "\n";

			file << "[CombatPitch]\n";
			file << "CombatUnlockPitch = " << FormatBool(*a_settings->CombatUnlockPitch) << "\n";
			file << "CombatKeepTacticalPitchLocked = " << FormatBool(*a_settings->CombatKeepTacticalPitchLocked) << "\n";
			file << "CombatUnlockedPitchMin = " << FormatDouble(*a_settings->CombatUnlockedPitchMin) << "\n";
			file << "CombatUnlockedPitchMax = " << FormatDouble(*a_settings->CombatUnlockedPitchMax) << "\n";
			file << "CombatOverrideLockedPitch = " << FormatBool(*a_settings->CombatOverrideLockedPitch) << "\n";
			file << "CombatLockedPitchClose = " << FormatDouble(*a_settings->CombatLockedPitchClose) << "\n";
			file << "CombatLockedPitchFar = " << FormatDouble(*a_settings->CombatLockedPitchFar) << "\n";
			file << "CombatLockedTacticalPitchClose = " << FormatDouble(*a_settings->CombatLockedTacticalPitchClose) << "\n";
			file << "CombatLockedTacticalPitchFar = " << FormatDouble(*a_settings->CombatLockedTacticalPitchFar) << "\n";
			file << "CombatLockedAltPitchClose = " << FormatDouble(*a_settings->CombatLockedAltPitchClose) << "\n";
			file << "CombatLockedAltPitchFar = " << FormatDouble(*a_settings->CombatLockedAltPitchFar) << "\n";
			file << "\n";

			file << "[CombatZoom]\n";
			file << "CombatOverrideZoom = " << FormatBool(*a_settings->CombatOverrideZoom) << "\n";
			file << "CombatZoomMin = " << FormatDouble(*a_settings->CombatZoomMin) << "\n";
			file << "CombatZoomMax = " << FormatDouble(*a_settings->CombatZoomMax) << "\n";
			file << "CombatTacticalZoomMin = " << FormatDouble(*a_settings->CombatTacticalZoomMin) << "\n";
			file << "CombatTacticalZoomMax = " << FormatDouble(*a_settings->CombatTacticalZoomMax) << "\n";
			file << "CombatAltZoomMin = " << FormatDouble(*a_settings->CombatAltZoomMin) << "\n";
			file << "CombatAltZoomMax = " << FormatDouble(*a_settings->CombatAltZoomMax) << "\n";
			file << "\n";

			file << "[CombatFOV]\n";
			file << "CombatOverrideFOV = " << FormatBool(*a_settings->CombatOverrideFOV) << "\n";
			file << "CombatFOVClose = " << FormatDouble(*a_settings->CombatFOVClose) << "\n";
			file << "CombatFOVFar = " << FormatDouble(*a_settings->CombatFOVFar) << "\n";
			file << "CombatTacticalFOV = " << FormatDouble(*a_settings->CombatTacticalFOV) << "\n";
			file << "CombatAltFOVClose = " << FormatDouble(*a_settings->CombatAltFOVClose) << "\n";
			file << "CombatAltFOVFar = " << FormatDouble(*a_settings->CombatAltFOVFar) << "\n";
			file << "\n";

			file << "[CombatOffset]\n";
			file << "CombatOverrideOffset = " << FormatBool(*a_settings->CombatOverrideOffset) << "\n";
			file << "CombatHorizontalOffsetMult = " << FormatDouble(*a_settings->CombatHorizontalOffsetMult) << "\n";
			file << "CombatVerticalOffsetMult = " << FormatDouble(*a_settings->CombatVerticalOffsetMult) << "\n";
			file << "\n";

			file << "[CombatCamera]\n";
			file << "CombatAlignBehindOnSwitch = " << FormatBool(*a_settings->CombatAlignBehindOnSwitch) << "\n";
			file << "CombatAlignBehindNPC = " << FormatBool(*a_settings->CombatAlignBehindNPC) << "\n";
			file << "CombatDisableCameraPan = " << FormatBool(*a_settings->CombatDisableCameraPan) << "\n";
			file << "\n";

			file << "[Mouse]\n";
			file << "MouseCameraRotationMult = " << FormatDouble(*a_settings->MouseCameraRotationMult) << "\n";
			file << "MousePitchMult = " << FormatDouble(*a_settings->MousePitchMult) << "\n";
			file << "MouseZoomMult = " << FormatDouble(*a_settings->MouseZoomMult) << "\n";
			file << "InvertMousePitch = " << FormatBool(*a_settings->InvertMousePitch) << "\n";
			file << "\n";

			file << "[Keyboard]\n";
			file << "KeyboardCameraRotationMult = " << FormatDouble(*a_settings->KeyboardCameraRotationMult) << "\n";
			file << "\n";

			file << "[Controller]\n";
			file << "ControllerCameraRotationMult = " << FormatDouble(*a_settings->ControllerCameraRotationMult) << "\n";
			file << "ControllerPitchMult = " << FormatDouble(*a_settings->ControllerPitchMult) << "\n";
			file << "ControllerZoomMult = " << FormatDouble(*a_settings->ControllerZoomMult) << "\n";
			file << "InvertControllerPitch = " << FormatBool(*a_settings->InvertControllerPitch) << "\n";
			file << "SwapZoomAndPitch = " << FormatBool(*a_settings->SwapZoomAndPitch) << "\n";
			file << "UseRightStickPressForZoom = " << FormatBool(*a_settings->UseRightStickPressForZoom) << "\n";
			file << "\n";

			file << "[ControllerDeadzone]\n";
			file << "OverrideRightStickDeadzone = " << FormatBool(*a_settings->OverrideRightStickDeadzone) << "\n";
			file << "NewDeadzone = " << FormatDouble(*a_settings->NewDeadzone) << "\n";
			file << "\n";

			WriteProfilesToFile(file);

			file.close();
			if (file.fail()) {
				std::error_code ec;
				std::filesystem::remove(tmpPath, ec);
				ERROR("Failed to write config file (stream error): {}", tmpPath.string())
				SetSaveStatus("Save failed: an error occurred while writing the config file. "
							  "The disk may be full or the file may be locked.",
							  true);
				return false;
			}
		}

		// rename temp file to toml
		std::error_code ec;
		std::filesystem::rename(tmpPath, basePath, ec);
		if (ec) {
			std::filesystem::remove(tmpPath, ec);
			ERROR("Failed to rename config temp file: {} -> {}: ({}) {}",
				  tmpPath.string(), basePath.string(), ec.value(), ec.message())
			SetSaveStatus(std::format("Save failed: could not finalize the config file ({})", ec.message()), true);
			return false;
		}

		INFO("Config saved to file from in-game menu")
		SetSaveStatus("Settings saved successfully.", false);
		return true;
	}

	 // Debug / Memory Explorer 

	static void DrawVec3ReadOnly(const char* a_label, const RE::Vector3& a_vec)
	{
		ImGui::Text("%s: (%.4f, %.4f, %.4f)", a_label, a_vec.x, a_vec.y, a_vec.z);
	}

	static void DrawQuatReadOnly(const char* a_label, const RE::Quaternion& a_quat)
	{
		ImGui::Text("%s: (%.4f, %.4f, %.4f, %.4f)", a_label, a_quat.x, a_quat.y, a_quat.z, a_quat.w);
	}

	static const char* CameraModeFlagsToString(uint32_t a_flags)
	{
		static char buf[256];
		buf[0] = '\0';
		if (a_flags & RE::kCombat) strcat_s(buf, "Combat ");
		if (a_flags & RE::kTactical) strcat_s(buf, "Tactical ");
		if (a_flags & RE::kMouseRotation) strcat_s(buf, "MouseRot ");
		if (a_flags & RE::kUnk2) strcat_s(buf, "Unk2 ");
		if (a_flags & RE::kUnk8) strcat_s(buf, "Unk8 ");
		if (a_flags & RE::kUnk10) strcat_s(buf, "Unk10 ");
		if (a_flags & RE::kUnk20) strcat_s(buf, "Unk20 ");
		if (a_flags & RE::kUnk40) strcat_s(buf, "Unk40 ");
		if (a_flags & RE::kUnk80) strcat_s(buf, "Unk80 ");
		if (buf[0] == '\0') strcpy_s(buf, "(none)");
		return buf;
	}

	static float QuatToYawDegrees(const RE::Quaternion& q)
	{
		float siny_cosp = 2.0f * (q.w * q.y + q.x * q.z);
		float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.x * q.x);
		float yaw = atan2f(-siny_cosp, -cosy_cosp);
		return yaw * CameraTweaks::RAD_TO_DEG;
	}

	static void DrawDebugTab()
	{
		ImGui::Spacing();

		const auto cameraTweaks = CameraTweaks::GetSingleton();
		const auto debugData = cameraTweaks->GetDebugData();
		const auto& cam = debugData.camera;
		const auto& cd = debugData.cameraDef;

		if (ImGui::CollapsingHeader("Camera Object State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(10.0f);

			if (cam.valid) {
				DrawVec3ReadOnly("Camera Root Pos", cam.cameraRootPos);
				DrawVec3ReadOnly("Camera Pos", cam.cameraPos);
				DrawVec3ReadOnly("Camera Rotation Vec", cam.cameraRotation);

				ImGui::Text("Current Zoom: %.4f", cam.currentZoomB);
				ImGui::Text("Desired Zoom: %.4f", cam.desiredZoom);
				ImGui::Text("Current Pitch: %.4f", cam.currentPitch);
				ImGui::Text("Camera Angle: %.4f rad", cam.cameraAngle);
				ImGui::Text("Mode Flags: 0x%X [%s]", cam.cameraModeFlags, CameraModeFlagsToString(cam.cameraModeFlags));
				ImGui::Text("Delta Time: %.6f", debugData.deltaTime);

				ImGui::Separator();
				ImGui::Text("CameraObject*: 0x%p", cam.ptr);
				DrawVec3ReadOnly("Desired Root Pos", cam.desiredCameraRootPos);
				ImGui::Text("Horizontal Pan Delta: %.4f", cam.horizontalPanDelta);
				ImGui::Text("Vertical Pan Delta: %.4f", cam.verticalPanDelta);
				ImGui::Text("Current Angle Delta: %.4f", cam.currentAngleDelta);
				ImGui::Text("Mouse Rotation Delta: %.4f", cam.mouseRotationDelta);
				ImGui::Text("Zoom Delta: %.4f", cam.zoomDelta);
				ImGui::Text("Rotation Speed: %.4f", cam.rotationSpeed);
				ImGui::Text("Vertical Offset: %.4f", cam.verticalOffset);
				ImGui::Text("Prev Zoom: %.4f", cam.prevZoom_15C);
				ImGui::Text("Timer_140: %.4f", cam.timer_140);
				DrawVec3ReadOnly("CameraPos_150", cam.cameraPos_150);
				DrawVec3ReadOnly("UnkCameraRootPosA", cam.unkCameraRootPosA);
				DrawVec3ReadOnly("UnkCameraRootPosB", cam.unkCameraRootPosB);
			} else {
				ImGui::TextDisabled("No active camera object");
			}

			ImGui::Unindent(10.0f);
		}

		if (ImGui::CollapsingHeader("Camera Definition")) {
			ImGui::Indent(10.0f);

			if (cd.valid) {
				ImGui::Text("CameraDefinition*: 0x%p", cd.ptr);
				ImGui::Separator();

				ImGui::TextColored(kGoldHeader, "Zoom");
				ImGui::Text("Max Zoom: %.4f", cd.maxZoom_28);
				ImGui::Text("Min Zoom: %.4f", cd.minZoom_2C);
				ImGui::Text("Alt Max Zoom (Controller): %.4f", cd.altMaxZoomController_30);
				ImGui::Text("Alt Min Zoom (Controller): %.4f", cd.altMinZoomController_34);
				ImGui::Text("Zoom Speed: %.4f", cd.zoomSpeed_AC);
				ImGui::Text("Scroll Speed: %.4f", cd.scrollSpeed_B0);
				ImGui::Text("Default Zoom: %.4f", cd.unkDefaultZoom_24);
				ImGui::Text("Tact Default Zoom: %.4f", cd.tactDefaultZoom_C4);
				ImGui::Text("Tact Min Zoom: %.4f", cd.tactMinZoom_C8);
				ImGui::Text("Tact Max Zoom: %.4f", cd.tactMaxZoom_CC);

				ImGui::Spacing();
				ImGui::TextColored(kGoldHeader, "FOV");
				ImGui::Text("FOV Close: %.4f", cd.fovClose_84);
				ImGui::Text("FOV Far: %.4f", cd.fovFar_88);
				ImGui::Text("FOV Close Alt: %.4f", cd.fovCloseAlt_8C);
				ImGui::Text("FOV Far Alt: %.4f", cd.fovFarAlt_90);
				ImGui::Text("Tactical FOV: %.4f", cd.tacticalFov_D0);

				ImGui::Spacing();
				ImGui::TextColored(kGoldHeader, "Pitch");
				ImGui::Text("Pitch Far: %.4f", cd.pitchFar_160);
				ImGui::Text("Pitch Close: %.4f", cd.pitchClose_164);
				ImGui::Text("Pitch Combat Far: %.4f", cd.pitchCombatFar_168);
				ImGui::Text("Pitch Combat Close: %.4f", cd.pitchCombatClose_16C);
				ImGui::Text("Tact Pitch Far: %.4f", cd.tacticalPitchFar_170);
				ImGui::Text("Tact Pitch Close: %.4f", cd.tacticalPitchClose_174);
				ImGui::Text("Pitch Far Alt: %.4f", cd.pitchFarAlt_178);
				ImGui::Text("Pitch Close Alt: %.4f", cd.pitchCloseAlt_17C);
				ImGui::Text("Pitch Adjust Speed A: %.4f", cd.pitchAdjustSpeedA_48);
				ImGui::Text("Pitch Adjust Speed B: %.4f", cd.pitchAdjustSpeedB_F0);
				ImGui::Text("Pitch Adjust Speed C: %.4f", cd.pitchAdjustSpeedC_F4);

				ImGui::Spacing();
				ImGui::TextColored(kGoldHeader, "Offsets");
				ImGui::Text("Horizontal Offset Mult: %.4f", cd.camHorizontalOffsetMult_64);
				ImGui::Text("Vertical Offset Mult: %.4f", cd.camVerticalOffsetMult_68);
				ImGui::Text("Alt Horiz Offset (Controller): %.4f", cd.altCamHorizontalOffsetMultController_70);
				ImGui::Text("Alt Vert Offset (Controller): %.4f", cd.altCamVerticalOffsetMultController_74);
				ImGui::Text("Max Cam Distance From Root: %.4f", cd.maxCamDistanceFromRoot_D4);
			} else {
				ImGui::TextDisabled("CameraDefinition unavailable");
			}

			ImGui::Unindent(10.0f);
		}

		if (ImGui::CollapsingHeader("Player / Game State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(10.0f);

			ImGui::Text("Player ID: %d", debugData.playerId);
			ImGui::Text("Controller Mode: %s", debugData.isControllerMode ? "Yes" : "No");
			ImGui::Text("Player*: 0x%p", debugData.playerPtr);
			ImGui::Text("Camera Singleton*: 0x%p", debugData.cameraSingletonPtr);
			ImGui::Text("UnkSingleton*: 0x%p", debugData.unkSingletonPtr);

			ImGui::Unindent(10.0f);
		}

		if (ImGui::CollapsingHeader("Derived Character Heading", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(10.0f);

			if (debugData.hasEntityRotation) {
				ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Entity Rotation Available (offset +0x%X)", debugData.entityTransformOffset);
				ImGui::Text("Entity Heading: %.2f deg  (%.4f rad)", debugData.entityHeadingDeg, debugData.entityHeadingRad);
				ImGui::Text("Quat: (%.3f, %.3f, %.3f, %.3f)",
					debugData.entityRotationQuat.x, debugData.entityRotationQuat.y,
					debugData.entityRotationQuat.z, debugData.entityRotationQuat.w);
				ImGui::Separator();
			} else if (debugData.characterPtr != 0) {
				ImGui::TextDisabled("Entity ptr found but no matching Transform (rotation unavailable)");
			}

			if (debugData.hasHeading) {
				static float camNorthOffsetDeg = 45.0f;
				ImGui::SliderFloat("Camera North Offset (deg)", &camNorthOffsetDeg, -180.f, 180.f, "%.1f");
				HelpMarker("Offset between the camera angle's zero and true north.\n"
					"Adjust until the blue compass line aligns with green when\n"
					"facing the camera in the same direction as movement.");

				const float camOffsetDeg = camNorthOffsetDeg;
				const float camForCompass = (cam.cameraAngle - camOffsetDeg) * CameraTweaks::DEG_TO_RAD;

				ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Heading: %.2f deg  (%.4f rad)", debugData.derivedHeadingDeg, debugData.derivedHeadingRad);
				ImGui::Text("Camera Angle: %.2f", cam.cameraAngle);
				ImGui::Separator();
				DrawVec3ReadOnly("Movement Delta", debugData.movementDelta);
				ImGui::Text("Movement Speed: %.4f units/sec", debugData.movementSpeed);

				ImGui::Spacing();
				ImGui::TextColored(kGoldHeader, "Compass");
				ImVec2 compassCenter = ImGui::GetCursorScreenPos();
				const float radius = 50.0f;
				compassCenter.x += radius + 10.0f;
				compassCenter.y += radius + 5.0f;
				ImGui::Dummy(ImVec2(radius * 2 + 20, radius * 2 + 15));

				auto* drawList = ImGui::GetWindowDrawList();
				drawList->AddCircle(compassCenter, radius, IM_COL32(100, 100, 100, 200), 32);

				drawList->AddText(ImVec2(compassCenter.x - 3, compassCenter.y - radius - 14), IM_COL32(180, 180, 180, 255), "N");
				drawList->AddText(ImVec2(compassCenter.x - 3, compassCenter.y + radius + 2), IM_COL32(180, 180, 180, 255), "S");
				drawList->AddText(ImVec2(compassCenter.x + radius + 4, compassCenter.y - 6), IM_COL32(180, 180, 180, 255), "E");
				drawList->AddText(ImVec2(compassCenter.x - radius - 12, compassCenter.y - 6), IM_COL32(180, 180, 180, 255), "W");

				// Character heading line (green)
				float screenAngle = debugData.derivedHeadingRad;
				ImVec2 headingEnd;
				headingEnd.x = compassCenter.x + sinf(screenAngle) * (radius - 5);
				headingEnd.y = compassCenter.y - cosf(screenAngle) * (radius - 5);
				drawList->AddLine(compassCenter, headingEnd, IM_COL32(50, 255, 50, 255), 2.5f);

				// Camera direction line (blue)
				ImVec2 camEnd;
				camEnd.x = compassCenter.x + sinf(camForCompass) * (radius - 5);
				camEnd.y = compassCenter.y - cosf(camForCompass) * (radius - 5);
				drawList->AddLine(compassCenter, camEnd, IM_COL32(80, 140, 255, 255), 2.0f);

				// Entity heading line (orange/red) 
				if (debugData.hasEntityRotation) {
					ImVec2 entityEnd;
					entityEnd.x = compassCenter.x + sinf(debugData.entityHeadingRad) * (radius - 5);
					entityEnd.y = compassCenter.y - cosf(debugData.entityHeadingRad) * (radius - 5);
					drawList->AddLine(compassCenter, entityEnd, IM_COL32(255, 120, 50, 255), 2.0f);
				}

				// Behind-entity target angle where camera should be placed
				if (debugData.hasBehindEntityAngle) {
					const float behindForCompass = (debugData.behindEntityAngleDeg - camOffsetDeg) * CameraTweaks::DEG_TO_RAD;
					ImVec2 behindEnd;
					behindEnd.x = compassCenter.x + sinf(behindForCompass) * (radius - 10);
					behindEnd.y = compassCenter.y - cosf(behindForCompass) * (radius - 10);
					drawList->AddLine(compassCenter, behindEnd, IM_COL32(255, 255, 80, 180), 1.5f);
				}

				ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Green = Character Heading");
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.3f, 0.55f, 1.0f, 1.0f), "Blue = Camera Angle");
				if (debugData.hasEntityRotation) {
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(1.0f, 0.47f, 0.2f, 1.0f), "Orange = Entity Rotation");
				}
				if (debugData.hasBehindEntityAngle) {
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "Yellow = Behind Target");
					ImGui::Text("Behind-Entity Angle: %.2f deg", debugData.behindEntityAngleDeg);
				}
				if (debugData.isTrackingNPC) {
					ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[NPC TRACKING]");
					if (debugData.hasBehindMovementAngle) {
						ImGui::SameLine();
						ImGui::Text("Behind-Movement Angle: %.2f deg", debugData.behindMovementAngleDeg);
					}
				}
			} else {
				ImGui::TextDisabled("Heading not available yet (move your character)");
			}

			ImGui::Unindent(10.0f);
		}

		if (ImGui::CollapsingHeader("Character Entity Explorer")) {
			ImGui::Indent(10.0f);

			ImGui::Text("GetCharacter result: 0x%llX", debugData.characterPtr);
			ImGui::Text("GetCharacter singleton: 0x%p", debugData.getCharacterSingletonPtr);

			if (debugData.hasEntityRotation) {
				ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Entity Transform FOUND (offset info: 0x%X)", debugData.entityTransformOffset);
				DrawQuatReadOnly("  Entity Rotation", debugData.entityRotationQuat);
				DrawVec3ReadOnly("  Entity Translate", debugData.entityTranslate);
				ImGui::Text("  Entity Heading: %.2f deg  (%.4f rad)", debugData.entityHeadingDeg, debugData.entityHeadingRad);
			} else if (debugData.characterPtr != 0) {
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Entity transform NOT found (deep scan with pointer-following)");
			}

			if (debugData.characterPtr != 0) {
				SectionSeparator();

				ImGui::TextColored(kGoldHeader, "Pointer Map (sub-objects from characterPtr)");
				HelpMarker(
					"Shows pointer-like values found in the character object.\n"
					"These are sub-objects that the backend scanner follows\n"
					"when searching for the Transform component."
				);

				static int ptrScanRange = 0x100;
				ImGui::SliderInt("Ptr Scan Range", &ptrScanRange, 0x40, 0x400, "0x%X");

				if (ImGui::BeginChild("##PtrMap", ImVec2(0, 200), true)) {
					for (int off = 0; off <= ptrScanRange; off += 8) {
						uintptr_t val = 0;
						if (!CameraTweaks::SafeReadPtr(debugData.characterPtr + off, val)) continue;
						
						// Check if it looks like a pointer
						bool isPtr = val > 0x10000 && (val & 0x3) == 0 && val != debugData.characterPtr;
						if (isPtr) {
							ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), "+0x%03X: 0x%llX  [PTR]", off, val);
						}
					}
				}
				ImGui::EndChild();

				SectionSeparator();

				ImGui::TextColored(kGoldHeader, "Manual Transform Scanner");
				HelpMarker(
					"Scans from a chosen base address for Transform structs.\n"
					"Pick a sub-object pointer from the map above, paste it,\n"
					"and scan that sub-object for matching transforms."
				);

				const RE::Vector3& camRoot = debugData.camera.cameraRootPos;

				static int scanStart = 0;
				static int scanEnd = 0x800;
				static float posThreshold = 5.0f;
				static bool followPointers = true;
				ImGui::DragIntRange2("Scan Offset Range", &scanStart, &scanEnd, 4, 0, 0x2000, "0x%X", "0x%X");
				ImGui::SliderFloat("Position Match Threshold", &posThreshold, 0.5f, 50.0f);
				ImGui::Checkbox("Also follow pointers (1 level)", &followPointers);

				struct TransformCandidate
				{
					uintptr_t baseAddr;  // object we found it in
					int ptrOffset;       // offset of the pointer we followed (-1 = direct)
					int offset;
					RE::Quaternion quat;
					RE::Vector3 translate;
					RE::Vector3 scale;
					float quatLength;
					float posDistance;
				};
				std::vector<TransformCandidate> candidates;

				auto scanBase = [&](uintptr_t baseAddr, int ptrOffset) {
					for (int offset = scanStart; offset <= scanEnd - static_cast<int>(sizeof(RE::Transform)); offset += 4) {
						RE::Quaternion quat;
						RE::Vector3 translate;
						RE::Vector3 scale;

						if (!CameraTweaks::SafeReadQuat(baseAddr + offset, quat)) continue;
						if (!CameraTweaks::SafeReadVec3(baseAddr + offset + 0x10, translate)) continue;
						if (!CameraTweaks::SafeReadVec3(baseAddr + offset + 0x1C, scale)) continue;

						// Reject NaN/Inf
						if (!std::isfinite(quat.x) || !std::isfinite(quat.y) || !std::isfinite(quat.z) || !std::isfinite(quat.w)) continue;
						if (!std::isfinite(translate.x) || !std::isfinite(translate.y) || !std::isfinite(translate.z)) continue;

						float quatLen = sqrtf(quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w);
						if (!(quatLen >= 0.9f && quatLen <= 1.1f)) continue;

						float dx = translate.x - camRoot.x;
						float dy = translate.y - camRoot.y;
						float dz = translate.z - camRoot.z;
						float dist = sqrtf(dx * dx + dy * dy + dz * dz);

						if (std::isfinite(dist) && dist < posThreshold) {
							candidates.push_back({ baseAddr, ptrOffset, offset, quat, translate, scale, quatLen, dist });
						}
					}
				};

				// Scan the character object directly
				scanBase(debugData.characterPtr, -1);

				// Follow pointers and scan sub-objects
				if (followPointers) {
					for (int off = 0; off <= 0x200; off += 8) {
						uintptr_t subPtr = 0;
						if (!CameraTweaks::SafeReadPtr(debugData.characterPtr + off, subPtr)) continue;
						if (subPtr < 0x10000 || subPtr == debugData.characterPtr || (subPtr & 0x3) != 0) continue;
						scanBase(subPtr, off);
					}
				}

				if (!candidates.empty()) {
					ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Found %d transform candidate(s):", static_cast<int>(candidates.size()));
					for (const auto& c : candidates) {
						ImGui::PushID(static_cast<int>(c.baseAddr + c.offset));
						const char* via = (c.ptrOffset < 0) ? "direct" : "";
						char label[128];
						if (c.ptrOffset < 0) {
							snprintf(label, sizeof(label), "Direct +0x%X (dist=%.2f)", c.offset, c.posDistance);
						} else {
							snprintf(label, sizeof(label), "Via ptr +0x%X -> +0x%X (dist=%.2f, base=0x%llX)", c.ptrOffset, c.offset, c.posDistance, c.baseAddr);
						}
						if (ImGui::TreeNode("", "%s", label)) {
							DrawQuatReadOnly("  RotationQuat", c.quat);
							float yaw = QuatToYawDegrees(c.quat);
							ImGui::Text("  Heading (Yaw): %.2f deg", yaw);
							DrawVec3ReadOnly("  Translate", c.translate);
							DrawVec3ReadOnly("  Scale", c.scale);
							ImGui::Text("  Quat Length: %.6f", c.quatLength);
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
				} else {
					ImGui::TextDisabled("No transform candidates found (direct + pointer-following).");
				}

				SectionSeparator();

				 // Process-Wide Memory Scan 
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Process-Wide Transform Scan");
				HelpMarker(
					"Scans readable memory in the game process for Transform structs\n"
					"whose translate matches cameraRootPos. Should find the Transform\n"
					"regardless of how many pointer hops away it is from the entity.\n\n"
					"Takes a few seconds and the game will freeze."
				);

				auto* cameraTweaks = CameraTweaks::GetSingleton();
				if (cameraTweaks->IsMemoryScanRunning()) {
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "Scanning... (game may freeze)");
				} else {
					if (ImGui::Button("Scan Entire Process Memory")) {
						cameraTweaks->RequestMemoryScan();
					}
					ImGui::SameLine();
					ImGui::TextDisabled("(searches for position %.1f, %.1f, %.1f)",
						debugData.camera.cameraRootPos.x, debugData.camera.cameraRootPos.y, debugData.camera.cameraRootPos.z);

					const auto& scanResults = cameraTweaks->GetMemoryScanResults();
					if (!scanResults.empty()) {
						auto scanPos = cameraTweaks->GetMemoryScanSearchPos();
						ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Found %d match(es) for pos (%.1f, %.1f, %.1f):",
							static_cast<int>(scanResults.size()), scanPos.x, scanPos.y, scanPos.z);

						if (ImGui::BeginChild("##MemScanResults", ImVec2(0, 300), true)) {
							for (int i = 0; i < static_cast<int>(scanResults.size()); ++i) {
								const auto& r = scanResults[i];
								ImGui::PushID(i);

								int64_t distFromEntity = static_cast<int64_t>(r.address) - static_cast<int64_t>(debugData.characterPtr);

								char label[256];
								snprintf(label, sizeof(label), "0x%llX  (dist=%.2f, heading=%.1f deg, %+lld from entity)",
									r.address, r.posDistance, r.headingDeg, distFromEntity);

								if (ImGui::TreeNode("", "%s", label)) {
									DrawQuatReadOnly("  RotationQuat", r.rotationQuat);
									ImGui::Text("  Heading (Yaw): %.2f deg", r.headingDeg);
									DrawVec3ReadOnly("  Translate", r.translate);
									DrawVec3ReadOnly("  Scale", r.scale);
									ImGui::Text("  Quat Length: %.6f", r.quatLength);
									ImGui::Text("  Abs Address: 0x%llX", r.address);
									ImGui::Text("  Distance from characterPtr: %+lld (0x%llX)", distFromEntity, distFromEntity >= 0 ? distFromEntity : -distFromEntity);

									// Try to help find the pointer path
									if (debugData.characterPtr != 0) {
										ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Nearby pointers to this region:");
										// Check if any pointer in characterPtr's first 0x200 bytes points within 0x800 of this address
										for (int pOff = 0; pOff <= 0x200; pOff += 8) {
											uintptr_t pVal = 0;
											if (!CameraTweaks::SafeReadPtr(debugData.characterPtr + pOff, pVal)) continue;
											if (pVal < 0x10000) continue;
											int64_t delta = static_cast<int64_t>(r.address) - static_cast<int64_t>(pVal);
											if (delta >= 0 && delta < 0x1000) {
												ImGui::Text("    entity+0x%X -> 0x%llX (transform at +0x%llX from that ptr)",
													pOff, pVal, static_cast<uint64_t>(delta));
											}
										}
									}
									ImGui::TreePop();
								}
								ImGui::PopID();
							}
						}
						ImGui::EndChild();
					}
				}

				SectionSeparator();

				ImGui::TextColored(kGoldHeader, "Raw Float Viewer");
				HelpMarker("Displays raw float values at offsets from the character pointer. Useful for finding unknown fields.");

				static int rawViewStart = 0;
				static int rawViewCount = 32;
				ImGui::DragInt("View Start Offset", &rawViewStart, 4, 0, 0x4000, "0x%X");
				ImGui::SliderInt("Float Count", &rawViewCount, 8, 128);

				if (ImGui::BeginChild("##FloatDump", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar)) {
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
					for (int i = 0; i < rawViewCount; ++i) {
						int off = rawViewStart + i * 4;
						float val;
						if (CameraTweaks::SafeReadFloat(debugData.characterPtr + off, val)) {
							// Color valid-looking floats
							bool isInteresting = (fabsf(val) > 0.001f && fabsf(val) < 10000.0f);
							if (isInteresting) {
								ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.5f, 1.0f), "+0x%04X: %12.4f  (0x%08X)", off, val, *reinterpret_cast<uint32_t*>(&val));
							} else {
								ImGui::TextDisabled("+0x%04X: %12.4f  (0x%08X)", off, val, *reinterpret_cast<uint32_t*>(&val));
							}
						} else {
							ImGui::TextDisabled("+0x%04X: <inaccessible>", off);
						}
					}
					ImGui::PopStyleVar();
				}
				ImGui::EndChild();
			} else {
				ImGui::TextDisabled("Character entity not available (not in-game or GetCharacter returned null)");
			}

			ImGui::Unindent(10.0f);
		}

		if (ImGui::CollapsingHeader("Singleton Pointers & Offsets")) {
			ImGui::Indent(10.0f);

			// Function pointers
			ImGui::Text("GetCurrentCameraDefinition: 0x%p", Hooks::Offsets::GetCurrentCameraDefinition);
			ImGui::Text("ShouldShowSneakCones: 0x%p", Hooks::Offsets::ShouldShowSneakCones);
			ImGui::Text("GetCharacter: 0x%p", Hooks::Offsets::GetCharacter);
			ImGui::Text("GetCharacterHeight: 0x%p", Hooks::Offsets::GetCharacterHeight);
			ImGui::Text("GetPlayerController: 0x%p", Hooks::Offsets::GetPlayerController);
			ImGui::Text("GetInputValue: 0x%p", Hooks::Offsets::GetInputValue);
			ImGui::Text("GetCameraMinZoom: 0x%p", Hooks::Offsets::GetCameraMinZoom);
			ImGui::Text("GetFloorLevel: 0x%p", Hooks::Offsets::GetFloorLevel);
			ImGui::Separator();
			// Singleton values cache
			ImGui::Text("CameraSingleton*: 0x%p", debugData.cameraSingletonPtr);
			ImGui::Text("UnkSingleton*: 0x%p", debugData.unkSingletonPtr);
			ImGui::Text("GetCharacterSingleton*: 0x%p", debugData.getCharacterSingletonPtr);
			if (debugData.getCharacterSingletonPtr == debugData.unkSingletonPtr && debugData.getCharacterSingletonPtr != nullptr) {
				ImGui::SameLine();
				ImGui::TextDisabled("(same as UnkSingleton)");
			}
			ImGui::Separator();
			ImGui::Text("cameraBoolOffset: 0x%X", Hooks::Offsets::cameraBoolOffset);
			ImGui::Text("unkCameraOffset: 0x%X", Hooks::Offsets::unkCameraOffset);
			ImGui::Text("explorationCameraOffset: 0x%X", Hooks::Offsets::explorationCameraOffset);
			ImGui::Text("combatCameraOffset: 0x%X", Hooks::Offsets::combatCameraOffset);

			ImGui::Unindent(10.0f);
		}
	}

	 // Theme 

	void ApplyBG3Theme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		style.WindowRounding = 6.0f;
		style.ChildRounding = 4.0f;
		style.FrameRounding = 3.0f;
		style.PopupRounding = 4.0f;
		style.ScrollbarRounding = 4.0f;
		style.GrabRounding = 3.0f;
		style.TabRounding = 4.0f;

		style.WindowPadding = ImVec2(12.0f, 12.0f);
		style.FramePadding = ImVec2(8.0f, 4.0f);
		style.ItemSpacing = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
		style.IndentSpacing = 20.0f;
		style.ScrollbarSize = 14.0f;
		style.GrabMinSize = 12.0f;

		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;

		// Colors

		colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.87f, 0.78f, 1.00f);
		colors[ImGuiCol_TextDisabled]          = ImVec4(0.55f, 0.50f, 0.42f, 1.00f);

		colors[ImGuiCol_WindowBg]              = ImVec4(0.08f, 0.07f, 0.06f, 0.96f);
		colors[ImGuiCol_ChildBg]               = ImVec4(0.10f, 0.09f, 0.07f, 0.50f);
		colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.09f, 0.07f, 0.96f);

		colors[ImGuiCol_Border]                = ImVec4(0.42f, 0.33f, 0.19f, 0.55f);
		colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.13f, 0.10f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.22f, 0.18f, 0.12f, 1.00f);
		colors[ImGuiCol_FrameBgActive]         = ImVec4(0.28f, 0.22f, 0.14f, 1.00f);

		colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.08f, 0.06f, 1.00f);
		colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.14f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.08f, 0.07f, 0.05f, 0.75f);

		colors[ImGuiCol_MenuBarBg]             = ImVec4(0.12f, 0.10f, 0.08f, 1.00f);

		colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.07f, 0.06f, 0.60f);
		colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.40f, 0.32f, 0.18f, 0.80f);
		colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.52f, 0.42f, 0.22f, 0.90f);
		colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.62f, 0.50f, 0.26f, 1.00f);

		colors[ImGuiCol_CheckMark]             = ImVec4(0.86f, 0.68f, 0.30f, 1.00f);

		colors[ImGuiCol_SliderGrab]            = ImVec4(0.62f, 0.50f, 0.26f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.80f, 0.64f, 0.30f, 1.00f);

		colors[ImGuiCol_Button]                = ImVec4(0.30f, 0.24f, 0.14f, 0.85f);
		colors[ImGuiCol_ButtonHovered]         = ImVec4(0.44f, 0.35f, 0.18f, 1.00f);
		colors[ImGuiCol_ButtonActive]          = ImVec4(0.56f, 0.44f, 0.22f, 1.00f);

		colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.20f, 0.12f, 0.80f);
		colors[ImGuiCol_HeaderHovered]         = ImVec4(0.38f, 0.30f, 0.16f, 0.85f);
		colors[ImGuiCol_HeaderActive]          = ImVec4(0.48f, 0.38f, 0.20f, 1.00f);

		colors[ImGuiCol_Separator]             = ImVec4(0.42f, 0.33f, 0.19f, 0.40f);
		colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.58f, 0.46f, 0.24f, 0.70f);
		colors[ImGuiCol_SeparatorActive]       = ImVec4(0.72f, 0.57f, 0.28f, 1.00f);

		colors[ImGuiCol_ResizeGrip]            = ImVec4(0.42f, 0.33f, 0.19f, 0.30f);
		colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.58f, 0.46f, 0.24f, 0.60f);
		colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.72f, 0.57f, 0.28f, 0.90f);

		colors[ImGuiCol_Tab]                   = ImVec4(0.18f, 0.14f, 0.10f, 1.00f);
		colors[ImGuiCol_TabHovered]            = ImVec4(0.48f, 0.38f, 0.20f, 0.90f);
		colors[ImGuiCol_TabActive]             = ImVec4(0.40f, 0.32f, 0.18f, 1.00f);
		colors[ImGuiCol_TabUnfocused]          = ImVec4(0.12f, 0.10f, 0.08f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.24f, 0.19f, 0.12f, 1.00f);

		colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.18f, 0.14f, 0.10f, 1.00f);
		colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.42f, 0.33f, 0.19f, 0.50f);
		colors[ImGuiCol_TableBorderLight]      = ImVec4(0.30f, 0.24f, 0.14f, 0.40f);
		colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

		colors[ImGuiCol_NavHighlight]          = ImVec4(0.72f, 0.57f, 0.28f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);

		colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.52f, 0.42f, 0.22f, 0.35f);
		colors[ImGuiCol_DragDropTarget]        = ImVec4(0.86f, 0.68f, 0.30f, 0.90f);

		ImGui::GetIO().FontGlobalScale = 1.05f;
	}

	void Draw()
	{
		auto* settings = Settings::Main::GetSingleton();
		bool anyChanged = false;

		if (!s_profilesLoaded) {
			LoadProfilesFromFile();
		}

		ImGui::SetNextWindowSize(ImVec2(540, 720), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

		if (!ImGui::Begin("Native Camera Tweaks###BG3CamTweaks", nullptr, windowFlags)) {
			ImGui::End();
			return;
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.86f, 0.68f, 0.30f, 1.0f));
			ImGui::TextUnformatted("Native Camera Tweaks");
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::TextDisabled("v%d.%d.%d", Plugin::Version / 10000, (Plugin::Version / 100) % 100, Plugin::Version % 100);
			{
				const auto& keyName = *settings->ToggleMenuKey;
				ImGui::TextDisabled("Press [%s] to toggle this menu", keyName.c_str());
			}
			ImGui::Separator();
			ImGui::Spacing();
		}

		if (ImGui::BeginTabBar("##MainTabs", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem("General")) {
				DrawGeneralSection(settings, anyChanged);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Exploration")) {
				DrawExplorationTab(settings, anyChanged);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Combat")) {
				DrawCombatTab(settings, anyChanged);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Input")) {
				DrawInputTab(settings, anyChanged);
				ImGui::EndTabItem();
			}

			if (*settings->EnableDebugMode) {
				if (ImGui::BeginTabItem("Debug")) {
					DrawDebugTab();
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.36f, 0.28f, 0.14f, 0.90f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.52f, 0.42f, 0.20f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.62f, 0.50f, 0.24f, 1.00f));

			if (ImGui::Button("Save to File", ImVec2(130, 28))) {
				SaveSettingsToFile(settings);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Write current settings to the TOML config file.");
			}

			ImGui::SameLine();

			if (ImGui::Button("Reload from File", ImVec2(140, 28))) {
				settings->Load();
				LoadProfilesFromFile();
				s_saveStatusTimer = 0.0f;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Reload settings from the TOML config file on disk.");
			}

			ImGui::PopStyleColor(3);

			// show save status below buttons
			if (s_saveStatusTimer > 0.0f) {
				s_saveStatusTimer -= ImGui::GetIO().DeltaTime;
				if (s_saveStatusTimer <= 0.0f) {
					s_saveStatusMessage.clear();
				} else {
					ImGui::Spacing();
					if (s_saveStatusIsError) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
					} else {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.90f, 0.40f, 1.0f));
					}
					ImGui::TextWrapped("%s", s_saveStatusMessage.c_str());
					ImGui::PopStyleColor();
					if (s_saveStatusScrollNeeded) {
						ImGui::SetScrollHereY(0.8f);
						s_saveStatusScrollNeeded = false;
					}
				}
			}
		}

		if (anyChanged) {
			WriteLocker locker(settings->Lock);
			settings->bChanged = true;
		}

		ImGui::End();
	}
}
