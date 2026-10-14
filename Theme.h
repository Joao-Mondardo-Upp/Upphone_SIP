#pragma once

#include <vector>

// Upphone: light/dark palette that follows the Windows "app mode" setting.

enum ThemeColorId {
	THEME_BG,
	THEME_SURFACE,
	THEME_SURFACE_HOT,
	THEME_BORDER,
	THEME_TEXT,
	THEME_TEXT2,
	THEME_DANGER,  // Upphone: missed calls, end/decline
	THEME_WARNING, // Upphone: call on hold
	THEME_SUCCESS, // Upphone: active call
	THEME_COLOR_COUNT
};

// true when Windows is set to dark app mode
bool ThemeIsDark();
// re-reads the Windows setting; returns true if the mode changed
bool ThemeRefresh();
COLORREF ThemeColor(ThemeColorId id);
// shared brush for the current mode, never delete it
HBRUSH ThemeBrush(ThemeColorId id);

// Upphone: sort column of a list view. CThemeDialog draws the arrow beside the column label,
// because the themed header draws it on the top edge, touching the tabs above the lists.
void ThemeSetListSort(HWND hList, int column, bool ascending);
bool ThemeGetListSort(HWND hList, int* column, bool* ascending);
void ThemeClearListSort(HWND hList);

// Upphone: icons beside the items of a popup menu, drawn from the Windows icon font (Segoe Fluent Icons on
// Windows 11, Segoe MDL2 Assets on Windows 10) in a given color. Keep the object alive while the menu is open:
// it deletes the bitmaps when it goes out of scope. Without either font the menu stays without icons.
class ThemeMenuIcons
{
public:
	~ThemeMenuIcons();
	void Set(HMENU menu, UINT item, wchar_t glyph, COLORREF color, bool byPosition = false);
private:
	std::vector<HBITMAP> m_bitmaps;
};
