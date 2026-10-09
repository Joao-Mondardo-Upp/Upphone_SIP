#pragma once

// Tab control painted with the Upphone dark theme.
class CDarkTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CDarkTabCtrl)
public:
	CDarkTabCtrl();
	virtual ~CDarkTabCtrl();
protected:
	int m_hotItem = -1;
	virtual void PreSubclassWindow();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

// Main menu drop-down button painted with the Upphone dark theme.
class CDarkArrowButton : public CButton
{
	DECLARE_DYNAMIC(CDarkArrowButton)
protected:
	bool m_hot = false;
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetState(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
