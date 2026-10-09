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
#include "define.h"

#include "AccountDlg.h"
#include "mainDlg.h"
#include "langpack.h"
#include "atlrx.h"
#include <ws2tcpip.h>
#include "json.h"

AccountDlg::AccountDlg(CWnd* pParent /*=NULL*/)
    : CThemeDialog(AccountDlg::IDD, pParent)
{
    accountId = -1;
    if (!Create(IDD, pParent)) {
        AfxMessageBox(_T("Failed to create account window on your system"));
        exit(0);
    }
}

AccountDlg::~AccountDlg(void)
{
}

int AccountDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (langPack.rtl) {
        ModifyStyleEx(0, WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT);
    }
    return 0;
}

BOOL AccountDlg::OnInitDialog()
{
    CString str;
    str.Format(_T("<a>%s</a>"), Translate(_T("Delete Account")));
    GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->SetWindowText(str);

    str.Format(_T("<a>%s</a>"), Translate(_T("display password")));
    GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->SetWindowText(str);

    m_bAllowDark = true;
    CThemeDialog::OnInitDialog();

    CRect rect;
    GetWindowRect(&rect);
    width = rect.Width() * 96 / dpiY;
    height = rect.Height() * 96 / dpiY;

    TranslateDialog(this->m_hWnd);

    return TRUE;
}

void AccountDlg::OnDestroy()
{
    mainDlg->accountDlg = nullptr;
    CThemeDialog::OnDestroy();
}

void AccountDlg::PostNcDestroy()
{
    CThemeDialog::PostNcDestroy();
    delete this;
}

BEGIN_MESSAGE_MAP(AccountDlg, CThemeDialog)
    ON_WM_CREATE()
    ON_WM_SYSCOMMAND()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_WM_NCHITTEST()
    ON_BN_CLICKED(IDCANCEL, &AccountDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &AccountDlg::OnBnClickedOk)
    ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISPLAY_PASSWORD, &AccountDlg::OnNMClickSyslinkDisplayPasswod)
    ON_NOTIFY(NM_RETURN, IDC_SYSLINK_DISPLAY_PASSWORD, &AccountDlg::OnNMClickSyslinkDisplayPasswod)
    ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ACCOUNT_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
    ON_NOTIFY(NM_RETURN, IDC_SYSLINK_ACCOUNT_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, &AccountDlg::OnChangePassword)

END_MESSAGE_MAP()

void AccountDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    __super::OnSysCommand(nID, lParam);
}

void AccountDlg::OnClose()
{
    DestroyWindow();
}

LRESULT AccountDlg::OnNcHitTest(CPoint point)
{
    ScreenToClient(&point);

    CRect rc;
    GetClientRect(&rc);

    //rc.bottom = rc.top + 100; 

    if (rc.PtInRect(point))
        return HTCAPTION;

    return CThemeDialog::OnNcHitTest(point);
}

void AccountDlg::OnBnClickedCancel()
{
    OnClose();
}

void AccountDlg::Load(int id)
{
    CEdit* edit;

    accountId = id;
    if (accountSettings.AccountLoad(id, &m_Account) || id == 0) {
        accountId = id;
        if (accountId && accountSettings.accountId == accountId && !accountSettings.account.rememberPassword) {
            m_Account.username = accountSettings.account.username;
            m_Account.password = accountSettings.account.password;
            m_Account.rememberPassword = false;
        }
    }
    else {
        accountId = -1;
    }

    bool isEdit = (accountId > 0 && (!m_Account.username.IsEmpty() || accountId > 1));

    edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_LABEL);
    edit->SetWindowText(m_Account.label);

    edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
    edit->SetWindowText(m_Account.server);

    edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
    if (
        accountId == -1 ||
        m_Account.password.IsEmpty()
        ) {
        GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->ShowWindow(SW_SHOW);
    }
    else {
        GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->ShowWindow(SW_HIDE);
    }
    edit->SetPasswordChar('*');
    edit->SetWindowText(m_Account.password);

    if (isEdit) {
        GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->ShowWindow(SW_SHOW);
    }
    else {
        GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->ShowWindow(SW_HIDE);
    }
}

