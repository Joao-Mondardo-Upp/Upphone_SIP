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
#include "StatusBar.h"   
#include "global.h"
#include "define.h"

 // StatusBar   
IMPLEMENT_DYNAMIC(StatusBar, CStatusBar)
StatusBar::StatusBar()
{
	CStatusBar::CStatusBar();
}

StatusBar::~StatusBar()
{
}

BEGIN_MESSAGE_MAP(StatusBar, CStatusBar)
	//{{AFX_MSG_MAP(StatusBar)   
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP   
END_MESSAGE_MAP()

LRESULT StatusBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible())
	{
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
		if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
	}
	return 0L;
}

void StatusBar::OnLButtonUp(UINT nFlags, CPoint point)
{
}

void StatusBar::OnMouseMove(UINT nFlags, CPoint point)
{
}

BOOL StatusBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void StatusBar::OnPaint()
{
	CPaintDC dc(this);
	CRect client;
	GetClientRect(&client);
	dc.FillSolidRect(client, _GLOBAL_THEME_BG);
	dc.FillSolidRect(CRect(client.left, client.top, client.right, client.top + 1), _GLOBAL_THEME_BORDER);
	CFont* oldFont = dc.SelectObject(GetFont());
	dc.SetBkMode(TRANSPARENT);
	int dpi = dc.GetDeviceCaps(LOGPIXELSY);
	int pad = MulDiv(4, dpi, 96);
	int iconSize = MulDiv(16, dpi, 96);
	int count = GetCount();
	for (int i = 0; i < count; i++) {
		CRect rect;
		GetItemRect(i, &rect);
		rect.DeflateRect(pad, 0);
		HICON hIcon = (HICON)::SendMessage(m_hWnd, SB_GETICON, i, 0);
		if (hIcon) {
			int y = rect.top + (rect.Height() - iconSize) / 2;
			DrawIconEx(dc.m_hDC, rect.left, y, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
			rect.left += iconSize + pad;
		}
		CString text = GetPaneText(i);
		// Upphone: a call on hold in the warning color, so it is not taken for an active call
		dc.SetTextColor(i == 0 ? (m_warning ? _GLOBAL_THEME_WARNING : _GLOBAL_THEME_TEXT) : _GLOBAL_THEME_TEXT2);
		dc.DrawText(text, rect, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | (i == 0 ? DT_LEFT : DT_RIGHT));
	}
	dc.SelectObject(oldFont);
}
