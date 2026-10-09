#pragma once

#include "CListCtrl_LabelTip.h"
#include <vector>

class CListCtrl_Sortable : public CListCtrl_LabelTip
{
	bool m_Ascending;
	int  m_SortCol;
	// Upphone: columns fitted to the list width (FitColumnsEnable)
	std::vector<int> m_fitMin;
	std::vector<int> m_fitPref;
	bool m_fitBusy = false;
	void FitColumns();

	DECLARE_MESSAGE_MAP();

	afx_msg BOOL OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnFitColumns(WPARAM wParam, LPARAM lParam);
	void PreSubclassWindow();

public:
	CListCtrl_Sortable()
		:m_Ascending(false)
		,m_SortCol(-1)
	{}
	int GetColumnData(int col) const;
	void SetSortArrow(int col, bool ascending);
	int GetSortColumn() const { return m_SortCol; }
	bool IsAscending() const { return m_Ascending; }
	void ResetSortOrder();
	virtual bool SortColumn(int columnIndex, bool ascending) = 0;
	void SetSortColumn(int columnIndex, bool ascending);
	// Upphone: minimum width of each column in 96-DPI pixels, in the order they give way
	void FitColumnsEnable(const std::vector<int>& minWidths);
	// Upphone: call from HDN_ENDTRACK with the new width of the dragged column
	void FitColumnsTrack(int column, int width);
};