void AccountDlg::OnBnClickedOk()
{
    CString str;

    GetDlgItem(IDC_ACCOUNT_LABEL)->GetWindowText(str);
    m_Account.label = str.Trim();

    GetDlgItem(IDC_EDIT_SERVER)->GetWindowText(str);
    m_Account.server = str.Trim();

    GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowText(str);
    m_Account.password = str.Trim();

    if (
        m_Account.label.IsEmpty() ||
        m_Account.server.IsEmpty() ||
        m_Account.password.IsEmpty()) {
        AfxMessageBox(_T("Preencha o Nome da Conta, o Servidor SIP e a Senha."));
        return;
    }

    // Campos preenchidos automaticamente a partir do Nome da Conta e do Servidor SIP
    m_Account.username = m_Account.label;
    m_Account.authID = m_Account.label;
    m_Account.displayName = m_Account.label;
    m_Account.proxy = m_Account.server;
    m_Account.domain = m_Account.server;

    // Demais opcoes fixas
    m_Account.voicemailNumber.Empty();
    m_Account.dialingPrefix.Empty();
    m_Account.dialPlan.Empty();
    m_Account.hideCID.Empty();
    m_Account.srtp.Empty();
    m_Account.transport = _T("udp");
    m_Account.publicAddr.Empty();
    m_Account.registerRefresh = 300;
    m_Account.keepAlive = 15;
    m_Account.publish = 1;
    m_Account.allowRewrite = 0;
    m_Account.ice = 0;
    m_Account.disableSessionTimer = 0;
    m_Account.rememberPassword = 1;

    this->ShowWindow(SW_HIDE);
    mainDlg->accountDlg = NULL;

    if (accountId == -1) { // find id for new account
        Account dummy;
        int i = 1;
        while (true) {
            if (!accountSettings.AccountLoad(i, &dummy)) {
                break;
            }
            i++;
        }
        accountId = i;
    }

    accountSettings.AccountSave(accountId, &m_Account);

    if (accountId) {
        mainDlg->PJAccountDelete(true);

        accountSettings.accountId = accountId;
        accountSettings.account = m_Account;
        accountSettings.AccountLoad(accountSettings.accountId, &accountSettings.account);
        mainDlg->OnAccountChanged();
        mainDlg->InitUI();
        accountSettings.SettingsSave();
        mainDlg->PJAccountAdd();
    }
    else {
        // local account
        mainDlg->PJAccountDeleteLocal();
        accountSettings.AccountLoad(0, &accountSettings.accountLocal);
        mainDlg->PJAccountAddLocal();
    }
    OnClose();
}

void AccountDlg::OnNMClickSyslinkDisplayPasswod(NMHDR * pNMHDR, LRESULT * pResult)
{
    GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->ShowWindow(SW_HIDE);
    CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
    edit->SetPasswordChar(0);
    edit->Invalidate();
    edit->SetFocus();
    int nLength = edit->GetWindowTextLength();
    edit->SetSel(nLength, nLength);
    *pResult = 0;
}

void AccountDlg::OnNMClickSyslinkDelete(NMHDR * pNMHDR, LRESULT * pResult)
{
    if (accountId > 0 && AfxMessageBox(Translate(_T("Are you sure you want to delete?")), MB_YESNO) == IDYES) {
        this->ShowWindow(SW_HIDE);
        mainDlg->accountDlg = NULL;

        Account account;
        int i = accountId;
        while (true) {
            if (!accountSettings.AccountLoad(i + 1, &account)) {
                break;
            }
            accountSettings.AccountSave(i, &account);
            i++;
        }
        accountSettings.AccountDelete(i);
        if (accountId && accountId == accountSettings.accountId) {
            mainDlg->PJAccountDelete(true);
            if (i > 1) {
                accountSettings.accountId = 1;
                accountSettings.AccountLoad(accountSettings.accountId, &accountSettings.account);
                mainDlg->OnAccountChanged();
                mainDlg->InitUI();
                mainDlg->PJAccountAdd();
            }
            else {
                accountSettings.accountId = 0;
                mainDlg->OnAccountChanged();
                mainDlg->InitUI();
            }
            accountSettings.SettingsSave();
        }
        OnClose();
    }
    *pResult = 0;
}

void AccountDlg::OnChangePassword()
{
    CString str;
    GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowText(str);
    if (str.IsEmpty()) {
        GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->ShowWindow(SW_SHOW);
        CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
        edit->SetPasswordChar('*');
    }
}


