// CSettingsDialog.cpp : implementation file
//

#include "pch.h"
#include "SpiceRunner.h"
#include "CSettingsDialog.h"


// CSettingsDialog dialog

IMPLEMENT_DYNAMIC(CSettingsDialog, CDialogEx)

CSettingsDialog::CSettingsDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS, pParent)
{

}

CSettingsDialog::~CSettingsDialog()
{
}

void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialogEx)
    ON_BN_CLICKED(IDC_SELECT_SPICE, &CSettingsDialog::OnSpiceSelectBin)
    ON_BN_CLICKED(IDC_SELECT_VPN, &CSettingsDialog::OnBnClickedSelectVpn)
END_MESSAGE_MAP()


// CSettingsDialog message handlers

BOOL CSettingsDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CheckDlgButton(IDC_ENABLE_LOG, Profile.m_DebugEnabled);
    SetDlgItemText(IDC_SPICE_BINARY, Profile.m_SpiceBinary);
    SetDlgItemText(IDC_VNC_BINARY, Profile.m_VncBinary);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDialog::OnOK()
{
    Profile.m_DebugEnabled = IsDlgButtonChecked(IDC_ENABLE_LOG);
    GetDlgItemText(IDC_SPICE_BINARY, Profile.m_SpiceBinary);
    GetDlgItemText(IDC_VNC_BINARY, Profile.m_VncBinary);

    CDialogEx::OnOK();
}


void CSettingsDialog::OnSpiceSelectBin()
{
    CFileDialog d(true, _T("*.exe"), Profile.m_SpiceBinary);
    if (d.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_SPICE_BINARY, d.GetPathName());
    }
}


void CSettingsDialog::OnBnClickedSelectVpn()
{
    CFileDialog d(true, _T("*.exe"), Profile.m_VncBinary);
    if (d.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_VNC_BINARY, d.GetPathName());
    }
}
