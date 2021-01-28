// WorkspacesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "WorkspacesDlg.h"
#include "afxdialogex.h"

// CWorkspacesDlg dialog

IMPLEMENT_DYNAMIC(CWorkspacesDlg, CDialogEx)

CWorkspacesDlg::CWorkspacesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWorkspacesDlg::IDD, pParent)
{
	bSelect = bOperational = bShowArchived = false;
}

CWorkspacesDlg::~CWorkspacesDlg()
{
}

void CWorkspacesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}


BEGIN_MESSAGE_MAP(CWorkspacesDlg, CDialogEx)
	ON_BN_CLICKED(IDC_DELETE, &CWorkspacesDlg::OnClickedDelete)
	ON_LBN_SELCHANGE(IDC_LIST1, &CWorkspacesDlg::OnSelchangeList1)
	ON_BN_CLICKED(IDC_ARCHIVE, &CWorkspacesDlg::OnBnClickedArchive)
	ON_BN_CLICKED(IDC_UNARCHIVE, &CWorkspacesDlg::OnBnClickedUnarchive)
	ON_BN_CLICKED(IDC_SHOW_ARCHIVED, &CWorkspacesDlg::OnBnClickedShowArchived)
END_MESSAGE_MAP()


// CWorkspacesDlg message handlers


void CWorkspacesDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (bSelect)
	{
		CDialogEx::OnOK();
	}
}

void CWorkspacesDlg::Search(CString& mask)
{
	BOOL bContinue;
	CFileFind ff;
	bContinue = ff.FindFile(mask);
	
	while (bContinue)
	{
		bContinue = ff.FindNextFile();
		CString name, path;
		path = ff.GetFilePath();
		name = ff.GetFileTitle();
		int index = m_List.AddString(name);
		if (index >= 0)
		{
			int n = Strings.Add(path);
			m_List.SetItemDataPtr(index, (PVOID)(LPCTSTR)Strings[n]);
		}
	}
}

void CWorkspacesDlg::FillList()
{
	m_List.ResetContent();
	tWorkspaceConfig temp;
	temp.fArchived = bShowArchived;
	temp.fOperational = true;
	temp.name = _T("*");
	CString mask = temp.Filename();
	if (!bSelect || bOperational) Search(mask);
	temp.fOperational = false;
	mask = temp.Filename();
	if (!bSelect || !bOperational) Search(mask);

	OnSelchangeList1();
}

void CWorkspacesDlg::SetupButtons()
{
	GetDlgItem(IDC_ARCHIVE)->ShowWindow((!bSelect && !bShowArchived) ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_UNARCHIVE)->ShowWindow((!bSelect && bShowArchived) ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DELETE)->ShowWindow(!bSelect ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_SHOW_ARCHIVED)->ShowWindow(!bSelect ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDOK)->ShowWindow(bSelect ?  SW_SHOW : SW_HIDE);
}


BOOL CWorkspacesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetupButtons();	

	FillList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CWorkspacesDlg::OnClickedDelete()
{
	int n = m_List.GetCurSel();
	if (n >= 0)
	{
		LPCTSTR p = (LPCTSTR)m_List.GetItemDataPtr(n);
		BOOL b = DeleteFile(p);
		if (b)
		{
			m_List.DeleteString(n);
		}
	}
}

static void CheckByFile(tWorkspaceConfig& w, CString& sFile)
{
	if (w.Filename().CompareNoCase(sFile))
	{
		w.fOperational = true;
		if (w.Filename().CompareNoCase(sFile))
		{
			w.fOperational = false;
			w.fInvalid = true;
		}
	}
}

