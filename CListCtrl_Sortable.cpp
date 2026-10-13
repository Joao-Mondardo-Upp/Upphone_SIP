#include "stdafx.h"

#include "CListCtrl_Sortable.h"
#include "Resource.h"
#include "mainDlg.h"

#include <uxtheme.h>
#include "Theme.h"

// Upphone: posted after the user resizes a column, once the header has applied the new width
static const UINT UM_FIT_COLUMNS = WM_APP + 1;

BEGIN_MESSAGE_MAP(CListCtrl_Sortable, CListCtrl_LabelTip)
    ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnHeaderClick)	// Column Click
    ON_WM_SIZE()
    ON_MESSAGE(UM_FIT_COLUMNS, OnFitColumns)
END_MESSAGE_MAP()

namespace {
    bool IsThemeEnabled()
    {
        return IsAppThemed() && IsThemeActive();
    }
}

BOOL CListCtrl_Sortable::OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLISTVIEW* pLV = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
    SetFocus();	// Ensure other controls gets kill-focus

    int colIndex = pLV->iSubItem;

    if (m_SortCol == colIndex)
    {
        m_Ascending = !m_Ascending;
    }
    else
    {
        m_SortCol = colIndex;
        m_Ascending = true;
    }

    if (SortColumn(m_SortCol, m_Ascending))
        SetSortArrow(m_SortCol, m_Ascending);

    return FALSE;	// Let parent-dialog get chance
}

