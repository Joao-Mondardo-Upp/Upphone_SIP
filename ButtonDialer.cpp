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
#include "ButtonDialer.h"
#include "define.h"
#include "Strsafe.h"
#include "const.h"

/////////////////////////////////////////////////////////////////////////////
// CButtonDialer

CButtonDialer::CButtonDialer()
{
	forceNumeric = false;
	m_map.SetAt(_T("1"), _T(""));
	m_map.SetAt(_T("2"), _T("ABC"));
	m_map.SetAt(_T("3"), _T("DEF"));
	m_map.SetAt(_T("4"), _T("GHI"));
	m_map.SetAt(_T("5"), _T("JKL"));
	m_map.SetAt(_T("6"), _T("MNO"));
	m_map.SetAt(_T("7"), _T("PQRS"));
	m_map.SetAt(_T("8"), _T("TUV"));
	m_map.SetAt(_T("9"), _T("WXYZ"));
	m_map.SetAt(_T("0"), _T(""));
	m_map.SetAt(_T("*"), _T(""));
	m_map.SetAt(_T("#"), _T(""));
}

CButtonDialer::~CButtonDialer()
{
	CloseTheme();
}


BEGIN_MESSAGE_MAP(CButtonDialer, CButton)
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonDialer message handlers

void CButtonDialer::PreSubclassWindow()
{
	OpenTheme();

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;
	GetObject(hFont, sizeof(LOGFONT), &lf);
	lf.lfHeight = 12;
	CDC *pDC = GetDC();
	if (pDC) {
		dpiY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
		lf.lfHeight = MulDiv(lf.lfHeight, dpiY, 96);
		ReleaseDC(pDC);
	}
	else {
		dpiY = 96;
	}
	StringCchCopy(lf.lfFaceName, LF_FACESIZE, _T("Microsoft Sans Serif"));
	m_FontLetters.CreateFontIndirect(&lf);

	DWORD dwStyle = ::GetClassLong(m_hWnd, GCL_STYLE);
	dwStyle &= ~CS_DBLCLKS;
	::SetClassLong(m_hWnd, GCL_STYLE, dwStyle);
}

LRESULT CButtonDialer::OnThemeChanged()
{
	CloseTheme();
	OpenTheme();
	return 0L;
}

void CButtonDialer::OnSize(UINT type, int w, int h)
{
	CButton::OnSize(type, w, h);
}

void CButtonDialer::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
	if (rect.PtInRect(point)) {
		if (GetCapture() != this) {
			SetCapture();
			Invalidate();
		}
	}
	else {
		ReleaseCapture();
		Invalidate();
	}
}

void CButtonDialer::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);		//Get device context object
	CRect rt;
	rt = lpDrawItemStruct->rcItem;		//Get button rect
	UINT state = lpDrawItemStruct->itemState;	//Get state of the button
	bool pressed = (state & ODS_SELECTED) != 0;
	bool hot = GetCapture() == this;

	dc.FillSolidRect(rt, _GLOBAL_THEME_BG);
	CRect rtFace = rt;
	rtFace.DeflateRect(1, 1);
	COLORREF face = pressed ? _GLOBAL_BRAND_ACCENT : (hot ? _GLOBAL_THEME_SURFACE_HOT : _GLOBAL_THEME_SURFACE);
	COLORREF border = (hot || pressed) ? _GLOBAL_BRAND_ACCENT : _GLOBAL_THEME_BORDER;
	CBrush brush(face);
	CPen pen(PS_SOLID, 1, border);
	CBrush* oldBrush = dc.SelectObject(&brush);
	CPen* oldPen = dc.SelectObject(&pen);
	int radius = MulDiv(6, dpiY, 96);
	dc.RoundRect(rtFace, CPoint(radius, radius));
	dc.SelectObject(oldBrush);
	dc.SelectObject(oldPen);
	dc.SetBkMode(TRANSPARENT);

	CRect rtl = rt;
	CString strTemp;
	GetWindowText(strTemp);		// Get the caption which have been set

	int x12 = MulDiv(12, dpiY, 96);
	int x14 = MulDiv(14, dpiY, 96);
	int x4 = MulDiv(4, dpiY, 96);

	COLORREF textColor = pressed ? RGB(255, 255, 255) : _GLOBAL_THEME_TEXT;
	COLORREF lettersColor = pressed ? RGB(255, 255, 255) : _GLOBAL_THEME_TEXT2;

	CString letters;
	COLORREF crOldColor;
	if (!forceNumeric && m_map.Lookup(strTemp, letters)) {
		rtl.left += x14;
		crOldColor = dc.SetTextColor(textColor);
		dc.DrawText(strTemp, rtl, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		HFONT hOldFont = (HFONT)SelectObject(dc.m_hDC, m_FontLetters);
		rtl.left += x12;
		rtl.right -= x4;
		dc.SetTextColor(lettersColor);
		dc.DrawText(letters, rtl, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		dc.SetTextColor(crOldColor);
		SelectObject(dc.m_hDC, hOldFont);
	}
	else {
		crOldColor = dc.SetTextColor(forceNumeric ? textColor : (pressed ? textColor : _GLOBAL_BRAND_ACCENT));
		dc.DrawText(strTemp, rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		dc.SetTextColor(crOldColor);
	}
	dc.Detach();
}
