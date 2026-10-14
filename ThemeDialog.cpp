/*
 * Copyright (C) 2011-2026 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"
#include "ThemeDialog.h"
#include "define.h"
#include "microsip.h"

#pragma comment(lib, "dwmapi.lib") 
#pragma comment(lib, "msimg32.lib") // Upphone: AlphaBlend for tinted icons

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

CThemeDialog::CThemeDialog(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
    : CDialog(nIDTemplate, pParent)
{
}

// Upphone: flat themed border for edit boxes (no Windows accent underline).
// dwRefData is the ThemeColorId of the control background, used for the inner line.
static LRESULT CALLBACK DarkEditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {
    case WM_NCPAINT: {
        LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        HDC hdc = GetWindowDC(hWnd);
        if (hdc) {
            RECT rc;
            GetWindowRect(hWnd, &rc);
            OffsetRect(&rc, -rc.left, -rc.top);
            bool focused = GetFocus() == hWnd;
            HBRUSH border = CreateSolidBrush(focused ? _GLOBAL_BRAND_ACCENT : _GLOBAL_THEME_BORDER);
            HBRUSH inner = CreateSolidBrush(ThemeColor((ThemeColorId)dwRefData));
            FrameRect(hdc, &rc, border);
            // a static edge is only 1px wide: the inner line would cover the text area
            LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
            if ((exStyle & WS_EX_CLIENTEDGE) || !(exStyle & WS_EX_STATICEDGE)) {
                InflateRect(&rc, -1, -1);
                FrameRect(hdc, &rc, inner);
            }
            DeleteObject(border);
            DeleteObject(inner);
            ReleaseDC(hWnd, hdc);
        }
        return res;
    }
    case WM_SETFOCUS:
    case WM_KILLFOCUS: {
        LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
        return res;
    }
    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, DarkEditSubclassProc, uIdSubclass);
        break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Upphone: small chevron at the right of the sorted column label (see ThemeSetListSort)
static void DrawSortArrow(HWND hList, LPNMCUSTOMDRAW cd)
{
    int column;
    bool ascending;
    if (!ThemeGetListSort(hList, &column, &ascending) || (int)cd->dwItemSpec != column) {
        return;
    }
    int dpi = GetDeviceCaps(cd->hdc, LOGPIXELSY);
    int w = MulDiv(8, dpi, 96);
    int h = w / 2;
    int pad = MulDiv(6, dpi, 96);
    RECT rc = cd->rc;
    if (rc.right - rc.left < w + 2 * pad) {
        return;
    }
    int x = rc.right - pad - w;
    int y = rc.top + (rc.bottom - rc.top - h) / 2;
    POINT pts[3];
    if (ascending) {
        pts[0] = { x, y + h };
        pts[1] = { x + w / 2, y };
        pts[2] = { x + w, y + h };
    }
    else {
        pts[0] = { x, y };
        pts[1] = { x + w / 2, y + h };
        pts[2] = { x + w, y };
    }
    HPEN pen = CreatePen(PS_SOLID, MulDiv(1, dpi, 96), _GLOBAL_THEME_TEXT2);
    HGDIOBJ oldPen = SelectObject(cd->hdc, pen);
    Polyline(cd->hdc, pts, 3);
    SelectObject(cd->hdc, oldPen);
    DeleteObject(pen);
}

// Upphone: readable column headers on dark list views
static LRESULT CALLBACK DarkListSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_NOTIFY) {
        LPNMHDR hdr = (LPNMHDR)lParam;
        if (hdr->hwndFrom == ListView_GetHeader(hWnd) && hdr->code == NM_CUSTOMDRAW) {
            LPNMCUSTOMDRAW cd = (LPNMCUSTOMDRAW)lParam;
            switch (cd->dwDrawStage) {
            case CDDS_PREPAINT:
                return CDRF_NOTIFYITEMDRAW;
            case CDDS_ITEMPREPAINT:
                SetTextColor(cd->hdc, _GLOBAL_THEME_TEXT);
                return CDRF_NOTIFYPOSTPAINT;
            case CDDS_ITEMPOSTPAINT:
                DrawSortArrow(hWnd, cd);
                return CDRF_DODEFAULT;
            }
        }
    }
    else if (uMsg == WM_NCDESTROY) {
        ThemeClearListSort(hWnd);
        RemoveWindowSubclass(hWnd, DarkListSubclassProc, uIdSubclass);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Upphone: push buttons with an icon (Hold, Mute, Transfer) painted like the dialer keys: rounded face with the
// accent color when checked or pressed. Old 16-color icons are black, invisible on the dark theme, so they are
// drawn as a silhouette in the text color; 32-bit icons (Mute) are drawn as they are.
static bool IconHasAlpha(HICON icon)
{
    ICONINFO info = {};
    if (!GetIconInfo(icon, &info)) {
        return false;
    }
    BITMAP bm = {};
    bool alpha = info.hbmColor && GetObject(info.hbmColor, sizeof(bm), &bm) && bm.bmBitsPixel == 32;
    if (info.hbmColor) {
        DeleteObject(info.hbmColor);
    }
    if (info.hbmMask) {
        DeleteObject(info.hbmMask);
    }
    return alpha;
}

// Upphone: draws a 32-bit icon in a single color, keeping its alpha (white glyph on the accent face, like the
// dialer keys). Returns false when the icon has no 32-bit color bitmap, so the caller can draw it as it is.
static bool DrawIconTinted(HDC hdc, HICON icon, int x, int y, int size, COLORREF color)
{
    ICONINFO info = {};
    if (!GetIconInfo(icon, &info)) {
        return false;
    }
    bool ok = false;
    BITMAP bm = {};
    if (info.hbmColor && GetObject(info.hbmColor, sizeof(bm), &bm) && bm.bmBitsPixel == 32) {
        int w = bm.bmWidth;
        int h = bm.bmHeight;
        BITMAPINFO bi = {};
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = w;
        bi.bmiHeader.biHeight = -h;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        void* bits = NULL;
        HBITMAP dib = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
        if (dib && GetDIBits(hdc, info.hbmColor, 0, h, bits, &bi, DIB_RGB_COLORS) == h) {
            DWORD* px = (DWORD*)bits;
            bool hasAlpha = false;
            for (int i = 0; i < w * h; i++) {
                DWORD a = px[i] >> 24;
                hasAlpha = hasAlpha || a != 0;
                // premultiplied, as AlphaBlend expects
                px[i] = (a << 24) | ((GetRValue(color) * a / 255) << 16) | ((GetGValue(color) * a / 255) << 8) | (GetBValue(color) * a / 255);
            }
            // a 32-bit icon without alpha would disappear: the caller draws it as it is
            if (hasAlpha) {
                HDC mem = CreateCompatibleDC(hdc);
                HGDIOBJ old = SelectObject(mem, dib);
                BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
                ok = AlphaBlend(hdc, x, y, size, size, mem, 0, 0, w, h, bf) != FALSE;
                SelectObject(mem, old);
                DeleteDC(mem);
            }
        }
        if (dib) {
            DeleteObject(dib);
        }
    }
    if (info.hbmColor) {
        DeleteObject(info.hbmColor);
    }
    if (info.hbmMask) {
        DeleteObject(info.hbmMask);
    }
    return ok;
}

static bool DrawThemedIconButton(LPNMCUSTOMDRAW cd)
{
    HWND hWnd = cd->hdr.hwndFrom;
    wchar_t cls[16]{};
    GetClassNameW(hWnd, cls, _countof(cls));
    if (wcscmp(cls, L"Button") != 0) {
        return false;
    }
    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
    LONG_PTR type = style & BS_TYPEMASK;
    bool push = type == BS_PUSHBUTTON || type == BS_DEFPUSHBUTTON;
    bool pushLikeCheck = (type == BS_CHECKBOX || type == BS_AUTOCHECKBOX) && (style & BS_PUSHLIKE);
    if (!(style & BS_ICON) || !(push || pushLikeCheck)) {
        return false;
    }
    if (cd->dwDrawStage != CDDS_PREERASE && cd->dwDrawStage != CDDS_PREPAINT) {
        return false;
    }
    HDC hdc = cd->hdc;
    RECT rc = cd->rc;
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    bool disabled = (cd->uItemState & CDIS_DISABLED) != 0;
    bool pressed = (cd->uItemState & CDIS_SELECTED) != 0;
    bool hot = (cd->uItemState & CDIS_HOT) != 0;
    bool checked = pushLikeCheck && ::SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
    bool accent = !disabled && (checked || pressed);

    HBRUSH bg = CreateSolidBrush(_GLOBAL_THEME_BG);
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);
    RECT face = rc;
    InflateRect(&face, -1, -1);
    HBRUSH faceBrush = CreateSolidBrush(accent ? _GLOBAL_BRAND_ACCENT : (hot && !disabled ? _GLOBAL_THEME_SURFACE_HOT : _GLOBAL_THEME_SURFACE));
    HPEN borderPen = CreatePen(PS_SOLID, 1, accent || (hot && !disabled) ? _GLOBAL_BRAND_ACCENT : _GLOBAL_THEME_BORDER);
    HGDIOBJ oldBrush = SelectObject(hdc, faceBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    int radius = MulDiv(6, dpi, 96);
    RoundRect(hdc, face.left, face.top, face.right, face.bottom, radius, radius);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(faceBrush);
    DeleteObject(borderPen);

    HICON icon = (HICON)::SendMessage(hWnd, BM_GETIMAGE, IMAGE_ICON, 0);
    if (icon) {
        int size = MulDiv(16, dpi, 96);
        size = min(size, (int)min(rc.right - rc.left, rc.bottom - rc.top) - 4);
        int x = rc.left + (rc.right - rc.left - size) / 2;
        int y = rc.top + (rc.bottom - rc.top - size) / 2;
        if (IconHasAlpha(icon)) {
            // Upphone: on the accent face (Mute on) the glyph is white, the gray icon had too little contrast
            if (!accent || !DrawIconTinted(hdc, icon, x, y, size, RGB(255, 255, 255))) {
                DrawIconEx(hdc, x, y, icon, size, size, 0, NULL, DI_NORMAL);
            }
        }
        else {
            HBRUSH glyph = CreateSolidBrush(accent ? RGB(255, 255, 255) : (disabled ? _GLOBAL_THEME_TEXT2 : _GLOBAL_THEME_TEXT));
            DrawState(hdc, glyph, NULL, (LPARAM)icon, 0, x, y, size, size, DST_ICON | DSS_MONO);
            DeleteObject(glyph);
        }
    }
    return true;
}

// Upphone: on the dark theme check boxes and radio buttons are unthemed (ApplyTheme), and Windows draws their
// disabled text embossed, doubled on the black background. After Windows paints, the text is drawn again in TEXT2.
static void DrawDisabledCheckText(HWND hWnd, HDC hdc)
{
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    RECT rc;
    GetClientRect(hWnd, &rc);
    rc.left += MulDiv(14, dpi, 96); // past the check box / radio glyph
    FillRect(hdc, &rc, ThemeBrush(THEME_BG));
    rc.left += MulDiv(2, dpi, 96);
    wchar_t label[256]{};
    GetWindowTextW(hWnd, label, _countof(label));
    HGDIOBJ oldFont = SelectObject(hdc, (HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, _GLOBAL_THEME_TEXT2);
    DrawTextW(hdc, label, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_HIDEPREFIX);
    SelectObject(hdc, oldFont);
}

static LRESULT CALLBACK DisabledCheckSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if ((uMsg == WM_PAINT || uMsg == WM_PRINTCLIENT) && ThemeIsDark() && !IsWindowEnabled(hWnd)) {
        LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        HDC hdc = uMsg == WM_PRINTCLIENT ? (HDC)wParam : GetDC(hWnd);
        DrawDisabledCheckText(hWnd, hdc);
        if (uMsg == WM_PAINT) {
            ReleaseDC(hWnd, hdc);
        }
        return result;
    }
    if (uMsg == WM_ENABLE) {
        InvalidateRect(hWnd, NULL, TRUE);
    }
    else if (uMsg == WM_NCDESTROY) {
        RemoveWindowSubclass(hWnd, DisabledCheckSubclassProc, uIdSubclass);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL CThemeDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    LPNMHDR hdr = (LPNMHDR)lParam;
    if (m_bAllowDark && hdr && hdr->code == NM_CUSTOMDRAW && DrawThemedIconButton((LPNMCUSTOMDRAW)lParam)) {
        *pResult = CDRF_SKIPDEFAULT;
        return TRUE;
    }
    return CDialog::OnNotify(wParam, lParam, pResult);
}

static bool IsComboEdit(HWND hWnd)
{
    wchar_t parentCls[32]{};
    GetClassNameW(GetParent(hWnd), parentCls, _countof(parentCls));
    return wcscmp(parentCls, L"ComboBox") == 0;
}

static bool HasBorder(HWND hWnd)
{
    return (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_BORDER) || (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & (WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
}

static void ApplyDarkFrame(HWND hWnd, ThemeColorId background)
{
    SetWindowSubclass(hWnd, DarkEditSubclassProc, 1, background);
    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

static void ApplyDarkEdit(HWND hWnd)
{
    if (IsComboEdit(hWnd)) {
        return;
    }
    if (HasBorder(hWnd)) {
        SetWindowTheme(hWnd, L"", L"");
        ApplyDarkFrame(hWnd, THEME_SURFACE);
    }
}

BEGIN_MESSAGE_MAP(CThemeDialog, CDialog)
    //{{AFX_MSG_MAP(CThemeDialog)
    ON_WM_CTLCOLOR()
    ON_WM_SETTINGCHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CThemeDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    lastDark = -1;

    ApplyTheme();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

HBRUSH CThemeDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    if (m_bAllowDark) {
        switch (nCtlColor)
        {
        case CTLCOLOR_EDIT:
        case CTLCOLOR_LISTBOX:
            pDC->SetTextColor(_GLOBAL_THEME_TEXT);
            pDC->SetBkColor(_GLOBAL_THEME_SURFACE);
            return ThemeBrush(THEME_SURFACE);
        case CTLCOLOR_STATIC: {
            wchar_t cls[64];
            GetClassName(pWnd->m_hWnd, cls, 64);
            pDC->SetTextColor(wcscmp(cls, L"SysLink") == 0 ? _GLOBAL_BRAND_ACCENT : _GLOBAL_THEME_TEXT);
            pDC->SetBkColor(_GLOBAL_THEME_BG);
            pDC->SetBkMode(TRANSPARENT);
            return ThemeBrush(THEME_BG);
        }
        case CTLCOLOR_BTN:
        case CTLCOLOR_DLG:
            pDC->SetTextColor(_GLOBAL_THEME_TEXT);
            pDC->SetBkColor(_GLOBAL_THEME_BG);
            pDC->SetBkMode(TRANSPARENT);
            return ThemeBrush(THEME_BG);
        }
    }
    HBRUSH br = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    return br;
}

void CThemeDialog::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    if (lpszSection &&
        wcscmp(lpszSection, L"ImmersiveColorSet") == 0) {
        // Upphone: Windows light/dark mode changed, re-theme this window and every embedded page
        ThemeRefresh();
        ApplyTheme();
        EnumChildWindows(GetSafeHwnd(), [](HWND hWnd, LPARAM) {
            CThemeDialog* page = dynamic_cast<CThemeDialog*>(CWnd::FromHandlePermanent(hWnd));
            if (page) {
                page->ApplyTheme();
            }
            return TRUE;
            }, 0);
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
    }
    CDialog::OnSettingChange(uFlags, lpszSection);
}

void CThemeDialog::ApplyTheme()
{
    BOOL currentDark = ThemeIsDark(); // Upphone: follow the Windows app mode
    if (lastDark == -1 || currentDark != lastDark) {
        lastDark = currentDark;
        DwmSetWindowAttribute(
            m_hWnd,
            DWMWA_USE_IMMERSIVE_DARK_MODE,
            &currentDark,
            sizeof(currentDark)
        );

        EnumChildWindows(GetSafeHwnd(), [](HWND hWnd, LPARAM lParam) {
            bool allowDark = lParam & (1 << 1);
            bool dark = lParam & 1;
            wchar_t cls[32]{};
            GetClassNameW(hWnd, cls, _countof(cls));
            if (wcscmp(cls, L"Button") == 0) {
                LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
                if (((style & BS_TYPEMASK) == BS_PUSHBUTTON
                    || (style & BS_TYPEMASK) == BS_DEFPUSHBUTTON
                    || (allowDark && (style & BS_TYPEMASK) == BS_OWNERDRAW)
                    ) && (!(style & BS_ICON) || allowDark)
                    ) {
                    SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
                }
                if (allowDark) {
                    if ((style & BS_TYPEMASK) == BS_AUTOCHECKBOX || (style & BS_TYPEMASK) == BS_AUTORADIOBUTTON) {
                        SetWindowSubclass(hWnd, DisabledCheckSubclassProc, 2, 0); // Upphone: disabled text on dark
                        if (dark) {
                            SetWindowTheme(hWnd, L"wstr", L"wstr");
                        }
                        else {
                            SetWindowTheme(hWnd, L"Explorer", nullptr);
                        }
                    }
                }
            }
            if (allowDark && wcscmp(cls, L"Button") == 0) {
                LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
                if (style & BS_PUSHLIKE) {
                    SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
                }
            }
            if (allowDark && wcscmp(cls, L"SysListView32") == 0) {
                SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
                ListView_SetBkColor(hWnd, _GLOBAL_THEME_BG);
                ListView_SetTextBkColor(hWnd, _GLOBAL_THEME_BG);
                ListView_SetTextColor(hWnd, _GLOBAL_THEME_TEXT);
                HWND hHeader = ListView_GetHeader(hWnd);
                if (hHeader) {
                    SetWindowTheme(hHeader, dark ? L"DarkMode_ItemsView" : L"ItemsView", nullptr);
                }
                SetWindowSubclass(hWnd, DarkListSubclassProc, 1, 0);
            }
            if (allowDark && wcscmp(cls, L"msctls_trackbar32") == 0) {
                SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
            }
            if (allowDark) {
                if (wcscmp(cls, L"ComboBox") == 0) {
                    SetWindowTheme(hWnd, dark ? L"DarkMode_CFD" : L"CFD", nullptr);
                }
                else if (wcscmp(cls, L"Edit") == 0 && !IsComboEdit(hWnd)) {
                    SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
                    ApplyDarkEdit(hWnd);
                }
                else if (_wcsnicmp(cls, L"RichEdit", 8) == 0) {
                    bool readOnly = (GetWindowLongPtr(hWnd, GWL_STYLE) & ES_READONLY) != 0;
                    ::SendMessage(hWnd, EM_SETBKGNDCOLOR, 0, readOnly ? _GLOBAL_THEME_BG : _GLOBAL_THEME_SURFACE);
                    CHARFORMAT cf = { 0 };
                    cf.cbSize = sizeof(cf);
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = _GLOBAL_THEME_TEXT;
                    ::SendMessage(hWnd, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf);
                    // Upphone: themed scrollbars and the same flat border as the edit boxes (chat window)
                    SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
                    if (HasBorder(hWnd)) {
                        ApplyDarkFrame(hWnd, readOnly ? THEME_BG : THEME_SURFACE);
                    }
                }
                else if (wcscmp(cls, L"Edit") == 0 || wcscmp(cls, L"Static") == 0 || wcscmp(cls, L"ListBox") == 0) {
                    SetWindowTheme(hWnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
                }
                else if ( wcscmp(cls, L"SysLink") == 0 ) {
                    LITEM item = { 0 };
                    item.iLink = 0;
                    item.mask = LIF_ITEMINDEX | LIF_STATE;
                    item.state = LIS_DEFAULTCOLORS;
                    item.stateMask = LIS_DEFAULTCOLORS;
                    ::SendMessage(hWnd, LM_SETITEM, 0, (LPARAM)&item);
                }
            }
            return TRUE;
            }, m_bAllowDark << 1 | currentDark);
        Invalidate(TRUE);
    }
}