void CListCtrl_Sortable::SetSortArrow(int colIndex, bool ascending)
{
    if (IsThemeEnabled())
    {
        // Upphone: the arrow is drawn by CThemeDialog beside the label, not by the header theme
        ThemeSetListSort(m_hWnd, colIndex, ascending);
        for (int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
        {
            HDITEM hditem = { 0 };
            hditem.mask = HDI_FORMAT;
            VERIFY(GetHeaderCtrl()->GetItem(i, &hditem));
            hditem.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
            VERIFY(CListCtrl_LabelTip::GetHeaderCtrl()->SetItem(i, &hditem));
        }
        GetHeaderCtrl()->Invalidate();
    }
    else
    {
        UINT bitmapID = m_Ascending ? IDB_UPARROW : IDB_DOWNARROW;
        for (int i = 0; i < GetHeaderCtrl()->GetItemCount(); ++i)
        {
            HDITEM hditem = { 0 };
            hditem.mask = HDI_BITMAP | HDI_FORMAT;
            VERIFY(GetHeaderCtrl()->GetItem(i, &hditem));
            if (hditem.fmt & HDF_BITMAP && hditem.fmt & HDF_BITMAP_ON_RIGHT)
            {
                if (hditem.hbm)
                {
                    DeleteObject(hditem.hbm);
                    hditem.hbm = NULL;
                }
                hditem.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
                VERIFY(CListCtrl_LabelTip::GetHeaderCtrl()->SetItem(i, &hditem));
            }
            if (i == colIndex)
            {
                hditem.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
                hditem.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
                VERIFY(hditem.hbm != NULL);
                VERIFY(CListCtrl_LabelTip::GetHeaderCtrl()->SetItem(i, &hditem));
            }
        }
    }
}

void CListCtrl_Sortable::PreSubclassWindow()
{
    CListCtrl_LabelTip::PreSubclassWindow();

    // Focus retangle is not painted properly without double-buffering
    SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
    SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    SetExtendedStyle(GetExtendedStyle() | LVS_EX_HEADERDRAGDROP);
    
    SetWindowTheme(GetSafeHwnd(), L"Explorer", nullptr);
}

void CListCtrl_Sortable::ResetSortOrder()
{
    m_Ascending = true;
    m_SortCol = -1;
    SetSortArrow(m_SortCol, m_Ascending);
}

// The column version of GetItemData(), one can specify an unique
// identifier when using InsertColumn()
int CListCtrl_Sortable::GetColumnData(int col) const
{
    LVCOLUMN lvc = { 0 };
    lvc.mask = LVCF_SUBITEM;
    VERIFY(GetColumn(col, &lvc));
    return lvc.iSubItem;
}

void CListCtrl_Sortable::SetSortColumn(int columnIndex, bool ascending)
{
    m_SortCol = columnIndex;
    m_Ascending = ascending;
    if (SortColumn(m_SortCol, m_Ascending)) {
        SetSortArrow(m_SortCol, m_Ascending);
    }
}

// Upphone: keep the columns inside the list width, so there is no horizontal scrollbar.
// The first columns that fit their minimum width are shown, in index order, sharing the width
// in proportion to the preferred widths; the others get width 0 until the list is wider.
void CListCtrl_Sortable::FitColumnsEnable(const std::vector<int>& minWidths)
{
    m_fitMin = minWidths;
    m_fitPref.clear();
    for (size_t i = 0; i < m_fitMin.size(); i++) {
        m_fitPref.push_back(max(GetColumnWidth((int)i), 1));
    }
    FitColumns();
}

// the user dragged a column border: the current widths become the preferred proportions
void CListCtrl_Sortable::FitColumnsTrack(int column, int width)
{
    for (size_t i = 0; i < m_fitPref.size(); i++) {
        int w = (int)i == column ? width : GetColumnWidth((int)i);
        if (w > 0) {
            m_fitPref[i] = w;
        }
    }
    PostMessage(UM_FIT_COLUMNS);
}

void CListCtrl_Sortable::FitColumns()
{
    if (m_fitMin.empty() || m_fitBusy || !GetSafeHwnd()) {
        return;
    }
    m_fitBusy = true;
    // a fit can show or hide the vertical scrollbar and change the client width again
    for (int pass = 0; pass < 2; pass++) {
        CRect rc;
        GetClientRect(rc);
        int avail = rc.Width();
        if (avail <= 0) {
            break;
        }
        int count = (int)m_fitMin.size();
        std::vector<int> minWidth(count);
        int visible = 0;
        int minSum = 0;
        while (visible < count) {
            minWidth[visible] = MulDiv(m_fitMin[visible], dpiY, 96);
            if (visible > 0 && minSum + minWidth[visible] > avail) {
                break;
            }
            minSum += minWidth[visible];
            visible++;
        }
        // proportional share, but never below the minimum: a column that would get less is
        // fixed at its minimum and the rest is shared again among the others
        std::vector<int> width(count, 0);
        std::vector<bool> atMin(count, false);
        int rest = avail;
        for (bool changed = true; changed;) {
            changed = false;
            int prefSum = 0;
            for (int i = 0; i < visible; i++) {
                if (!atMin[i]) {
                    prefSum += m_fitPref[i];
                }
            }
            for (int i = 0; i < visible && prefSum; i++) {
                if (!atMin[i] && MulDiv(rest, m_fitPref[i], prefSum) < minWidth[i]) {
                    atMin[i] = true;
                    width[i] = minWidth[i];
                    rest -= minWidth[i];
                    changed = true;
                }
            }
        }
        int prefSum = 0;
        int last = visible - 1; // takes the rounding remainder
        for (int i = 0; i < visible; i++) {
            if (!atMin[i]) {
                prefSum += m_fitPref[i];
                last = i;
            }
        }
        int used = 0;
        for (int i = 0; i < visible; i++) {
            if (!atMin[i] && i != last) {
                width[i] = MulDiv(rest, m_fitPref[i], prefSum);
            }
            if (i != last) {
                used += width[i];
            }
        }
        if (visible > 0) {
            width[last] = avail - used;
        }
        for (int i = 0; i < count; i++) {
            if (GetColumnWidth(i) != width[i]) {
                SetColumnWidth(i, width[i]);
            }
        }
        CRect after;
        GetClientRect(after);
        if (after.Width() == avail) {
            break;
        }
    }
    // after a big shrink the list view keeps the horizontal scrollbar it showed for the old widths;
    // a zero scroll makes it check the scrollbars again
    if (GetStyle() & WS_HSCROLL) {
        Scroll(CSize(0, 0));
    }
    m_fitBusy = false;
}

void CListCtrl_Sortable::OnSize(UINT nType, int cx, int cy)
{
    CListCtrl_LabelTip::OnSize(nType, cx, cy);
    FitColumns();
}

LRESULT CListCtrl_Sortable::OnFitColumns(WPARAM, LPARAM)
{
    FitColumns();
    return 0;
}