void CWorkspacesDlg::OnSelchangeList1()
{
	GetDlgItem(IDC_DELETE)->EnableWindow(false);
	GetDlgItem(IDC_ARCHIVE)->EnableWindow(false);
	GetDlgItem(IDC_UNARCHIVE)->EnableWindow(false);
	GetDlgItem(IDOK)->EnableWindow(false);
	SelectedName.Empty(); 
	int n = m_List.GetCurSel();
	if (n >= 0)
	{
		LPCTSTR p = (LPCTSTR)m_List.GetItemDataPtr(n);
		if (p)
		{
			CFileFind ff;
			if (ff.FindFile(p))
			{
				tWorkspaceConfig tempWorkspace;
				tempWorkspace.fArchived = bShowArchived;
				GetDlgItem(IDOK)->EnableWindow(true);
				GetDlgItem(IDC_DELETE)->EnableWindow(true);
				m_List.GetText(n, SelectedName); 
				ff.FindNextFile();
				CTime t;
				tempWorkspace.name = SelectedName;
				CheckByFile(tempWorkspace, ff.GetFilePath());
				if (ff.GetLastWriteTime(t)) 
					GetDlgItem(IDC_LAST_UPDATE)->SetWindowText(t.Format(_T("%B %d, %Y")));
				if (tempWorkspace.CheckFileExists())
				{
					ShowWorkspaceDetails(tempWorkspace);
				}
				GetDlgItem(IDC_ARCHIVE)->EnableWindow(!tempWorkspace.fArchived);
				GetDlgItem(IDC_UNARCHIVE)->EnableWindow(tempWorkspace.fArchived);
			}
		}
	}
	else
	{
		// cleanup
		CheckDlgButton(IDC_SALES, false );
		CheckDlgButton(IDC_PURCHASES, false);
		CheckDlgButton(IDC_EXPENSES, false);
		CheckDlgButton(IDC_CASH, false);
		CheckDlgButton(IDC_PERIODIC_DATA, false);
		//
		SetDlgItemText(IDC_LAST_UPDATE, _T(""));
		SetDlgItemText(IDC_TYPE, _T(""));
		SetDlgItemText(IDC_DATES, _T(""));
	}
}

void CWorkspacesDlg::ShowWorkspaceDetails(tWorkspaceConfig& w)
{
	LoadClearCfg loadCfg;
	CString s;
	if (w.fInvalid) s = _T("Invalid");
	else s = w.fOperational ? _T("Operational") : _T("Analysis");
	if (w.fArchived) s += _T(" (A)");
	GetDlgItem(IDC_TYPE)->SetWindowText(s);
	tMonthlyDataCollection::CheckCanLoad(loadCfg, w.Filename());
	CheckDlgButton(IDC_SALES, 2 * loadCfg.sales.check );
	CheckDlgButton(IDC_PURCHASES, 2 * loadCfg.purchases.check);
	CheckDlgButton(IDC_EXPENSES, 2 * loadCfg.expenses.check);
	CheckDlgButton(IDC_CASH, 2 * loadCfg.cash.check);
	CheckDlgButton(IDC_PERIODIC_DATA, 2 * loadCfg.periodic.check);

	if (loadCfg.workspace.check)
	{
		tProfileSetup ps(w.Filename());
		w.Deserialize();
		CString sFrom = w.From.GetString();
		CString sTo = w.To.GetString();
		sFrom += _T("...");
		sFrom += sTo;
		SetDlgItemText(IDC_DATES, sFrom);
	}
	else
	{
		SetDlgItemText(IDC_DATES, _T(""));
	}
}


void CWorkspacesDlg::OnBnClickedArchive()
{
	int n = m_List.GetCurSel();
	if (n >= 0)
	{
		tWorkspaceConfig w;
		CString sFile = (LPCTSTR)m_List.GetItemDataPtr(n);
		m_List.GetText(n, w.name);
		CheckByFile(w, sFile);
		if (!w.fInvalid)
		{
			CTime t = CurrentTime();
			w.name += t.Format(_T("_Archived_%Y-%b-%d-%H-%M-%S"));
			w.fArchived = true;
			MoveFile(sFile, w.Filename());
			FillList();
		}
	}
}


void CWorkspacesDlg::OnBnClickedUnarchive()
{
	int n = m_List.GetCurSel();
	if (n >= 0)
	{
		tWorkspaceConfig w;
		w.fArchived = bShowArchived;
		CString sFile = (LPCTSTR)m_List.GetItemDataPtr(n);
		m_List.GetText(n, w.name);
		CheckByFile(w, sFile);
		if (!w.fInvalid && w.fArchived)
		{
			CTime t = CurrentTime();
			w.fArchived = false;
			if (!w.CheckFileExists())			
				MoveFile(sFile, w.Filename());
			FillList();
		}
	}
}


void CWorkspacesDlg::OnBnClickedShowArchived()
{
	bShowArchived = !!IsDlgButtonChecked(IDC_SHOW_ARCHIVED);
	SetupButtons();
	FillList();
}
