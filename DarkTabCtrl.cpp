#include "stdafx.h"
#include "DarkTabCtrl.h"
#include "define.h"

IMPLEMENT_DYNAMIC(CDarkTabCtrl, CTabCtrl)

CDarkTabCtrl::CDarkTabCtrl()
{
}

CDarkTabCtrl::~CDarkTabCtrl()
{
}

BEGIN_MESSAGE_MAP(CDarkTabCtrl, CTabCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

void CDarkTabCtrl::PreSubclassWindow()
{
	CTabCtrl::PreSubclassWindow();
	ModifyStyle(0, WS_CLIPSIBLINGS);
}

BOOL CDarkTabCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CDarkTabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	TCHITTESTINFO hit = { point, 0 };
	int item = HitTest(&hit);
	if (item != m_hotItem) {
		m_hotItem = item;
		Invalidate(FALSE);
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd, 0 };
		TrackMouseEvent(&tme);
	}
	CTabCtrl::OnMouseMove(nFlags, point);
}

LRESULT CDarkTabCtrl::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	if (m_hotItem != -1) {
		m_hotItem = -1;
		Invalidate(FALSE);
	}
	return 0;
}

void CDarkTabCtrl::OnPaint()
{
	CPaintDC paintDC(this);
	CRect client;
	GetClientRect(&client);

	// double buffering to avoid flicker
	CDC dc;
	dc.CreateCompatibleDC(&paintDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&paintDC, client.Width(), client.Height());
	CBitmap* oldBmp = dc.SelectObject(&bmp);

	dc.FillSolidRect(client, _GLOBAL_THEME_BG);
	CFont* oldFont = dc.SelectObject(GetFont());
	dc.SetBkMode(TRANSPARENT);

	int dpi = dc.GetDeviceCaps(LOGPIXELSY);
	int line = MulDiv(2, dpi, 96);
	int sel = GetCurSel();
	int count = GetItemCount();
	for (int i = 0; i < count; i++) {
		CRect rect;
		GetItemRect(i, &rect);
		TCHAR label[256] = { 0 };
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = label;
		tci.cchTextMax = _countof(label);
		GetItem(i, &tci);

		if (i == sel) {
			dc.FillSolidRect(rect, _GLOBAL_THEME_SURFACE);
			dc.FillSolidRect(CRect(rect.left, rect.bottom - line, rect.right, rect.bottom), _GLOBAL_BRAND_ACCENT);
			dc.SetTextColor(_GLOBAL_THEME_TEXT);
		}
		else {
			if (i == m_hotItem) {
				dc.FillSolidRect(rect, _GLOBAL_THEME_SURFACE_HOT);
			}
			dc.SetTextColor(_GLOBAL_THEME_TEXT2);
		}
		dc.DrawText(label, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);
	}

	dc.SelectObject(oldFont);
	paintDC.BitBlt(0, 0, client.Width(), client.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(oldBmp);
}

IMPLEMENT_DYNAMIC(CDarkArrowButton, CButton)

BEGIN_MESSAGE_MAP(CDarkArrowButton, CButton)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(BM_SETSTATE, OnSetState)
END_MESSAGE_MAP()

BOOL CDarkArrowButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CDarkArrowButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_hot) {
		m_hot = true;
		Invalidate(FALSE);
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd, 0 };
		TrackMouseEvent(&tme);
	}
	CButton::OnMouseMove(nFlags, point);
}

LRESULT CDarkArrowButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_hot = false;
	Invalidate(FALSE);
	return 0;
}

LRESULT CDarkArrowButton::OnSetState(WPARAM wParam, LPARAM lParam)
{
	LRESULT res = DefWindowProc(BM_SETSTATE, wParam, lParam);
	Invalidate(FALSE);
	return res;
}

void CDarkArrowButton::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	bool pressed = (GetState() & BST_PUSHED) != 0;
	COLORREF face = pressed ? _GLOBAL_BRAND_ACCENT : (m_hot ? _GLOBAL_THEME_SURFACE_HOT : _GLOBAL_THEME_BG);
	dc.FillSolidRect(rect, face);

	// down arrow
	int w = rect.Width() * 2 / 5;
	if (w % 2) w++;
	int h = w / 2;
	int cx = rect.left + rect.Width() / 2;
	int cy = rect.top + rect.Height() / 2;
	POINT pts[3] = {
		{ cx - w / 2, cy - h / 2 },
		{ cx + w / 2, cy - h / 2 },
		{ cx, cy + h / 2 + 1 }
	};
	COLORREF arrow = pressed ? RGB(255, 255, 255) : _GLOBAL_BRAND_ACCENT;
	CBrush brush(arrow);
	CPen pen(PS_SOLID, 1, arrow);
	CBrush* oldBrush = dc.SelectObject(&brush);
	CPen* oldPen = dc.SelectObject(&pen);
	dc.Polygon(pts, 3);
	dc.SelectObject(oldBrush);
	dc.SelectObject(oldPen);
}
