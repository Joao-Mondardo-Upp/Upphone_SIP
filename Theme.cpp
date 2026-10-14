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
		RGB(204, 37, 29),   // THEME_DANGER
		RGB(176, 106, 0),   // THEME_WARNING
		RGB(25, 135, 60),   // THEME_SUCCESS
	},
	// dark
	{
		RGB(0, 0, 0),       // THEME_BG
		RGB(26, 26, 26),    // THEME_SURFACE
		RGB(44, 44, 44),    // THEME_SURFACE_HOT
		RGB(58, 58, 58),    // THEME_BORDER
		RGB(240, 240, 240), // THEME_TEXT
		RGB(150, 150, 150), // THEME_TEXT2
		RGB(255, 105, 97),  // THEME_DANGER
		RGB(255, 184, 77),  // THEME_WARNING
		RGB(80, 200, 120),  // THEME_SUCCESS
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

ThemeMenuIcons::~ThemeMenuIcons()
{
	for (HBITMAP bmp : m_bitmaps) {
		DeleteObject(bmp);
	}
}

void ThemeMenuIcons::Set(HMENU menu, UINT item, wchar_t glyph, COLORREF color, bool byPosition)
{
	if (GetMenuState(menu, item, byPosition ? MF_BYPOSITION : MF_BYCOMMAND) == (UINT)-1) {
		return;
	}
	int size = GetSystemMetrics(SM_CXSMICON);
	BITMAPINFO bi = {};
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = size;
	bi.bmiHeader.biHeight = -size;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	void* bits = NULL;
	HDC screen = GetDC(NULL);
	HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	HDC mem = CreateCompatibleDC(screen);
	ReleaseDC(NULL, screen);
	if (!bmp || !mem) {
		if (bmp) {
			DeleteObject(bmp);
		}
		if (mem) {
			DeleteDC(mem);
		}
		return;
	}
	HGDIOBJ oldBmp = SelectObject(mem, bmp);
	memset(bits, 0, size * size * 4);
	HFONT font = NULL;
	HGDIOBJ oldFont = NULL;
	for (const wchar_t* face : { L"Segoe Fluent Icons", L"Segoe MDL2 Assets" }) {
		HFONT f = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, face);
		HGDIOBJ old = SelectObject(mem, f);
		wchar_t got[LF_FACESIZE] = { 0 };
		GetTextFaceW(mem, LF_FACESIZE, got);
		if (_wcsicmp(got, face) == 0) {
			font = f;
			oldFont = old;
			break;
		}
		SelectObject(mem, old);
		DeleteObject(f);
	}
	bool drawn = false;
	if (font) {
		// white glyph on black, then its brightness becomes the alpha of the given color (premultiplied)
		SetBkMode(mem, TRANSPARENT);
		SetTextColor(mem, RGB(255, 255, 255));
		RECT rc = { 0, 0, size, size };
		DrawTextW(mem, &glyph, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
		GdiFlush();
		DWORD* px = (DWORD*)bits;
		for (int i = 0; i < size * size; i++) {
			DWORD a = max(max(px[i] & 0xFF, (px[i] >> 8) & 0xFF), (px[i] >> 16) & 0xFF);
			px[i] = (a << 24) | ((GetRValue(color) * a / 255) << 16) | ((GetGValue(color) * a / 255) << 8) | (GetBValue(color) * a / 255);
		}
		SelectObject(mem, oldFont);
		DeleteObject(font);
		drawn = true;
	}
	SelectObject(mem, oldBmp);
	DeleteDC(mem);
	MENUITEMINFOW mii = {};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = bmp;
	if (drawn && SetMenuItemInfoW(menu, item, byPosition, &mii)) {
		m_bitmaps.push_back(bmp);
	}
	else {
		DeleteObject(bmp);
	}
}
