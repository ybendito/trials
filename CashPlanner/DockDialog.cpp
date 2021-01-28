// DockDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "DockDialog.h"
#include "afxdialogex.h"


// CDockDialog dialog

IMPLEMENT_DYNAMIC(CDockDialog, CDialogEx)

CDockDialog::CDockDialog(UINT idd, CWnd* pParent /*=NULL*/)
	: CDialogEx(idd, pParent), docker(*this)
{
	bAllowOK = false;
}

CDockDialog::~CDockDialog()
{
}

//void CDockDialog::DoDataExchange(CDataExchange* pDX)
//{
//	CDialogEx::DoDataExchange(pDX);
//}


BEGIN_MESSAGE_MAP(CDockDialog, CDialogEx)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CDockDialog message handlers


BOOL CDockDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	OnModelessCreate();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDockDialog::OnModelessCreate()
{
	const tDockTable *table = GetDockedControlsTable();

	while (table && table->id && table->DOCKF_flags)
	{
		docker.AddControl(table->id, table->DOCKF_flags);
		table++;
	}
	docker.Start();
}


void CDockDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	OutputDebugStringX(_T("%s: cx %d, cy %d"), _T(__FUNCTION__), cx, cy);
	POINT pt = docker.Track(cx, cy);
	if (pt.x || pt.y) ResizeDone(pt);
}


void CDockDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	OutputDebugStringX(_T("%s: => min size %d, %d"), _T(__FUNCTION__), lpMMI->ptMinTrackSize.x, lpMMI->ptMinTrackSize.y);
	docker.MinMaxInfo(lpMMI);
	OutputDebugStringX(_T("%s: <= min size %d, %d"), _T(__FUNCTION__), lpMMI->ptMinTrackSize.x, lpMMI->ptMinTrackSize.y);
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CDockDialog::OnOK()
{
	if (!bAllowOK)
	{
		CWnd *pw = GetFocus();
		if (pw)
		{
			pw = GetNextDlgTabItem(pw);
			if (pw) pw->SetFocus();
		}
	}
}
