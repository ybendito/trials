// MergeDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "MergeDataDlg.h"
#include "afxdialogex.h"


// CMergeDataDlg dialog

IMPLEMENT_DYNAMIC(CMergeDataDlg, CDialogEx)

CMergeDataDlg::CMergeDataDlg(LoadClearCfg& _clear, LoadClearCfg& _load, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMergeDataDlg::IDD, pParent),
	cfgClear(_clear),
	cfgLoad(_load),
	m_ClearBar(cfgClear), 
	m_LoadBar(cfgLoad)
{
	cfgClear.workspace.check = false;
	cfgClear.workspace.show = false;
}

CMergeDataDlg::~CMergeDataDlg()
{
}

void CMergeDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	if (!pDX->m_bSaveAndValidate)
	{
		DDX_Control(pDX, IDC_BAR1, m_Placeholder1);
		DDX_Control(pDX, IDC_BAR2, m_Placeholder2);
	}
}


BEGIN_MESSAGE_MAP(CMergeDataDlg, CDialogEx)
END_MESSAGE_MAP()


// CMergeDataDlg message handlers


BOOL CMergeDataDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	PrepareControl(m_LoadBar, m_Placeholder2);
	PrepareControl(m_ClearBar, m_Placeholder1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMergeDataDlg::OnOK()
{
	m_LoadBar.Update(true);
	m_ClearBar.Update(true);
	CDialogEx::OnOK();
}

void CMergeDataDlg::PrepareControl(CLoadClearOptionsBar& bar, CStatic& ctl)
{
	ULONG style = 0;
	int id = ctl.GetDlgCtrlID();
	WINDOWINFO controlInfo, info;
	controlInfo.cbSize = sizeof(controlInfo);
	info.cbSize = sizeof(info);
	GetWindowInfo(&info);
	ctl.GetWindowInfo(&controlInfo);
	controlInfo.rcWindow.left -= info.rcClient.left;
	controlInfo.rcWindow.right -= info.rcClient.left;
	controlInfo.rcWindow.top -= info.rcClient.top;
	controlInfo.rcWindow.bottom -= info.rcClient.top;
	bar.Create(this, CLoadClearOptionsBar::IDD, style | CBRS_NOALIGN, id);
	bar.SetWindowPos(&wndTop, controlInfo.rcWindow.left, controlInfo.rcWindow.top, controlInfo.rcWindow.right - controlInfo.rcWindow.left, controlInfo.rcWindow.bottom - controlInfo.rcWindow.top, SWP_SHOWWINDOW);
	bar.ModifyStyle(0, WS_TABSTOP | WS_GROUP);
	ctl.DestroyWindow();
	bar.Update(false);
}
