// WaitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "WaitDlg.h"
#include "afxdialogex.h"


// CWaitDlg dialog

IMPLEMENT_DYNAMIC(CWaitDlg, CDialogEx)

CWaitDlg::CWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWaitDlg::IDD, pParent)
{
	Create(CWaitDlg::IDD, pParent);
	ShowWindow(SW_SHOW);
	SetDlgItemText(IDC_MESSAGE, _T("Please wait..."));
}

CWaitDlg::~CWaitDlg()
{
}

void CWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWaitDlg, CDialogEx)
END_MESSAGE_MAP()


// CWaitDlg message handlers


void CWaitDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnCancel();
}


void CWaitDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
