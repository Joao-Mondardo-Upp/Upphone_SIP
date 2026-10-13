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
