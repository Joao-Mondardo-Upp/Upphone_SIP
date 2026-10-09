#pragma once

// Upphone: light/dark palette that follows the Windows "app mode" setting.

enum ThemeColorId {
	THEME_BG,
	THEME_SURFACE,
	THEME_SURFACE_HOT,
	THEME_BORDER,
	THEME_TEXT,
	THEME_TEXT2,
	THEME_COLOR_COUNT
};

// true when Windows is set to dark app mode
bool ThemeIsDark();
// re-reads the Windows setting; returns true if the mode changed
bool ThemeRefresh();
COLORREF ThemeColor(ThemeColorId id);
// shared brush for the current mode, never delete it
HBRUSH ThemeBrush(ThemeColorId id);
