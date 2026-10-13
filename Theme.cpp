#include "stdafx.h"
#include "Theme.h"

static int g_themeDark = -1;

static const COLORREF g_palette[2][THEME_COLOR_COUNT] = {
	// light
	{
		RGB(255, 255, 255), // THEME_BG
		RGB(240, 240, 240), // THEME_SURFACE
		RGB(225, 225, 225), // THEME_SURFACE_HOT
		RGB(204, 204, 204), // THEME_BORDER
		RGB(26, 26, 26),    // THEME_TEXT
		RGB(110, 110, 110), // THEME_TEXT2
	},
	// dark
	{
		RGB(0, 0, 0),       // THEME_BG
		RGB(26, 26, 26),    // THEME_SURFACE
		RGB(44, 44, 44),    // THEME_SURFACE_HOT
		RGB(58, 58, 58),    // THEME_BORDER
		RGB(240, 240, 240), // THEME_TEXT
		RGB(150, 150, 150), // THEME_TEXT2
	},
};

static HBRUSH g_brushes[2][THEME_COLOR_COUNT] = {};

static bool ReadWindowsDarkMode()
{
	// override for testing/support: UPPHONE_THEME=light|dark
	wchar_t forced[16] = { 0 };
	if (GetEnvironmentVariableW(L"UPPHONE_THEME", forced, _countof(forced))) {
		if (_wcsicmp(forced, L"dark") == 0) return true;
		if (_wcsicmp(forced, L"light") == 0) return false;
	}
	DWORD value = 1;
	DWORD size = sizeof(value);
	LSTATUS res = RegGetValueW(
		HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"AppsUseLightTheme",
		RRF_RT_REG_DWORD,
		nullptr,
		&value, // 0 = dark
		&size
	);
	return res == ERROR_SUCCESS && value == 0;
}

bool ThemeIsDark()
{
	if (g_themeDark == -1) {
		g_themeDark = ReadWindowsDarkMode() ? 1 : 0;
	}
	return g_themeDark == 1;
}

bool ThemeRefresh()
{
	int dark = ReadWindowsDarkMode() ? 1 : 0;
	bool changed = dark != g_themeDark;
	g_themeDark = dark;
	return changed;
}

COLORREF ThemeColor(ThemeColorId id)
{
	return g_palette[ThemeIsDark() ? 1 : 0][id];
}

HBRUSH ThemeBrush(ThemeColorId id)
{
	int mode = ThemeIsDark() ? 1 : 0;
	if (!g_brushes[mode][id]) {
		g_brushes[mode][id] = CreateSolidBrush(g_palette[mode][id]);
	}
	return g_brushes[mode][id];
}

static const wchar_t* kListSortProp = L"Upphone.ListSort";

void ThemeSetListSort(HWND hList, int column, bool ascending)
{
	if (column < 0) {
		ThemeClearListSort(hList);
		return;
	}
	// column + 1 in the low word (0 = no sort), direction in the high word
	SetPropW(hList, kListSortProp, (HANDLE)(INT_PTR)MAKELONG(column + 1, ascending ? 1 : 0));
}

bool ThemeGetListSort(HWND hList, int* column, bool* ascending)
{
	DWORD value = (DWORD)(INT_PTR)GetPropW(hList, kListSortProp);
	if (!LOWORD(value)) {
		return false;
	}
	*column = LOWORD(value) - 1;
	*ascending = HIWORD(value) != 0;
	return true;
}

void ThemeClearListSort(HWND hList)
{
	RemovePropW(hList, kListSortProp);
}
