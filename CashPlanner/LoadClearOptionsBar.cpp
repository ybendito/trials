// LoadClearOptionsBar.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "LoadClearOptionsBar.h"
#include "afxdialogex.h"


// CLoadClearOptionsBar dialog

IMPLEMENT_DYNAMIC(CLoadClearOptionsBar, CDialogBar)

CLoadClearOptionsBar::CLoadClearOptionsBar(LoadClearCfg& _cfg) : cfg(_cfg)
{

}

CLoadClearOptionsBar::~CLoadClearOptionsBar()
{
}

bool CLoadClearOptionsBar::SetupControl(LoadClearCfg::boxCfg& _cfg, int id)
{
	CWnd *p = GetDlgItem(id);
	if (p)
	{
		p->EnableWindow(!!_cfg.show);
		CheckDlgButton(id, _cfg.check);
	}
	return _cfg.check;
}

void CLoadClearOptionsBar::GetControlState(LoadClearCfg::boxCfg& _cfg, int id)
{
	CWnd *p = GetDlgItem(id);
	if (p)
	{
		_cfg.check = !!IsDlgButtonChecked(id);
	}
}


void CLoadClearOptionsBar::Update(bool bSave)
{
	if (!bSave)
	{
		int n = 0;
		n += SetupControl(cfg.cash, IDC_CASH);
		n += SetupControl(cfg.expenses, IDC_EXPENSES);
		n += SetupControl(cfg.purchases, IDC_PURCHASES);
		n += SetupControl(cfg.sales, IDC_SALES);
		n += SetupControl(cfg.workspace, IDC_WORKSPACE);
		n += SetupControl(cfg.periodic, IDC_PERIODIC_DATA);
		if (n == 6)
			n = 1;
		else if (n)
			n = 2;
		SetupControl(cfg.all, IDC_ALL_DATA);
		CheckDlgButton(IDC_ALL_DATA, n);
	}
	else
	{
		GetControlState(cfg.cash, IDC_CASH);
		GetControlState(cfg.expenses, IDC_EXPENSES);
		GetControlState(cfg.purchases, IDC_PURCHASES);
		GetControlState(cfg.sales, IDC_SALES);
		GetControlState(cfg.workspace, IDC_WORKSPACE);
		GetControlState(cfg.periodic, IDC_PERIODIC_DATA);
	}
}


BEGIN_MESSAGE_MAP(CLoadClearOptionsBar, CDialogBar)
	ON_BN_CLICKED(IDC_ALL_DATA, &CLoadClearOptionsBar::OnBnClickedAllData)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// CLoadClearOptionsBar message handlers


void CLoadClearOptionsBar::OnBnClickedAllData()
{
	int nAll = IsDlgButtonChecked(IDC_ALL_DATA);
	if (nAll == 2)
	{
		Update(false);
	}
	else
	{
		if (cfg.cash.show) { CheckDlgButton(IDC_CASH, nAll); } 
		if (cfg.expenses.show) { CheckDlgButton(IDC_EXPENSES, nAll); } 
		if (cfg.purchases.show) { CheckDlgButton(IDC_PURCHASES, nAll); } 
		if (cfg.sales.show) { CheckDlgButton(IDC_SALES, nAll); } 
		if (cfg.workspace.show) { CheckDlgButton(IDC_WORKSPACE, nAll); } 
		if (cfg.periodic.show) { CheckDlgButton(IDC_PERIODIC_DATA, nAll); } 
	}
}


void CLoadClearOptionsBar::OnSetFocus(CWnd* pOldWnd)
{
	CDialogBar::OnSetFocus(pOldWnd);

	GetDlgItem(IDC_ALL_DATA)->SetFocus();
}
