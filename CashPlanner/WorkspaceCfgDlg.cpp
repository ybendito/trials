// WorkspaceCfgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "WorkspaceCfgDlg.h"
#include "afxdialogex.h"


// CWorkspaceCfgDlg dialog

IMPLEMENT_DYNAMIC(CWorkspaceCfgDlg, CDialogEx)

CWorkspaceCfgDlg::CWorkspaceCfgDlg(tWorkspaceConfig& _cfg, CWnd* pParent /*=NULL*/)
	: CDialogEx(CWorkspaceCfgDlg::IDD, pParent), initialConfig(_cfg)
{
	cfg = _cfg;
}

CWorkspaceCfgDlg::~CWorkspaceCfgDlg()
{
}

void CWorkspaceCfgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATE_FROM, m_FromBox);
	DDX_Control(pDX, IDC_DATE_TO, m_ToBox);
}


BEGIN_MESSAGE_MAP(CWorkspaceCfgDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_OPERATIONAL, &CWorkspaceCfgDlg::OnBnClickedRadioOperational)
	ON_BN_CLICKED(IDC_RADIO_ANALYSIS, &CWorkspaceCfgDlg::OnBnClickedRadioAnalysis)
	ON_EN_CHANGE(IDC_NAME, &CWorkspaceCfgDlg::OnEnChangeName)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CWorkspaceCfgDlg message handlers


BOOL CWorkspaceCfgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CTime t;
	bool b;
	
	if (cfg.From.IsValid())
	{
		t = cfg.From.ToCTime();
		m_FromBox.SetTime(&t);
	}
	if (cfg.To.IsValid())
	{
		t = cfg.To.ToCTime();
		m_ToBox.SetTime(&t);
	}
	SetTimeFormat(m_FromBox);
	SetTimeFormat(m_ToBox);
	
	//b = !cfg.bOperational || !cfg.bExisting;
	b = true;

	m_FromBox.EnableWindow(b);
	m_ToBox.EnableWindow(b);

	b = cfg.bTypeChangeable;

	// Allow any change for now?
	//GetDlgItem(IDC_RADIO_OPERATIONAL)->EnableWindow(b);
	//GetDlgItem(IDC_RADIO_ANALYSIS)->EnableWindow(b);
	
	//GetDlgItem(IDC_NAME)->EnableWindow(b);

	if (cfg.fOperational) CheckDlgButton(IDC_RADIO_OPERATIONAL, true);
	else CheckDlgButton(IDC_RADIO_ANALYSIS, true);

	
	SetDlgItemText(IDC_NAME, cfg.name);
	SetDlgItemText(IDC_DESC, cfg.description);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CWorkspaceCfgDlg::OnOK()
{
	CTime t;
	UINT id = 0;

	GetDlgItemText(IDC_NAME, cfg.name);
	GetDlgItemText(IDC_DESC, cfg.description);

	m_FromBox.GetTime(t);
	cfg.From = tDate(t);

	m_ToBox.GetTime(t);
	cfg.To = tDate(t);
	
	cfg.bAutosave = !!IsDlgButtonChecked(IDC_KEEP_UPDATED);	

	if (!CheckValid(id))
	{
		CWnd *pWnd = GetDlgItem(id);
		if (pWnd) pWnd->SetFocus();
	}
	else
	{
		initialConfig = cfg;
		CDialogEx::OnOK();
	}
}

void CWorkspaceCfgDlg::InternalUpdate()
{
	bool bError = false;

	if (cfg.name.IsEmpty())
	{
		GetDlgItem(IDC_KEEP_UPDATED)->EnableWindow(false);
		CheckDlgButton(IDC_KEEP_UPDATED, false);
		//GetDlgItem(IDC_DESC)->EnableWindow(false);
	}
	else if (cfg.fOperational)
	{
		cfg.bAutosave = true;
		GetDlgItem(IDC_KEEP_UPDATED)->EnableWindow(false);
		CheckDlgButton(IDC_KEEP_UPDATED, true);
		//GetDlgItem(IDC_DESC)->EnableWindow(true);
	}
	else
	{
		GetDlgItem(IDC_KEEP_UPDATED)->EnableWindow(true);
		CheckDlgButton(IDC_KEEP_UPDATED, initialConfig.fOperational ? false : initialConfig.bAutosave);
		//GetDlgItem(IDC_DESC)->EnableWindow(true);
	}

	// indicate if such workspace name already exists
	bool bAlready = false;
	CString sFilename = cfg.Filename();
	if (!sFilename.IsEmpty() && cfg.Filename().CompareNoCase(initialConfig.Filename()))
	{
		bAlready = cfg.CheckFileExists();
		if (bAlready) bError = true;
	}
	GetDlgItem(IDC_EXISTING_WORKSPACE)->ShowWindow(bAlready ? SW_SHOW : SW_HIDE);

	GetDlgItem(IDOK)->EnableWindow(!bError);
}


void CWorkspaceCfgDlg::OnBnClickedRadioOperational()
{
	if (!cfg.fOperational)
	{
		cfg.fOperational = true;
		if (initialConfig.fOperational)
		{
			cfg.description = initialConfig.description;
			cfg.name = initialConfig.name;
		}
		else 
		{
			cfg.description.Empty();
			cfg.name = _T("");
			GetDlgItem(IDC_DESC)->SetFocus();
		}
		SetDlgItemText(IDC_NAME, cfg.name);
		SetDlgItemText(IDC_DESC, cfg.description);
		InternalUpdate();
	}
}


void CWorkspaceCfgDlg::OnBnClickedRadioAnalysis()
{
	if (cfg.fOperational)
	{
		cfg.fOperational = false;
		if (!initialConfig.fOperational)
		{
			cfg.description = initialConfig.description;
			cfg.name = initialConfig.name;
		}
		else
		{
			if (!initialConfig.name.IsEmpty())
				cfg.description.Format(_T("Created from %s"), initialConfig.name);
			cfg.name = _T("");
		}
		SetDlgItemText(IDC_NAME, cfg.name);
		SetDlgItemText(IDC_DESC, cfg.description);
		InternalUpdate();
	}
}

bool CWorkspaceCfgDlg::CheckValid(UINT& id)
{
	bool b = true;
	CTime t1, t2;
	
	m_FromBox.GetTime(t1);
	m_ToBox.GetTime(t2);

	if (t2 < t1)
	{
		b = false;
		id = IDC_DATE_TO;
	}
	return b;
}




void CWorkspaceCfgDlg::OnEnChangeName()
{
	GetDlgItemText(IDC_NAME, cfg.name);
	cfg.name.Trim();
	InternalUpdate();
}



HBRUSH CWorkspaceCfgDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_EXISTING_WORKSPACE)
	{
		pDC->SetTextColor(RGB(255,0,0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
