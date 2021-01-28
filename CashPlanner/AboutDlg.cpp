// AboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "AboutDlg.h"
#include "afxdialogex.h"


// CAboutDlg dialog

IMPLEMENT_DYNAMIC(CAboutDlg, CDialogEx)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAboutDlg::IDD, pParent)
{

}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAboutDlg message handlers


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString sBuild;
	sBuild.Format(TEXT("Build: %s %s"), TEXT(__DATE__), TEXT(__TIME__));
	// TODO:  Add extra initialization here
	SetDlgItemText(IDC_BUILD_INFO, sBuild);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
